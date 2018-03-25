
#include "memory_manager.h"
#include "dimemory.h"
#include "address_space_specific.h"

class address_space_config;

memory_manager::memory_manager(running_machine &machine)
	: m_machine(machine),
	m_initialized(false),
	m_banknext(STATIC_BANK1)
{
	memset(m_bank_ptr, 0, sizeof(m_bank_ptr));
}

//-------------------------------------------------
//  allocate - allocate memory spaces
//-------------------------------------------------

void memory_manager::allocate(device_memory_interface &memory)
{
	for (int spacenum = 0; spacenum < memory.max_space_count(); ++spacenum)
	{
		// if there is a configuration for this space, we need an address space
		address_space_config const *const spaceconfig = memory.space_config(spacenum);
		if (spaceconfig)
		{
			// allocate one of the appropriate type
			bool const large(spaceconfig->addr2byte_end(0xffffffffUL >> (32 - spaceconfig->m_addr_width)) >= (1 << 18));

			switch (spaceconfig->data_width())
			{
			case 8:
				if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
				{
					if (large)
						memory.allocate<address_space_8_8le_large>(*this, spacenum);
					else
						memory.allocate<address_space_8_8le_small>(*this, spacenum);
				}
				else
				{
					if (large)
						memory.allocate<address_space_8_8be_large>(*this, spacenum);
					else
						memory.allocate<address_space_8_8be_small>(*this, spacenum);
				}
				break;

			case 16:
				switch (spaceconfig->addr_shift())
				{
				case  3:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_16_1le_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_1le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_16_1be_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_1be_small>(*this, spacenum);
					}
					break;

				case  0:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_16_8le_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_8le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_16_8be_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_8be_small>(*this, spacenum);
					}
					break;

				case -1:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_16_16le_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_16le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_16_16be_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_16be_small>(*this, spacenum);
					}
					break;
				}
				break;

			case 32:
				switch (spaceconfig->addr_shift())
				{
				case  0:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_32_8le_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_8le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_32_8be_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_8be_small>(*this, spacenum);
					}
					break;

				case -1:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_32_16le_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_16le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_32_16be_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_16be_small>(*this, spacenum);
					}
					break;

				case -2:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_32_32le_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_32le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_32_32be_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_32be_small>(*this, spacenum);
					}
					break;
				}
				break;

			case 64:
				switch (spaceconfig->addr_shift())
				{
				case  0:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_64_8le_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_8le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_64_8be_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_8be_small>(*this, spacenum);
					}
					break;

				case -1:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_64_16le_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_16le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_64_16be_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_16be_small>(*this, spacenum);
					}
					break;

				case -2:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_64_32le_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_32le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_64_32be_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_32be_small>(*this, spacenum);
					}
					break;

				case -3:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_64_64le_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_64le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_64_64be_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_64be_small>(*this, spacenum);
					}
					break;
				}
				break;

			default:
				throw emu_fatalerror("Invalid width %d specified for address_space::allocate", spaceconfig->data_width());
			}
		}
	}
}

//-------------------------------------------------
//  initialize - initialize the memory system
//-------------------------------------------------

void memory_manager::initialize()
{
	// loop over devices and spaces within each device
	memory_interface_iterator iter(machine().root_device());
	std::vector<device_memory_interface *> memories;
	for (device_memory_interface &memory : iter)
	{
		memories.push_back(&memory);
		allocate(memory);
	}

	allocate(m_machine.m_dummy_space);

	// construct and preprocess the address_map for each space
	for (auto const memory : memories)
		memory->prepare_maps();

	// create the handlers from the resulting address maps
	for (auto const memory : memories)
		memory->populate_from_maps();

	// allocate memory needed to back each address space
	for (auto const memory : memories)
		memory->allocate_memory();

	// find all the allocated pointers
	for (auto const memory : memories)
		memory->locate_memory();

	// disable logging of unmapped access when no one receives it
	if (!machine().options().log() && !machine().options().oslog() && !(machine().debug_flags & DEBUG_FLAG_ENABLED))
		for (auto const memory : memories)
			memory->set_log_unmap(false);

	// register a callback to reset banks when reloading state
	machine().save().register_postload(save_prepost_delegate(FUNC(memory_manager::bank_reattach), this));

	// dump the final memory configuration
	generate_memdump(machine());

	// we are now initialized
	m_initialized = true;
}


//-------------------------------------------------
//  region_alloc - allocates memory for a region
//-------------------------------------------------

memory_region *memory_manager::region_alloc(const char *name, u32 length, u8 width, endianness_t endian)
{
	osd_printf_verbose("Region '%s' created\n", name);
	// make sure we don't have a region of the same name; also find the end of the list
	if (m_regionlist.find(name) != m_regionlist.end())
		fatalerror("region_alloc called with duplicate region name \"%s\"\n", name);

	// allocate the region
	m_regionlist.emplace(name, std::make_unique<memory_region>(machine(), name, length, width, endian));
	return m_regionlist.find(name)->second.get();
}


//-------------------------------------------------
//  region_free - releases memory for a region
//-------------------------------------------------

void memory_manager::region_free(const char *name)
{
	m_regionlist.erase(name);
}


//-------------------------------------------------
//  region_containing - helper to determine if
//  a block of memory is part of a region
//-------------------------------------------------

memory_region *memory_manager::region_containing(const void *memory, offs_t bytes) const
{
	const u8 *data = reinterpret_cast<const u8 *>(memory);

	// look through the region list and return the first match
	for (auto &region : m_regionlist)
		if (data >= region.second->base() && (data + bytes) < region.second->end())
			return region.second.get();

	// didn't find one
	return nullptr;
}
