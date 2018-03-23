// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    emumem.h

    Functions which handle device memory accesses.

***************************************************************************/

#pragma once
#include <cstdint>
#include "../devdelegate.h"
#include "../../core/endian.h"
#include "../../core/exceptions.h"
#include "../../core/memcore.h"

#include "mem_defs.h"

using s8 = std::int8_t;
using u8 = std::uint8_t;
using s16 = std::int16_t;
using u16 = std::uint16_t;
using s32 = std::int32_t;
using u32 = std::uint32_t;
using s64 = std::int64_t;
using u64 = std::uint64_t;


//**************************************************************************
//  CONSTANTS
//**************************************************************************



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// private classes declared in emumem.cpp
class address_table;
class address_table_read;
class address_table_setoffset;
class address_table_write;



class address_map;
class address_map_entry;
class address_space;
class memory_manager;

// address map constructors are delegates that build up an address_map
using address_map_constructor = named_delegate<void (address_map &)>;

// struct with function pointers for accessors; use is generally discouraged unless necessary
struct data_accessors
{
	u8      (*read_byte)(address_space &space, offs_t address);
	u16     (*read_word)(address_space &space, offs_t address);
	u16     (*read_word_masked)(address_space &space, offs_t address, u16 mask);
	u32     (*read_dword)(address_space &space, offs_t address);
	u32     (*read_dword_masked)(address_space &space, offs_t address, u32 mask);
	u64     (*read_qword)(address_space &space, offs_t address);
	u64     (*read_qword_masked)(address_space &space, offs_t address, u64 mask);

	void    (*write_byte)(address_space &space, offs_t address, u8 data);
	void    (*write_word)(address_space &space, offs_t address, u16 data);
	void    (*write_word_masked)(address_space &space, offs_t address, u16 data, u16 mask);
	void    (*write_dword)(address_space &space, offs_t address, u32 data);
	void    (*write_dword_masked)(address_space &space, offs_t address, u32 data, u32 mask);
	void    (*write_qword)(address_space &space, offs_t address, u64 data);
	void    (*write_qword_masked)(address_space &space, offs_t address, u64 data, u64 mask);
};


// ======================> read_delegate

// declare delegates for each width
typedef device_delegate<u8 (address_space &, offs_t, u8)> read8_delegate;
typedef device_delegate<u16 (address_space &, offs_t, u16)> read16_delegate;
typedef device_delegate<u32 (address_space &, offs_t, u32)> read32_delegate;
typedef device_delegate<u64 (address_space &, offs_t, u64)> read64_delegate;


// ======================> write_delegate

// declare delegates for each width
typedef device_delegate<void (address_space &, offs_t, u8, u8)> write8_delegate;
typedef device_delegate<void (address_space &, offs_t, u16, u16)> write16_delegate;
typedef device_delegate<void (address_space &, offs_t, u32, u32)> write32_delegate;
typedef device_delegate<void (address_space &, offs_t, u64, u64)> write64_delegate;

// ======================> setoffset_delegate

typedef device_delegate<void (address_space &, offs_t)> setoffset_delegate;



// ======================> address_space_config

// describes an address space and provides basic functions to map addresses to bytes
class address_space_config
{
public:
	// construction/destruction
	address_space_config();
	address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift = 0, address_map_constructor internal = address_map_constructor(), address_map_constructor defmap = address_map_constructor());
	address_space_config(const char *name, endianness_t endian, u8 datawidth, u8 addrwidth, s8 addrshift, u8 logwidth, u8 pageshift, address_map_constructor internal = address_map_constructor(), address_map_constructor defmap = address_map_constructor());

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
	u8                  m_data_width;
	u8                  m_addr_width;
	s8                  m_addr_shift;
	u8                  m_logaddr_width;
	u8                  m_page_shift;
	bool                m_is_octal;                 // to determine if messages/debugger will show octal or hex

	address_map_constructor m_internal_map;
	address_map_constructor m_default_map;
};


// ======================> memory_bank

// a memory bank is a global pointer to memory that can be shared across devices and changed dynamically
class memory_bank
{
	// a bank reference is an entry in a list of address spaces that reference a given bank
	class bank_reference
	{
	public:
		// construction/destruction
		bank_reference(address_space &space, read_or_write readorwrite)
			: m_space(space),
				m_readorwrite(readorwrite) { }

		// getters
		address_space &space() const { return m_space; }

		// does this reference match the space+read/write combination?
		bool matches(const address_space &space, read_or_write readorwrite) const
		{
			return (&space == &m_space && (readorwrite == read_or_write::READWRITE || readorwrite == m_readorwrite));
		}

	private:
		// internal state
		address_space &         m_space;            // address space that references us
		read_or_write           m_readorwrite;      // used for read or write?
	};

	// a bank_entry contains a pointer
	struct bank_entry
	{
		u8 *    m_ptr;
	};

public:
	// construction/destruction
	memory_bank(address_space &space, int index, offs_t start, offs_t end, const char *tag = nullptr);
	~memory_bank();

	// getters
	running_machine &machine() const { return m_machine; }
	int index() const { return m_index; }
	int entry() const { return m_curentry; }
	bool anonymous() const { return m_anonymous; }
	offs_t addrstart() const { return m_addrstart; }
	void *base() const { return *m_baseptr; }
	const char *tag() const { return m_tag.c_str(); }
	const char *name() const { return m_name.c_str(); }

	// compare a range against our range
	bool matches_exactly(offs_t addrstart, offs_t addrend) const { return (m_addrstart == addrstart && m_addrend == addrend); }
	bool fully_covers(offs_t addrstart, offs_t addrend) const { return (m_addrstart <= addrstart && m_addrend >= addrend); }
	bool is_covered_by(offs_t addrstart, offs_t addrend) const { return (m_addrstart >= addrstart && m_addrend <= addrend); }
	bool straddles(offs_t addrstart, offs_t addrend) const { return (m_addrstart < addrend && m_addrend > addrstart); }

	// track and verify address space references to this bank
	bool references_space(const address_space &space, read_or_write readorwrite) const;
	void add_reference(address_space &space, read_or_write readorwrite);

	// set the base explicitly
	void set_base(void *base);

	// configure and set entries
	void configure_entry(int entrynum, void *base);
	void configure_entries(int startentry, int numentries, void *base, offs_t stride);
	void set_entry(int entrynum);

private:
	// internal helpers
	void invalidate_references();
	void expand_entries(int entrynum);

	// internal state
	running_machine &       m_machine;              // need the machine to free our memory
	u8 **                   m_baseptr;              // pointer to our base pointer in the global array
	u16                     m_index;                // array index for this handler
	bool                    m_anonymous;            // are we anonymous or explicit?
	offs_t                  m_addrstart;            // start offset
	offs_t                  m_addrend;              // end offset
	int                     m_curentry;             // current entry
	std::vector<bank_entry> m_entry;                // array of entries (dynamically allocated)
	std::string             m_name;                 // friendly name for this bank
	std::string             m_tag;                  // tag for this bank
	std::vector<std::unique_ptr<bank_reference>> m_reflist;          // linked list of address spaces referencing this bank
};


// ======================> memory_share

// a memory share contains information about shared memory region
class memory_share
{
public:
	// construction/destruction
	memory_share(u8 width, size_t bytes, endianness_t endianness, void *ptr = nullptr)
		: m_ptr(ptr),
			m_bytes(bytes),
			m_endianness(endianness),
			m_bitwidth(width),
			m_bytewidth(width <= 8 ? 1 : width <= 16 ? 2 : width <= 32 ? 4 : 8)
	{ }

	// getters
	void *ptr() const { return m_ptr; }
	size_t bytes() const { return m_bytes; }
	endianness_t endianness() const { return m_endianness; }
	u8 bitwidth() const { return m_bitwidth; }
	u8 bytewidth() const { return m_bytewidth; }

	// setters
	void set_ptr(void *ptr) { m_ptr = ptr; }

private:
	// internal state
	void *                  m_ptr;                  // pointer to the memory backing the region
	size_t                  m_bytes;                // size of the shared region in bytes
	endianness_t            m_endianness;           // endianness of the memory
	u8                      m_bitwidth;             // width of the shared region in bits
	u8                      m_bytewidth;            // width in bytes, rounded up to a power of 2

};


// ======================> memory_region

// memory region object
class memory_region
{
	DISABLE_COPYING(memory_region);

	friend class memory_manager;
public:
	// construction/destruction
	memory_region(running_machine &machine, const char *name, u32 length, u8 width, endianness_t endian);

	// getters
	running_machine &machine() const { return m_machine; }
	u8 *base() { return (m_buffer.size() > 0) ? &m_buffer[0] : nullptr; }
	u8 *end() { return base() + m_buffer.size(); }
	u32 bytes() const { return m_buffer.size(); }
	const char *name() const { return m_name.c_str(); }

	// flag expansion
	endianness_t endianness() const { return m_endianness; }
	u8 bitwidth() const { return m_bitwidth; }
	u8 bytewidth() const { return m_bytewidth; }

	// data access
	u8 &as_u8(offs_t offset = 0) { return m_buffer[offset]; }
	u16 &as_u16(offs_t offset = 0) { return reinterpret_cast<u16 *>(base())[offset]; }
	u32 &as_u32(offs_t offset = 0) { return reinterpret_cast<u32 *>(base())[offset]; }
	u64 &as_u64(offs_t offset = 0) { return reinterpret_cast<u64 *>(base())[offset]; }

private:
	// internal data
	running_machine &       m_machine;
	std::string             m_name;
	std::vector<u8>         m_buffer;
	endianness_t            m_endianness;
	u8                      m_bitwidth;
	u8                      m_bytewidth;
};


//**************************************************************************
//  MACROS
//**************************************************************************

// space read/write handler function macros
#define READ8_MEMBER(name)              u8     name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u8 mem_mask)
#define WRITE8_MEMBER(name)             void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u8 data, ATTR_UNUSED u8 mem_mask)
#define READ16_MEMBER(name)             u16    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u16 mem_mask)
#define WRITE16_MEMBER(name)            void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u16 data, ATTR_UNUSED u16 mem_mask)
#define READ32_MEMBER(name)             u32    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u32 mem_mask)
#define WRITE32_MEMBER(name)            void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u32 data, ATTR_UNUSED u32 mem_mask)
#define READ64_MEMBER(name)             u64    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u64 mem_mask)
#define WRITE64_MEMBER(name)            void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u64 data, ATTR_UNUSED u64 mem_mask)

#define DECLARE_READ8_MEMBER(name)      u8     name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u8 mem_mask = 0xff)
#define DECLARE_WRITE8_MEMBER(name)     void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u8 data, ATTR_UNUSED u8 mem_mask = 0xff)
#define DECLARE_READ16_MEMBER(name)     u16    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u16 mem_mask = 0xffff)
#define DECLARE_WRITE16_MEMBER(name)    void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u16 data, ATTR_UNUSED u16 mem_mask = 0xffff)
#define DECLARE_READ32_MEMBER(name)     u32    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u32 mem_mask = 0xffffffff)
#define DECLARE_WRITE32_MEMBER(name)    void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u32 data, ATTR_UNUSED u32 mem_mask = 0xffffffff)
#define DECLARE_READ64_MEMBER(name)     u64    name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u64 mem_mask = 0xffffffffffffffffU)
#define DECLARE_WRITE64_MEMBER(name)    void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED u64 data, ATTR_UNUSED u64 mem_mask = 0xffffffffffffffffU)

#define SETOFFSET_MEMBER(name)          void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset)
#define DECLARE_SETOFFSET_MEMBER(name)  void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset)

// device delegate macros
#define READ8_DELEGATE(_class, _member)                     read8_delegate(FUNC(_class::_member), this)
#define WRITE8_DELEGATE(_class, _member)                    write8_delegate(FUNC(_class::_member), this)
#define READ16_DELEGATE(_class, _member)                    read16_delegate(FUNC(_class::_member), this)
#define WRITE16_DELEGATE(_class, _member)                   write16_delegate(FUNC(_class::_member), this)
#define READ32_DELEGATE(_class, _member)                    read32_delegate(FUNC(_class::_member), this)
#define WRITE32_DELEGATE(_class, _member)                   write32_delegate(FUNC(_class::_member), this)
#define READ64_DELEGATE(_class, _member)                    read64_delegate(FUNC(_class::_member), this)
#define WRITE64_DELEGATE(_class, _member)                   write64_delegate(FUNC(_class::_member), this)

#define READ8_DEVICE_DELEGATE(_device, _class, _member)     read8_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE8_DEVICE_DELEGATE(_device, _class, _member)    write8_delegate(FUNC(_class::_member), (_class *)_device)
#define READ16_DEVICE_DELEGATE(_device, _class, _member)    read16_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE16_DEVICE_DELEGATE(_device, _class, _member)   write16_delegate(FUNC(_class::_member), (_class *)_device)
#define READ32_DEVICE_DELEGATE(_device, _class, _member)    read32_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE32_DEVICE_DELEGATE(_device, _class, _member)   write32_delegate(FUNC(_class::_member), (_class *)_device)
#define READ64_DEVICE_DELEGATE(_device, _class, _member)    read64_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE64_DEVICE_DELEGATE(_device, _class, _member)   write64_delegate(FUNC(_class::_member), (_class *)_device)


// helper macro for merging data with the memory mask
#define COMBINE_DATA(varptr)            (*(varptr) = (*(varptr) & ~mem_mask) | (data & mem_mask))

#define ACCESSING_BITS_0_7              ((mem_mask & 0x000000ffU) != 0)
#define ACCESSING_BITS_8_15             ((mem_mask & 0x0000ff00U) != 0)
#define ACCESSING_BITS_16_23            ((mem_mask & 0x00ff0000U) != 0)
#define ACCESSING_BITS_24_31            ((mem_mask & 0xff000000U) != 0)
#define ACCESSING_BITS_32_39            ((mem_mask & 0x000000ff00000000U) != 0)
#define ACCESSING_BITS_40_47            ((mem_mask & 0x0000ff0000000000U) != 0)
#define ACCESSING_BITS_48_55            ((mem_mask & 0x00ff000000000000U) != 0)
#define ACCESSING_BITS_56_63            ((mem_mask & 0xff00000000000000U) != 0)

#define ACCESSING_BITS_0_15             ((mem_mask & 0x0000ffffU) != 0)
#define ACCESSING_BITS_16_31            ((mem_mask & 0xffff0000U) != 0)
#define ACCESSING_BITS_32_47            ((mem_mask & 0x0000ffff00000000U) != 0)
#define ACCESSING_BITS_48_63            ((mem_mask & 0xffff000000000000U) != 0)

#define ACCESSING_BITS_0_31             ((mem_mask & 0xffffffffU) != 0)
#define ACCESSING_BITS_32_63            ((mem_mask & 0xffffffff00000000U) != 0)


// macros for accessing bytes and words within larger chunks

// read/write a byte to a 16-bit space
#define BYTE_XOR_BE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0))
#define BYTE_XOR_LE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,1))

// read/write a byte to a 32-bit space
#define BYTE4_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(3,0))
#define BYTE4_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,3))

// read/write a word to a 32-bit space
#define WORD_XOR_BE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(2,0))
#define WORD_XOR_LE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,2))

// read/write a byte to a 64-bit space
#define BYTE8_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(7,0))
#define BYTE8_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,7))

// read/write a word to a 64-bit space
#define WORD2_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(6,0))
#define WORD2_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,6))

// read/write a dword to a 64-bit space
#define DWORD_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(4,0))
#define DWORD_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,4))


// helpers for checking address alignment
#define WORD_ALIGNED(a)                 (((a) & 1) == 0)
#define DWORD_ALIGNED(a)                (((a) & 3) == 0)
#define QWORD_ALIGNED(a)                (((a) & 7) == 0)



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

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

template<int AddrShift> inline u8 direct_read_data<AddrShift>::read_byte(offs_t address, offs_t directxor)
{
	if(AddrShift <= -1)
		fatalerror("Can't direct_read_data::read_byte on a memory space with address shift %d", AddrShift);
	if (address_is_valid(address))
		return m_ptr[offset_to_byte((address ^ directxor) & m_addrmask)];
	return m_space.read_byte(address);
}


//-------------------------------------------------
//  read_word - read a word via the
//  direct_read_data class
//-------------------------------------------------

template<int AddrShift> inline u16 direct_read_data<AddrShift>::read_word(offs_t address, offs_t directxor)
{
	if(AddrShift <= -2)
		fatalerror("Can't direct_read_data::read_word on a memory space with address shift %d", AddrShift);
	if (address_is_valid(address))
		return *reinterpret_cast<u16 *>(&m_ptr[offset_to_byte((address ^ directxor) & m_addrmask)]);
	return m_space.read_word(address);
}


//-------------------------------------------------
//  read_dword - read a dword via the
//  direct_read_data class
//-------------------------------------------------

template<int AddrShift> inline u32 direct_read_data<AddrShift>::read_dword(offs_t address, offs_t directxor)
{
	if(AddrShift <= -3)
		fatalerror("Can't direct_read_data::read_dword on a memory space with address shift %d", AddrShift);
	if (address_is_valid(address))
		return *reinterpret_cast<u32 *>(&m_ptr[offset_to_byte((address ^ directxor) & m_addrmask)]);
	return m_space.read_dword(address);
}


//-------------------------------------------------
//  read_qword - read a qword via the
//  direct_read_data class
//-------------------------------------------------

template<int AddrShift> inline u64 direct_read_data<AddrShift>::read_qword(offs_t address, offs_t directxor)
{
	if (address_is_valid(address))
		return *reinterpret_cast<u64 *>(&m_ptr[offset_to_byte((address ^ directxor) & m_addrmask)]);
	return m_space.read_qword(address);
}

