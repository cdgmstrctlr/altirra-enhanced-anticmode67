# altirra-enhanced-anticmode67
A modification to the Altirra Atari 8-bit emulator to include support for double-byte ANTIC modes 6 and 7, allowing access to the full character set and per-character 16 color selectable FG+BG.

BUILD: Refer to the Altirra build instructions.

USAGE: Go to System -> Configure System... -> Computer -> System and enable the "Modes 6/7 Double Byte Enhancement".

By default, even when this feature is enabled in the emulator configuration, it is not turned on until the enable bits are set in the ANTIC and GTIA registers CHACTL and GRACTL or their corresponding shadow registers.

CHACTL bit 3: Enable double-byte mode if bit is set to 1. ANTIC reads two bytes per character. Allows all 256 characters to be displayed at the same time (128 + inverse) in these text modes.

CHACTL bit 4: Send GTIA 16 colors instead of 5 if bit 4 is set to 1. (enable virtual AN3-AN4 ANTIC -> GTIA lines.)

GRACTL bit 3: Enable 8 color + luma shift mode - FG and BG use color registers PM0-BAK, colors 8-15 display 6 luminance levels brighter.

GRACTL bit 4: Enable 8 color + chroma shift mode - FG and BG use color registers PM0-BAK, colors 8-15 display 6 chroma values higher.

If both bits 3 and 4 are set, both modes are enabled at the same time.

If you set either bits 3 or 4 in GRACTL but fail to set bit 3 in CHACTL, you will not get the additional colors for the character display.

You can use the individually selectable FG+BG mode in 5 color mode if you don't need the extra colors. It is more compatible with PM graphics than the 16 color mode is since 16 color mode uses the PM graphics color registers as part of the new available colors. The collision detection in 16 color mode is also a somewhat different due to the PM color usage.

The screen device S: is not familiar with GR 1 and 2 containing 40 bytes per line, so use of it will work strangely. Also, the GR mode 0 4-line text box at the bottom will overwrite the bottom of the GR 1 and 2 display due to the upper part of the screen requiring twice as much memory for character data. It is recommended to POKE directly to screen memory when using double-byte GR 1 and 2. If you do plan to use this in BASIC programs, there are ways to repoint the text box at the bottom to another area of memory so programs work as before.
