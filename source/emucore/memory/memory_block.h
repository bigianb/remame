#pragma once

#include <vector>

#include "../../core/memcore.h"
#include "../../core/macros.h"

class address_space;
class running_machine;

/** A chunk of RAM associated with a range of memory in a device's address space. */
class memory_block
{
	DISABLE_COPYING(memory_block);

public:
	// construction/destruction
	memory_block(address_space &space, offs_t start, offs_t end, void *memory = nullptr);
	~memory_block();

	// getters
	running_machine &machine() const { return m_machine; }
	offs_t addrstart() const { return m_addrstart; }
	offs_t addrend() const { return m_addrend; }
	std::uint8_t *data() const { return m_data; }

	// is the given range contained by this memory block?
	bool contains(address_space &space, offs_t addrstart, offs_t addrend) const
	{
		return (&space == &m_space && m_addrstart <= addrstart && m_addrend >= addrend);
	}

private:
	// internal state
	running_machine &       m_machine;              // need the machine to free our memory
	address_space &         m_space;                // which address space are we associated with?
	offs_t                  m_addrstart, m_addrend; // start/end for verifying a match
	std::uint8_t *          m_data;                 // pointer to the data for this block
	std::vector<std::uint8_t>  m_allocated;         // pointer to the actually allocated block
};

