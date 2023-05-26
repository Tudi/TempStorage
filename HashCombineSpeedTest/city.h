#pragma once

#include <stdint.h>

uint64_t CityHash64(const char* s, size_t len);

template <size_t len>
uint64_t CityHash64_(const char* s) { return CityHash64(s, len); }