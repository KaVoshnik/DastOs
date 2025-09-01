ENTRY(_start)

SECTIONS
{
    .multiboot : {
        *(.multiboot)
    } = 0x0

    /* Kernel code starts at 1MB */
    . = 1M;
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}