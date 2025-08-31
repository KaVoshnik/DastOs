ENTRY(start)

SECTIONS
{
    . = 0x1000;

    .text : {
        *(.text)
        *(.rodata)
        *(.init)
        *(.fini)
        *(.gnu.linkonce.t.*)
    }

    .data : {
        *(.data)
        *(.got)
        *(.got.plt)
    }

    .bss : {
        *(.bss)
        *(.sbss)
        *(COMMON)
    }

    /* Символы */
    _etext = .;
    _edata = .;
    _end = .;
}