include 'loonyvm.inc'

DEVICE_START    = 0x02
DEVICE_END      = 0x0E
DISK_ID         = 0xB10C
VIDEO_MEMORY    = 0x60000

BOOT_SIG        = 0x55AA
BOOT_SIG_OFFSET = 510
BOOT_ADDR       = 0x5000
BOOT_SIG_ADDR   = BOOT_ADDR + BOOT_SIG_OFFSET


bios:
    mov r9, DEVICE_START

.search:
    cmp r9, DEVICE_END
    ja noBoot

    ; identify
    mov r0, 100
    int r9
    cmp r0, DISK_ID
    jne .cont

    ; read sector
    mov r0, 1
    mov r1, BOOT_ADDR
    xor r3, r3
    int r9

    cmp word [BOOT_SIG_ADDR], BOOT_SIG
    jne .cont

    jmp BOOT_ADDR

.cont:
    inc r9
    jmp .search


; no bootable device, print message and hang
noBoot:
    mov r0, msgNoBoot
    mov r1, VIDEO_MEMORY

.write:
    cmp byte [r0], byte [r0]
    jz .hang
    mov byte [r1], byte [r0]
    mov byte [r1 + 1], 0x0F
    inc r0
    add r1, 2
    jmp .write

.hang:
    jmp .hang

msgNoBoot: db 'No bootable devices found', 0
