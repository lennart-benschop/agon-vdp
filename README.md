# agon-vdp

Part of the official Quark firmware for the Agon series of microcomputers

Modifications by Lennart Benschop to add a UTF-8 font with Latin, Greek,
Cyrillic and more.

New command added
`VDU 23, 26, fontid, size, width, height`
The size, width and height commands are ignored. fontid:
* 0 standard 8x8 BBC font, user-modifiable, one byte to select each character
  between 32 and 126, between 128 and 255.
* 1 standard 8x8 BBC font as above, but use 2 byte UTF-8 encoding to select
  characters above 127.
* 2 Unicode 8x16 font, fixed in flash, one byte to select each character,
  only characters lower than 256 are available (ISO Latin1 set).
* 3 Unicode 8x16 font, fixed in flash, all 1356 characters are available,
  select with 1, 2 or 3 byte UTF-8 sequences. This contains Latin (including
  accented letters for all European languages), Greek, Cyrillic, math symbols,
  box drawing graphics and even Hebrew (but no way to print right-to-left).
* 4 and 5 Unicode 8x16 bold font.
* 6 and 7 Unicode 6x12 font.
* 8 and 9 Fixedsys Excelsior Unicode 8x16 font, different character repertoire.
* 10 and 11 Unscii Unicode 8x16 font, different character repertoire again.

Example: `VDU 23, 26, 3, 0, 0, 0`
The intention is that odd-numbered font IDs select the same font as the
next lower even number, but with UTF-8 encoding enabled. UTF-8 encoding
is currently only implemented in the display driver, not in the keyboard driver.

Restriction: no way to get the character code from the screen using GET(x,y)
function when the 8x16 font is in use.

The program `showfont.bas` displays all printable characters from the
included Unicode font. Additional Basic programs for the other 2 Unicode sets.

The Unicode fonts 2..7 are derived from the "Terminus Font", which is
Copyright (C) 2020 Dimitar Toshkov Zhekov and released under the OFL
(included in this repository). The files `ter_*.bdf` were taken
unmodified from the Terminus Font package and converted to a form
suitable for use in Agon VDP using the program `bdf2c.py`.  The
original package can be found at
https://terminus-font.sourceforge.net/
The other two Unicode fonts are in the public domain according to the
source repository
https://github.com/viznut/unscii

### What is the Agon

Agon is a modern, fully open-source, 8-bit microcomputer and microcontroller in one small, low-cost board. As a computer, it is a standalone device that requires no host PC: it puts out its own video (VGA), audio (2 identical mono channels), accepts a PS/2 keyboard and has its own mass-storage in the form of a µSD card.

https://www.thebyteattic.com/p/agon.html

### What is the VDP

The VDP is a serial graphics terminal that takes a BBC Basic text output stream as input. The output is via the VGA connector on the Agon.

It will process any valid BBC Basic VDU commands (starting with a character between 0 and 31).

For example:

`VDU 25, mode, x; y;` is the same as `PLOT mode, x, y` 

### Documentation

The AGON documentation can now be found on the [Agon Light Documentation Wiki](https://github.com/breakintoprogram/agon-docs/wiki)

### Building

The ESP32 is programmed via the USB connector at the top of the board using the Arduino IDE. It has been tested on version 1.8.19 and the latest 2.0.4.

#### Arduino IDE settings

In order to add the ESP32 as a supported board in the Arudino IDE, you will need to add in this URL into the board manager:

Select Preferences from the File menu

In the Additional Board Manager URLs text box, enter the following URL:

`https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

In the Board Manager (from the Tools menu), select the esp32 board, making sure version 2.0.4 is installed.

Now the board can be selected and configured:

* Board: “ESP32 Dev Module”
* Upload Speed: “921600”
* CPU Frequency: “240Mhz (WiFi/BT)”
* Flash Frequency: “80Mhz”
* Flash Mode: “QIO”
* Flash Size: “4MB (32Mb)”
* Partition Scheme: “Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)”
* Core Debug Level: “None”
* PSRAM: “Enabled”

Bernado writes:

> Although I am using 4MB for the PSRAM coupled to the ESP32 (the FabGL assumption), that memory actually has 8MB, so if you find a way to use the extra 4MB, please change the configuration to 8MB.

And for the Port, you will need to determine the Com port that the Agon Light is assigned from your OS after it is connected.

Now the third party libraries will need to be installed from the Library Manager in the Tools menu

* FabGL version 1.0.8
* ESP32Time version 2.0.0

It is important you use these exact versions otherwise I cannot guarantee the code will compile or run correctly.

NB:

- If you are using version 2.0.x of the IDE and get the following message during the upload stage: `ModuleNotFoundError: No module named 'serial'` then you will need to install the python3-serial package.
- If you are using an Apple Mac with an M chipset and are having difficulties uploading to the Agon, try changing the upload speed from 921600 to 115200 - some users have reported that works.