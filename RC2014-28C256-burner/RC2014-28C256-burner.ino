// Fragmenty kodu skopiowane z oprogramowania A040618 (Z80-MBC2) autor J4F (Just For Fun)
// Na pewno mozliwe jest uproszczenie / skasowanie nieuzywanych zmiennych, stałych i innych definicji.
// Program umozliwia przeslanie danych do pamieci AT28C256 (innych nie testowalem).
// Zapis "single byte mode".
// Użyłem PCB SC108 na którym umieściłem tylko Z80 (CMOS) i U3 jako gniazdo programatora.
// Nalezy zmodyfikowac polaczenia U3:
// 1. wyizolowac PIN1 i podłączyć tam A14 (np. do PIN3 U2)
// 2. wyizolowac PIN27 i podlaczyc do WR_ (np. do PIN29 U2)
// 3. wyizolowac PIN20 i podlaczyc do MREQ_ (np. w U3 PIN22 polaczyc z PIN30)
// 4. wyizolowac PIN22 (OE_) i podlaczyc do +5V przez rezystor 10k (pull up)
// Dobrej zabawy!!!
// Zegar.

#include <SPI.h>
#include <SD.h>

#define VER "\r\nEEPROM 28C256 burner for RC2014.\r\n" //For info
//definicje SD
#define FILES_NUMBER 5
#define NAME_WIDTH 9
#define EXTENSION String(".HEX")
#define SD_CS  4           // PB4 pin 5    SD SPI
File myFile;
char          c;
byte          namesNumber;
char          fileNames[FILES_NUMBER][NAME_WIDTH];
String        fileName = "28C256.txt";
byte          burningTime = 10;      // See datasheet of EEPROM
word          bytesBurned   = 0;

// ------------------------------------------------------------------------------
//
// Hardware definitions for A040618 (Z80-MBC2) - Base system
//
// ------------------------------------------------------------------------------

#define   D0            24    // PA0 pin 40   Z80 data bus
#define   D1            25    // PA1 pin 39
#define   D2            26    // PA2 pin 38
#define   D3            27    // PA3 pin 37
#define   D4            28    // PA4 pin 36
#define   D5            29    // PA5 pin 35
#define   D6            30    // PA6 pin 34
#define   D7            31    // PA7 pin 33

#define   AD0           18    // PC2 pin 24   Z80 A0
#define   WR_           19    // PC3 pin 25   Z80 WR
#define   RD_           20    // PC4 pin 26   Z80 RD
#define   MREQ_         21    // PC5 pin 27   Z80 MREQ
#define   RESET_        22    // PC6 pin 28   Z80 RESET
#define   MCU_RTS_      23    // PC7 pin 29   * RESERVED - NOT USED *
#define   MCU_CTS_      10    // PD2 pin 16   * RESERVED - NOT USED *
#define   INT_           1    // PB1 pin 2    Z80 control bus
#define   MEM_EN        2    // PB2 pin 3    RAM Chip Enable (CE2). Active HIGH. Used only during boot
#define   WAIT_          3    // PB3 pin 4    Z80 WAIT
#define   SS_            4    // PB4 pin 5    SD SPI
#define   MOSI           5    // PB5 pin 6    SD SPI
#define   MISO           6    // PB6 pin 7    SD SPI
#define   SCK            7    // PB7 pin 8    SD SPI
#define   BUSREQ_       14    // PD6 pin 20   Z80 BUSRQ
#define   CLK           15    // PD7 pin 21   Z80 CLK
#define   SCL_PC0       16    // PC0 pin 22   IOEXP connector (I2C)
#define   SDA_PC1       17    // PC1 pin 23   IOEXP connector (I2C)
#define   LED_IOS        0    // PB0 pin 1    Led LED_IOS is ON if HIGH
#define   WAIT_RES_      0    // PB0 pin 1    Reset the Wait FF
#define   USER          13    // PD5 pin 19   Led USER and key (led USER is ON if LOW)

// ------------------------------------------------------------------------------
//
// Atmega clock speed check
//
// ------------------------------------------------------------------------------

#if F_CPU == 20000000
#define CLOCK_LOW   "5"
#define CLOCK_HIGH  "10"
#else
#define CLOCK_LOW   "4"
#define CLOCK_HIGH  "8"
#endif

// ------------------------------------------------------------------------------
//
//  Constants
//
// ------------------------------------------------------------------------------

const byte    LD_HL        =  0x36;       // Opcode of the Z80 instruction: LD(HL), n
const byte    INC_HL       =  0x23;       // Opcode of the Z80 instruction: INC HL
const byte    LD_HLnn      =  0x21;       // Opcode of the Z80 instruction: LD HL, nn
const byte    JP_nn        =  0xC3;       // Opcode of the Z80 instruction: JP nn

void sendNopS(int NOPsToSend)
{
  for(int i = 0; i < NOPsToSend; i++)
  {
    sendNop();     //Z80 NOP 
  }
}

void readFile(String fileName)
{
  Serial.println(F("Initializing SD card..."));
  delay(200);
  if (!SD.begin(SD_CS))
  {
    Serial.println(F("initialization failed!"));
    return;
  }
  Serial.println(F("initialization done."));
  if (SD.exists(fileName))
  {
    Serial.print(fileName);
    Serial.println(F(" exists."));
  }
  else
  {
    Serial.print(fileName);
    Serial.println(F(" doesn't exist."));
  }
  myFile = SD.open(fileName); // re-open the file for reading:
  if (myFile)
  {
    namesNumber = 1;
    byte i, j;
    for (j = 0; j < FILES_NUMBER; j++)
    {
      for (i = 0; i < NAME_WIDTH; i++)
      {
        if (myFile.available())// read from the file until there's nothing else in it
        {
          c = (myFile.read());
          Serial.print(c);
          if ((c < '0') || (c > 'z'))
          {
            if (c == ' ')
            {
              if (i == 0)
              {
                Serial.println(F("Name is to short!"));
                return;
              }
              else
              {
                fileNames[j][i] = NULL;
                i = NAME_WIDTH;
                namesNumber++;
              }
            }
          }
          else
          {
            fileNames[j][i] = c;
          }
        }
        else
        {
          Serial.println(F("\r\n* End of file *"));
          myFile.close();
          return;
        }
      }
    }
  }
}

void sendFilesFromSD()
{
  byte j;
  Serial.println();
  sendNopS(32);
  for (j = 0; j < namesNumber; j++)
  {
    fileName = String(String(fileNames[j]) + EXTENSION);
    if (SD.exists(fileName))
    {
      Serial.print(F("Burning "));
      Serial.println(fileName);
      sendNopS(32);
      sendFileFromSD(fileName);
    }
    else
    {
      Serial.print(fileName);
      Serial.println(F(" doesn't exist."));
    }
  }
}

void sendRecord()
{
  if (myFile.available())
  {
    byte i = getByteFromFile();
    word adr = getAdrFromFile();
    byte typ = getByteFromFile();
    if (typ == 0)
    {
      burnData(i, adr);
    }
    else
    {
      Serial.print(F("End of file: "));
      Serial.println(fileName);
    }
    byte suma = getByteFromFile(); // zakladam, ze plik jest poprawny i nie sprawdzam sumy
    // ale trzeba ja przeczytac!!!
    myFile.read();   // tak jak znaki CR
    myFile.read();   // i LF na koncu rekordu :-)
  }
}

byte getByteFromFile()
{
  byte h, l;
  if (myFile.available())
  {
    h = myFile.read();
    h = asciiToDigit(h);
  }
  if (myFile.available())
  {
    l = myFile.read();
    l = asciiToDigit(l);
  }
  return ((16 * h) + l);
}

byte asciiToDigit(byte l)
{
  l = l - 0x30;
  if (l > 0x09)
  {
    l = l - 0x07;
  }
  return l;
}

word getAdrFromFile()
{
  byte h = getByteFromFile();
  byte l = getByteFromFile();
  return ((256 * h) + l);
}

void burnData(byte l, word adr)  //l - liczba bajtow do przeslania, adr - adres pierwszego bajtu
{
  loadHL(adr);                   // Set Z80 HL = SEC (used as pointer to EEPROM);
  if (64 - (adr % 64) >= l)
  {
    for (byte i = 0; i < l; i++)
    {
      byte b = getByteFromFile();
      burnByteIncHL(b);         // Write current data byte into EEPROM (adr w HL, HL++),
    }
    delay(burningTime);         // Page write
  }
  else
  {
    for (byte i = 0; i < l; i++)
    {
      byte b = getByteFromFile();
      burnByteIncHL(b);         // Write current data byte into EEPROM (adr w HL, HL++),
      delay(burningTime);       // Single byte write
    }
  }
}

void sendFileFromSD(String fileName)
{
  myFile = SD.open(fileName); // re-open the file for reading:
  if (myFile)
  {
    while (myFile.available())// read from the file until there's nothing else in it
    {
      byte c = (myFile.read());
      if (c  == 0x3a)
      {
        sendRecord();
      }
      else
      {
        Serial.print(F("Wrong format file: "));
        Serial.println(fileName);
        myFile.close();
        return;
      }
    }
    myFile.close();
  }
}



void setup()
{
  // ------------------------------------------------------------------------------
  //
  //  Local variables
  //
  // ------------------------------------------------------------------------------

  byte          data;                       // External EEPROM data byte
  word          address;                    // External EEPROM current address;
  char          minBootChar   = '1';        // Minimum allowed ASCII value selection (boot selection)
  char          maxSelChar    = '8';        // Maximum allowed ASCII value selection (boot selection)
  byte          maxBootMode   = 4;          // Default maximum allowed value for bootMode [0..4]
  byte          bootSelection = 0;          // Flag to enter into the boot mode selection
  
  
  // ------------------------------------------------------------------------------

  // Print some system information
  Serial.begin(115200);
  Serial.println(F(VER));
  pinsSetings();

  while (Serial.available() > 0) Serial.read();   // Flush serial Rx buffer
  Serial.println(F("\r\nPress any key to burn."));
  while (Serial.available() < 1); // Wait a key

  Serial.println(F("Loading..."));
  digitalWrite(USER, LOW);
  unsigned long startTime = millis();
  readFile(fileName);
  SDP_disable();
  sendFilesFromSD();
  SDP_enable();
  unsigned long endTime = millis();
  Serial.print(F("\r\nBurned "));
  Serial.print(bytesBurned);
  Serial.println(F(" bytes"));
  Serial.print(F("\r\nBurning time: "));
  Serial.print(endTime - startTime);
  Serial.println(" ms.");
  Serial.println(F("\r\nDone."));
  digitalWrite(USER, HIGH);

}

// ------------------------------------------------------------------------------

void loop()
{
  // Nothing to do
}
// ------------------------------------------------------------------------------

// Z80 bootstrap routines

// ------------------------------------------------------------------------------

#define CLK_HIGH    PORTD |= B10000000
#define CLK_LOW     PORTD &= B01111111
#define MEM_EN_HIGH PORTB |= B00000100
#define MEM_EN_LOW  PORTB &= B10111011

void pulseClock(byte numPulse)
// Generate <numPulse> clock pulses on the Z80 clock pin.
// The steady clock level is LOW, e.g. one clock pulse is a 0-1-0 transition
{
  byte    i;
  for (i = 0; i < numPulse; i++)
    // Generate one clock pulse
  {
    // Send one impulse (0-1-0) on the CLK output
    CLK_HIGH;         //Bylo: digitalWrite(CLK, HIGH);
    CLK_LOW;          //      digitalWrite(CLK, LOW);
  }
}

// ------------------------------------------------------------------------------

void memoryWrite() //zapis do EEPROM
// Generate 3 clock pulses on the Z80 clock pin.
// The steady clock level is LOW, e.g. one clock pulse is a 0-1-0 transition
{
  // Generate 3 clock pulses and enable EEPROM to write

  // Send one impulse (0-1-0) on the CLK output
  pulseClock(2);
  MEM_EN_LOW;   //should be used as WR_
  MEM_EN_HIGH;  //should be used as WR_
  pulseClock(1);
  //delay(10);
}

// ------------------------------------------------------------------------------

void burnByteIncHL(byte value)
// Load a given byte to EEPROM using a sequence of two Z80 instructions forced on the data bus.
// The MEM_EN signal is used to force the EEPROM in HiZ, so the Atmega can write the needed instruction/data
//  on the data bus. Controlling the clock signal and knowing exactly how many clocks pulse are required it
//  is possible control the whole loading process.
// In the following "T" are the T-cycles of the Z80 (See the Z80 datashet).
// The two instruction are "LD (HL), n" and "INC (HL)".
{
  // Execute the LD(HL),n instruction (T = 4+3+3). See the Z80 datasheet and manual.
  // After the execution of this instruction the <value> byte is loaded in the memory address pointed by HL.
  burnByte(value);
  // Execute the INC(HL) instruction (T = 6). See the Z80 datasheet and manual.
  // After the execution of this instruction HL points to the next memory address.
  pulseClock(1);                      // Execute the T1 cycle of M1 (Opcode Fetch machine cycle)
  DDRA = 0xFF;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as output
  PORTA = INC_HL;                     // Write "INC(HL)" opcode on data bus
  pulseClock(2);                      // Execute T2 and T3 cycles of M1
  DDRA = 0x00;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as input...
  PORTA = 0xFF;                       // ...with pull-up
  pulseClock(3);                      // Execute all the remaining T cycles
}

// ------------------------------------------------------------------------------

void loadHL(word value)
// Load "value" word into the HL registers inside the Z80 CPU, using the "LD HL,nn" instruction.
// In the following "T" are the T-cycles of the Z80 (See the Z80 datashet).
{
  // Execute the LD dd,nn instruction (T = 4+3+3), with dd = HL and nn = value. See the Z80 datasheet and manual.
  // After the execution of this instruction the word "value" (16bit) is loaded into HL.
  pulseClock(1);                      // Execute the T1 cycle of M1 (Opcode Fetch machine cycle)
  //digitalWrite(MEM_EN, !LOW);         // Force the EEPROM in HiZ (CE2 = LOW)
  DDRA = 0xFF;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as output
  PORTA = LD_HLnn;                    // Write "LD HL, n" opcode on data bus
  pulseClock(2);                      // Execute T2 and T3 cycles of M1
  DDRA = 0x00;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as input...
  PORTA = 0xFF;                       // ...with pull-up
  pulseClock(2);                      // Complete the execution of M1 and execute the T1 cycle of the following
  // Memory Read machine cycle
  DDRA = 0xFF;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as output
  PORTA = lowByte(value);             // Write first byte of "value" to load in HL
  pulseClock(3);                      // Execute the T2 and T3 cycles of the first Memory Read machine cycle
  // and T1, of the second Memory Read machine cycle
  PORTA = highByte(value);            // Write second byte of "value" to load in HL
  pulseClock(2);                      // Execute the T2 and T3 cycles of the second Memory Read machine cycle
  DDRA = 0x00;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as input...
  PORTA = 0xFF;                       // ...with pull-up
}

// ------------------------------------------------------------------------------
void pinsSetings()
{
  // ----------------------------------------
  // INITIALIZATION
  // ----------------------------------------

  // Initialize RESET_ and WAIT_RES_
  pinMode(RESET_, OUTPUT);                        // Configure RESET_ and set it ACTIVE
  digitalWrite(RESET_, LOW);
  pinMode(WAIT_RES_, OUTPUT);                     // Configure WAIT_RES_ and set it ACTIVE to reset the WAIT FF (U1C/D)
  digitalWrite(WAIT_RES_, LOW);

  // Check USER Key for boot mode changes
  pinMode(USER, INPUT_PULLUP);                    // Read USER Key to enter into the boot mode selection
  // Initialize USER,  INT_, MEM_EN, and BUSREQ_
  pinMode(USER, OUTPUT);                          // USER led OFF
  digitalWrite(USER, HIGH);
  pinMode(INT_, INPUT_PULLUP);                    // Configure INT_ and set it NOT ACTIVE
  pinMode(MEM_EN, OUTPUT);                       // Configure MEM_EN as output
  digitalWrite(MEM_EN, HIGH);                    // Set MEM_EN HZ
  pinMode(WAIT_, INPUT);                          // Configure WAIT_ as input
  pinMode(BUSREQ_, INPUT_PULLUP);                 // Set BUSREQ_ HIGH
  pinMode(BUSREQ_, OUTPUT);
  digitalWrite(BUSREQ_, HIGH);

  // Initialize D0-D7, AD0, MREQ_, RD_ and WR_
  DDRA = 0x00;                                    // Configure Z80 data bus D0-D7 (PA0-PA7) as input with pull-up
  PORTA = 0xFF;
  pinMode(MREQ_, INPUT_PULLUP);                   // Configure MREQ_ as input with pull-up
  pinMode(RD_, INPUT_PULLUP);                     // Configure RD_ as input with pull-up
  pinMode(WR_, INPUT_PULLUP);                     // Configure WR_ as input with pull-up
  pinMode(AD0, INPUT_PULLUP);

  // Initialize CLK (single clock pulses mode) and reset the Z80 CPU
  pinMode(CLK, OUTPUT);                           // Set CLK as output
  digitalWrite(CLK, LOW);
  singlePulsesResetZ80();                         // Reset the Z80 CPU using single clock pulses

  // Initialize (park) MCU_RTS and MCU_CTS
  pinMode(MCU_RTS_, INPUT_PULLUP);                // Parked (not used)
  pinMode(MCU_CTS_, INPUT_PULLUP);
}

void singlePulsesResetZ80()
// Reset the Z80 CPU using single pulses clock
{
  digitalWrite(RESET_, LOW);          // Set RESET_ active
  pulseClock(6);                      // Generate twice the needed clock pulses to reset the Z80
  digitalWrite(RESET_, HIGH);         // Set RESET_ not active
  pulseClock(2);                      // Needed two more clock pulses after RESET_ goes HIGH
}

void SDP_enable()   //see datasheet of EEPROM
{
  loadHL(0x5555);
  burnByte(0xaa);
  loadHL(0x2aaa);
  burnByte(0x55);
  loadHL(0x5555);
  burnByte(0xa0);
  loadHL(0x7fff);
  burnByte(0xff);
  burnByte(0xff);
  bytesBurned -= 5;
}

void SDP_disable()  //see datasheet of EEPROM
{
  loadHL(0x5555);
  burnByte(0xaa);
  loadHL(0x2aaa);
  burnByte(0x55);
  loadHL(0x5555);
  burnByte(0x80);
  burnByte(0xaa);
  loadHL(0x2aaa);
  burnByte(0x55);
  loadHL(0x5555);
  burnByte(0x20);
  loadHL(0x7fff);
  burnByte(0xff);
  burnByte(0xff);
  bytesBurned -= 8;
}

void sendNop()
{
  // Execute the NOP instruction (T = 4). See the Z80 datasheet and manual.
  pulseClock(1);                      // Execute the T1 cycle of M1 (Opcode Fetch machine cycle)
  DDRA = 0xFF;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as output
  PORTA = 0;                          // Write "NOP" opcode on data bus
  pulseClock(2);                      // Execute T2 and T3 cycles of M1
  DDRA = 0x00;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as input...
  PORTA = 0xFF;                       // ...with pull-up
  pulseClock(1);                      // Complete the execution of M1
}

void burnByte(byte value)
// Load a given byte to EEPROM.
// In the following "T" are the T-cycles of the Z80 (See the Z80 datashet).
// Only one instruction is "LD (HL), n".
{
  // Execute the LD(HL),n instruction (T = 4+3+3). See the Z80 datasheet and manual.
  pulseClock(1);                      // Execute the T1 cycle of M1 (Opcode Fetch machine cycle)
  DDRA = 0xFF;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as output
  PORTA = LD_HL;                      // Write "LD (HL), n" opcode on data bus
  pulseClock(2);                      // Execute T2 and T3 cycles of M1
  DDRA = 0x00;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as input...
  PORTA = 0xFF;                       // ...with pull-up
  pulseClock(2);                      // Complete the execution of M1 and execute the T1 cycle of the following
  // Memory Read machine cycle
  DDRA = 0xFF;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as output
  PORTA = value;                      // Write the byte to load in EEPROM on data bus
  pulseClock(2);                      // Execute the T2 and T3 cycles of the Memory Read machine cycle
  DDRA = 0x00;                        // Configure Z80 data bus D0-D7 (PA0-PA7) as input...
  PORTA = 0xFF;                       // ...with pull-up
  memoryWrite();
  bytesBurned++;
}

