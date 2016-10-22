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
#ifndef Modbus_CoProcessor_h
#define Modbus_CoProcessor_h

#include "Arduino.h"
#include <Adafruit_GFX.h>

#define SERIALDEVICE Serial3 // This serial port should be connected to an RS485 converter

// Note that this library is tailored to 98x16. It would require work to adapt to other sizes.
const int xSize = 98; // Enter the real number of x dots (1 indexed)
const int ySize = 16; // Enter the real number of y dots (1 indexed)
const int byteStreamSize = ((xSize*ySize) / 8); // Number of bytes of sign data
const int eolDelay = 10; // Number of milliseconds to delay after each EOL (10 is good, 9 minimum)
const int endOfUpdateDelay = 0; // Number of milliseconds to delay after each sign update. (0 is default)

class mcp : public Adafruit_GFX
{
  public:
    mcp(int baudRate);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void dotOn(byte x, byte y);
    void dotOff(byte x, byte y);
    void dotAllOn();
    void dotAllOff();
    void invertAll();
    void UpdateSign();
    void ConvertBitmapToBytestream();
    void InitSign();
    void CloseSign();
    void PrintString(String in);
    void PrintRegister0();
    void PrintRegister1();
    void PrintRegister2();
    void PrintRegister3();
    void PrintRegister4();
    void PrintRegister5();
    void PrintRegister6();
    void PrintRegister7();
    void PrintRegister8();
    void PrintRegister9();
    void PrintRegisterA();
    void PrintRegisterB();
    void PrintRegisterC();
    void PrintRegisterD();
    void PrintRegisterE();
    // Credit to author Kunchala Anil for C++ Arduino modbus LRC calculation code below:
    String calculateLRC(String input);
    int toDec(char val);
    int conv(char val1,char val2);
    int find_sum(const int * val, int myLength);
    
  private:
    bool BitmapMatrix[xSize][ySize]; // Create a 2D array to hold the "human" readable bitmap sign image.
    byte Bytestream[byteStreamSize]; // Create a stream of bytes that will be sent to the sign via modbus
};

#endif
