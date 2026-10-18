#pragma once
// Minimal SHA-256 (FIPS 180-4) for test oracles — no third-party dependency, any platform.
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

namespace s100test {

std::string sha256_hex(const void* data, std::size_t size);
inline std::string sha256_hex(const std::string& s) { return sha256_hex(s.data(), s.size()); }
/** sha256 of a file's bytes; empty string if the file cannot be read. */
std::string sha256_file_hex(const std::filesystem::path& path);

}  // namespace s100test
