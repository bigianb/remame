#pragma once

#include "../../core/memcore.h"
#include "../../core/endian.h"
#include "address_map.h"

/** describes an address space and provides basic functions to map addresses to bytes. */
class address_space_config
{
public:
	// construction/destruction
	address_space_config() : m_name("unknown"),
		m_endianness(ENDIANNESS_NATIVE),
		m_data_width(0),
		m_addr_width(0),
		m_addr_shift(0),
		m_logaddr_width(0),
		m_page_shift(0),
		m_is_octal(false),
		m_internal_map(address_map_constructor()),
		m_default_map(address_map_constructor())
	{
	}

	/**
	@param name
	@param endian CPU endianness
	@param datawidth CPU parallelism bits
	@param addrwidth address bits
	@param addrshift
	@param internal
	@param defmap
	*/
	address_space_config(const char *name, endianness_t endian, std::uint8_t datawidth, std::uint8_t addrwidth, std::int8_t addrshift = 0, address_map_constructor internal = address_map_constructor(), address_map_constructor defmap = address_map_constructor()) :
		m_name(name),
		m_endianness(endian),
		m_data_width(datawidth),
		m_addr_width(addrwidth),
		m_addr_shift(addrshift),
		m_logaddr_width(addrwidth),
		m_page_shift(0),
		m_is_octal(false),
		m_internal_map(internal),
		m_default_map(defmap)
	{
	}

	address_space_config(const char *name, endianness_t endian, std::uint8_t datawidth, std::uint8_t addrwidth, std::int8_t addrshift, std::uint8_t logwidth, std::uint8_t pageshift, address_map_constructor internal = address_map_constructor(), address_map_constructor defmap = address_map_constructor()):
		m_name(name),
		m_endianness(endian),
		m_data_width(datawidth),
		m_addr_width(addrwidth),
		m_addr_shift(addrshift),
		m_logaddr_width(logwidth),
		m_page_shift(pageshift),
		m_is_octal(false),
		m_internal_map(internal),
		m_default_map(defmap)
	{
	}

	// getters
	const char *name() const { return m_name; }
	endianness_t endianness() const { return m_endianness; }
	int data_width() const { return m_data_width; }
	int addr_width() const { return m_addr_width; }
	int addr_shift() const { return m_addr_shift; }

	// Actual alignment of the bus addresses
	int alignment() const { int bytes = m_data_width / 8; return m_addr_shift < 0 ? bytes >> -m_addr_shift : bytes << m_addr_shift; }

	// Address delta to byte delta helpers
	inline offs_t addr2byte(offs_t address) const { return (m_addr_shift < 0) ? (address << -m_addr_shift) : (address >> m_addr_shift); }
	inline offs_t byte2addr(offs_t address) const { return (m_addr_shift > 0) ? (address << m_addr_shift) : (address >> -m_addr_shift); }

	// address-to-byte conversion helpers
	inline offs_t addr2byte_end(offs_t address) const { return (m_addr_shift < 0) ? ((address << -m_addr_shift) | ((1 << -m_addr_shift) - 1)) : (address >> m_addr_shift); }
	inline offs_t byte2addr_end(offs_t address) const { return (m_addr_shift > 0) ? ((address << m_addr_shift) | ((1 << m_addr_shift) - 1)) : (address >> -m_addr_shift); }

	// state
	const char *        m_name;
	endianness_t        m_endianness;
	std::uint8_t        m_data_width;
	std::uint8_t        m_addr_width;
	std::int8_t         m_addr_shift;
	std::uint8_t        m_logaddr_width;
	std::uint8_t        m_page_shift;
	bool                m_is_octal;                 // to determine if messages/debugger will show octal or hex

	address_map_constructor m_internal_map;
	address_map_constructor m_default_map;
};
