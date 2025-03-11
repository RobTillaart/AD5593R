//
//    FILE: AD5593R.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.1
//    DATE: 2024-01-30
// PURPOSE: Arduino library for AD5593R, I2C, 8 channel ADC / DAC / GPIO device.
//     URL: https://github.com/RobTillaart/AD5593R


#include "AD5593R.h"


//  CONFIG REGISTERS (aka pointer bytes)
#define AD5593_NOP                      0x00
#define AD5593_ADC_SEQ                  0x02
#define AD5593_GEN_CTRL_REG             0x03
#define AD5593_ADC_CONFIG               0x04
#define AD5593_DAC_CONFIG               0x05
#define AD5593_PULLDOWN_CONFIG          0x06
#define AD5593_LDAC_MODE                0x07
#define AD5593_GPIO_CONFIG              0x08
#define AD5593_GPIO_OUTPUT              0x09
#define AD5593_GPIO_INPUT               0x0A
#define AD5593_POWERDOWN_REF_CTRL       0x0B
#define AD5593_GPIO_OPENDRAIN_CONFIG    0x0C
#define AD5593_IO_TS_CONFIG             0x0D

#define AD5593_SW_RESET                 0x0F

//  IO REGISTERS
#define AD5593_DAC_WRITE(x)             (0x10 + (x))
#define AD5593_ADC_READ                 0x40
#define AD5593_DAC_READ(x)              (0x50 + (x))
#define AD5593_GPIO_READ                0x60
#define AD5593_GPIO_READ_CONFIG         0x70



AD5593R::AD5593R(const uint8_t deviceAddress, TwoWire *wire)
{
  _address    = deviceAddress;
  _wire       = wire;
  _error      = AD5593R_OK;
}

bool AD5593R::begin()
{
  if (! isConnected()) return false;
  return true;
}

bool AD5593R::isConnected()
{
  _wire->beginTransmission(_address);
  return ( _wire->endTransmission() == 0);
}

uint8_t AD5593R::getAddress()
{
  return _address;
}


////////////////////////////////////////////////////////////
//
//  CONFIGURE ADC / DAC RANGE
//
int AD5593R::setADCRange2x(bool flag)
{
  uint16_t bitMask = readRegister(AD5593_GEN_CTRL_REG);
  if (flag) bitMask |= 0x0020;
  else bitMask &= ~0x0020;
  return writeRegister(AD5593_GEN_CTRL_REG, bitMask);
}

int AD5593R::setDACRange2x(bool flag)
{
  uint16_t bitMask = readRegister(AD5593_GEN_CTRL_REG);
  if (flag) bitMask |= 0x0010;
  else bitMask &= ~0x0010;
  return writeRegister(AD5593_GEN_CTRL_REG, bitMask);
}


////////////////////////////////////////////////////////////
//
//  MODE
//
int AD5593R::setMode(const char config[9])
{
  //  all channels need to be addressed
  if (strlen(config) != 8) return -1;
  uint8_t bitMaskDAC = 0x00;
  uint8_t bitMaskADC = 0x00;
  uint8_t bitMaskIn  = 0x00;
  uint8_t bitMaskOut = 0x00;

  //  parse configuration string.
  uint8_t bm = 0x01;
  for (int i = 0; i < 8; i++)
  {
    switch(config[i])
    {
      case 'a':
      case 'A': bitMaskADC |= bm; break;
      case 'd':
      case 'D': bitMaskDAC |= bm; break;
      case 'i':
      case 'I': bitMaskIn  |= bm; break;
      case 'o':
      case 'O': bitMaskOut |= bm; break;
      default:  break;
    }
    bm <<= 1;
  }
  //  TODO handle return values.
  setADCmode(bitMaskADC);
  setDACmode(bitMaskDAC);
  setINPUTmode(bitMaskIn);
  setOUTPUTmode(bitMaskOut);
  return 0;
}

int AD5593R::setADCmode(uint8_t bitMask)
{
  //  Page 25 / 32
  return writeRegister(AD5593_ADC_CONFIG, bitMask);
}

int AD5593R::setDACmode(uint8_t bitMask)
{
  //  Page 35
  return writeRegister(AD5593_DAC_CONFIG, bitMask);
}

int AD5593R::setINPUTmode(uint8_t bitMask)
{
  //  Page 26
  //  1's => INPUT
  return writeRegister(AD5593_GPIO_INPUT, bitMask);
}

int AD5593R::setOUTPUTmode(uint8_t bitMask)
{
  //  Page 26
  //  1's => OUTPUT
  return writeRegister(AD5593_GPIO_CONFIG, bitMask);

  //  not implemented yet (flag or 2nd bitmap?)
  //  GPIO_OPENDRAIN_CONFIG  Page 26/42
  //  set default values for output? write8(0x0000);
  //  IO_TS_CONFIG           Page 42  3e bitMask?
}

int AD5593R::setPULLDOWNmode(uint8_t bitMask)
{
  //  page 36
  return writeRegister(AD5593_PULLDOWN_CONFIG, bitMask);
}

//  TODO - latch / direct DAC.
//  int AD5593R::setLDACmode( ??? );
//  check DAC pins?

//  TODO - opendrain - output mode - page 26 - pull up resistor needed.
//  int AD5593R::setOpenDrainMode( ??? );
//  check output pins?


////////////////////////////////////////////////////////////
//
//  DIGITAL
//
//  Page 26
uint16_t AD5593R::write1(uint8_t pin, uint8_t value)
{
  if (pin > 7) return AD5593R_PIN_ERROR;
  //  TODO does the read works?
  uint8_t bitMask = readRegister(AD5593_GPIO_OUTPUT);
  if (value == LOW) bitMask &= ~(1 << pin);
  else              bitMask |= (1 << pin);
  return writeRegister(AD5593_GPIO_OUTPUT, bitMask);
}

uint16_t AD5593R::read1(uint8_t pin)
{
  if (pin > 7) return AD5593R_PIN_ERROR;
  uint8_t bitMask = readRegister(AD5593_GPIO_READ);
  return (bitMask >> pin) & 0x01;
}

uint16_t AD5593R::write8(uint8_t bitMask)
{
  return writeRegister(AD5593_GPIO_OUTPUT, bitMask);
}

uint16_t AD5593R::read8()
{
  return readRegister(AD5593_GPIO_READ);
}


////////////////////////////////////////////////////////////
//
//  ANALOG
//
uint16_t AD5593R::writeDAC(uint8_t pin, uint16_t value)
{
  if (pin > 7) return AD5593R_PIN_ERROR;
  //  max 12 bit == 4095 => clipping
  if (value > 0x0FFF)
  {
    value = 0x0FFF;
  }
  //  TODO do we need this?
  //  return writeRegister(AD5593_DAC_WRITE(pin), value | 0x8000 | (pin << 12));
  return writeRegister(AD5593_DAC_WRITE(pin), value);
}

uint16_t AD5593R::readDAC(uint8_t pin)
{
  if (pin > 7) return AD5593R_PIN_ERROR;
  return readRegister(AD5593_DAC_READ(pin));
}

uint16_t AD5593R::readADC(uint8_t pin)
{
  if (pin > 7) return AD5593R_PIN_ERROR;
  //  add all to the sequence including temperature.
  //  0x0200 = REPeat bit
  //  0x0100 = TEMPerature include bit
  //  TODO  0x0000 or 0x0200?
  writeRegister(AD5593_ADC_SEQ, 0x0200 | (1 << pin));
  //  read one ADC conversion.
  return readRegister(AD5593_ADC_READ);
}

uint16_t AD5593R::readTemperature()
{
  //  0x0200 = REPeat bit
  //  0x0100 = TEMPerature include bit
  writeRegister(AD5593_ADC_SEQ, 0x0300);
  //  read one ADC conversion.
  //  TODO mapping to °C
  return readRegister(AD5593_ADC_READ);
}


////////////////////////////////////////////////////////////
//
//  V-REFERENCE
//
int AD5593R::setExternalReference(bool flag, float Vref)
{
  //  Page 40
  uint8_t bitMask = readRegister(AD5593_POWERDOWN_REF_CTRL);
  if (flag)  //  external
  {
    bitMask &= ~0x0200;
    _Vref = Vref;
  }
  else       //  internal
  {
    bitMask |= 0x0200;
    _Vref = 2.5;
  }
  return writeRegister(AD5593_POWERDOWN_REF_CTRL, bitMask);
}

int AD5593R::powerDown()
{
  //  Page 40
  uint8_t bitMask = readRegister(AD5593_POWERDOWN_REF_CTRL);
  bitMask |= 0x0400;
  return writeRegister(AD5593_POWERDOWN_REF_CTRL, bitMask);
}

int AD5593R::wakeUp()
{
  _Vref = 2.5;
  //  Page 40
  uint8_t bitMask = readRegister(AD5593_POWERDOWN_REF_CTRL);
  bitMask &= ~0x0400;
  return writeRegister(AD5593_POWERDOWN_REF_CTRL, bitMask);
}

//  TODO
//int AD5593R::powerDownDacChannel(uint8_t channel)
//{
//  Page 40
//}


////////////////////////////////////////////////////////////
//
//  RESET
//
int AD5593R::reset()
{
  //  page 19
  return writeRegister(AD5593_SW_RESET, 0x0DAC);
  _Vref = 2.5;
}


////////////////////////////////////////////////////////////
//
//  PROTECTED
//
//  Figure 36, page 20
int AD5593R::writeRegister(uint8_t reg, uint16_t data)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _wire->write(data >> 8);
  _wire->write(data & 0xFF);
  _error = _wire->endTransmission();
  return _error;
}


uint16_t AD5593R::readRegister(uint8_t reg)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _error = _wire->endTransmission();
  if (_wire->requestFrom(_address, (uint8_t)2) != 2)
  {
    _error = AD5593R_I2C_ERROR;
    return 0;
  }
  uint16_t data = _wire->read() << 8;
  data += _wire->read();
  return data;
}

//  -- END OF FILE --

