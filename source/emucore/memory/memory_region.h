#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "../../core/memcore.h"
#include "../../core/macros.h"
#include "../../core/endian.h"

// IJB: Why do we need to hold a reference to a running machine?
class running_machine;

/** A region of memory. */
class memory_region
{
	DISABLE_COPYING(memory_region);

public:
	// construction/destruction
	memory_region(running_machine &machine, const char *name, std::uint32_t length, std::uint8_t width, endianness_t endian) :
		m_machine(machine),
		m_name(name),
		m_buffer(length),
		m_endianness(endian),
		m_bitwidth(width * 8),
		m_bytewidth(width)
		{
			assert(width == 1 || width == 2 || width == 4 || width == 8);
		}
	}

	// getters
	running_machine &machine() const { return m_machine; }
	std::uint8_t *base() { return (m_buffer.size() > 0) ? &m_buffer[0] : nullptr; }
	std::uint8_t *end() { return base() + m_buffer.size(); }
	std::uint32_t bytes() const { return m_buffer.size(); }
	const char *name() const { return m_name.c_str(); }

	// flag expansion
	endianness_t endianness() const { return m_endianness; }
	std::uint8_t bitwidth() const { return m_bitwidth; }
	std::uint8_t bytewidth() const { return m_bytewidth; }

	// data access
	std::uint8_t &as_u8(offs_t offset = 0) { return m_buffer[offset]; }
	std::uint16_t &as_u16(offs_t offset = 0) { return reinterpret_cast<std::uint16_t *>(base())[offset]; }
	std::uint32_t &as_u32(offs_t offset = 0) { return reinterpret_cast<std::uint32_t *>(base())[offset]; }
	std::uint64_t &as_u64(offs_t offset = 0) { return reinterpret_cast<std::uint64_t *>(base())[offset]; }

private:
	// internal data
	running_machine & m_machine;
	std::string             m_name;
	std::vector<std::uint8_t> m_buffer;
	endianness_t            m_endianness;
	std::uint8_t            m_bitwidth;
	std::uint8_t            m_bytewidth;
};

