// Copyright 2023 Michael Fabian Dirks <info@xamyar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include <map>
#include <stdexcept>
#include <vector>

namespace ms {
	/** Extremely simple file collection.
	 *
	 * Literally just a list of headers, and all the content at the end.
	 */
	class groupfile {
		struct header_t {
			char     name[256];
			uint32_t offset;
			uint32_t size;
		};

		std::map<std::string, std::vector<uint8_t>> _entries;

		public:
		groupfile();
		~groupfile();

		public /* Entries */:

		void add(const std::string name, const std::vector<uint8_t>& data);

		void remove(const std::string name);

		const decltype(_entries)& raw();

		size_t count();

		public /* Storage */:
		void read(const std::vector<uint8_t>& buffer);

		void write(std::vector<uint8_t>& buffer);

		size_t size();
	};
} // namespace ms
