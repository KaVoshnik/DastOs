#ifndef ELF_H
#define ELF_H

#include "types.h"

// ELF Identification
#define EI_MAG0         0
#define EI_MAG1         1
#define EI_MAG2         2
#define EI_MAG3         3
#define EI_CLASS        4
#define EI_DATA         5
#define EI_VERSION      6
#define EI_PAD          7
#define EI_NIDENT       16

// Magic bytes
#define ELFMAG0         0x7f
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'

// Class types
#define ELFCLASSNONE    0
#define ELFCLASS32      1
#define ELFCLASS64      2

// Data encoding
#define ELFDATANONE     0
#define ELFDATA2LSB     1    // Little endian
#define ELFDATA2MSB     2    // Big endian

// ELF version
#define EV_NONE         0
#define EV_CURRENT      1

// Object file types
#define ET_NONE         0    // No file type
#define ET_REL          1    // Relocatable file
#define ET_EXEC         2    // Executable file
#define ET_DYN          3    // Shared object file
#define ET_CORE         4    // Core file

// Machine types
#define EM_NONE         0    // No machine
#define EM_386          3    // Intel 80386
#define EM_X86_64       62   // AMD x86-64

// Section types
#define SHT_NULL        0    // Section header table entry unused
#define SHT_PROGBITS    1    // Program data
#define SHT_SYMTAB      2    // Symbol table
#define SHT_STRTAB      3    // String table
#define SHT_RELA        4    // Relocation entries with addends
#define SHT_HASH        5    // Symbol hash table
#define SHT_DYNAMIC     6    // Dynamic linking information
#define SHT_NOTE        7    // Notes
#define SHT_NOBITS      8    // Program space with no data (bss)
#define SHT_REL         9    // Relocation entries, no addends

// Section flags
#define SHF_WRITE       0x1  // Writable
#define SHF_ALLOC       0x2  // Occupies memory during execution
#define SHF_EXECINSTR   0x4  // Executable

// Program header types
#define PT_NULL         0    // Program header table entry unused
#define PT_LOAD         1    // Loadable program segment
#define PT_DYNAMIC      2    // Dynamic linking information
#define PT_INTERP       3    // Program interpreter
#define PT_NOTE         4    // Auxiliary information
#define PT_SHLIB        5    // Reserved
#define PT_PHDR         6    // Entry for header table itself

// Program header flags
#define PF_X            0x1  // Execute
#define PF_W            0x2  // Write
#define PF_R            0x4  // Read

// Symbol binding
#define STB_LOCAL       0    // Local symbols
#define STB_GLOBAL      1    // Global symbols
#define STB_WEAK        2    // Weak symbols

// Symbol types
#define STT_NOTYPE      0    // Symbol type is unspecified
#define STT_OBJECT      1    // Symbol is a data object
#define STT_FUNC        2    // Symbol is a code object
#define STT_SECTION     3    // Symbol associated with a section
#define STT_FILE        4    // Symbol's name is file name

// ELF header structure (32-bit)
typedef struct {
    uint8_t  e_ident[EI_NIDENT]; // Magic number and other info
    uint16_t e_type;              // Object file type
    uint16_t e_machine;           // Architecture
    uint32_t e_version;           // Object file version
    uint32_t e_entry;             // Entry point virtual address
    uint32_t e_phoff;             // Program header table file offset
    uint32_t e_shoff;             // Section header table file offset
    uint32_t e_flags;             // Processor-specific flags
    uint16_t e_ehsize;            // ELF header size in bytes
    uint16_t e_phentsize;         // Program header table entry size
    uint16_t e_phnum;             // Program header table entry count
    uint16_t e_shentsize;         // Section header table entry size
    uint16_t e_shnum;             // Section header table entry count
    uint16_t e_shstrndx;          // Section header string table index
} elf_header_t;

// Program header structure (32-bit)
typedef struct {
    uint32_t p_type;              // Segment type
    uint32_t p_offset;            // Segment file offset
    uint32_t p_vaddr;             // Segment virtual address
    uint32_t p_paddr;             // Segment physical address
    uint32_t p_filesz;            // Segment size in file
    uint32_t p_memsz;             // Segment size in memory
    uint32_t p_flags;             // Segment flags
    uint32_t p_align;             // Segment alignment
} elf_program_header_t;

// Section header structure (32-bit)
typedef struct {
    uint32_t sh_name;             // Section name (string tbl index)
    uint32_t sh_type;             // Section type
    uint32_t sh_flags;            // Section flags
    uint32_t sh_addr;             // Section virtual addr at execution
    uint32_t sh_offset;           // Section file offset
    uint32_t sh_size;             // Section size in bytes
    uint32_t sh_link;             // Link to another section
    uint32_t sh_info;             // Additional section information
    uint32_t sh_addralign;        // Section alignment
    uint32_t sh_entsize;          // Entry size if section holds table
} elf_section_header_t;

// Symbol table entry structure (32-bit)
typedef struct {
    uint32_t st_name;             // Symbol name (string table index)
    uint32_t st_value;            // Symbol value
    uint32_t st_size;             // Symbol size
    uint8_t  st_info;             // Symbol type and binding
    uint8_t  st_other;            // Symbol visibility
    uint16_t st_shndx;            // Section index
} elf_symbol_t;

// Relocation entry with addend (32-bit)
typedef struct {
    uint32_t r_offset;            // Location at which to apply the action
    uint32_t r_info;              // Symbol table index and type of relocation
    int32_t  r_addend;            // Addend used to compute value
} elf_rela_t;

// Relocation entry without addend (32-bit)
typedef struct {
    uint32_t r_offset;            // Location at which to apply the action
    uint32_t r_info;              // Symbol table index and type of relocation
} elf_rel_t;

// Dynamic section entry (32-bit)
typedef struct {
    int32_t  d_tag;               // Dynamic entry type
    union {
        uint32_t d_val;           // Integer value
        uint32_t d_ptr;           // Address value
    } d_un;
} elf_dynamic_t;

// ELF loader context
typedef struct {
    uint8_t* data;                // ELF file data in memory
    uint32_t size;                // Size of ELF file
    elf_header_t* header;         // Pointer to ELF header
    elf_program_header_t* pheaders; // Pointer to program headers
    elf_section_header_t* sheaders; // Pointer to section headers
    char* string_table;           // Pointer to string table
    uint32_t load_base;           // Base address where ELF is loaded
    uint32_t entry_point;         // Entry point address
    int valid;                    // Whether ELF file is valid
} elf_loader_t;

// Function declarations
int elf_validate(uint8_t* data, uint32_t size);
int elf_parse(elf_loader_t* loader, uint8_t* data, uint32_t size);
uint32_t elf_load_program(elf_loader_t* loader);
int elf_get_entry_point(elf_loader_t* loader, uint32_t* entry);
void elf_cleanup(elf_loader_t* loader);
char* elf_get_string(elf_loader_t* loader, uint32_t offset);
elf_section_header_t* elf_get_section_by_name(elf_loader_t* loader, const char* name);

// Helper macros
#define ELF32_ST_BIND(i)    ((i) >> 4)
#define ELF32_ST_TYPE(i)    ((i) & 0xf)
#define ELF32_ST_INFO(b,t)  (((b) << 4) + ((t) & 0xf))

#define ELF32_R_SYM(i)      ((i) >> 8)
#define ELF32_R_TYPE(i)     ((unsigned char)(i))
#define ELF32_R_INFO(s,t)   (((s) << 8) + (unsigned char)(t))

#endif // ELF_H