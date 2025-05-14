//
//    FILE: AD5593R_test_dac.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: test dac mode
//     URL: https://github.com/RobTillaart/AD5593R
//
//  outputs should have different output voltage
//  from zero to full scale in 8 steps.
//

#include "AD5593R.h"
#include "Wire.h"

AD5593R AD(0x10);


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

  Serial.print("Connect: ");
  Serial.println(AD.isConnected());
  Serial.print("Address: ");
  Serial.println(AD.getAddress(), HEX);

  //  set all eight pins to DAC mode.
  AD.setDACmode(0xFF);
  //  COPY input register direct to DAC
  AD.setLDACmode(0);
  //  use internal Vref 2.5V
  AD.setExternalReference(false, 0);
}


void loop()
{
  //  WRITE 8 different values
  for (int pin = 0; pin < 8; pin++)
  {
    AD.wakeUpDac(pin);
    delay(10);
    //  expect 0's
    Serial.print(AD.writeDAC(pin, (pin * 4095) / 7 ));
    Serial.print("\t");
  }
  Serial.println();

  //  READ BACK
  for (int pin = 0; pin < 8; pin++)
  {
    //  expect increasing values.
    Serial.print(AD.readDAC(pin), HEX);
    Serial.print("\t");
  }
  Serial.println();
  Serial.println();


  delay(1000);
}


//  -- END OF FILE --
