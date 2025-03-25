#pragma once
#include <cstdint>

namespace ds {

uint32_t Crc32(uint32_t startingValue, uint8_t* data, size_t dataSize, bool finalize);

}