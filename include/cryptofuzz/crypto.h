#pragma once

#include <cstdint>
#include <vector>

namespace cryptofuzz {
namespace crypto {

std::vector<uint8_t> sha1(const uint8_t* data, const size_t size);
std::vector<uint8_t> sha1(const std::vector<uint8_t> data);

} /* namespace crypto */
} /* namespace cryptofuzz */
