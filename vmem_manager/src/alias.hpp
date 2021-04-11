/*
 * alias.hpp
 *
 *  Created on: Apr 9, 2021
 *      Author: Jacob Fano
 */

#include <cstdint>

#ifndef ALIAS_HPP_
#define ALIAS_HPP_

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;
using byte = char;

static_assert(sizeof(u64) == 8);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u8) == 1);
static_assert(sizeof(u8) == sizeof(byte));

#endif /* ALIAS_HPP_ */
