/* STM32F103C8T6 Memory Map */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
    SRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
    .text :
    {
        *(.vector_table)
        *(.text)
        *(.rodata)
        _etext = .;
    } > FLASH

    .data :
    {
        _sdata = .;
        *(.data)
        _edata = .;
    } > SRAM AT > FLASH

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = .;
    } > SRAM

    . = ALIGN(4);
    end = .;
    _end = .;
}
