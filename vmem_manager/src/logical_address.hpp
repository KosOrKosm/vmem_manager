/*
 * logical_address.hpp
 *
 *  Created on: Apr 9, 2021
 *      Author: Jacob Fano
 */

#include "bits.hpp"

#ifndef LOGICAL_ADDRESS_HPP_
#define LOGICAL_ADDRESS_HPP_

class LogicalAddress {

	u32 raw;

public:

	constexpr LogicalAddress(u32 raw) :
			raw(raw) {}

	constexpr LogicalAddress() :
			LogicalAddress(0) {}

	constexpr LogicalAddress(const LogicalAddress& copy) :
			raw(copy.raw) {}

	constexpr LogicalAddress(LogicalAddress&& copy) :
			raw(copy.raw) {
		copy.raw = 0;
	}

	constexpr const LogicalAddress& operator=(const LogicalAddress& other) {
		raw = other.raw;
		return *this;
	}
	constexpr const LogicalAddress& operator=(LogicalAddress&& other) {
		raw = other.raw;
		other.raw = 0;
		return *this;
	}

	constexpr operator u32() const { return raw; }
	constexpr u8 offset() const { return extractBits(0, 8, raw); }
	constexpr u8 page() const { return extractBits(8, 16, raw); }

};

static_assert(LogicalAddress(0xFFFFFFFF).offset() == 0xFF);
static_assert(LogicalAddress(0xFFFFFF00).offset() == 0x00);
static_assert(LogicalAddress(0xFFFFFFFF).page() == 0xFF);
static_assert(LogicalAddress(0xFFFF00FF).page() == 0x00);
static_assert(std::is_literal_type_v<LogicalAddress>);

#endif /* LOGICAL_ADDRESS_HPP_ */
