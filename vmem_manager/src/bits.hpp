/*
 * bits.hpp
 *
 *  Created on: Apr 9, 2021
 *      Author: Jacob Fano
 */

#ifndef BITS_HPP_
#define BITS_HPP_

#include <limits>

#include "alias.hpp"

template<typename BITS_T>
static constexpr BITS_T pow2(u8 n) {
	return 1 << n;
}

template<typename BITS_T>
static constexpr BITS_T bitmask(u8 low, u8 high) {
	// BITS_T will overflow if asked to store a pow2(sizeof(BITS_T) * 8) or greater.
	// In the event that this would occur, instead avoid the problem all together
	if (high == sizeof(BITS_T) * 8)
		return std::numeric_limits<BITS_T>::max() - (pow2<BITS_T>(low) - 1);
	return (pow2<BITS_T>(high - low) - 1) << low;
}

template<typename BITS_T>
static constexpr BITS_T extractBits(u8 low, u8 high, BITS_T bits) {
	return (bits & bitmask<BITS_T>(low, high)) >> low;
}

static_assert(pow2<u8>(0) == 1);
static_assert(pow2<u8>(1) == 2);
static_assert(pow2<u8>(2) == 4);
static_assert(pow2<u8>(3) == 8);
static_assert(pow2<u16>(15) == 32768);

static_assert(bitmask<u16>( 0, 16) == 0xFFFF);
static_assert(bitmask<u16>( 0,  8) == 0x00FF);
static_assert(bitmask<u16>( 8, 16) == 0xFF00);
static_assert(bitmask<u32>( 0, 32) == 0xFFFFFFFF);
static_assert(bitmask<u32>( 0, 16) == 0x0000FFFF);
static_assert(bitmask<u32>(16, 32) == 0xFFFF0000);

static_assert(extractBits( 0, 16, 0xFFFF) == 0xFFFF);
static_assert(extractBits( 0,  8, 0xFFFF) == 0x00FF);
static_assert(extractBits( 8, 16, 0xFFFF) == 0x00FF);
static_assert(extractBits(12, 16, 0xAAAA) == 0x000A);

#define isInteger(INT) std::enable_if_t<std::is_integral<INT>::value, int> = 1
#define isEnum(E) std::enable_if_t<std::is_enum<E>::value, int> = 1
#define isNotEnum(E) std::enable_if_t<std::is_enum<E>::value, int> = 0
#define getEnumType(E) typename std::underlying_type<E>::type

template<typename E, isEnum(E)>
static constexpr getEnumType(E) enumToPrimitive(E e) noexcept {
	return static_cast<getEnumType(E)>(e);
}

template<typename I, isInteger(I)>
static constexpr bool areMaskedBitsSet(I bits, I mask) {
	return ((mask ^ (bits & mask)) == 0) ? true : false;
}

template<typename I, isInteger(I), typename E, isEnum(E)>
static constexpr bool areMaskedBitsSet(I bits, E mask) {
	return areMaskedBitsSet(bits, enumToPrimitive(mask));
}

template<typename E, isEnum(E)>
static constexpr bool areMaskedBitsSet(E bits, E mask) {
	return areMaskedBitsSet(enumToPrimitive(bits), enumToPrimitive(mask));
}

template<typename I, isInteger(I)>
static constexpr bool isBitSet(I bits, u8 bit) {
	return areMaskedBitsSet(bits, bitmask<I>(bit, bit + 1));
}

static_assert(areMaskedBitsSet(0x0F00, 0x0F00) == true);
static_assert(areMaskedBitsSet(0x0F00, 0x00F0) == false);
static_assert(areMaskedBitsSet(0x1000, 0x1000) == true);
static_assert(areMaskedBitsSet(0x1000, 0x100F) == false);
static_assert(isBitSet(0x0001, 0) == true);
static_assert(isBitSet(0x0010, 4) == true);
static_assert(isBitSet(0x0100, 8) == true);
static_assert(isBitSet(0x1000, 12) == true);
static_assert(isBitSet(0x0010, 0) == false);
static_assert(isBitSet(0x0100, 0) == false);
static_assert(isBitSet(0x1000, 0) == false);

#endif /* BITS_HPP_ */
