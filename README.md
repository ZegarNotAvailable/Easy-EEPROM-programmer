# Easy-EEPROM-programmer
## Simply burn 2864 and 28256 EEPROM's.
- CA80 is all you need to burn 28C64.
- No additional harware needed.
- 28C256 is too big, then I build simply burner.
## A few years ago I built a simple EEPROM programmer from a Z80-MBC2.
![28C64 burner](https://github.com/ZegarNotAvailable/Easy-EEPROM-programmer/blob/main/Pictures/28C64-burner.jpg)
## Today I decided to make a similar one for RC2014.
- The program allows you to transfer data to the AT28C256 memory (others have not been tested).
- I use "page write mode" when possible. Otherwise "single byte mode".
- I used a SC108 PCB on which I put only Z80 (CMOS) and U3 as programmer socket.
    - Modify the U3 connections:
        1. isolate PIN1 and connect A14 there (e.g. to PIN3 U2)
        2. isolate PIN27 and connect to WR_ (e.g. to PIN29 U2)
        3. PIN 20 connect to MREQ_ (e.g. in U3 connect PIN 22 to PIN30)
        4. isolate PIN22 (OE_) and connect to +5V through a 10k resistor (pull up)
- I used the Z80-MBC2 software (by J4F), however, I made a lot of modifications and new features.
