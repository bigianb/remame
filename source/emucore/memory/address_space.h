#pragma once

#include "../emumem.h"

// address_space holds live information about an address space
class address_space
{
	friend class address_table;
	friend class address_table_read;
	friend class address_table_write;
	friend class address_table_setoffset;
	friend class direct_read_data<3>;
	friend class direct_read_data<0>;
	friend class direct_read_data<-1>;
	friend class direct_read_data<-2>;
	friend class direct_read_data<-3>;
	friend class memory_bank;
	friend class memory_block;

protected:
	// construction/destruction
	address_space(memory_manager &manager, device_memory_interface &memory, int spacenum, bool large);

public:
	virtual ~address_space();

	// getters
	device_t &device() const { return m_device; }
	const char *name() const { return m_name; }
	int spacenum() const { return m_spacenum; }
	address_map *map() const { return m_map.get(); }

	template<int AddrShift> direct_read_data<AddrShift> *direct() const {
		static_assert(AddrShift == 3 || AddrShift == 0 || AddrShift == -1 || AddrShift == -2 || AddrShift == -3, "Unsupported AddrShift in direct()");
		if(AddrShift != m_config.addr_shift())
			fatalerror("Requesing direct() with address shift %d while the config says %d\n", AddrShift, m_config.addr_shift());
		return static_cast<direct_read_data<AddrShift> *>(m_direct);
	}

	int data_width() const { return m_config.data_width(); }
	int addr_width() const { return m_config.addr_width(); }
	int alignment() const { return m_config.alignment(); }
	endianness_t endianness() const { return m_config.endianness(); }
	int addr_shift() const { return m_config.addr_shift(); }
	u64 unmap() const { return m_unmap; }
	bool is_octal() const { return m_config.m_is_octal; }

	offs_t addrmask() const { return m_addrmask; }
	u8 addrchars() const { return m_addrchars; }
	offs_t logaddrmask() const { return m_logaddrmask; }
	u8 logaddrchars() const { return m_logaddrchars; }

	// debug helpers
	const char *get_handler_string(read_or_write readorwrite, offs_t byteaddress);
	bool log_unmap() const { return m_log_unmap; }
	void set_log_unmap(bool log) { m_log_unmap = log; }
	void dump_map(FILE *file, read_or_write readorwrite);

	// watchpoint enablers
	virtual void enable_read_watchpoints(bool enable = true) = 0;
	virtual void enable_write_watchpoints(bool enable = true) = 0;

	// general accessors
	virtual void accessors(data_accessors &accessors) const = 0;
	virtual void *get_read_ptr(offs_t address) = 0;
	virtual void *get_write_ptr(offs_t address) = 0;

	// read accessors
	virtual u8 read_byte(offs_t address) = 0;
	virtual u16 read_word(offs_t address) = 0;
	virtual u16 read_word(offs_t address, u16 mask) = 0;
	virtual u16 read_word_unaligned(offs_t address) = 0;
	virtual u16 read_word_unaligned(offs_t address, u16 mask) = 0;
	virtual u32 read_dword(offs_t address) = 0;
	virtual u32 read_dword(offs_t address, u32 mask) = 0;
	virtual u32 read_dword_unaligned(offs_t address) = 0;
	virtual u32 read_dword_unaligned(offs_t address, u32 mask) = 0;
	virtual u64 read_qword(offs_t address) = 0;
	virtual u64 read_qword(offs_t address, u64 mask) = 0;
	virtual u64 read_qword_unaligned(offs_t address) = 0;
	virtual u64 read_qword_unaligned(offs_t address, u64 mask) = 0;

	// write accessors
	virtual void write_byte(offs_t address, u8 data) = 0;
	virtual void write_word(offs_t address, u16 data) = 0;
	virtual void write_word(offs_t address, u16 data, u16 mask) = 0;
	virtual void write_word_unaligned(offs_t address, u16 data) = 0;
	virtual void write_word_unaligned(offs_t address, u16 data, u16 mask) = 0;
	virtual void write_dword(offs_t address, u32 data) = 0;
	virtual void write_dword(offs_t address, u32 data, u32 mask) = 0;
	virtual void write_dword_unaligned(offs_t address, u32 data) = 0;
	virtual void write_dword_unaligned(offs_t address, u32 data, u32 mask) = 0;
	virtual void write_qword(offs_t address, u64 data) = 0;
	virtual void write_qword(offs_t address, u64 data, u64 mask) = 0;
	virtual void write_qword_unaligned(offs_t address, u64 data) = 0;
	virtual void write_qword_unaligned(offs_t address, u64 data, u64 mask) = 0;

	// Set address. This will invoke setoffset handlers for the respective entries.
	virtual void set_address(offs_t address) = 0;

	// address-to-byte conversion helpers
	offs_t address_to_byte(offs_t address) const { return m_config.addr2byte(address); }
	offs_t address_to_byte_end(offs_t address) const { return m_config.addr2byte_end(address); }
	offs_t byte_to_address(offs_t address) const { return m_config.byte2addr(address); }
	offs_t byte_to_address_end(offs_t address) const { return m_config.byte2addr_end(address); }

	// umap ranges (short form)
	void unmap_read(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::READ, false); }
	void unmap_write(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::WRITE, false); }
	void unmap_readwrite(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::READWRITE, false); }
	void nop_read(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::READ, true); }
	void nop_write(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::WRITE, true); }
	void nop_readwrite(offs_t addrstart, offs_t addrend, offs_t addrmirror = 0) { unmap_generic(addrstart, addrend, addrmirror, read_or_write::READWRITE, true); }

	// install ports, banks, RAM (short form)
	void install_read_port(offs_t addrstart, offs_t addrend, const char *rtag) { install_read_port(addrstart, addrend, 0, rtag); }
	void install_write_port(offs_t addrstart, offs_t addrend, const char *wtag) { install_write_port(addrstart, addrend, 0, wtag); }
	void install_readwrite_port(offs_t addrstart, offs_t addrend, const char *rtag, const char *wtag) { install_readwrite_port(addrstart, addrend, 0, rtag, wtag); }
	void install_read_bank(offs_t addrstart, offs_t addrend, const char *tag) { install_read_bank(addrstart, addrend, 0, tag); }
	void install_write_bank(offs_t addrstart, offs_t addrend, const char *tag) { install_write_bank(addrstart, addrend, 0, tag); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, const char *tag) { install_readwrite_bank(addrstart, addrend, 0, tag); }
	void install_read_bank(offs_t addrstart, offs_t addrend, memory_bank *bank) { install_read_bank(addrstart, addrend, 0, bank); }
	void install_write_bank(offs_t addrstart, offs_t addrend, memory_bank *bank) { install_write_bank(addrstart, addrend, 0, bank); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, memory_bank *bank) { install_readwrite_bank(addrstart, addrend, 0, bank); }
	void install_rom(offs_t addrstart, offs_t addrend, void *baseptr = nullptr) { install_rom(addrstart, addrend, 0, baseptr); }
	void install_writeonly(offs_t addrstart, offs_t addrend, void *baseptr = nullptr) { install_writeonly(addrstart, addrend, 0, baseptr); }
	void install_ram(offs_t addrstart, offs_t addrend, void *baseptr = nullptr) { install_ram(addrstart, addrend, 0, baseptr); }

	// install ports, banks, RAM (with mirror/mask)
	void install_read_port(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *rtag) { install_readwrite_port(addrstart, addrend, addrmirror, rtag, nullptr); }
	void install_write_port(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *wtag) { install_readwrite_port(addrstart, addrend, addrmirror, nullptr, wtag); }
	void install_readwrite_port(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *rtag, const char *wtag);
	void install_read_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *tag) { install_bank_generic(addrstart, addrend, addrmirror, tag, nullptr); }
	void install_write_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *tag) { install_bank_generic(addrstart, addrend, addrmirror, nullptr, tag); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *tag)  { install_bank_generic(addrstart, addrend, addrmirror, tag, tag); }
	void install_read_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_bank *bank) { install_bank_generic(addrstart, addrend, addrmirror, bank, nullptr); }
	void install_write_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_bank *bank) { install_bank_generic(addrstart, addrend, addrmirror, nullptr, bank); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_bank *bank)  { install_bank_generic(addrstart, addrend, addrmirror, bank, bank); }
	void install_rom(offs_t addrstart, offs_t addrend, offs_t addrmirror, void *baseptr = nullptr) { install_ram_generic(addrstart, addrend, addrmirror, read_or_write::READ, baseptr); }
	void install_writeonly(offs_t addrstart, offs_t addrend, offs_t addrmirror, void *baseptr = nullptr) { install_ram_generic(addrstart, addrend, addrmirror, read_or_write::WRITE, baseptr); }
	void install_ram(offs_t addrstart, offs_t addrend, offs_t addrmirror, void *baseptr = nullptr) { install_ram_generic(addrstart, addrend, addrmirror, read_or_write::READWRITE, baseptr); }

	// install device memory maps
	template <typename T> void install_device(offs_t addrstart, offs_t addrend, T &device, void (T::*map)(address_map &map), u64 unitmask = 0, int cswidth = 0) {
		address_map_constructor delegate(map, "dynamic_device_install", &device);
		install_device_delegate(addrstart, addrend, device, delegate, unitmask, cswidth);
	}

	void install_device_delegate(offs_t addrstart, offs_t addrend, device_t &device, address_map_constructor &map, u64 unitmask = 0, int cswidth = 0);

	// install setoffset handler
	void install_setoffset_handler(offs_t addrstart, offs_t addrend, setoffset_delegate sohandler, u64 unitmask = 0, int cswidth = 0) { install_setoffset_handler(addrstart, addrend, 0, 0, 0, sohandler, unitmask, cswidth); }
	void install_setoffset_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, setoffset_delegate sohandler, u64 unitmask = 0, int cswidth = 0);

	// install new-style delegate handlers (short form)
	void install_read_handler(offs_t addrstart, offs_t addrend, read8_delegate rhandler, u64 unitmask = 0, int cswidth = 0) { install_read_handler(addrstart, addrend, 0, 0, 0, rhandler, unitmask, cswidth); }
	void install_write_handler(offs_t addrstart, offs_t addrend, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0) { install_write_handler(addrstart, addrend, 0, 0, 0, whandler, unitmask, cswidth); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, read8_delegate rhandler, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0) { return install_readwrite_handler(addrstart, addrend, 0, 0, 0, rhandler, whandler, unitmask, cswidth); }
	void install_read_handler(offs_t addrstart, offs_t addrend, read16_delegate rhandler, u64 unitmask = 0, int cswidth = 0) { install_read_handler(addrstart, addrend, 0, 0, 0, rhandler, unitmask, cswidth); }
	void install_write_handler(offs_t addrstart, offs_t addrend, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0) { install_write_handler(addrstart, addrend, 0, 0, 0, whandler, unitmask, cswidth); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, read16_delegate rhandler, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0) { return install_readwrite_handler(addrstart, addrend, 0, 0, 0, rhandler, whandler, unitmask, cswidth); }
	void install_read_handler(offs_t addrstart, offs_t addrend, read32_delegate rhandler, u64 unitmask = 0, int cswidth = 0) { install_read_handler(addrstart, addrend, 0, 0, 0, rhandler, unitmask, cswidth); }
	void install_write_handler(offs_t addrstart, offs_t addrend, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0) { install_write_handler(addrstart, addrend, 0, 0, 0, whandler, unitmask, cswidth); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, read32_delegate rhandler, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0) { return install_readwrite_handler(addrstart, addrend, 0, 0, 0, rhandler, whandler, unitmask, cswidth); }
	void install_read_handler(offs_t addrstart, offs_t addrend, read64_delegate rhandler, u64 unitmask = 0, int cswidth = 0) { install_read_handler(addrstart, addrend, 0, 0, 0, rhandler, unitmask, cswidth); }
	void install_write_handler(offs_t addrstart, offs_t addrend, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0) { install_write_handler(addrstart, addrend, 0, 0, 0, whandler, unitmask, cswidth); }
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, read64_delegate rhandler, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0) { install_readwrite_handler(addrstart, addrend, 0, 0, 0, rhandler, whandler, unitmask, cswidth); }

	// install new-style delegate handlers (with mirror/mask)
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8_delegate rhandler, u64 unitmask = 0, int cswidth = 0);
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8_delegate rhandler, write8_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16_delegate rhandler, u64 unitmask = 0, int cswidth = 0);
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16_delegate rhandler, write16_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32_delegate rhandler, u64 unitmask = 0, int cswidth = 0);
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32_delegate rhandler, write32_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64_delegate rhandler, u64 unitmask = 0, int cswidth = 0);
	void install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0);
	void install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64_delegate rhandler, write64_delegate whandler, u64 unitmask = 0, int cswidth = 0);

	// setup
	void prepare_map();
	void populate_from_map(address_map *map = nullptr);
	void allocate_memory();
	void locate_memory();

	void invalidate_read_caches();
	void invalidate_read_caches(u16 entry);
	void invalidate_read_caches(offs_t start, offs_t end);

private:
	// internal helpers
	virtual address_table_read &read() = 0;
	virtual address_table_write &write() = 0;
	virtual address_table_setoffset &setoffset() = 0;

	void populate_map_entry(const address_map_entry &entry, read_or_write readorwrite);
	void populate_map_entry_setoffset(const address_map_entry &entry);
	void unmap_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite, bool quiet);
	void install_ram_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite, void *baseptr);
	void install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *rtag, const char *wtag);
	void install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_bank *rbank, memory_bank *wbank);
	void adjust_addresses(offs_t &start, offs_t &end, offs_t &mask, offs_t &mirror);
	void *find_backing_memory(offs_t addrstart, offs_t addrend);
	bool needs_backing_store(const address_map_entry &entry);
	memory_bank &bank_find_or_allocate(const char *tag, offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite);
	memory_bank *bank_find_anonymous(offs_t bytestart, offs_t byteend) const;
	address_map_entry *block_assign_intersecting(offs_t bytestart, offs_t byteend, u8 *base);
	void check_optimize_all(const char *function, int width, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror, u64 &nunitmask, int &ncswidth);
	void check_optimize_mirror(const char *function, offs_t addrstart, offs_t addrend, offs_t addrmirror, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror);
	void check_address(const char *function, offs_t addrstart, offs_t addrend);

protected:
	// private state
	const address_space_config &m_config;       // configuration of this space
	device_t &              m_device;           // reference to the owning device
	std::unique_ptr<address_map> m_map;         // original memory map
	offs_t                  m_addrmask;         // physical address mask
	offs_t                  m_logaddrmask;      // logical address mask
	u64                     m_unmap;            // unmapped value
	int                     m_spacenum;         // address space index
	bool                    m_log_unmap;        // log unmapped accesses in this space?
	void *                  m_direct;           // fast direct-access read info
	const char *            m_name;             // friendly name of the address space
	u8                      m_addrchars;        // number of characters to use for physical addresses
	u8                      m_logaddrchars;     // number of characters to use for logical addresses

private:
	memory_manager &        m_manager;          // reference to the owning manager
};
