ENTRY(start)
SECTIONS
{
    . = 0x1000;
    .text : {
        *(.text)
        _etext = .;
    } :text = 0x90
    .data : {
        *(.data)
        _edata = .;
    } :data = 0x90
    .bss : {
        *(.bss)
        _end = .;
    } :bss = 0x90
}