# altirra-enhanced-anticmode67
A modification to the Altirra Atari 8-bit emulator to include support for double-byte ANTIC modes 6 and 7, allowing access to the full character set and per-character 16 color selectable FG+BG.

### BUILD
Refer to the Altirra build instructions.

### USAGE
Go to System -> Configure System... -> Computer -> System and enable the "Modes 6/7 Double Byte Enhancement". This works in any of the system emulation modes (400, 800, XL, XE, etc). It is not implemented under VBXE, however you can still use VBXE without needing to disable the enhanced mode 6/7 feature in the system settings.

By default, even when this feature is enabled in the emulator configuration, it is not turned on until the enable bits are set in the ANTIC and GTIA registers CHACTL and GRACTL or their corresponding shadow registers.

CHACTL bit 3: Enable double-byte mode if bit is set to 1. ANTIC reads two bytes per character. Allows all 256 characters to be displayed at the same time (128 + inverse) in these text modes.

CHACTL bit 4: Send GTIA 16 colors instead of 5 if bit 4 is set to 1. (enable virtual AN3-AN5 ANTIC -> GTIA signal lines.)

GRACTL bit 3: Enable 8 color + luma shift mode - FG and BG use color registers PM0-BAK, using colors 8-14 display 6 luminance levels brighter.

GRACTL bit 4: Enable 8 color + chroma shift mode - FG and BG use color registers PM0-BAK, using colors 8-14 display 6 chroma values higher.

If both bits 3 and 4 are set, both luma and chroma modes are enabled at the same time for colors 8-14.

If you set either bits 3 or 4 in GRACTL but fail to set bit 3 in CHACTL, you will not get the additional colors for the character display.

The layout of the characters in double-byte mode is even offset bytes (low byte) in screen memory select the character glyph, odd bytes (high bytes) are the control bytes. Bits 0-3 select the foreground color for the character, and bits 4-7 select the background color for the character. If enhanced color mode is not enabled, only bits 0-1 and bits 4-5 of the control bytes are used, providing selection of 4 colors plus the background color.

You can use the individually selectable FG+BG mode in 5 color mode described above if you don't need the extra colors. It is more compatible with PM graphics than the 16 color mode is since 16 color mode uses the PM graphics color registers as part of the new available colors. The collision detection in 16 color mode is also a somewhat different due to the PM color usage.

The screen device S: is not familiar with GR 1 and 2 containing 40 bytes per line, so use of it will work strangely. Also, the GR mode 0 4-line text box at the bottom will overwrite the bottom of the GR 1 and 2 display due to the upper part of the screen requiring twice as much memory for character data. It is recommended to POKE directly to screen memory when using double-byte GR 1 and 2. If you do plan to use this in BASIC programs, there are ways to repoint the text box at the bottom to another area of memory so programs work as before.

It should be possible to set CHACTL and GRACTL in a display list interrupt and have both "classic" modes 6/7 and enhanced modes 6/7 stacked vertically on the display at the same time if desired.

Here is an example Atari BASIC program that displays 15 individually colored background squares (character values are spaces) plus the screen background color for a total 16 colors using luma shift for the 7 additional colors. It also displays the color selector value in hex as the foreground character in each square.

```
10 GRAPHICS 18:POKE 755,26:POKE 53277,8
15 FOR I=704 TO 712:READ C:POKE I,C:NEXT I
20 SC=PEEK(88)+256*PEEK(89)
30 FOR I=0 TO 8:READ CC,C:POKE SC+I*2,CC:POKE SC+1+I*2,C*16+(I<>0)*12:NEXT I
40 FOR I=0 TO 8:READ CC,C:POKE SC+40+I*2,CC:POKE SC+41+I*2,C*16+7-(CC=120)*6:NEXT I
100 GOTO 100
1000 DATA 54,98,180,14,146,68,242,226,0
1010 DATA 144,12,17,1,18,2,19,3,20,4,21,5,22,6,23,7,16,0
1020 DATA 24,8,25,9,33,10,34,11,120,0,120,0,35,12,36,13,37,14
```

The data statement at line 1000 specifies the color register values for 704 - 712.\
The data statements at line 1010 and 1020 specify the character and control byte upper nybble for each character to change the background color for that character. The lower nybble of the control byte is set by the loop.

Note the way to select color register PF0 as the background is to use an inverse character or define a custom character with a solid background. The "foreground" color in such cases is selected by the background nybble in the control byte. This is demonstrated above by the first two bytes in the line 1010 DATA statement: 144 is an inverse "0" and the FOR loop leaves the foreground color at 0 for that one column while putting the 12 in the background which causes the inverse "0" to appear using color 12.

Astute readers will notice here is a quirk in that selecting color 15 reselects color register PM3. This is due to there only being 15 color values + a background available in the ANTIC pixel expansion tables. I would like color 15 do something special, perhaps invert the enhanced color selection bits in GRACTL: Luma shift mode becomes chroma shift mode for that character, and vice-versa. But I have to figure out a new way to encode the expansion and merge tables to do it.

Another possible future change is to have color selection values 4 and 5 always use chroma/luma shift mode. Currently, they map directly to color registers PM0 and PM1. It might be beneficial to always use the alternate color values since there are no chroma/luma shift selectors assigned to those registers. This would create a distinction between player-missle colors and the playfield when a character is using the PM0 and PM1 colors.

Here is a table of color selectors in the control byte (values in upper and lower nybble for FG/BG work the same, except for value 0x00) and how they map to the color registers:
```
 0: PF0 or BAK (BAK if char bitmap pixel is clear)
 1: PF1
 2: PF2
 3: PF3
 4: PM0
 5: PM1
 6: PM2
 7: PM3
 8: PF0 with chroma/luma shift.
 9: PF1 with chroma/luma shift.
10: PF2 with chroma/luma shift.
11: PF3 with chroma/luma shift.
12: PM2 with chroma/luma shift.
13: PM3 with chroma/luma shift.
14: BAK with chroma/luma shift.
15: PM3
```

### KNOWN ISSUES
The "illegal" conflicting PRIOR modes are not implemented yet for enhanced color mode, nor is PRIOR 0 fully implemented. In the conflict mode instances PRIOR falls back to the standard 5 color priority mode combinations. For PRIOR 0, PRIOR 1 is used instead. PRIOR values 1, 2, 4 and 8, 16, and 32 are fully implemented, and if using the standard 5 color mode instead of enhanced color, PRIOR works the same as a standard GTIA chip. Combining PRIOR values 16 and 32 (and GTIA enable) works as before since they don't apply to the playfield/PM graphics priorities.

Also there may be an issue with the save/load state of CHACTL and GRACTL, so saving the machine state may reset the new bits in these two registers when starting a machine from a state restore.
