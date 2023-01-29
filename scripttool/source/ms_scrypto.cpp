// Copyright 2023 Michael Fabian Dirks <info@xamyar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "ms_scrypto.hpp"
#include <stdexcept>
#include <cstring>

ms::scrypto::scrypto() {}

ms::scrypto::~scrypto() {}

void ms::scrypto::encrypt(const std::vector<uint8_t>& decrypted, std::vector<uint8_t>& encrypted)
{
	uint32_t checksum;
	size_t   decrypted_size = decrypted.size();

	// Calculate a checksum on the original data
	checksum = 678;
	for (size_t i = 0; i < decrypted_size; i++) {
		if (i % 2) {
			checksum += (decrypted[i] * 2);
		} else {
			checksum -= decrypted[i];
		}
	}

	// Allocate enough space for the encrypted content.
	encrypted.clear();
	encrypted.resize(decrypted_size + sizeof(uint32_t));

	// Perform "encryption" LULW.
	// 1. Perform a simple offset via addition.
	for (size_t idx = 0; idx < decrypted_size; idx++) {
		encrypted[idx] = (decrypted[idx] + ((idx * 4) % 128)) & 0xFF;
	}
	// 2. Swap bytes around.
	for (size_t idx = 0; idx < decrypted_size; idx++) {
		std::swap(encrypted[idx], encrypted[(idx + 7) % decrypted_size]);
	}

	// Finalize checksum and store it.
	reinterpret_cast<uint32_t&>(encrypted.at(decrypted_size)) = checksum;
	encrypted[decrypted_size + 0]                             = (checksum / 0x1000000ul);
	encrypted[decrypted_size + 1]                             = (checksum / 0x10000ul);
	encrypted[decrypted_size + 2]                             = (checksum / 0x100ul);
	encrypted[decrypted_size + 3]                             = (checksum % 0x100ul);
}

void ms::scrypto::decrypt(const std::vector<uint8_t>& encrypted, std::vector<uint8_t>& decrypted)
{
	size_t encrypted_size = encrypted.size();

	if (encrypted_size < sizeof(uint32_t))
		throw std::runtime_error("Encrypted data is too small");

	// Find the size without the checksum.
	size_t decrypted_size = encrypted_size - sizeof(uint32_t);

	// Read the checksum.
	uint32_t checksum_r = reinterpret_cast<const uint32_t&>(encrypted.at(decrypted_size));
	uint32_t checksum   = static_cast<uint32_t>(encrypted[decrypted_size + 3]);
	checksum += static_cast<uint32_t>(encrypted[decrypted_size + 2]) * 0x100ul;
	checksum += static_cast<uint32_t>(encrypted[decrypted_size + 1]) * 0x10000ul;
	checksum += static_cast<uint32_t>(encrypted[decrypted_size + 0]) * 0x1000000ul;

	// Allocate decrypted buffer.
	decrypted.clear();
	decrypted.resize(decrypted_size);

	// Clone encrypted data (necessary due to byte swapping).
	memcpy(decrypted.data(), encrypted.data(), decrypted_size);

	// Perform "decryption" LULW.
	// 1. Undo byte swapping.
	for (size_t idx = decrypted_size - 1; (idx >= 0) && (idx < decrypted_size); idx -= 1) {
		std::swap(decrypted[idx], decrypted[(idx + 7) % decrypted_size]);
	}
	// 2. Undo offset.
	for (size_t idx = decrypted_size - 1; (idx >= 0) && (idx < decrypted_size); idx -= 1) {
		decrypted[idx] -= (idx * 4) % 128;
	}

	// Validate checksum with decrypted data.
	uint32_t dchecksum = 678;
	for (size_t i = 0; i < decrypted_size; i++) {
		if ((i % 2) != 0) {
			dchecksum += static_cast<uint32_t>(decrypted[i]) * 2ul;
		} else {
			dchecksum -= static_cast<uint32_t>(decrypted[i]);
		}
	}

	// Verify checksum.
	if (dchecksum != checksum) {
		throw checksum_exception();
	}
}
