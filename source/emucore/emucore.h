// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

emucore.h

General core utilities and macros used throughout the emulator.
***************************************************************************/

#pragma once

// standard C includes
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// standard C++ includes
#include <cassert>
#include <exception>
#include <type_traits>
#include <typeinfo>

#include "../core/coretmpl.h"
#include "../core/compiler_specifics.h"

class device_t;

//**************************************************************************
//  FUNDAMENTAL TYPES
//**************************************************************************

// genf is a generic function pointer; cast function pointers to this instead of void *
typedef void genf(void);

// pen_t is used to represent pixel values in bitmaps
typedef std::uint32_t pen_t;

// stream_sample_t is used to represent a single sample in a sound stream
typedef std::int32_t stream_sample_t;

//**************************************************************************
//  COMMON CONSTANTS
//**************************************************************************

// M_PI is not part of the C/C++ standards and is not present on
// strict ANSI compilers or when compiling under GCC with -ansi
#ifndef M_PI
#define M_PI                            3.14159265358979323846
#endif


// orientation of bitmaps
constexpr int ORIENTATION_FLIP_X = 0x0001;  // mirror everything in the X direction
constexpr int ORIENTATION_FLIP_Y = 0x0002;  // mirror everything in the Y direction
constexpr int ORIENTATION_SWAP_XY = 0x0004;  // mirror along the top-left/bottom-right diagonal

constexpr int ROT0 = 0;
constexpr int ROT90 = ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X;  // rotate clockwise 90 degrees
constexpr int ROT180 = ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y;   // rotate 180 degrees
constexpr int ROT270 = ORIENTATION_SWAP_XY | ORIENTATION_FLIP_Y;  // rotate counter-clockwise 90 degrees


//**************************************************************************
//  COMMON MACROS
//**************************************************************************


// macros to convert radians to degrees and degrees to radians
#define RADIAN_TO_DEGREE(x)   ((180.0 / M_PI) * (x))
#define DEGREE_TO_RADIAN(x)   ((M_PI / 180.0) * (x))

// useful functions to deal with bit shuffling encryptions
template <typename T, typename U> constexpr T BIT(T x, U n) { return (x >> n) & T(1); }

template <typename T, typename U> constexpr T bitswap(T val, U b)
{
	return BIT(val, b) << 0U;
}

template <typename T, typename U, typename... V> constexpr T bitswap(T val, U b, V... c)
{
	return (BIT(val, b) << sizeof...(c)) | bitswap(val, c...);
}

// explicit version that checks number of bit position arguments
template <unsigned B, typename T, typename... U> T bitswap(T val, U... b)
{
	static_assert(sizeof...(b) == B, "wrong number of bits");
	static_assert((sizeof(std::remove_reference_t<T>) * 8) >= B, "return type too small for result");
	return bitswap(val, b...);
}


//**************************************************************************
//  CASTING TEMPLATES
//**************************************************************************

void report_bad_cast(const std::type_info &src_type, const std::type_info &dst_type);
void report_bad_device_cast(const device_t *dev, const std::type_info &src_type, const std::type_info &dst_type);

template <typename Dest, typename Source>
inline std::enable_if_t<std::is_base_of<device_t, Source>::value> report_bad_cast(Source *const src)
{
	if (src) report_bad_device_cast(src, typeid(Source), typeid(Dest));
	else report_bad_cast(typeid(Source), typeid(Dest));
}

template <typename Dest, typename Source>
inline std::enable_if_t<!std::is_base_of<device_t, Source>::value> report_bad_cast(Source *const src)
{
	device_t const *dev(dynamic_cast<device_t const *>(src));
	if (dev) report_bad_device_cast(dev, typeid(Source), typeid(Dest));
	else report_bad_cast(typeid(Source), typeid(Dest));
}

// template function for casting from a base class to a derived class that is checked
// in debug builds and fast in release builds
template <typename Dest, typename Source>
inline Dest downcast(Source *src)
{
#if defined(MAME_DEBUG) && !defined(MAME_DEBUG_FAST)
	Dest const chk(dynamic_cast<Dest>(src));
	if (chk != src) report_bad_cast<std::remove_pointer_t<Dest>, Source>(src);
#endif
	return static_cast<Dest>(src);
}

template<class Dest, class Source>
inline Dest downcast(Source &src)
{
#if defined(MAME_DEBUG) && !defined(MAME_DEBUG_FAST)
	std::remove_reference_t<Dest> *const chk(dynamic_cast<std::remove_reference_t<Dest> *>(&src));
	if (chk != &src) report_bad_cast<std::remove_reference_t<Dest>, Source>(&src);
#endif
	return static_cast<Dest>(src);
}

// template function which takes a strongly typed enumerator and returns its value as a compile-time constant
template <typename E>
using enable_enum_t = typename std::enable_if_t<std::is_enum<E>::value, typename std::underlying_type_t<E>>;

template <typename E>
constexpr inline enable_enum_t<E>
underlying_value(E e) noexcept
{
	return static_cast< typename std::underlying_type<E>::type >(e);
}

// template function which takes an integral value and returns its representation as enumerator (even strongly typed)
template <typename E, typename T>
constexpr inline typename std::enable_if_t<std::is_enum<E>::value && std::is_integral<T>::value, E>
enum_value(T value) noexcept
{
	return static_cast<E>(value);
}

// constexpr absolute value of an integer
template <typename T>
constexpr std::enable_if_t<std::is_signed<T>::value, T> iabs(T v)
{
	return (v < T(0)) ? -v : v;
}

