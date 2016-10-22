#include "Modbus_CoProcessor.h"

// Please install Adafruit GFX: https://learn.adafruit.com/adafruit-gfx-graphics-library/overview
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold9pt7b.h>

int statusLed = 13; // LED used for status

mcp mcp(19200); // Prepare object, set serial baud to 19200

void setup() {
  pinMode(statusLed, OUTPUT);

  delay(1000);

  digitalWrite(statusLed, HIGH);
  mcp.InitSign(); // This usually should only be run once after the sign is first powered on.
  digitalWrite(statusLed, LOW);


  // Turn on all dots as a test
  delay(1000);
  digitalWrite(statusLed, HIGH);
  mcp.dotAllOn();
  mcp.UpdateSign();
  digitalWrite(statusLed, LOW);
  delay(5000);


  // Display large text with a special font
  delay(1000);
  digitalWrite(statusLed, HIGH);
  mcp.dotAllOff();
  mcp.setFont(&FreeMonoBold9pt7b);
  mcp.setTextColor(1);
  mcp.setCursor(0, 13);
  mcp.print("Hello!!");
  mcp.UpdateSign();
  digitalWrite(statusLed, LOW);
  delay(5000);

  // Display the number of micros() uptime with the default font @2x size
  for (int i = 0; i < 10; i++) {
    digitalWrite(statusLed, HIGH);
    mcp.dotAllOff();
    mcp.setFont();
    mcp.setTextSize(2);
    mcp.setTextColor(1);
    mcp.setCursor(1, 1);
    mcp.print(micros());
    mcp.UpdateSign();
    digitalWrite(statusLed, LOW);
    delay(1000);
  }

  // Demo two lines of the basic font
  delay(1000);
  digitalWrite(statusLed, HIGH);
  mcp.dotAllOff();
  mcp.setFont();
  mcp.setTextSize(1);
  mcp.setTextColor(1);
  mcp.setCursor(1, 1);
  mcp.println("Line one.");
  mcp.print("The second line.");
  mcp.UpdateSign();
  digitalWrite(statusLed, LOW);
  delay(5000);



  // Display a circle that moves across the sign
  for (int i = 0; i < xSize; i++) {
    digitalWrite(statusLed, HIGH);
    mcp.dotAllOff();
    mcp.fillCircle(i, 7, 5, 1);
    mcp.UpdateSign();
    digitalWrite(statusLed, LOW);
    delay(700);
  }

  digitalWrite(statusLed, HIGH);
  mcp.dotAllOff();
  mcp.UpdateSign();
  digitalWrite(statusLed, LOW);


  delay(5000);
  // mcp.CloseSign(); // The sign generally needs to be rebooted after this command.
  delay(100);


}

void loop() {
  // Flash LED to indicate program is finished.
  digitalWrite(statusLed, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(50);               // wait for a second
  digitalWrite(statusLed, LOW);    // turn the LED off by making the voltage LOW
  delay(50);               // wait for a second
}
