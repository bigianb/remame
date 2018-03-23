#pragma once

#include "../../core/memcore.h"

enum { TOTAL_MEMORY_BANKS = 512 };

// address space names for common use
constexpr int AS_PROGRAM = 0; // program address space
constexpr int AS_DATA    = 1; // data address space
constexpr int AS_IO      = 2; // I/O address space
constexpr int AS_OPCODES = 3; // (decrypted) opcodes, when separate from data accesses

// read or write constants
enum class read_or_write
{
	READ = 1,
	WRITE = 2,
	READWRITE = 3
};

// banking constants
const int BANK_ENTRY_UNSPECIFIED = -1;

// other address map constants
const int MEMORY_BLOCK_CHUNK = 65536;                   // minimum chunk size of allocated memory blocks

														// static data access handler constants
enum
{
	STATIC_INVALID = 0,                                 // invalid - should never be used
	STATIC_BANK1 = 1,                                   // first memory bank
	STATIC_BANKMAX = 0xfb,                              // last memory bank
	STATIC_NOP,                                         // NOP - reads = unmapped value; writes = no-op
	STATIC_UNMAP,                                       // unmapped - same as NOP except we log errors
	STATIC_WATCHPOINT,                                  // watchpoint - used internally
	STATIC_COUNT                                        // total number of static handlers
};
