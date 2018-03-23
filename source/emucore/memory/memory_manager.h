#pragma once

#include <cstdint>
#include <unordered_map>

#include "../../core/endian.h"
#include "mem_defs.h"
#include "memory_block.h"

class running_machine;
class memory_region;
class memory_bank;
class memory_share;
class device_memory_interface;

/** holds internal state for the memory system. */
class memory_manager
{
	friend class address_space;
	//friend memory_region::memory_region(running_machine &machine, const char *name, u32 length, u8 width, endianness_t endian);
public:
	// construction/destruction
	memory_manager(running_machine &machine);
	void initialize();

	// getters
	running_machine &machine() const { return m_machine; }
	const std::unordered_map<std::string, std::unique_ptr<memory_bank>> &banks() const { return m_banklist; }
	const std::unordered_map<std::string, std::unique_ptr<memory_region>> &regions() const { return m_regionlist; }
	const std::unordered_map<std::string, std::unique_ptr<memory_share>> &shares() const { return m_sharelist; }

	// pointers to a bank pointer (internal usage only)
	std::uint8_t **bank_pointer_addr(std::uint8_t index) { return &m_bank_ptr[index]; }

	// regions
	memory_region *region_alloc(const char *name, std::uint32_t length, std::uint8_t width, endianness_t endian);
	void region_free(const char *name);
	memory_region *region_containing(const void *memory, offs_t bytes) const;

private:
	// internal helpers
	void bank_reattach();
	void allocate(device_memory_interface &memory);

	// internal state
	running_machine &           m_machine;              // reference to the machine
	bool                        m_initialized;          // have we completed initialization?

	std::uint8_t *                        m_bank_ptr[TOTAL_MEMORY_BANKS];  // array of bank pointers

	std::vector<std::unique_ptr<memory_block>>   m_blocklist;            // head of the list of memory blocks

	std::unordered_map<std::string,std::unique_ptr<memory_bank>>    m_banklist;             // data gathered for each bank
	std::uint16_t                      m_banknext;             // next bank to allocate

	std::unordered_map<std::string, std::unique_ptr<memory_share>>   m_sharelist;            // map for share lookups

	std::unordered_map<std::string, std::unique_ptr<memory_region>>  m_regionlist;           // list of memory regions
};
