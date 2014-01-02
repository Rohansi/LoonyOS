
-- Copy bootloader
LoadDriver("RAW.dll");
if Mount("bin/disk.img") then
    -- copy boot sector
    RawWrite("bin/boot.bin", 0, 0, 6);
    RawWrite("bin/boot.bin", 90, 90, 422);

    -- copy bootloader
    RawWrite("bin/boot.bin", 512, 16 * 512, 4 * 512);
else
    return
end
Dismount();


-- Copy files
LoadDriver("FAT32.dll");
if Mount("bin/disk.img") then

    DirCreate("loony")
    ChDir("/loony/")
    
    CopyIn("bin/kernel.bin", "kernel.bin")

else
    return
end
Dismount();
