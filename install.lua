
LoadDriver("RAW.dll");
Mount("bin/disk.img")

-- copy boot sector
RawWrite("bin/boot.bin", 0, 0, 6);
RawWrite("bin/boot.bin", 90, 90, 422);

-- copy bootloader
RawWrite("bin/boot.bin", 512, 16 * 512, 4 * 512);

Dismount();


LoadDriver("FAT32.dll");
Mount("bin/disk.img")

CopyOut("test", "out.bin")

DirCreate("loony")
ChDir("/loony/")

CopyIn("bin/kernel.bin", "kernel.bin")

Dismount();

