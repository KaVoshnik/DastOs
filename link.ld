ENTRY(start)
SECTIONS
{
    . = 0x1000;         /* Начальный адрес ядра */
    .text : {
        *(.text)
        _etext = .;
    } :text = 0x90      /* Права RX */
    .data : {
        *(.data)
        _edata = .;
    } :data = 0x90      /* Права RW */
    .bss : {
        *(.bss)
        _end = .;
    } :bss = 0x90       /* Права RW */
}