grub-mkimage -d /usr/lib/grub/loongarch64-efi -c pxeloongarch64.cfg -p /loongarch64 -o BOOTLOONGARCHPXE.EFI -O loongarch64-efi part_gpt part_msdos disk fat exfat ext2 ntfs xfs hfs iso9660 normal search_fs_file search_fs_uuid search_label search configfile linux chain loopback echo efi_gop all_video file gfxmenu gfxterm gfxterm_background gfxterm_menu halt reboot help jpeg ls png true loadenv test tftp http probe
