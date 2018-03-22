#pragma once

#include <cstdint>

// convert a series of 32 bits into a float
inline float u2f(std::uint32_t v)
{
	union {
		float ff;
		std::uint32_t vv;
	} u;
	u.vv = v;
	return u.ff;
}


// convert a float into a series of 32 bits
inline std::uint32_t f2u(float f)
{
	union {
		float ff;
		std::uint32_t vv;
	} u;
	u.ff = f;
	return u.vv;
}


// convert a series of 64 bits into a double
inline double u2d(std::uint64_t v)
{
	union {
		double dd;
		std::uint64_t vv;
	} u;
	u.vv = v;
	return u.dd;
}


// convert a double into a series of 64 bits
inline std::uint64_t d2u(double d)
{
	union {
		double dd;
		std::uint64_t vv;
	} u;
	u.dd = d;
	return u.vv;
}
