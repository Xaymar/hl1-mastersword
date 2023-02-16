// Copyright 2023 Michael Fabian Dirks <info@xaymar.com>
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#include "ms_groupfile.hpp"
#include "ms_scrypto.hpp"

enum class mode : uint32_t {
	UNDEFINED,
	DECRYPT,
	ENCRYPT,
	PACK,
	UNPACK,
};

int main(int argc, const char* argv[])
{
	std::filesystem::path output_path;
	std::filesystem::path input_path;
	mode                  operation       = mode::UNDEFINED;
	size_t                pargs_idx       = 0;
	bool                  ignore_checksum = false;

	for (size_t idx = 1; idx < argc; idx++) {
		std::string_view arg = argv[idx];
		if (arg.rfind("--", 0) == 0) {
			// --key[=value]
			if (arg.rfind("--ignore-checksum", 0) == 0) {
				ignore_checksum = true;
			} else {
				printf("Unknown argument '%s'.\n", arg.data());
				return -1;
			}
		} else if (arg.rfind("-", 0) == 0) {
			// -key [value]
			if (arg.rfind("-decrypt", 0) == 0) {
				operation = mode::DECRYPT;
			} else if (arg.rfind("-encrypt", 0) == 0) {
				operation = mode::ENCRYPT;
			} else if (arg.rfind("-pack", 0) == 0) {
				operation = mode::PACK;
			} else if (arg.rfind("-unpack", 0) == 0) {
				operation = mode::UNPACK;
			} else {
				printf("Unknown argument '%s'.\n", arg.data());
				return -1;
			}
		} else {
			// Positional argument
			switch (pargs_idx) {
			case 0:
				output_path = arg;
				break;
			case 1:
				input_path = arg;
				break;
			default:
				printf("Too many positional arguments: '%s' is unexpected.", arg.data());
				return -1;
			}

			pargs_idx++;
		}
	}

	// Verify arguments
	if (operation == mode::UNDEFINED) {
		printf("Usage: %s [-decrypt|-encrypt|-unpack|-pack] <output> <input>\n", argv[0]);
		return -1;
	}
	if (!std::filesystem::exists(input_path)) {
		printf("Input path '%s' must exist.", input_path.u8string().c_str());
		return -1;
	}

	// Do things based on operation mode.
	if (operation == mode::DECRYPT) {
		try {
			std::vector<uint8_t> input_data;

			{ // Try and read the input file.
				size_t input_size = std::filesystem::file_size(input_path);
				input_data.resize(input_size);

				std::ifstream input{input_path, std::ios_base::binary};
				input.read(reinterpret_cast<char*>(input_data.data()), input_size);
				input.close();
			}

			// Decrypt the contents.
			std::vector<uint8_t> output_data;
			ms::scrypto          crypt;
			try {
				crypt.decrypt(input_data, output_data);
			} catch (ms::scrypto::checksum_exception const& ex) {
				if (!ignore_checksum) {
					throw ex;
				}
			} catch (...) {
				throw;
			}

			// Try and write the output.
			std::ofstream output{output_path, std::ios_base::binary | std::ios_base::trunc};
			output.write(reinterpret_cast<char*>(output_data.data()), output_data.size());
		} catch (std::exception const& ex) {
			printf("Decrypting '%s' failed with error: %s.\n", input_path.u8string().c_str(), ex.what());
			return -1;
		} catch (...) {
			printf("Decrypting '%s' failed with unknown error.\n", input_path.u8string().c_str());
			return -1;
		}
	} else if (operation == mode::UNPACK) {
		try {
			std::vector<uint8_t> input_data;

			{ // Try and read the input file.
				size_t input_size = std::filesystem::file_size(input_path);
				input_data.resize(input_size);

				std::ifstream input{input_path, std::ios_base::binary};
				input.read(reinterpret_cast<char*>(input_data.data()), input_size);
				input.close();
			}

			// Open the groupfile.
			ms::groupfile groupfile;
			groupfile.read(input_data);

			// Unpack all the files, one by one.
			for (auto entry : groupfile.raw()) {
				std::filesystem::path file_path = output_path;
				file_path /= entry.first;

				std::filesystem::path container_path = file_path.parent_path();

				std::filesystem::create_directories(container_path);
				std::ofstream output{file_path, std::ios_base::binary | std::ios_base::trunc};
				output.write(reinterpret_cast<char*>(entry.second.data()), entry.second.size());

				printf("Unpacked file '%s'.\n", entry.first.c_str());
			}
		} catch (...) {
			printf("Unpacking '%s' failed with unknown error.\n", input_path.u8string().c_str());
			return -1;
		}
	} else if (operation == mode::PACK) {
		try {
			std::vector<uint8_t> output_data;

			if (!std::filesystem::is_directory(input_path)) {
				printf("Input path '%s' must exist and be a directory.\n", input_path.u8string().c_str());
				return -1;
			}

			{
				ms::groupfile groupfile;

				// Try and read all files to be included.
				std::filesystem::recursive_directory_iterator files{input_path};
				for (auto file : files) {
					auto file_path = std::filesystem::relative(file.path(), input_path);

					if (!file.is_regular_file()) {
						//printf("Skipping path '%s'...\n", file_path.u8string().c_str());
						continue;
					}

					auto file_name = file_path.filename().u8string();
					auto file_ext  = file_path.extension().u8string();
					if ((file_name != "items.txt") && (file_ext != ".script")) {
						printf("Skipping file '%s'...\n", file_path.u8string().c_str());
						continue;
					}

					{ // Try and read the file.
						std::vector<uint8_t> input_data(file.file_size());
						std::ifstream        input{file.path(), std::ios_base::binary};
						input.read(reinterpret_cast<char*>(input_data.data()), input_data.size());
						input.close();

						groupfile.add(file_path.generic_u8string(), input_data);
						printf("Packed file '%s'.\n", file_path.generic_u8string().c_str());
					}
				}

				// Generate output information.
				groupfile.write(output_data);
			}

			{ // Try and write the output.
				std::ofstream output{output_path, std::ios_base::binary | std::ios_base::trunc};
				output.write(reinterpret_cast<char*>(output_data.data()), output_data.size());
				output.close();
			}

		} catch (...) {
			printf("Packing '%s' failed with unknown error.\n", input_path.u8string().c_str());
			return -1;
		}
	} else if (operation == mode::ENCRYPT) {
		try {
			std::vector<uint8_t> input_data;

			{ // Try and read the input file.
				size_t input_size = std::filesystem::file_size(input_path);
				input_data.resize(input_size);

				std::ifstream input{input_path, std::ios_base::binary};
				input.read(reinterpret_cast<char*>(input_data.data()), input_size);
				input.close();
			}

			// Encrypt the contents.
			std::vector<uint8_t> output_data;
			ms::scrypto          crypt;
			crypt.encrypt(input_data, output_data);

			// Try and write the output.
			std::ofstream output{output_path, std::ios_base::binary | std::ios_base::trunc};
			output.write(reinterpret_cast<char*>(output_data.data()), output_data.size());
		} catch (std::exception const& ex) {
			printf("Encrypting '%s' failed with error: %s.\n", input_path.u8string().c_str(), ex.what());
			return -1;
		} catch (...) {
			printf("Encrypting '%s' failed with unknown error.\n", input_path.u8string().c_str());
			return -1;
		}
	}

	return 0;
}
