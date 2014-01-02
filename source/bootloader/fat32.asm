struct FATDIRECTORYENTRY
    name                rb 11   ; 8.3 format
    attrib              db ?    ; Attributes
    reserved            db ?    ; Reserved for WinNT
    creationTimeLo      db ?    ; Creation time
    creationTimeHi      dw ?    ; Creation time
    creationDate        dw ?    ; Creation date
    accessDate          dw ?    ; Access date
    firstClusterHi      dw ?    ; First cluster
    lastModTime         dw ?    ; Last modification time
    lastModDate         dw ?    ; Last modification date
    firstClusterLo      dw ?    ; First cluster
    size                dd ?    ; File size
ends

DOS83_MAX_LEN           = 11
DIRECTORYBUFF_SIZE      = 4 * 1024
FAT_BAD                 = 0x0FFFFFF7
FAT_EOF                 = 0x0FFFFFF8
FAT_FREE                = 0x00000000

FatInitialize:
    push bp
    mov bp, sp
    push r0
    push r1

    ; setup stuff
    mov r0, byte [bpbSectorsPerCluster]
    mul r0, word [bpbBytesPerSector]
    mov [bytesPerCluster], r0

    mov r0, byte [bpbSectorsPerCluster]
    mul r0, word [bpbBytesPerSector]
    div r0, sizeof.FATDIRECTORYENTRY
    mov [entriesPerCluster], r0

    mov r0, word [bpbReservedSectors]
    mov r1, byte [bpbNumberOfFATs]
    mul r1, dword [bpbSectorsPerFAT32]
    add r0, r1
    sub r0, 2
    mov [clusterOffset], r0

    mov r0, DIRECTORYBUFF_SIZE
    div r0, [bytesPerCluster]
    mov [maxDirectoryClusters], r0

    mov r0, word [bpbBytesPerSector]
    div r0, 4
    mov [fatPerSector], r0

    mov [cachedFatSector], -1

    ; load root dir
    invoke FatReadDirectoryImpl, [bpbRootCluster]

.return:
    pop r1
    pop r0
    pop bp
    ret

; void FatLoad(FATDIRECTORYENTRY* entry, byte* dest)
FatLoad:
    push bp
    mov bp, sp
    push r1
    push r2
    push r3

    mov r1, [bp + 8]
    mov r2, [bp + 12]
    mov r3, word [r1 + FATDIRECTORYENTRY.firstClusterHi]
    shl r3, 16
    or  r3, word [r1 + FATDIRECTORYENTRY.firstClusterLo]

.read:
    invoke FatReadCluster, r3, r2
    add r2, [bytesPerCluster]

    invoke FatReadFat, r3
    mov r3, r0

    cmp r3, FAT_EOF
    jb .read

.return:
    pop r3
    pop r2
    pop r1
    pop bp
    retn 8

; FATDIRECTORYENTRY* FatFindEntry(byte* name)
FatFindEntry:
    push bp
    mov bp, sp
    push r1
    push r2

    mov r1, directoryBuff
    mov r2, [entriesPerCluster]

.search:
    cmp r2, r2
    jz .notFound
    
    int 0
    invoke strncmp, [bp + 8], r1, DOS83_MAX_LEN
    cmp r0, r0
    jz .found

    add r1, sizeof.FATDIRECTORYENTRY
    dec r2
    jmp .search

.notFound:
    xor r0, r0
    jmp .return

.found:
    mov r0, r1

.return:
    pop r2
    pop r1
    pop bp
    retn 4

; void FatReadDirectory(FATDIRECTORYENTRY* entry)
FatReadDirectory:
    push bp
    mov bp, sp
    push r1
    push r2

    mov r1, [bp + 8]
    mov r2, word [r1 + FATDIRECTORYENTRY.firstClusterHi]
    shl r2, 16
    or  r2, word [r1 + FATDIRECTORYENTRY.firstClusterLo]
    invoke FatReadDirectoryImpl, r2

.return:
    pop r2
    pop r1
    pop bp
    retn 4

; void FatReadDirectoryImpl(int startCluster)
FatReadDirectoryImpl:
    push bp
    mov bp, sp
    push r1
    push r2
    push r3

    mov r1, [maxDirectoryClusters]
    mov r2, directoryBuff
    mov r3, [bp + 8]

.read:
    cmp r1, r1
    jz .return
    dec r1

    invoke FatReadCluster, r3, r2
    add r2, [bytesPerCluster]

    invoke FatReadFat, r3
    mov r3, r0

    cmp r3, FAT_EOF
    jb .read

.return:
    pop r3
    pop r2
    pop r1
    pop bp
    retn 4

; int FatReadFat(int index)
FatReadFat:
    push bp
    mov bp, sp
    push r1

    mov r1, [bp + 8]
    mul r1, 4
    div r1, word [bpbBytesPerSector]
    add r1, word [bpbReservedSectors]

    cmp r1, [cachedFatSector]
    je .done
    invoke DiskReadSector, r1, cachedFat
    mov [cachedFatSector], r1

.done:
    mov r0, [bp + 8]
    rem r0, [fatPerSector]
    mul r0, 4
    add r0, cachedFat
    mov r0, [r0]

.return:
    pop r1
    pop bp
    retn 4

; void FatReadCluster(int cluster, void* dest)
FatReadCluster:
    push bp
    mov bp, sp
    push r0
    push r1
    push r2

    mov r0, [bp + 8]  ; cluster
    mul r0, byte [bpbSectorsPerCluster]
    add r0, [clusterOffset]

    mov r1, [bp + 12] ; dest
    mov r2, byte [bpbSectorsPerCluster]

.read:
    cmp r2, r2
    jz .return

    invoke DiskReadSector, r0, r1
    inc r0
    add r1, word [bpbBytesPerSector]

    dec r2
    jmp .read

.return:
    pop r2
    pop r1
    pop r0
    pop bp
    retn 8

; void DiskReadSector(int sector, void* dest)
DiskReadSector:
    push bp
    mov bp, sp
    push r0
    push r1
    push r2

    mov r0, 1
    mov r1, [bp + 12]
    mov r2, [bp + 8]
    int byte [bootDrive]

.return:
    pop r2
    pop r1
    pop r0
    pop bp
    retn 8

; int strncmp(byte* str1, byte* str2, int len)
strncmp:
    push bp
    mov bp, sp
    push r1
    push r2
    push r3

    mov r1, [bp + 8]  ; str1
    mov r2, [bp + 12] ; str2
    mov r3, [bp + 16] ; len

@@:
    cmp byte [r1], byte [r1]
    jz .done
    cmp byte [r1], byte [r2]
    jne .done
    dec r3
    jz .done
    inc r1
    inc r2
    jmp @b

.done:
    mov r0, byte [r1]
    sub r0, byte [r2]

.return:
    pop r3
    pop r2
    pop r1
    pop bp
    retn 12
