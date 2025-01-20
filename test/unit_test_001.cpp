//
//    FILE: unit_test_001.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.0
//    DATE: 2024-11-27
// PURPOSE: unit tests for the AD5933R IO device
//          https://github.com/RobTillaart/AD5933R
//          https://github.com/Arduino-CI/arduino_ci/blob/master/REFERENCE.md
//

// supported assertions
// ----------------------------
// assertEqual(expected, actual)
// assertNotEqual(expected, actual)
// assertLess(expected, actual)
// assertMore(expected, actual)
// assertLessOrEqual(expected, actual)
// assertMoreOrEqual(expected, actual)
// assertTrue(actual)
// assertFalse(actual)
// assertNull(actual)


#include <ArduinoUnitTests.h>

#include "Arduino.h"
#include "AD5593R.h"


unittest_setup()
{
  fprintf(stderr, "AD5593R_LIB_VERSION:\t%s\n", (char *) AD5593R_LIB_VERSION);
}


unittest_teardown()
{
}


unittest(test_begin)
{
  AD5593R ad;

  assertEqual(1, 1);
}


unittest_main()


//  -- END OF FILE --

