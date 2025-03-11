//
//    FILE: AD5593R_setMode.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: test setMode
//     URL: https://github.com/RobTillaart/AD5593R


#include "AD5593R.h"
#include "Wire.h"

AD5593R AD(0x10);

uint16_t n = 0;

void setup()
{
  while (!Serial);
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("AD5593R_LIB_VERSION: ");
  Serial.println(AD5593R_LIB_VERSION);
  Serial.println();

  Wire.begin();

  Serial.println(AD.begin());

  //  set pin 0,1,2 as ADC, pin 3 as DAC and pin 4,5,6,7 as OUTPUT
  AD.setMode("AAADOOOO");
}


void loop()
{
  //  just do something.
  for (int pin = 0; pin < 3; pin++)
  {
    Serial.print(AD.readADC(pin));
    Serial.print("\t");
  }

  n = (n + 101) % 4095;
  AD.writeDAC(4, n);
  Serial.print(n);
  Serial.print("\t");
  for (int pin = 4; pin < 8; pin++)
  {
    AD.write1(pin, n & 0x01);
  }
  Serial.println();

  delay(1000);
}


//  -- END OF FILE --
