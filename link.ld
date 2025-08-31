ENTRY(start)

PHDRS {
    text PT_LOAD FLAGS(5); /* R-X */
    data PT_LOAD FLAGS(6); /* RW- */
}

SECTIONS {
    . = 0x1000;

    .text : {
        *(.text)
        *(.rodata*)
        *(.init)
        *(.fini)
        *(.gnu.linkonce.t.*)
    } :text

    .data : {
        *(.data)
        *(.got)
        *(.got.plt)
    } :data

    .bss : {
        *(.bss)
        *(.sbss)
        *(COMMON)
    } :data

    /DISCARD/ : {
        *(.note.GNU-stack)
    }

    _etext = .;
    _edata = .;
    _end = .;
}