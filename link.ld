ENTRY(start)

SECTIONS
{
    . = 0x1000;

    .text : ALIGN(4096) {
        *(.text)
        *(.rodata)
        *(.init)
        *(.fini)
    } > .text

    .data : ALIGN(4096) {
        *(.data)
        *(.got)
        *(.got.plt)
    } > .data

    .bss : ALIGN(4096) {
        *(.bss)
        *(.sbss)
        *(COMMON)
    } > .bss

    /* Символы */
    _etext = .;
    _edata = .;
    _end = .;
}