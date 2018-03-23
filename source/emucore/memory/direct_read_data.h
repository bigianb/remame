#pragma once

#include <cstdint>
#include <list>

#include "../../core/delegate.h"
#include "../../core/memcore.h"
#include "../../core/macros.h"
#include "mem_defs.h"
#include "../emucore.h"

class address_space;
class direct_range;

/** direct_read_data contains state data for direct read access. */
template<int AddrShift> class direct_read_data
{
	friend class address_table;

public:
	using direct_update_delegate = delegate<offs_t (direct_read_data<AddrShift> &, offs_t)>;

	// direct_range is an internal class that is part of a list of start/end ranges
	class direct_range
	{
	public:
		// construction
		direct_range(): m_addrstart(0),m_addrend(~0) { }

		inline bool operator==(direct_range val) noexcept
		{   // return true if _Left and _Right identify the same thread
			return (m_addrstart == val.m_addrstart) && (m_addrend == val.m_addrend);
		}

		// internal state
		offs_t                  m_addrstart;            // starting offset of the range
		offs_t                  m_addrend;              // ending offset of the range
	};

	// construction/destruction
	direct_read_data(address_space &space);
	~direct_read_data();

	// getters
	address_space &space() const { return m_space; }
	std::uint8_t *ptr() const { return m_ptr; }

	// see if an address is within bounds, or attempt to update it if not
	bool address_is_valid(offs_t address) { return EXPECTED(address >= m_addrstart && address <= m_addrend) || set_direct_region(address); }

	// force a recomputation on the next read
	void force_update() { m_addrend = 0; m_addrstart = 1; }
	void force_update(std::uint16_t if_match) { if (m_entry == if_match) force_update(); }

	// accessor methods
	void *read_ptr(offs_t address, offs_t directxor = 0);
	std::uint8_t read_byte(offs_t address, offs_t directxor = 0);
	std::uint16_t read_word(offs_t address, offs_t directxor = 0);
	std::uint32_t read_dword(offs_t address, offs_t directxor = 0);
	std::uint64_t read_qword(offs_t address, offs_t directxor = 0);

	void remove_intersecting_ranges(offs_t start, offs_t end);

	static constexpr offs_t offset_to_byte(offs_t offset) { return AddrShift < 0 ? offset << iabs(AddrShift) : offset >> iabs(AddrShift); }

private:
	// internal helpers
	bool set_direct_region(offs_t address);
	direct_range *find_range(offs_t address, std::uint16_t &entry);

	// internal state
	address_space &             m_space;
	std::uint8_t *              m_ptr;                  // direct access data pointer
	offs_t                      m_addrmask;             // address mask
	offs_t                      m_addrstart;            // minimum valid address
	offs_t                      m_addrend;              // maximum valid address
	std::uint16_t               m_entry;                // live entry
	std::list<direct_range>     m_rangelist[TOTAL_MEMORY_BANKS];  // list of ranges for each entry
};


//-------------------------------------------------
//  read_ptr - return a pointer to valid RAM
//  referenced by the address, or nullptr if no RAM
//  backing that address
//-------------------------------------------------

template<int AddrShift> inline void *direct_read_data<AddrShift>::read_ptr(offs_t address, offs_t directxor)
{
	if (address_is_valid(address))
		return &m_ptr[offset_to_byte(((address ^ directxor) & m_addrmask))];
	return nullptr;
}


//-------------------------------------------------
//  read_byte - read a byte via the
//  direct_read_data class
//-------------------------------------------------

template<int AddrShift> inline std::uint8_t direct_read_data<AddrShift>::read_byte(offs_t address, offs_t directxor)
{
	if (AddrShift <= -1)
		fatalerror("Can't direct_read_data::read_byte on a memory space with address shift %d", AddrShift);
	if (address_is_valid(address))
		return m_ptr[offset_to_byte((address ^ directxor) & m_addrmask)];
	return m_space.read_byte(address);
}


//-------------------------------------------------
//  read_word - read a word via the
//  direct_read_data class
//-------------------------------------------------

template<int AddrShift> inline std::uint16_t direct_read_data<AddrShift>::read_word(offs_t address, offs_t directxor)
{
	if (AddrShift <= -2)
		fatalerror("Can't direct_read_data::read_word on a memory space with address shift %d", AddrShift);
	if (address_is_valid(address))
		return *reinterpret_cast<std::uint16_t *>(&m_ptr[offset_to_byte((address ^ directxor) & m_addrmask)]);
	return m_space.read_word(address);
}


//-------------------------------------------------
//  read_dword - read a dword via the
//  direct_read_data class
//-------------------------------------------------

template<int AddrShift> inline std::uint32_t direct_read_data<AddrShift>::read_dword(offs_t address, offs_t directxor)
{
	if (AddrShift <= -3)
		fatalerror("Can't direct_read_data::read_dword on a memory space with address shift %d", AddrShift);
	if (address_is_valid(address))
		return *reinterpret_cast<std::uint32_t *>(&m_ptr[offset_to_byte((address ^ directxor) & m_addrmask)]);
	return m_space.read_dword(address);
}


//-------------------------------------------------
//  read_qword - read a qword via the
//  direct_read_data class
//-------------------------------------------------

template<int AddrShift> inline std::uint64_t direct_read_data<AddrShift>::read_qword(offs_t address, offs_t directxor)
{
	if (address_is_valid(address))
		return *reinterpret_cast<std::uint64_t *>(&m_ptr[offset_to_byte((address ^ directxor) & m_addrmask)]);
	return m_space.read_qword(address);
}
