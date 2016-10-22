/*
   This library will allow basic software control of a flipdot sign.

   The library extends Adafruit_GFX to allow easy drawing.

   Built for a:
   GTI Luminator MegaMax 3000 Front Sign 98x16 (Sign ID: 6)
   All copyright for sign and communication protocol to their
   original owners/creators.

   The sign uses a form of ASCII modbus to communicate
   over RS485. This library outputs serial data, which
   should be physically connected to a TTL->RS485 device.

   Harrison Shutan - Oct 2016


*/
#include "Arduino.h"
#include "Modbus_CoProcessor.h"
#include <Adafruit_GFX.h>

mcp::mcp(int baudRate) : Adafruit_GFX(xSize, ySize)
{
  // These items are ran when the class is instantiated.
  SERIALDEVICE.begin(baudRate);

  // Init the human readable bitmap with 0s
  dotAllOff();

  // Init the byte array with 0s
  for (int i = 0; i < byteStreamSize; i++) {
    Bytestream[i] = 0;
  }

}

void mcp::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // This overrides the Adafruit_GFX draw pixel.
  // Pass in x,y coords and "color" (a color of 1 means dot on, anything else means dot off)

  // If out of bounds, ignore and return
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    return;

  // Otherwise,
  if (color == 1) {
    dotOn(x, y);
  } else {
    dotOff(x, y);
  }
}

void mcp::dotOn(byte x, byte y)
{
  // Generally you will want to use the Adafruit_GFX library, however
  // you can directly call "dotOn" if you want. There is no OOB checking.
  BitmapMatrix[x][y] = 1;
}

void mcp::dotOff(byte x, byte y)
{
  // Generally you will want to use the Adafruit_GFX library, however
  // you can directly call "dotOff" if you want. There is no OOB checking.
  BitmapMatrix[x][y] = 0;
}

void mcp::dotAllOn()
{
  // Init the 2D array with all 1s
  for (int x = 0; x < xSize; x++) {
    for (int y = 0; y < ySize; y++) {
      BitmapMatrix[x][y] = 1;
    }// close for x
  }// close for y
}

void mcp::dotAllOff()
{
  // Init the 2D array with all 0s
  for (int x = 0; x < xSize; x++) {
    for (int y = 0; y < ySize; y++) {
      BitmapMatrix[x][y] = 0;
    }// close for x
  }// close for y
}

void mcp::invertAll()
{
  // Invert all dots
  for (int x = 0; x < xSize; x++) {
    for (int y = 0; y < ySize; y++) {
      if (BitmapMatrix[x][y] == 0) {
        BitmapMatrix[x][y] = 1;
      } else {
        BitmapMatrix[x][y] = 0;
      }
    }// close for x
  }// close for y
}


void mcp::UpdateSign()
{
  // This will take everything in the human bitmap,
  // Convert it to the correct ordering and stream of bytes
  // Then print those bytes out as ASCII, with checksum, to serial
  // Essentially, this tells the sign to display what you have
  // written to the bitmap array with dotOn()/dotOff()

  // This command is blocking and takes about 615ms to run on a 96MHz MCU
  // ~400ms of calculations + serial writing, and ~200ms of required EOL delays

  // Convert bitmap to correct stream of bytes
  ConvertBitmapToBytestream();

  // Tell sign we are about to send a new image
  PrintString(":01000603A254");

  // Print computed registers 0 thru E
  // These registers contain the sign image data
  PrintRegister0();
  PrintRegister1();
  PrintRegister2();
  PrintRegister3();
  PrintRegister4();
  PrintRegister5();
  PrintRegister6();
  PrintRegister7();
  PrintRegister8();
  PrintRegister9();
  PrintRegisterA();
  PrintRegisterB();
  PrintRegisterC();
  PrintRegisterD();
  PrintRegisterE();

  // Tell the sign to display the image!
  PrintString(":00000F01F0");
  PrintString(":0100060200F7");
  PrintString(":0100060600F3");
  PrintString(":0100060200F7");
  PrintString(":01000603A94D");

  delay(endOfUpdateDelay); // This delay is 0 by default
}

void mcp::ConvertBitmapToBytestream()
{
  int byteStreamCounter = 0;
  for (int x = 0; x < xSize; x++) { // Loop thru each column

    for (int y1 = 7; y1 > -1; y1--) { // Loop thru the first 8 bits of a column
      if (BitmapMatrix[x][y1] == 1) {
        bitSet(Bytestream[byteStreamCounter], 0); // Set bit to a 1 in the right most place of the byte
      } else {
        bitClear(Bytestream[byteStreamCounter], 0); // Set bit to a 0 in the right most place of the byte
      }
      if (y1 > 0) { // This if prevents the bitshift << from happening more than 7 times
        Bytestream[byteStreamCounter] <<= 1; // Because bitSet and bitClear write to the LSB (right most) bit,
        //bump all the bits over to the left one position to prepare the LSB for next set or clear
      }
    }// close for first 8 bits of a column
    byteStreamCounter++;

    for (int y2 = 15; y2 > 7; y2--) { // Loop thru the second 8 bits of a column
      if (BitmapMatrix[x][y2] == 1) {
        bitSet(Bytestream[byteStreamCounter], 0); // Set bit to a 1
      } else {
        bitClear(Bytestream[byteStreamCounter], 0); // Set bit to a 0
      }
      if (y2 > 8) { // This if prevents the bitshift << from happening more than 7 times
        Bytestream[byteStreamCounter] <<= 1;
      }
    }// close for second 8 bits of a column
    byteStreamCounter++;
  }// close for x
}

void mcp::InitSign()
{
  // This must be run once after the sign is physically powered on.
  // This command puts the sign into "ready" mode, where it waits for new data.
  // Note that this is hardcoded to sign ID 6, also checksums are hardcoded.
  PrintString(":01000502FFF9");
  PrintString(":01000602FFF8");
  PrintString(":01000603A155");
  PrintString(":100000000447000F101C1C1C1C1000000000000006");
  PrintString(":00000101FE");
  PrintString(":0100060200F7");
}

void mcp::CloseSign()
{
  // This can be run before 24v power is shutdown to the sign.
  // This command will fully clear the sign, writing a hard flip off to each dot.
  // Note that this is hardcoded to sign ID 6, also checksums are hardcoded.
  // The sign generally needs to be power cycled after you shut it down with this command.
  // Also note, if the 12v power alone is removed from the sign, the sign will initiate
  // this same shutdown code on its own.
  PrintString(":01000603A94D");
  PrintString(":01000603AA4C");
  PrintString(":01007F02FF7F");
  PrintString(":0100060255A2");
  PrintString(":01000603A650");
}

void mcp::PrintString(String in)
{
  // This will write data to the serial device that is hooked to RS485
  // In case the sign is talking to us, wait for it to finish
  while (SERIALDEVICE.available() > 0) {
    SERIALDEVICE.read(); // Throw away everything the sign says, we don't care.
  }

  SERIALDEVICE.println(in); // println default EOL is CRLF, good for modbus
  SERIALDEVICE.flush(); // Wait for the serial port to finish sending
  delay(eolDelay); // Delay in ms after each line is sent

  // In case the sign is responding, wait for it to finish
  while (SERIALDEVICE.available() > 0) {
    SERIALDEVICE.read(); // Throw away everything the sign says, we don't care.
  }
}

void mcp::PrintRegister0()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 0; // R0 starts at byte 0
  int numOfBytesToChewOff = 12; // R0 needs 12 bytes of data
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":10000000010A0000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegister1()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 12; // R1 starts at byte 12
  int numOfBytesToChewOff = 16; // R1 needs 16 bytes of data
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":10001000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegister2()
{
  // This register is left totally blank, because I have 98x16 sign.
  // The 98x16 sign has a chunk of dots missing, so we will just send blank
  // to the sign controller, and it will make the image seamless.
  PrintString(":1000200000000000000000000000000000000000D0");
}

void mcp::PrintRegister3()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  // R3 is still making up for the blank section

  int startingPoint = 28; // R3 starts at byte 28
  int numOfBytesToChewOff = 4; // R3 needs 4 bytes of data
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":10003000000000000000000000000000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegister4()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 32;
  int numOfBytesToChewOff = 16;
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":10004000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegister5()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 48;
  int numOfBytesToChewOff = 16;
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":10005000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegister6()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 64;
  int numOfBytesToChewOff = 16;
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":10006000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegister7()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 80;
  int numOfBytesToChewOff = 16;
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":10007000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegister8()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 96;
  int numOfBytesToChewOff = 16;
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":10008000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegister9()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 112;
  int numOfBytesToChewOff = 16;
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":10009000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegisterA()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 128;
  int numOfBytesToChewOff = 16;
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":1000A000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegisterB()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 144;
  int numOfBytesToChewOff = 16;
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":1000B000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegisterC()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 160;
  int numOfBytesToChewOff = 16;
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":1000C000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegisterD()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 176;
  int numOfBytesToChewOff = 16;
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":1000D000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}

void mcp::PrintRegisterE()
{
  // Loop thru correct number of bytes for this register
  // Concat any before/after text
  // Call calculate checksum
  // Print final String

  int startingPoint = 192;
  int numOfBytesToChewOff = 4;
  String prependData = ""; // Per-register string to start with
  String middleData = "";
  String checksum = ""; // Per-register checksum to be calculated

  prependData = ":1000E000";

  for (int i = startingPoint; i < startingPoint + numOfBytesToChewOff; i++) {
    char sBuffer [4];
    sprintf(sBuffer, "%0.2X", Bytestream[i]);
    middleData.concat(sBuffer[0]);
    middleData.concat(sBuffer[1]);
  }

  middleData.concat("000000000000000000000000"); // Special for end register, E

  checksum = calculateLRC(prependData + middleData);
  PrintString(prependData + middleData + checksum);
}


// LRC calculation code, credit to author Kunchala Anil
String mcp::calculateLRC(String input)
{
  char * a = (char *)input.c_str();
  int myLength = strlen(a);
  int val[myLength / 2];
  for (int i = 1, j = 0 ; i < strlen(a); i = i + 2, j++ )
  {
    val[j] = conv(a[i], a[i + 1]);
  }
  int sum = find_sum(val, myLength);
  char hex[5];
  utoa((unsigned)sum, hex, 16);
  int hex_val = (int)strtol(hex, NULL, 16);
  hex_val = ((~hex_val) + B01) & 0xff;
  char hex_val_str[4];
  sprintf(hex_val_str, "%0.2X", hex_val);
  String finally = hex_val_str;
  return finally;
}

int mcp::toDec(char val)
{
  if (val <= '9')
  {
    return val - '0';
  }
  else
  {
    return val - '0' - 7;
  }
}

int mcp::conv(char val1, char val2)
{
  int val_a = toDec(val1);
  int val_b = toDec(val2);
  return (val_a * 16) + val_b;
}

int mcp::find_sum(const int * val, int myLength)
{
  {
    int sum = 0;
    for (int i = 0; i <= (myLength / 2) - 1; i++)
    {
      sum = sum + val[i];
    }
    return sum;
  }
}


