# BKEmu Qt Linux

Qt port of BKEmu BK-0010 Family emulator ([http://gid.pdp-11.ru/](http://gid.pdp-11.ru/ "http://gid.pdp-11.ru/")) with many improvements in Debugger.
All models of BK-0010(11) implemented in the original version are working. Some additional features not affecting the main Emulator functionality like Configuration editor, Palitre editor, etc. are not implemented yet. Work still in progress...

Because the main goal was to make the emulator working in Linux, it supports OpenGL screen view only and use OpenAL for audio output. 
All development done in Qt 5.15.2

[![Main Screen](MainScreen.png  "Main Screen")](MainScreen.png  "Main Screen")

## Emulator improvements for Debugging

- Plain and Conditional breakpoints. Each breakpoint can be enabled/disabled. Memory Access breakpoints are implemented but not fully integrated yet. Conditional breakpoints use Lua as backend and some predefined variale and functions:
   - predefined variables **R0-R5**, **SP**, **PS**, **PWS** for accessing RON values
   - **mem(addr)** for accessing uint16 value at the addr
   - **memb(addr)** for accessing uint8 value at the addr
   - Symbol name or **var("symbol name")** to get value of the symbol. To get value from the memory address pointed by symbol use **mem(symbol)**
   - Bit operations are natively supported by Lua engine
- Save/Load breakpoints
- Time machine. You can step back/forward from any instruction with restoring correct state of RONs/Memory. For now Step Back/Forward does not update periferal's registers (Timer, FDD controller) correctly. So after stepping back it is recommended to do 'step forward' instead of executing next instruction.  Use F9/Shift-F9 for Step Back/Forward.
- Symbol table. Emulator can load ether .STB file generated by pclink11 ([https://github.com/andpp/pclink11](https://github.com/andpp/pclink11 "https://github.com/andpp/pclink11")) or .LST file generated by BKTurbo8 (http://gid.pdp-11.ru/). Also symbols can be added directly in Debug view.
- Save/Load symbol table. Symbol table stored in extended .STB format supporintg long symbol names.

## Assembler development tools

Macro11 assembler ([https://github.com/andpp/macro11](https://github.com/andpp/macro11 "https://github.com/andpp/macro11")) and pclink11 linker ([https://github.com/andpp/pclink11](https://github.com/andpp/pclink11 "https://github.com/andpp/pclink11")) were updated for supporting long symbol names. Also all string data definitions in assembled file optioanly could be stored to .obj as BK KOI-8 characters.

Linker can generate BK-0010 .bin files with Load address and Program length as the first two words. Work in progress for creating a command line port of BKDE ([http://gid.pdp-11.ru/](http://gid.pdp-11.ru/ "http://gid.pdp-11.ru/")) for copying images to/from BK .IMG files and intergare it to bkemu-qt. The idea is to create a simple build/debug environment.

Also there is a [port to Linux](https://github.com/andpp/BKTurbo8 "https://github.com/andpp/BKTurbo8") for BKTurbo8 assembler/linker from ([http://gid.pdp-11.ru/](http://gid.pdp-11.ru/ "http://gid.pdp-11.ru/")). Because the main goal of the port was just make it working, it supports only files in UTF-8 encoding. Other encodings supported by the original BKTurbo8 (UTF-16, KOI-8) possibly will be added later.

