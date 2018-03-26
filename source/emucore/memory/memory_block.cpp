#include "memory_block.h"
#include "address_space.h"

memory_block::memory_block(address_space &space, offs_t addrstart, offs_t addrend, void *memory)
	: m_machine(space.m_manager.machine()),
	m_space(space),
	m_addrstart(addrstart),
	m_addrend(addrend),
	m_data(reinterpret_cast<std::uint8_t *>(memory))
{
	offs_t const length = space.address_to_byte(addrend + 1 - addrstart);
//	VPRINTF(("block_allocate('%s',%s,%08X,%08X,%p)\n", space.device().tag(), space.name(), addrstart, addrend, memory));

	// allocate a block if needed
	if (m_data == nullptr)
	{
		if (length < 4096)
		{
			m_allocated.resize(length);
			memset(&m_allocated[0], 0, length);
			m_data = &m_allocated[0];
		}
		else
		{
			m_allocated.resize(length + 0xfff);
			memset(&m_allocated[0], 0, length + 0xfff);
			m_data = reinterpret_cast<std::uint8_t *>((reinterpret_cast<uintptr_t>(&m_allocated[0]) + 0xfff) & ~0xfff);
		}
	}

	// register for saving, but only if we're not part of a memory region
	// TODO: This is too tightly coupled.
	if (space.m_manager.region_containing(m_data, length) != nullptr) {
//		VPRINTF(("skipping save of this memory block as it is covered by a memory region\n"));
	}
	else
	{
		int bytes_per_element = space.data_width() / 8;
		std::string name = string_format("%08x-%08x", addrstart, addrend);
		machine().save().save_memory(&space.device(), "memory", space.device().tag(), space.spacenum(), name.c_str(), m_data, bytes_per_element, (u32)length / bytes_per_element);
	}
}

memory_block::~memory_block()
{
}

