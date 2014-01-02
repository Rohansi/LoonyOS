include 'loonyvm.inc'

VIDEO_MEMORY = 0x60000

;
; State 1 Bootloader
;

org 0x5000
jmp entryPoint

; BPB
bpbOEMName                      rb  5
bpbBytesPerSector               dw  ?
bpbSectorsPerCluster            db  ?
bpbReservedSectors              dw  ?
bpbNumberOfFATs                 db  ?
bpbRootEntryCount               dw  ?
bpbTotalSectors16               dw  ?
bpbMedia                        db  ?
bpbSectorsPerFAT16              dw  ?
bpbSectorsPerTrack              dw  ?
bpbNumberOfHeads                dw  ?
bpbHiddenSectors                dd  ?
bpbTotalSectors32               dd  ?

; Extended BPB (FAT32)
bpbSectorsPerFAT32              dd  ?
bpbFlags                        dw  ?
bpbVersion                      dw  ?
bpbRootCluster                  dd  ?
bpbFSInfo                       dw  ?
bpbBkBootSector                 dw  ?
bpbReserved1                    rb  12
bpbDriveNumber                  db  ?
bpbReserved2                    db  ?
bpbBootSignature                db  ?
bpbVolumeId                     dd  ?
bpbVolumeLabel                  rb  11
bpbFileSystemType               rb  8

entryPoint:
    mov byte [bootDrive], r9

    mov r0, 3        ; read sectors
    mov r1, 0x6000   ; destination
    mov r2, 16       ; start sector
    mov r3, 4        ; number of sectors
    int r9           ; call hdd

    jmp 0x6000

; r0 - str to print
panic:
    mov r1, VIDEO_MEMORY

.write:
    cmp byte [r0], 0
    je .hang
    mov byte [r1], byte [r0]
    mov byte [r1 + 1], 0x0F
    inc r0
    add r1, 2
    jmp .write

.hang:
    jmp .hang

bootDrive:      db 0

dirLoony:       db 'LOONY      ', 0
filKernel:      db 'KERNEL  BIN', 0

msgNoLoony:     db 'LoonyOS not found', 0
msgNoKernel:    db 'Kernel not found', 0

rb 510 - ($-$$)
dw 0x55AA


;
; Stage 2 Bootloader
;

org 0x6000

bootloader:
    invoke FatInitialize

    invoke FatFindEntry, dirLoony
    cmp r0, r0
    jnz @f
    mov r0, msgNoLoony
    jmp panic
@@:
    
    invoke FatReadDirectory, r0
    invoke FatFindEntry, filKernel
    cmp r0, r0
    jnz @f
    mov r0, msgNoKernel
    jmp panic
@@:
    
    invoke FatLoad, r0, 0x10000
    jmp 0x10000

include 'fat32.asm'

rb 2046 - ($-$$)
dw 0x55AA

bytesPerCluster:        rd 1
entriesPerCluster:      rd 1
clusterOffset:          rd 1
fatPerSector:           rd 1
maxDirectoryClusters:   rd 1
cachedFatSector:        rd 1
cachedFat:              rd 1024
directoryBuff:          rd DIRECTORYBUFF_SIZE
