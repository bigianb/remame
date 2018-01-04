// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  eivc.h
//
//  Inline implementations for MSVC compiler.
//
//============================================================

#pragma once

#include <intrin.h>
#pragma intrinsic(_BitScanReverse)


/***************************************************************************
    INLINE BIT MANIPULATION FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    count_leading_zeros - return the number of
    leading zero bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_zeros
#define count_leading_zeros _count_leading_zeros
inline std::uint8_t _count_leading_zeros(std::uint32_t value)
{
	unsigned long index;
	unsigned long count = _BitScanReverse(&index, value) ? (31 - index) : 32;
	return static_cast<std::uint8_t>(count);
}
#endif


/*-------------------------------------------------
    count_leading_ones - return the number of
    leading one bits in a 32-bit value
-------------------------------------------------*/

#ifndef count_leading_ones
#define count_leading_ones _count_leading_ones
inline std::uint8_t _count_leading_ones(std::uint32_t value)
{
	unsigned long index;
	unsigned long count = _BitScanReverse(&index, ~value) ? (31 - index) : 32;
	return static_cast<std::uint8_t>(count);
}
#endif
