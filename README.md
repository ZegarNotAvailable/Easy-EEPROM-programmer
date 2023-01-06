# Easy-EEPROM-programmer
## Simply burn 2864 and 28256 EEPROM's.
- CA80 is all you need to burn 28C64.
- No additional harware needed.
- 28C256 is too big, then I build simply burner.
## A few years ago I built a simple EEPROM programmer from a Z80-MBC2.
![28C64 burner](https://github.com/ZegarNotAvailable/Easy-EEPROM-programmer/blob/main/Pictures/28C64-burner.jpg)
## Today I decided to make a similar one for RC2014.
- I used a SC108 PCB on which I put only Z80 (CMOS) and U3 as programmer socket.
![Hardware](https://github.com/ZegarNotAvailable/Easy-EEPROM-programmer/blob/main/Pictures/SC108-as-burner.jpg)
![Hardware](https://github.com/ZegarNotAvailable/Easy-EEPROM-programmer/blob/main/Pictures/Bootloader-as-burner.jpg)
![Changes](https://github.com/ZegarNotAvailable/Easy-EEPROM-programmer/blob/main/Pictures/Only-few-changes.jpg)
    - Modify the U3 connections:
        1. isolate PIN1 and connect A14 there (e.g. to PIN3 U2)
        2. isolate PIN27 and connect to WR_ (e.g. to PIN29 U2)
        3. PIN 20 connect to MREQ_ (e.g. in U2 connect PIN 22 to PIN30)
        4. isolate PIN22 (OE_) and connect to +5V through a 10k resistor (pull up)
![Ready to use](https://github.com/ZegarNotAvailable/Easy-EEPROM-programmer/blob/main/Pictures/Burner-ready-to-use.jpg)
- I used the Z80-MBC2 software (by J4F), however, I made a lot of modifications and new features.
- The code allows you to transfer data to the AT28C256 memory (others have not been tested).
- I use "page write mode" when possible. Otherwise "single byte mode".
- First sidable SDP.
![SDP disable](https://github.com/ZegarNotAvailable/Easy-EEPROM-programmer/blob/main/Pictures/SDP-disable.png)
- HL register is a memory pointer.
![LD HL,addr](https://github.com/ZegarNotAvailable/Easy-EEPROM-programmer/blob/main/Pictures/LD-HL-addr.png)
- The two instruction are "LD (HL),n" and "INC HL".
![LD (HL),data INC HL](https://github.com/ZegarNotAvailable/Easy-EEPROM-programmer/blob/main/Pictures/LD-M-data-INC-HL.png)
![Burn record](https://github.com/ZegarNotAvailable/Easy-EEPROM-programmer/blob/main/Pictures/burn-record.png)
## User manual.
- Just put the *.HEX (Intel HEX) files and the 28C256.txt file on the SD card.
- Up to five HEX files with name format 8.3
- In the 28C256.txt file, put the names without extensions separated by a space.
![Burning](https://github.com/ZegarNotAvailable/Easy-EEPROM-programmer/blob/main/Pictures/CA80-monitor-burning.png)
## You can use breadboard instead of "CA80-bootloader".
- Just connect D0...D7, CLK, RESET and power.
- The description can be found in the code.