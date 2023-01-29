// Copyright 2023 Michael Fabian Dirks <info@xamyar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "ms_groupfile.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>

ms::groupfile::groupfile() {}

ms::groupfile::~groupfile() {}

const decltype(ms::groupfile::_entries)& ms::groupfile::raw()
{
	return _entries;
}

void ms::groupfile::add(const std::string name, const std::vector<uint8_t>& data)
{
	_entries.emplace(name, data);
}

void ms::groupfile::remove(const std::string name)
{
	_entries.erase(name);
}

size_t ms::groupfile::count()
{
	return _entries.size();
}

void ms::groupfile::read(const std::vector<uint8_t>& buffer)
{
	decltype(_entries) entries;

	// Check if the buffer is even large enough to contain anything.
	if (sizeof(uint32_t) > buffer.size()) {
		throw std::runtime_error("File is too small to contain anything");
	}

	// Read the number of headers, and verify that this works out.
	size_t num_entries = reinterpret_cast<const uint32_t&>(buffer[0]);
	if ((num_entries * sizeof(header_t)) > buffer.size()) {
		throw std::runtime_error("Header claims to contain more files than the file can reasonably hold");
	}

	// Read each header.
	for (size_t idx = 0; idx < num_entries; idx++) {
		size_t          off    = idx * sizeof(header_t) + sizeof(uint32_t);
		const header_t& header = reinterpret_cast<const header_t&>(buffer[off]);
		if (header.offset >= buffer.size()) {
			throw std::runtime_error("Entry claims to be located outside of the file itself");
		} else if (header.offset + header.size > buffer.size()) {
			throw std::runtime_error("Entry claims to be so large it ends up outside of the file itself");
		}
		if (header.size > 0) {
			const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buffer.data()) + header.offset;
			entries.emplace(header.name, std::vector<uint8_t>{ptr, ptr + header.size});
		} else {
			entries.emplace(header.name, std::vector<uint8_t>{});
		}
	}

	// If everything worked, replace the contained map.
	_entries = entries;
}

void ms::groupfile::write(std::vector<uint8_t>& buffer)
{
	buffer.resize(size());

	size_t header_size   = _entries.size() * sizeof(header_t);
	size_t header_offset = sizeof(uint32_t);
	size_t data_offset   = header_offset + header_size;

	// Write the number of headers.
	reinterpret_cast<uint32_t&>(buffer[0]) = _entries.size();

	// Write each entry.
	for (auto entry : _entries) {
		// Write header
		header_t& header = reinterpret_cast<header_t&>(buffer.at(header_offset));
		memset(header.name, 0, sizeof(header.name));
		memcpy(header.name, entry.first.c_str(), std::min(sizeof(header.name), entry.first.size()));
		header.offset = data_offset;
		header.size   = entry.second.size();

		// Write data
		memcpy(buffer.data() + data_offset, entry.second.data(), entry.second.size());

		// Advance pointers
		header_offset += sizeof(header_t);
		data_offset += entry.second.size();
	}
}

size_t ms::groupfile::size()
{
	size_t content_size = 0;
	size_t header_size  = 0;
	for (auto entry : _entries) {
		header_size += sizeof(header_t);
		content_size += entry.second.size();
	}
	return size_t(4) + header_size + content_size;
}
