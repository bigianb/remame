#pragma once

#include "address_space.h"

/** this is a derived class of address_space with specific width, endianness, and table size. */
template<typename NativeType, endianness_t Endian, int AddrShift, bool Large>
class address_space_specific : public address_space
{
	typedef address_space_specific<NativeType, Endian, AddrShift, Large> this_type;

	// constants describing the native size
	static constexpr u32 NATIVE_BYTES = sizeof(NativeType);
	static constexpr u32 NATIVE_STEP = AddrShift >= 0 ? NATIVE_BYTES << iabs(AddrShift) : NATIVE_BYTES >> iabs(AddrShift);
	static constexpr u32 NATIVE_MASK = NATIVE_STEP - 1;
	static constexpr u32 NATIVE_BITS = 8 * NATIVE_BYTES;

	// helpers to simplify core code
	u32 read_lookup(offs_t address) const { return Large ? m_read.lookup_live_large(address) : m_read.lookup_live_small(address); }
	u32 write_lookup(offs_t address) const { return Large ? m_write.lookup_live_large(address) : m_write.lookup_live_small(address); }
	u32 setoffset_lookup(offs_t address) const { return Large ? m_setoffset.lookup_live_large(address) : m_setoffset.lookup_live_small(address); }

	static constexpr offs_t offset_to_byte(offs_t offset) { return AddrShift < 0 ? offset << iabs(AddrShift) : offset >> iabs(AddrShift); }

public:
	// construction/destruction
	address_space_specific(memory_manager &manager, device_memory_interface &memory, int spacenum)
		: address_space(manager, memory, spacenum, Large),
		m_read(*this, Large),
		m_write(*this, Large),
		m_setoffset(*this, Large)
	{
#if (TEST_HANDLER)
		// test code to verify the read/write handlers are touching the correct bits
		// and returning the correct results

		// install some dummy RAM for the first 16 bytes with well-known values
		u8 buffer[16];
		for (int index = 0; index < 16; index++)
			buffer[index ^ ((Endian == ENDIANNESS_NATIVE) ? 0 : (data_width() / 8 - 1))] = index * 0x11;
		install_ram_generic(0x00, 0x0f, 0x0f, 0, read_or_write::READWRITE, buffer);
		printf("\n\naddress_space(%d, %s, %s)\n", NATIVE_BITS, (Endian == ENDIANNESS_LITTLE) ? "little" : "big", Large ? "large" : "small");

		// walk through the first 8 addresses
		for (int address = 0; address < 8; address++)
		{
			// determine expected values
			u64 expected64 = (u64((address + ((Endian == ENDIANNESS_LITTLE) ? 7 : 0)) * 0x11) << 56) |
				(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 6 : 1)) * 0x11) << 48) |
				(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 5 : 2)) * 0x11) << 40) |
				(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 4 : 3)) * 0x11) << 32) |
				(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 3 : 4)) * 0x11) << 24) |
				(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 2 : 5)) * 0x11) << 16) |
				(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 1 : 6)) * 0x11) << 8) |
				(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 0 : 7)) * 0x11) << 0);
			u32 expected32 = (Endian == ENDIANNESS_LITTLE) ? expected64 : (expected64 >> 32);
			u16 expected16 = (Endian == ENDIANNESS_LITTLE) ? expected32 : (expected32 >> 16);
			u8 expected8 = (Endian == ENDIANNESS_LITTLE) ? expected16 : (expected16 >> 8);

			u64 result64;
			u32 result32;
			u16 result16;
			u8 result8;

			// validate byte accesses
			printf("\nAddress %d\n", address);
			printf("   read_byte = "); printf("%02X\n", result8 = read_byte(address)); assert(result8 == expected8);

			// validate word accesses (if aligned)
			if (WORD_ALIGNED(address)) { printf("   read_word = "); printf("%04X\n", result16 = read_word(address)); assert(result16 == expected16); }
			if (WORD_ALIGNED(address)) { printf("   read_word (0xff00) = "); printf("%04X\n", result16 = read_word(address, 0xff00)); assert((result16 & 0xff00) == (expected16 & 0xff00)); }
			if (WORD_ALIGNED(address)) { printf("             (0x00ff) = "); printf("%04X\n", result16 = read_word(address, 0x00ff)); assert((result16 & 0x00ff) == (expected16 & 0x00ff)); }

			// validate unaligned word accesses
			printf("   read_word_unaligned = "); printf("%04X\n", result16 = read_word_unaligned(address)); assert(result16 == expected16);
			printf("   read_word_unaligned (0xff00) = "); printf("%04X\n", result16 = read_word_unaligned(address, 0xff00)); assert((result16 & 0xff00) == (expected16 & 0xff00));
			printf("                       (0x00ff) = "); printf("%04X\n", result16 = read_word_unaligned(address, 0x00ff)); assert((result16 & 0x00ff) == (expected16 & 0x00ff));

			// validate dword acceses (if aligned)
			if (DWORD_ALIGNED(address)) { printf("   read_dword = "); printf("%08X\n", result32 = read_dword(address)); assert(result32 == expected32); }
			if (DWORD_ALIGNED(address)) { printf("   read_dword (0xff000000) = "); printf("%08X\n", result32 = read_dword(address, 0xff000000)); assert((result32 & 0xff000000) == (expected32 & 0xff000000)); }
			if (DWORD_ALIGNED(address)) { printf("              (0x00ff0000) = "); printf("%08X\n", result32 = read_dword(address, 0x00ff0000)); assert((result32 & 0x00ff0000) == (expected32 & 0x00ff0000)); }
			if (DWORD_ALIGNED(address)) { printf("              (0x0000ff00) = "); printf("%08X\n", result32 = read_dword(address, 0x0000ff00)); assert((result32 & 0x0000ff00) == (expected32 & 0x0000ff00)); }
			if (DWORD_ALIGNED(address)) { printf("              (0x000000ff) = "); printf("%08X\n", result32 = read_dword(address, 0x000000ff)); assert((result32 & 0x000000ff) == (expected32 & 0x000000ff)); }
			if (DWORD_ALIGNED(address)) { printf("              (0xffff0000) = "); printf("%08X\n", result32 = read_dword(address, 0xffff0000)); assert((result32 & 0xffff0000) == (expected32 & 0xffff0000)); }
			if (DWORD_ALIGNED(address)) { printf("              (0x0000ffff) = "); printf("%08X\n", result32 = read_dword(address, 0x0000ffff)); assert((result32 & 0x0000ffff) == (expected32 & 0x0000ffff)); }
			if (DWORD_ALIGNED(address)) { printf("              (0xffffff00) = "); printf("%08X\n", result32 = read_dword(address, 0xffffff00)); assert((result32 & 0xffffff00) == (expected32 & 0xffffff00)); }
			if (DWORD_ALIGNED(address)) { printf("              (0x00ffffff) = "); printf("%08X\n", result32 = read_dword(address, 0x00ffffff)); assert((result32 & 0x00ffffff) == (expected32 & 0x00ffffff)); }

			// validate unaligned dword accesses
			printf("   read_dword_unaligned = "); printf("%08X\n", result32 = read_dword_unaligned(address)); assert(result32 == expected32);
			printf("   read_dword_unaligned (0xff000000) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0xff000000)); assert((result32 & 0xff000000) == (expected32 & 0xff000000));
			printf("                        (0x00ff0000) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0x00ff0000)); assert((result32 & 0x00ff0000) == (expected32 & 0x00ff0000));
			printf("                        (0x0000ff00) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0x0000ff00)); assert((result32 & 0x0000ff00) == (expected32 & 0x0000ff00));
			printf("                        (0x000000ff) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0x000000ff)); assert((result32 & 0x000000ff) == (expected32 & 0x000000ff));
			printf("                        (0xffff0000) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0xffff0000)); assert((result32 & 0xffff0000) == (expected32 & 0xffff0000));
			printf("                        (0x0000ffff) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0x0000ffff)); assert((result32 & 0x0000ffff) == (expected32 & 0x0000ffff));
			printf("                        (0xffffff00) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0xffffff00)); assert((result32 & 0xffffff00) == (expected32 & 0xffffff00));
			printf("                        (0x00ffffff) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0x00ffffff)); assert((result32 & 0x00ffffff) == (expected32 & 0x00ffffff));

			// validate qword acceses (if aligned)
			if (QWORD_ALIGNED(address)) { printf("   read_qword = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address), 16)); assert(result64 == expected64); }
			if (QWORD_ALIGNED(address)) { printf("   read_qword (0xff00000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xff00000000000000U), 16)); assert((result64 & 0xff00000000000000U) == (expected64 & 0xff00000000000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00ff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00ff000000000000U), 16)); assert((result64 & 0x00ff000000000000U) == (expected64 & 0x00ff000000000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ff0000000000U), 16)); assert((result64 & 0x0000ff0000000000U) == (expected64 & 0x0000ff0000000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000ff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000ff00000000U), 16)); assert((result64 & 0x000000ff00000000U) == (expected64 & 0x000000ff00000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00000000ff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00000000ff000000U), 16)); assert((result64 & 0x00000000ff000000U) == (expected64 & 0x00000000ff000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000000000ff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000000000ff0000U), 16)); assert((result64 & 0x0000000000ff0000U) == (expected64 & 0x0000000000ff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000000000ff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000000000ff00U), 16)); assert((result64 & 0x000000000000ff00U) == (expected64 & 0x000000000000ff00U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00000000000000ff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00000000000000ffU), 16)); assert((result64 & 0x00000000000000ffU) == (expected64 & 0x00000000000000ffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffff000000000000U), 16)); assert((result64 & 0xffff000000000000U) == (expected64 & 0xffff000000000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ffff00000000U), 16)); assert((result64 & 0x0000ffff00000000U) == (expected64 & 0x0000ffff00000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00000000ffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00000000ffff0000U), 16)); assert((result64 & 0x00000000ffff0000U) == (expected64 & 0x00000000ffff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000000000ffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000000000ffffU), 16)); assert((result64 & 0x000000000000ffffU) == (expected64 & 0x000000000000ffffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffffff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffffff0000000000U), 16)); assert((result64 & 0xffffff0000000000U) == (expected64 & 0xffffff0000000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ffffff000000U), 16)); assert((result64 & 0x0000ffffff000000U) == (expected64 & 0x0000ffffff000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000ffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000ffffff0000U), 16)); assert((result64 & 0x000000ffffff0000U) == (expected64 & 0x000000ffffff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000000000ffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000000000ffffffU), 16)); assert((result64 & 0x0000000000ffffffU) == (expected64 & 0x0000000000ffffffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffffffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffffffff00000000U), 16)); assert((result64 & 0xffffffff00000000U) == (expected64 & 0xffffffff00000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00ffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00ffffffff000000U), 16)); assert((result64 & 0x00ffffffff000000U) == (expected64 & 0x00ffffffff000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ffffffff0000U), 16)); assert((result64 & 0x0000ffffffff0000U) == (expected64 & 0x0000ffffffff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000ffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000ffffffff00U), 16)); assert((result64 & 0x000000ffffffff00U) == (expected64 & 0x000000ffffffff00U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00000000ffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00000000ffffffffU), 16)); assert((result64 & 0x00000000ffffffffU) == (expected64 & 0x00000000ffffffffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffffffffff000000U), 16)); assert((result64 & 0xffffffffff000000U) == (expected64 & 0xffffffffff000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00ffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00ffffffffff0000U), 16)); assert((result64 & 0x00ffffffffff0000U) == (expected64 & 0x00ffffffffff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ffffffffff00U), 16)); assert((result64 & 0x0000ffffffffff00U) == (expected64 & 0x0000ffffffffff00U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000ffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000ffffffffffU), 16)); assert((result64 & 0x000000ffffffffffU) == (expected64 & 0x000000ffffffffffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffffffffffff0000U), 16)); assert((result64 & 0xffffffffffff0000U) == (expected64 & 0xffffffffffff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00ffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00ffffffffffff00U), 16)); assert((result64 & 0x00ffffffffffff00U) == (expected64 & 0x00ffffffffffff00U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ffffffffffffU), 16)); assert((result64 & 0x0000ffffffffffffU) == (expected64 & 0x0000ffffffffffffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffffffffffffff00U), 16)); assert((result64 & 0xffffffffffffff00U) == (expected64 & 0xffffffffffffff00U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00ffffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00ffffffffffffffU), 16)); assert((result64 & 0x00ffffffffffffffU) == (expected64 & 0x00ffffffffffffffU)); }

			// validate unaligned qword accesses
			printf("   read_qword_unaligned = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address), 16)); assert(result64 == expected64);
			printf("   read_qword_unaligned (0xff00000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xff00000000000000U), 16)); assert((result64 & 0xff00000000000000U) == (expected64 & 0xff00000000000000U));
			printf("                        (0x00ff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00ff000000000000U), 16)); assert((result64 & 0x00ff000000000000U) == (expected64 & 0x00ff000000000000U));
			printf("                        (0x0000ff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ff0000000000U), 16)); assert((result64 & 0x0000ff0000000000U) == (expected64 & 0x0000ff0000000000U));
			printf("                        (0x000000ff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000ff00000000U), 16)); assert((result64 & 0x000000ff00000000U) == (expected64 & 0x000000ff00000000U));
			printf("                        (0x00000000ff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00000000ff000000U), 16)); assert((result64 & 0x00000000ff000000U) == (expected64 & 0x00000000ff000000U));
			printf("                        (0x0000000000ff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000000000ff0000U), 16)); assert((result64 & 0x0000000000ff0000U) == (expected64 & 0x0000000000ff0000U));
			printf("                        (0x000000000000ff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000000000ff00U), 16)); assert((result64 & 0x000000000000ff00U) == (expected64 & 0x000000000000ff00U));
			printf("                        (0x00000000000000ff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00000000000000ffU), 16)); assert((result64 & 0x00000000000000ffU) == (expected64 & 0x00000000000000ffU));
			printf("                        (0xffff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffff000000000000U), 16)); assert((result64 & 0xffff000000000000U) == (expected64 & 0xffff000000000000U));
			printf("                        (0x0000ffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ffff00000000U), 16)); assert((result64 & 0x0000ffff00000000U) == (expected64 & 0x0000ffff00000000U));
			printf("                        (0x00000000ffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00000000ffff0000U), 16)); assert((result64 & 0x00000000ffff0000U) == (expected64 & 0x00000000ffff0000U));
			printf("                        (0x000000000000ffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000000000ffffU), 16)); assert((result64 & 0x000000000000ffffU) == (expected64 & 0x000000000000ffffU));
			printf("                        (0xffffff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffffff0000000000U), 16)); assert((result64 & 0xffffff0000000000U) == (expected64 & 0xffffff0000000000U));
			printf("                        (0x0000ffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ffffff000000U), 16)); assert((result64 & 0x0000ffffff000000U) == (expected64 & 0x0000ffffff000000U));
			printf("                        (0x000000ffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000ffffff0000U), 16)); assert((result64 & 0x000000ffffff0000U) == (expected64 & 0x000000ffffff0000U));
			printf("                        (0x0000000000ffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000000000ffffffU), 16)); assert((result64 & 0x0000000000ffffffU) == (expected64 & 0x0000000000ffffffU));
			printf("                        (0xffffffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffffffff00000000U), 16)); assert((result64 & 0xffffffff00000000U) == (expected64 & 0xffffffff00000000U));
			printf("                        (0x00ffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00ffffffff000000U), 16)); assert((result64 & 0x00ffffffff000000U) == (expected64 & 0x00ffffffff000000U));
			printf("                        (0x0000ffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ffffffff0000U), 16)); assert((result64 & 0x0000ffffffff0000U) == (expected64 & 0x0000ffffffff0000U));
			printf("                        (0x000000ffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000ffffffff00U), 16)); assert((result64 & 0x000000ffffffff00U) == (expected64 & 0x000000ffffffff00U));
			printf("                        (0x00000000ffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00000000ffffffffU), 16)); assert((result64 & 0x00000000ffffffffU) == (expected64 & 0x00000000ffffffffU));
			printf("                        (0xffffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffffffffff000000U), 16)); assert((result64 & 0xffffffffff000000U) == (expected64 & 0xffffffffff000000U));
			printf("                        (0x00ffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00ffffffffff0000U), 16)); assert((result64 & 0x00ffffffffff0000U) == (expected64 & 0x00ffffffffff0000U));
			printf("                        (0x0000ffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ffffffffff00U), 16)); assert((result64 & 0x0000ffffffffff00U) == (expected64 & 0x0000ffffffffff00U));
			printf("                        (0x000000ffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000ffffffffffU), 16)); assert((result64 & 0x000000ffffffffffU) == (expected64 & 0x000000ffffffffffU));
			printf("                        (0xffffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffffffffffff0000U), 16)); assert((result64 & 0xffffffffffff0000U) == (expected64 & 0xffffffffffff0000U));
			printf("                        (0x00ffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00ffffffffffff00U), 16)); assert((result64 & 0x00ffffffffffff00U) == (expected64 & 0x00ffffffffffff00U));
			printf("                        (0x0000ffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ffffffffffffU), 16)); assert((result64 & 0x0000ffffffffffffU) == (expected64 & 0x0000ffffffffffffU));
			printf("                        (0xffffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffffffffffffff00U), 16)); assert((result64 & 0xffffffffffffff00U) == (expected64 & 0xffffffffffffff00U));
			printf("                        (0x00ffffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00ffffffffffffffU), 16)); assert((result64 & 0x00ffffffffffffffU) == (expected64 & 0x00ffffffffffffffU));
		}
#endif
	}

	// accessors
	virtual address_table_read &read() override { return m_read; }
	virtual address_table_write &write() override { return m_write; }
	virtual address_table_setoffset &setoffset() override { return m_setoffset; }

	// watchpoint control
	virtual void enable_read_watchpoints(bool enable = true) override { m_read.enable_watchpoints(enable); }
	virtual void enable_write_watchpoints(bool enable = true) override { m_write.enable_watchpoints(enable); }

	// generate accessor table
	virtual void accessors(data_accessors &accessors) const override
	{
		accessors.read_byte = reinterpret_cast<u8(*)(address_space &, offs_t)>(&read_byte_static);
		accessors.read_word = reinterpret_cast<u16(*)(address_space &, offs_t)>(&read_word_static);
		accessors.read_word_masked = reinterpret_cast<u16(*)(address_space &, offs_t, u16)>(&read_word_masked_static);
		accessors.read_dword = reinterpret_cast<u32(*)(address_space &, offs_t)>(&read_dword_static);
		accessors.read_dword_masked = reinterpret_cast<u32(*)(address_space &, offs_t, u32)>(&read_dword_masked_static);
		accessors.read_qword = reinterpret_cast<u64(*)(address_space &, offs_t)>(&read_qword_static);
		accessors.read_qword_masked = reinterpret_cast<u64(*)(address_space &, offs_t, u64)>(&read_qword_masked_static);
		accessors.write_byte = reinterpret_cast<void(*)(address_space &, offs_t, u8)>(&write_byte_static);
		accessors.write_word = reinterpret_cast<void(*)(address_space &, offs_t, u16)>(&write_word_static);
		accessors.write_word_masked = reinterpret_cast<void(*)(address_space &, offs_t, u16, u16)>(&write_word_masked_static);
		accessors.write_dword = reinterpret_cast<void(*)(address_space &, offs_t, u32)>(&write_dword_static);
		accessors.write_dword_masked = reinterpret_cast<void(*)(address_space &, offs_t, u32, u32)>(&write_dword_masked_static);
		accessors.write_qword = reinterpret_cast<void(*)(address_space &, offs_t, u64)>(&write_qword_static);
		accessors.write_qword_masked = reinterpret_cast<void(*)(address_space &, offs_t, u64, u64)>(&write_qword_masked_static);
	}

	// return a pointer to the read bank, or nullptr if none
	virtual void *get_read_ptr(offs_t address) override
	{
		// perform the lookup
		address &= m_addrmask;
		u32 entry = read_lookup(address);
		const handler_entry_read &handler = m_read.handler_read(entry);

		// 8-bit case: RAM/ROM
		if (entry > STATIC_BANKMAX)
			return nullptr;
		return handler.ramptr(handler.offset(address));
	}

	// return a pointer to the write bank, or nullptr if none
	virtual void *get_write_ptr(offs_t address) override
	{
		// perform the lookup
		address &= m_addrmask;
		u32 entry = write_lookup(address);
		const handler_entry_write &handler = m_write.handler_write(entry);

		// 8-bit case: RAM/ROM
		if (entry > STATIC_BANKMAX)
			return nullptr;
		return handler.ramptr(handler.offset(address));
	}

	// native read
	NativeType read_native(offs_t offset, NativeType mask)
	{
		g_profiler.start(PROFILER_MEMREAD);

		if (TEST_HANDLER) printf("[r%X,%s]", offset, core_i64_hex_format(mask, sizeof(NativeType) * 2));

		// look up the handler
		offs_t address = offset & m_addrmask;
		u32 entry = read_lookup(address);
		const handler_entry_read &handler = m_read.handler_read(entry);

		// either read directly from RAM, or call the delegate
		offset = offset_to_byte(handler.offset(address));
		NativeType result;
		if (entry <= STATIC_BANKMAX) result = *reinterpret_cast<NativeType *>(handler.ramptr(offset));
		else if (sizeof(NativeType) == 1) result = handler.read8(*this, offset, mask);
		else if (sizeof(NativeType) == 2) result = handler.read16(*this, offset >> 1, mask);
		else if (sizeof(NativeType) == 4) result = handler.read32(*this, offset >> 2, mask);
		else if (sizeof(NativeType) == 8) result = handler.read64(*this, offset >> 3, mask);

		g_profiler.stop();
		return result;
	}

	// mask-less native read
	NativeType read_native(offs_t offset)
	{
		g_profiler.start(PROFILER_MEMREAD);

		if (TEST_HANDLER) printf("[r%X]", offset);

		// look up the handler
		offs_t address = offset & m_addrmask;
		u32 entry = read_lookup(address);
		const handler_entry_read &handler = m_read.handler_read(entry);

		// either read directly from RAM, or call the delegate
		offset = offset_to_byte(handler.offset(address));
		NativeType result;
		if (entry <= STATIC_BANKMAX) result = *reinterpret_cast<NativeType *>(handler.ramptr(offset));
		else if (sizeof(NativeType) == 1) result = handler.read8(*this, offset, 0xff);
		else if (sizeof(NativeType) == 2) result = handler.read16(*this, offset >> 1, 0xffff);
		else if (sizeof(NativeType) == 4) result = handler.read32(*this, offset >> 2, 0xffffffff);
		else if (sizeof(NativeType) == 8) result = handler.read64(*this, offset >> 3, 0xffffffffffffffffU);

		g_profiler.stop();
		return result;
	}

	// native write
	void write_native(offs_t offset, NativeType data, NativeType mask)
	{
		g_profiler.start(PROFILER_MEMWRITE);

		// look up the handler
		offs_t address = offset & m_addrmask;
		u32 entry = write_lookup(address);
		const handler_entry_write &handler = m_write.handler_write(entry);

		// either write directly to RAM, or call the delegate
		offset = offset_to_byte(handler.offset(address));
		if (entry <= STATIC_BANKMAX)
		{
			NativeType *dest = reinterpret_cast<NativeType *>(handler.ramptr(offset));
			*dest = (*dest & ~mask) | (data & mask);
		}
		else if (sizeof(NativeType) == 1) handler.write8(*this, offset, data, mask);
		else if (sizeof(NativeType) == 2) handler.write16(*this, offset >> 1, data, mask);
		else if (sizeof(NativeType) == 4) handler.write32(*this, offset >> 2, data, mask);
		else if (sizeof(NativeType) == 8) handler.write64(*this, offset >> 3, data, mask);

		g_profiler.stop();
	}

	// mask-less native write
	void write_native(offs_t offset, NativeType data)
	{
		g_profiler.start(PROFILER_MEMWRITE);

		// look up the handler
		offs_t address = offset & m_addrmask;
		u32 entry = write_lookup(address);
		const handler_entry_write &handler = m_write.handler_write(entry);

		// either write directly to RAM, or call the delegate
		offset = offset_to_byte(handler.offset(address));
		if (entry <= STATIC_BANKMAX) *reinterpret_cast<NativeType *>(handler.ramptr(offset)) = data;
		else if (sizeof(NativeType) == 1) handler.write8(*this, offset, data, 0xff);
		else if (sizeof(NativeType) == 2) handler.write16(*this, offset >> 1, data, 0xffff);
		else if (sizeof(NativeType) == 4) handler.write32(*this, offset >> 2, data, 0xffffffff);
		else if (sizeof(NativeType) == 8) handler.write64(*this, offset >> 3, data, 0xffffffffffffffffU);

		g_profiler.stop();
	}

	// generic direct read
	template<typename TargetType, bool Aligned>
	TargetType read_direct(offs_t address, TargetType mask)
	{
		const u32 TARGET_BYTES = sizeof(TargetType);
		const u32 TARGET_BITS = 8 * TARGET_BYTES;

		// equal to native size and aligned; simple pass-through to the native reader
		if (NATIVE_BYTES == TARGET_BYTES && (Aligned || (address & NATIVE_MASK) == 0))
			return read_native(address & ~NATIVE_MASK, mask);

		// if native size is larger, see if we can do a single masked read (guaranteed if we're aligned)
		if (NATIVE_BYTES > TARGET_BYTES)
		{
			u32 offsbits = 8 * (offset_to_byte(address) & (NATIVE_BYTES - (Aligned ? TARGET_BYTES : 1)));
			if (Aligned || (offsbits + TARGET_BITS <= NATIVE_BITS))
			{
				if (Endian != ENDIANNESS_LITTLE) offsbits = NATIVE_BITS - TARGET_BITS - offsbits;
				return read_native(address & ~NATIVE_MASK, (NativeType)mask << offsbits) >> offsbits;
			}
		}

		// determine our alignment against the native boundaries, and mask the address
		u32 offsbits = 8 * (offset_to_byte(address) & (NATIVE_BYTES - 1));
		address &= ~NATIVE_MASK;

		// if we're here, and native size is larger or equal to the target, we need exactly 2 reads
		if (NATIVE_BYTES >= TARGET_BYTES)
		{
			// little-endian case
			if (Endian == ENDIANNESS_LITTLE)
			{
				// read lower bits from lower address
				TargetType result = 0;
				NativeType curmask = (NativeType)mask << offsbits;
				if (curmask != 0) result = read_native(address, curmask) >> offsbits;

				// read upper bits from upper address
				offsbits = NATIVE_BITS - offsbits;
				curmask = mask >> offsbits;
				if (curmask != 0) result |= read_native(address + NATIVE_STEP, curmask) << offsbits;
				return result;
			}

			// big-endian case
			else
			{
				// left-justify the mask to the target type
				const u32 LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT = ((NATIVE_BITS >= TARGET_BITS) ? (NATIVE_BITS - TARGET_BITS) : 0);
				NativeType result = 0;
				NativeType ljmask = (NativeType)mask << LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;
				NativeType curmask = ljmask >> offsbits;

				// read upper bits from lower address
				if (curmask != 0) result = read_native(address, curmask) << offsbits;
				offsbits = NATIVE_BITS - offsbits;

				// read lower bits from upper address
				curmask = ljmask << offsbits;
				if (curmask != 0) result |= read_native(address + NATIVE_STEP, curmask) >> offsbits;

				// return the un-justified result
				return result >> LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;
			}
		}

		// if we're here, then we have 2 or more reads needed to get our final result
		else
		{
			// compute the maximum number of loops; we do it this way so that there are
			// a fixed number of loops for the compiler to unroll if it desires
			const u32 MAX_SPLITS_MINUS_ONE = TARGET_BYTES / NATIVE_BYTES - 1;
			TargetType result = 0;

			// little-endian case
			if (Endian == ENDIANNESS_LITTLE)
			{
				// read lowest bits from first address
				NativeType curmask = mask << offsbits;
				if (curmask != 0) result = read_native(address, curmask) >> offsbits;

				// read middle bits from subsequent addresses
				offsbits = NATIVE_BITS - offsbits;
				for (u32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
				{
					address += NATIVE_STEP;
					curmask = mask >> offsbits;
					if (curmask != 0) result |= (TargetType)read_native(address, curmask) << offsbits;
					offsbits += NATIVE_BITS;
				}

				// if we're not aligned and we still have bits left, read uppermost bits from last address
				if (!Aligned && offsbits < TARGET_BITS)
				{
					curmask = mask >> offsbits;
					if (curmask != 0) result |= (TargetType)read_native(address + NATIVE_STEP, curmask) << offsbits;
				}
			}

			// big-endian case
			else
			{
				// read highest bits from first address
				offsbits = TARGET_BITS - (NATIVE_BITS - offsbits);
				NativeType curmask = mask >> offsbits;
				if (curmask != 0) result = (TargetType)read_native(address, curmask) << offsbits;

				// read middle bits from subsequent addresses
				for (u32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
				{
					offsbits -= NATIVE_BITS;
					address += NATIVE_STEP;
					curmask = mask >> offsbits;
					if (curmask != 0) result |= (TargetType)read_native(address, curmask) << offsbits;
				}

				// if we're not aligned and we still have bits left, read lowermost bits from the last address
				if (!Aligned && offsbits != 0)
				{
					offsbits = NATIVE_BITS - offsbits;
					curmask = mask << offsbits;
					if (curmask != 0) result |= read_native(address + NATIVE_STEP, curmask) >> offsbits;
				}
			}
			return result;
		}
	}

	// generic direct write
	template<typename TargetType, bool Aligned>
	void write_direct(offs_t address, TargetType data, TargetType mask)
	{
		const u32 TARGET_BYTES = sizeof(TargetType);
		const u32 TARGET_BITS = 8 * TARGET_BYTES;

		// equal to native size and aligned; simple pass-through to the native writer
		if (NATIVE_BYTES == TARGET_BYTES && (Aligned || (address & NATIVE_MASK) == 0))
			return write_native(address & ~NATIVE_MASK, data, mask);

		// if native size is larger, see if we can do a single masked write (guaranteed if we're aligned)
		if (NATIVE_BYTES > TARGET_BYTES)
		{
			u32 offsbits = 8 * (offset_to_byte(address) & (NATIVE_BYTES - (Aligned ? TARGET_BYTES : 1)));
			if (Aligned || (offsbits + TARGET_BITS <= NATIVE_BITS))
			{
				if (Endian != ENDIANNESS_LITTLE) offsbits = NATIVE_BITS - TARGET_BITS - offsbits;
				return write_native(address & ~NATIVE_MASK, (NativeType)data << offsbits, (NativeType)mask << offsbits);
			}
		}

		// determine our alignment against the native boundaries, and mask the address
		u32 offsbits = 8 * (offset_to_byte(address) & (NATIVE_BYTES - 1));
		address &= ~NATIVE_MASK;

		// if we're here, and native size is larger or equal to the target, we need exactly 2 writes
		if (NATIVE_BYTES >= TARGET_BYTES)
		{
			// little-endian case
			if (Endian == ENDIANNESS_LITTLE)
			{
				// write lower bits to lower address
				NativeType curmask = (NativeType)mask << offsbits;
				if (curmask != 0) write_native(address, (NativeType)data << offsbits, curmask);

				// write upper bits to upper address
				offsbits = NATIVE_BITS - offsbits;
				curmask = mask >> offsbits;
				if (curmask != 0) write_native(address + NATIVE_STEP, data >> offsbits, curmask);
			}

			// big-endian case
			else
			{
				// left-justify the mask and data to the target type
				const u32 LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT = ((NATIVE_BITS >= TARGET_BITS) ? (NATIVE_BITS - TARGET_BITS) : 0);
				NativeType ljdata = (NativeType)data << LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;
				NativeType ljmask = (NativeType)mask << LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;

				// write upper bits to lower address
				NativeType curmask = ljmask >> offsbits;
				if (curmask != 0) write_native(address, ljdata >> offsbits, curmask);

				// write lower bits to upper address
				offsbits = NATIVE_BITS - offsbits;
				curmask = ljmask << offsbits;
				if (curmask != 0) write_native(address + NATIVE_STEP, ljdata << offsbits, curmask);
			}
		}

		// if we're here, then we have 2 or more writes needed to get our final result
		else
		{
			// compute the maximum number of loops; we do it this way so that there are
			// a fixed number of loops for the compiler to unroll if it desires
			const u32 MAX_SPLITS_MINUS_ONE = TARGET_BYTES / NATIVE_BYTES - 1;

			// little-endian case
			if (Endian == ENDIANNESS_LITTLE)
			{
				// write lowest bits to first address
				NativeType curmask = mask << offsbits;
				if (curmask != 0) write_native(address, data << offsbits, curmask);

				// write middle bits to subsequent addresses
				offsbits = NATIVE_BITS - offsbits;
				for (u32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
				{
					address += NATIVE_STEP;
					curmask = mask >> offsbits;
					if (curmask != 0) write_native(address, data >> offsbits, curmask);
					offsbits += NATIVE_BITS;
				}

				// if we're not aligned and we still have bits left, write uppermost bits to last address
				if (!Aligned && offsbits < TARGET_BITS)
				{
					curmask = mask >> offsbits;
					if (curmask != 0) write_native(address + NATIVE_STEP, data >> offsbits, curmask);
				}
			}

			// big-endian case
			else
			{
				// write highest bits to first address
				offsbits = TARGET_BITS - (NATIVE_BITS - offsbits);
				NativeType curmask = mask >> offsbits;
				if (curmask != 0) write_native(address, data >> offsbits, curmask);

				// write middle bits to subsequent addresses
				for (u32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
				{
					offsbits -= NATIVE_BITS;
					address += NATIVE_STEP;
					curmask = mask >> offsbits;
					if (curmask != 0) write_native(address, data >> offsbits, curmask);
				}

				// if we're not aligned and we still have bits left, write lowermost bits to the last address
				if (!Aligned && offsbits != 0)
				{
					offsbits = NATIVE_BITS - offsbits;
					curmask = mask << offsbits;
					if (curmask != 0) write_native(address + NATIVE_STEP, data << offsbits, curmask);
				}
			}
		}
	}

	// Allows to announce a pending read or write operation on this address.
	// The user of the address_space calls a set_address operation which leads
	// to some particular set_offset operation for an entry in the address map.
	void set_address(offs_t address) override
	{
		address &= m_addrmask;
		u32 entry = setoffset_lookup(address);
		const handler_entry_setoffset &handler = m_setoffset.handler_setoffset(entry);

		offs_t offset = handler.offset(address);
		handler.setoffset(*this, offset / sizeof(NativeType));
	}

	// virtual access to these functions
	u8 read_byte(offs_t address) override { return (NATIVE_BITS == 8) ? read_native(address & ~NATIVE_MASK) : read_direct<u8, true>(address, 0xff); }
	u16 read_word(offs_t address) override { return (NATIVE_BITS == 16) ? read_native(address & ~NATIVE_MASK) : read_direct<u16, true>(address, 0xffff); }
	u16 read_word(offs_t address, u16 mask) override { return read_direct<u16, true>(address, mask); }
	u16 read_word_unaligned(offs_t address) override { return read_direct<u16, false>(address, 0xffff); }
	u16 read_word_unaligned(offs_t address, u16 mask) override { return read_direct<u16, false>(address, mask); }
	u32 read_dword(offs_t address) override { return (NATIVE_BITS == 32) ? read_native(address & ~NATIVE_MASK) : read_direct<u32, true>(address, 0xffffffff); }
	u32 read_dword(offs_t address, u32 mask) override { return read_direct<u32, true>(address, mask); }
	u32 read_dword_unaligned(offs_t address) override { return read_direct<u32, false>(address, 0xffffffff); }
	u32 read_dword_unaligned(offs_t address, u32 mask) override { return read_direct<u32, false>(address, mask); }
	u64 read_qword(offs_t address) override { return (NATIVE_BITS == 64) ? read_native(address & ~NATIVE_MASK) : read_direct<u64, true>(address, 0xffffffffffffffffU); }
	u64 read_qword(offs_t address, u64 mask) override { return read_direct<u64, true>(address, mask); }
	u64 read_qword_unaligned(offs_t address) override { return read_direct<u64, false>(address, 0xffffffffffffffffU); }
	u64 read_qword_unaligned(offs_t address, u64 mask) override { return read_direct<u64, false>(address, mask); }

	void write_byte(offs_t address, u8 data) override { if (NATIVE_BITS == 8) write_native(address & ~NATIVE_MASK, data); else write_direct<u8, true>(address, data, 0xff); }
	void write_word(offs_t address, u16 data) override { if (NATIVE_BITS == 16) write_native(address & ~NATIVE_MASK, data); else write_direct<u16, true>(address, data, 0xffff); }
	void write_word(offs_t address, u16 data, u16 mask) override { write_direct<u16, true>(address, data, mask); }
	void write_word_unaligned(offs_t address, u16 data) override { write_direct<u16, false>(address, data, 0xffff); }
	void write_word_unaligned(offs_t address, u16 data, u16 mask) override { write_direct<u16, false>(address, data, mask); }
	void write_dword(offs_t address, u32 data) override { if (NATIVE_BITS == 32) write_native(address & ~NATIVE_MASK, data); else write_direct<u32, true>(address, data, 0xffffffff); }
	void write_dword(offs_t address, u32 data, u32 mask) override { write_direct<u32, true>(address, data, mask); }
	void write_dword_unaligned(offs_t address, u32 data) override { write_direct<u32, false>(address, data, 0xffffffff); }
	void write_dword_unaligned(offs_t address, u32 data, u32 mask) override { write_direct<u32, false>(address, data, mask); }
	void write_qword(offs_t address, u64 data) override { if (NATIVE_BITS == 64) write_native(address & ~NATIVE_MASK, data); else write_direct<u64, true>(address, data, 0xffffffffffffffffU); }
	void write_qword(offs_t address, u64 data, u64 mask) override { write_direct<u64, true>(address, data, mask); }
	void write_qword_unaligned(offs_t address, u64 data) override { write_direct<u64, false>(address, data, 0xffffffffffffffffU); }
	void write_qword_unaligned(offs_t address, u64 data, u64 mask) override { write_direct<u64, false>(address, data, mask); }

	// static access to these functions
	static u8 read_byte_static(this_type &space, offs_t address) { return (NATIVE_BITS == 8) ? space.read_native(address & ~NATIVE_MASK) : space.read_direct<u8, true>(address, 0xff); }
	static u16 read_word_static(this_type &space, offs_t address) { return (NATIVE_BITS == 16) ? space.read_native(address & ~NATIVE_MASK) : space.read_direct<u16, true>(address, 0xffff); }
	static u16 read_word_masked_static(this_type &space, offs_t address, u16 mask) { return space.read_direct<u16, true>(address, mask); }
	static u32 read_dword_static(this_type &space, offs_t address) { return (NATIVE_BITS == 32) ? space.read_native(address & ~NATIVE_MASK) : space.read_direct<u32, true>(address, 0xffffffff); }
	static u32 read_dword_masked_static(this_type &space, offs_t address, u32 mask) { return space.read_direct<u32, true>(address, mask); }
	static u64 read_qword_static(this_type &space, offs_t address) { return (NATIVE_BITS == 64) ? space.read_native(address & ~NATIVE_MASK) : space.read_direct<u64, true>(address, 0xffffffffffffffffU); }
	static u64 read_qword_masked_static(this_type &space, offs_t address, u64 mask) { return space.read_direct<u64, true>(address, mask); }
	static void write_byte_static(this_type &space, offs_t address, u8 data) { if (NATIVE_BITS == 8) space.write_native(address & ~NATIVE_MASK, data); else space.write_direct<u8, true>(address, data, 0xff); }
	static void write_word_static(this_type &space, offs_t address, u16 data) { if (NATIVE_BITS == 16) space.write_native(address & ~NATIVE_MASK, data); else space.write_direct<u16, true>(address, data, 0xffff); }
	static void write_word_masked_static(this_type &space, offs_t address, u16 data, u16 mask) { space.write_direct<u16, true>(address, data, mask); }
	static void write_dword_static(this_type &space, offs_t address, u32 data) { if (NATIVE_BITS == 32) space.write_native(address & ~NATIVE_MASK, data); else space.write_direct<u32, true>(address, data, 0xffffffff); }
	static void write_dword_masked_static(this_type &space, offs_t address, u32 data, u32 mask) { space.write_direct<u32, true>(address, data, mask); }
	static void write_qword_static(this_type &space, offs_t address, u64 data) { if (NATIVE_BITS == 64) space.write_native(address & ~NATIVE_MASK, data); else space.write_direct<u64, true>(address, data, 0xffffffffffffffffU); }
	static void write_qword_masked_static(this_type &space, offs_t address, u64 data, u64 mask) { space.write_direct<u64, true>(address, data, mask); }

	address_table_read      m_read;             // memory read lookup table
	address_table_write     m_write;            // memory write lookup table
	address_table_setoffset m_setoffset;        // memory setoffset lookup table
};

typedef address_space_specific<u8, ENDIANNESS_LITTLE, 0, false> address_space_8_8le_small;
typedef address_space_specific<u8, ENDIANNESS_BIG, 0, false> address_space_8_8be_small;
typedef address_space_specific<u16, ENDIANNESS_LITTLE, 3, false> address_space_16_1le_small;
typedef address_space_specific<u16, ENDIANNESS_BIG, 3, false> address_space_16_1be_small;
typedef address_space_specific<u16, ENDIANNESS_LITTLE, 0, false> address_space_16_8le_small;
typedef address_space_specific<u16, ENDIANNESS_BIG, 0, false> address_space_16_8be_small;
typedef address_space_specific<u16, ENDIANNESS_LITTLE, -1, false> address_space_16_16le_small;
typedef address_space_specific<u16, ENDIANNESS_BIG, -1, false> address_space_16_16be_small;
typedef address_space_specific<u32, ENDIANNESS_LITTLE, 0, false> address_space_32_8le_small;
typedef address_space_specific<u32, ENDIANNESS_BIG, 0, false> address_space_32_8be_small;
typedef address_space_specific<u32, ENDIANNESS_LITTLE, -1, false> address_space_32_16le_small;
typedef address_space_specific<u32, ENDIANNESS_BIG, -1, false> address_space_32_16be_small;
typedef address_space_specific<u32, ENDIANNESS_LITTLE, -2, false> address_space_32_32le_small;
typedef address_space_specific<u32, ENDIANNESS_BIG, -2, false> address_space_32_32be_small;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, 0, false> address_space_64_8le_small;
typedef address_space_specific<u64, ENDIANNESS_BIG, 0, false> address_space_64_8be_small;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -1, false> address_space_64_16le_small;
typedef address_space_specific<u64, ENDIANNESS_BIG, -1, false> address_space_64_16be_small;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -2, false> address_space_64_32le_small;
typedef address_space_specific<u64, ENDIANNESS_BIG, -2, false> address_space_64_32be_small;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -3, false> address_space_64_64le_small;
typedef address_space_specific<u64, ENDIANNESS_BIG, -3, false> address_space_64_64be_small;

typedef address_space_specific<u8, ENDIANNESS_LITTLE, 0, true>  address_space_8_8le_large;
typedef address_space_specific<u8, ENDIANNESS_BIG, 0, true>  address_space_8_8be_large;
typedef address_space_specific<u16, ENDIANNESS_LITTLE, 3, true>  address_space_16_1le_large;
typedef address_space_specific<u16, ENDIANNESS_BIG, 3, true>  address_space_16_1be_large;
typedef address_space_specific<u16, ENDIANNESS_LITTLE, 0, true>  address_space_16_8le_large;
typedef address_space_specific<u16, ENDIANNESS_BIG, 0, true>  address_space_16_8be_large;
typedef address_space_specific<u16, ENDIANNESS_LITTLE, -1, true>  address_space_16_16le_large;
typedef address_space_specific<u16, ENDIANNESS_BIG, -1, true>  address_space_16_16be_large;
typedef address_space_specific<u32, ENDIANNESS_LITTLE, 0, true>  address_space_32_8le_large;
typedef address_space_specific<u32, ENDIANNESS_BIG, 0, true>  address_space_32_8be_large;
typedef address_space_specific<u32, ENDIANNESS_LITTLE, -1, true>  address_space_32_16le_large;
typedef address_space_specific<u32, ENDIANNESS_BIG, -1, true>  address_space_32_16be_large;
typedef address_space_specific<u32, ENDIANNESS_LITTLE, -2, true>  address_space_32_32le_large;
typedef address_space_specific<u32, ENDIANNESS_BIG, -2, true>  address_space_32_32be_large;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, 0, true>  address_space_64_8le_large;
typedef address_space_specific<u64, ENDIANNESS_BIG, 0, true>  address_space_64_8be_large;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -1, true>  address_space_64_16le_large;
typedef address_space_specific<u64, ENDIANNESS_BIG, -1, true>  address_space_64_16be_large;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -2, true>  address_space_64_32le_large;
typedef address_space_specific<u64, ENDIANNESS_BIG, -2, true>  address_space_64_32be_large;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -3, true>  address_space_64_64le_large;
typedef address_space_specific<u64, ENDIANNESS_BIG, -3, true>  address_space_64_64be_large;


