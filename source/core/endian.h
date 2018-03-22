#pragma once

#include <cstdint>

/** Defines the endianness. */
enum endianness_t
{
	ENDIANNESS_LITTLE,
	ENDIANNESS_BIG
};

// declare native endianness to be one or the other
#ifdef LSB_FIRST
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_LITTLE;
#else
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_BIG;
#endif

/** PAIR is an endian-safe union useful for representing 32-bit CPU registers. */
union PAIR
{
#ifdef LSB_FIRST
	struct { u8 l, h, h2, h3; } b;
	struct { u16 l, h; } w;
	struct { s8 l, h, h2, h3; } sb;
	struct { s16 l, h; } sw;
#else
	struct { std::uint8_t h3, h2, h, l; } b;
	struct { std::int8_t h3, h2, h, l; } sb;
	struct { std::uint16_t h, l; } w;
	struct { std::int16_t h, l; } sw;
#endif
	std::uint32_t d;
	std::int32_t sd;
};


/** PAIR16 is a 16-bit extension of a PAIR. */
union PAIR16
{
#ifdef LSB_FIRST
	struct { u8 l, h; } b;
	struct { s8 l, h; } sb;
#else
	struct { std::uint8_t h, l; } b;
	struct { std::int8_t h, l; } sb;
#endif
	std::uint16_t w;
	std::int16_t sw;
};


/** PAIR64 is a 64-bit extension of a PAIR. */
union PAIR64
{
#ifdef LSB_FIRST
	struct { u8 l, h, h2, h3, h4, h5, h6, h7; } b;
	struct { u16 l, h, h2, h3; } w;
	struct { u32 l, h; } d;
	struct { s8 l, h, h2, h3, h4, h5, h6, h7; } sb;
	struct { s16 l, h, h2, h3; } sw;
	struct { s32 l, h; } sd;
#else
	struct { std::uint8_t h7, h6, h5, h4, h3, h2, h, l; } b;
	struct { std::uint16_t h3, h2, h, l; } w;
	struct { std::uint32_t h, l; } d;
	struct { std::int8_t h7, h6, h5, h4, h3, h2, h, l; } sb;
	struct { std::int16_t h3, h2, h, l; } sw;
	struct { std::int32_t h, l; } sd;
#endif
	std::uint64_t q;
	std::int64_t sq;
};

// endian-based value: first value is if 'endian' is little-endian, second is if 'endian' is big-endian
#define ENDIAN_VALUE_LE_BE(endian,leval,beval)  (((endian) == ENDIANNESS_LITTLE) ? (leval) : (beval))

// endian-based value: first value is if native endianness is little-endian, second is if native is big-endian
#define NATIVE_ENDIAN_VALUE_LE_BE(leval,beval)  ENDIAN_VALUE_LE_BE(ENDIANNESS_NATIVE, leval, beval)

// endian-based value: first value is if 'endian' matches native, second is if 'endian' doesn't match native
#define ENDIAN_VALUE_NE_NNE(endian,neval,nneval) (((endian) == ENDIANNESS_NATIVE) ? (neval) : (nneval))

