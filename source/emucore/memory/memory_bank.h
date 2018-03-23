#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../../core/memcore.h"
#include "../../core/macros.h"
#include "mem_defs.h"

class address_space;

/** a memory bank is a global pointer to memory that can be shared across devices and changed dynamically. */
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
		address_space & m_space;            // address space that references us
		read_or_write   m_readorwrite;      // used for read or write?
	};

	// a bank_entry contains a pointer
	struct bank_entry
	{
		std::uint8_t *    m_ptr;
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
	std::uint8_t **         m_baseptr;              // pointer to our base pointer in the global array
	std::uint16_t           m_index;                // array index for this handler
	bool                    m_anonymous;            // are we anonymous or explicit?
	offs_t                  m_addrstart;            // start offset
	offs_t                  m_addrend;              // end offset
	int                     m_curentry;             // current entry
	std::vector<bank_entry> m_entry;                // array of entries (dynamically allocated)
	std::string             m_name;                 // friendly name for this bank
	std::string             m_tag;                  // tag for this bank
	std::vector<std::unique_ptr<bank_reference>> m_reflist;          // linked list of address spaces referencing this bank
};

