// ASSUMING 4 KB PAGE SIZE
#define PAGE_SIZE 4096

// ASSUMING VIRTUAL MEMORY SIZE 4GB
#define VIRTUAL_MEM_SIZE (1ULL << 32)

// ASSUMING THE PHYSICAL MEMORY SIZE 4GB
#define PHYSICAL_MEM_SIZE (1ULL << 32)

// ASSUMING THE MULTI LEVEL PAGE SIZE OF 4KB
#define PAGE_TABLE_LEVEL_1_SIZE (1ULL << 8)
#define PAGE_TABLE_LEVEL_2_SIZE (1ULL << 8)
#define PAGE_TABLE_LEVEL_3_SIZE (1ULL << 8)
#define PAGE_TABLE_OFFSET_BITS 12

const unsigned long long TEXT_SECTION_START = 0x10000000;
const unsigned long long DATA_SECTION_START = 0x20000000;
const unsigned long long STACK_SECTION_START = 0x30000000;
const unsigned long long SHARED_LIB_SECTION_START = 0x40000000;
const unsigned long long HEAP_SECTION_START = 0x50000000;
