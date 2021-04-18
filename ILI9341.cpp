#include "ILI9341.h"
#include <chrono>
#include <cstdarg>
#include <cstdint>

/*
  ILI9341(SPI*, PinName, PinName, PinName) initializes class and all needed pins for ILI9341 Display.
*/
ILI9341::ILI9341(PinName mosi, PinName miso, PinName clk, PinName cs, PinName rst, PinName dc) : spi(mosi, miso, clk), chipSelect(cs), reset(rst), dataCommand(dc)
{
  orientation = 0;
  width = ILI9341_TFTWIDTH;
  height = ILI9341_TFTHEIGHT;
}

// ILI9341 initialization commands (Source: https://github.com/adafruit/Adafruit_ILI9341/blob/master/Adafruit_ILI9341.cpp)
static const uint8_t initCommands[] = 
{
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  ILI9341_PWCTR1, 1, 0x23,
  ILI9341_PWCTR2, 1, 0x10,
  ILI9341_VMCTR1, 2, 0x3E, 0x28,
  ILI9341_VMCTR2, 1, 0x86,
  ILI9341_MADCTL, 1, (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR),
  ILI9341_VSCRSADD, 1, 0x00,
  ILI9341_PIXFMT, 1, 0x55,
  ILI9341_FRMCTR1, 2, 0x00, 0x18,
  ILI9341_DFUNCTR, 3, 0x08, 0x82, 0x27,
  0xF2, 1, 0x00,
  ILI9341_GAMMASET, 1, 0x01,
  ILI9341_GMCTRP1, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  ILI9341_GMCTRN1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT, 0x00,
  ILI9341_DISPON, 0x00,
  0x00 
};

// Adafruit GFX standard ASCII 5x7 font (Source: https://github.com/adafruit/Adafruit-GFX-Library/blob/master/glcdfont.c)
static const unsigned char font[] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x5B, 0x4F, 0x5B, 0x3E, 0x3E, 0x6B,
  0x4F, 0x6B, 0x3E, 0x1C, 0x3E, 0x7C, 0x3E, 0x1C, 0x18, 0x3C, 0x7E, 0x3C,
  0x18, 0x1C, 0x57, 0x7D, 0x57, 0x1C, 0x1C, 0x5E, 0x7F, 0x5E, 0x1C, 0x00,
  0x18, 0x3C, 0x18, 0x00, 0xFF, 0xE7, 0xC3, 0xE7, 0xFF, 0x00, 0x18, 0x24,
  0x18, 0x00, 0xFF, 0xE7, 0xDB, 0xE7, 0xFF, 0x30, 0x48, 0x3A, 0x06, 0x0E,
  0x26, 0x29, 0x79, 0x29, 0x26, 0x40, 0x7F, 0x05, 0x05, 0x07, 0x40, 0x7F,
  0x05, 0x25, 0x3F, 0x5A, 0x3C, 0xE7, 0x3C, 0x5A, 0x7F, 0x3E, 0x1C, 0x1C,
  0x08, 0x08, 0x1C, 0x1C, 0x3E, 0x7F, 0x14, 0x22, 0x7F, 0x22, 0x14, 0x5F,
  0x5F, 0x00, 0x5F, 0x5F, 0x06, 0x09, 0x7F, 0x01, 0x7F, 0x00, 0x66, 0x89,
  0x95, 0x6A, 0x60, 0x60, 0x60, 0x60, 0x60, 0x94, 0xA2, 0xFF, 0xA2, 0x94,
  0x08, 0x04, 0x7E, 0x04, 0x08, 0x10, 0x20, 0x7E, 0x20, 0x10, 0x08, 0x08,
  0x2A, 0x1C, 0x08, 0x08, 0x1C, 0x2A, 0x08, 0x08, 0x1E, 0x10, 0x10, 0x10,
  0x10, 0x0C, 0x1E, 0x0C, 0x1E, 0x0C, 0x30, 0x38, 0x3E, 0x38, 0x30, 0x06,
  0x0E, 0x3E, 0x0E, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5F,
  0x00, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x14, 0x7F, 0x14, 0x7F, 0x14,
  0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x23, 0x13, 0x08, 0x64, 0x62, 0x36, 0x49,
  0x56, 0x20, 0x50, 0x00, 0x08, 0x07, 0x03, 0x00, 0x00, 0x1C, 0x22, 0x41,
  0x00, 0x00, 0x41, 0x22, 0x1C, 0x00, 0x2A, 0x1C, 0x7F, 0x1C, 0x2A, 0x08,
  0x08, 0x3E, 0x08, 0x08, 0x00, 0x80, 0x70, 0x30, 0x00, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x00, 0x00, 0x60, 0x60, 0x00, 0x20, 0x10, 0x08, 0x04, 0x02,
  0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x42, 0x7F, 0x40, 0x00, 0x72, 0x49,
  0x49, 0x49, 0x46, 0x21, 0x41, 0x49, 0x4D, 0x33, 0x18, 0x14, 0x12, 0x7F,
  0x10, 0x27, 0x45, 0x45, 0x45, 0x39, 0x3C, 0x4A, 0x49, 0x49, 0x31, 0x41,
  0x21, 0x11, 0x09, 0x07, 0x36, 0x49, 0x49, 0x49, 0x36, 0x46, 0x49, 0x49,
  0x29, 0x1E, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x40, 0x34, 0x00, 0x00,
  0x00, 0x08, 0x14, 0x22, 0x41, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 0x41,
  0x22, 0x14, 0x08, 0x02, 0x01, 0x59, 0x09, 0x06, 0x3E, 0x41, 0x5D, 0x59,
  0x4E, 0x7C, 0x12, 0x11, 0x12, 0x7C, 0x7F, 0x49, 0x49, 0x49, 0x36, 0x3E,
  0x41, 0x41, 0x41, 0x22, 0x7F, 0x41, 0x41, 0x41, 0x3E, 0x7F, 0x49, 0x49,
  0x49, 0x41, 0x7F, 0x09, 0x09, 0x09, 0x01, 0x3E, 0x41, 0x41, 0x51, 0x73,
  0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x41, 0x7F, 0x41, 0x00, 0x20, 0x40,
  0x41, 0x3F, 0x01, 0x7F, 0x08, 0x14, 0x22, 0x41, 0x7F, 0x40, 0x40, 0x40,
  0x40, 0x7F, 0x02, 0x1C, 0x02, 0x7F, 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x3E,
  0x41, 0x41, 0x41, 0x3E, 0x7F, 0x09, 0x09, 0x09, 0x06, 0x3E, 0x41, 0x51,
  0x21, 0x5E, 0x7F, 0x09, 0x19, 0x29, 0x46, 0x26, 0x49, 0x49, 0x49, 0x32,
  0x03, 0x01, 0x7F, 0x01, 0x03, 0x3F, 0x40, 0x40, 0x40, 0x3F, 0x1F, 0x20,
  0x40, 0x20, 0x1F, 0x3F, 0x40, 0x38, 0x40, 0x3F, 0x63, 0x14, 0x08, 0x14,
  0x63, 0x03, 0x04, 0x78, 0x04, 0x03, 0x61, 0x59, 0x49, 0x4D, 0x43, 0x00,
  0x7F, 0x41, 0x41, 0x41, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x41, 0x41,
  0x41, 0x7F, 0x04, 0x02, 0x01, 0x02, 0x04, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x00, 0x03, 0x07, 0x08, 0x00, 0x20, 0x54, 0x54, 0x78, 0x40, 0x7F, 0x28,
  0x44, 0x44, 0x38, 0x38, 0x44, 0x44, 0x44, 0x28, 0x38, 0x44, 0x44, 0x28,
  0x7F, 0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 0x08, 0x7E, 0x09, 0x02, 0x18,
  0xA4, 0xA4, 0x9C, 0x78, 0x7F, 0x08, 0x04, 0x04, 0x78, 0x00, 0x44, 0x7D,
  0x40, 0x00, 0x20, 0x40, 0x40, 0x3D, 0x00, 0x7F, 0x10, 0x28, 0x44, 0x00,
  0x00, 0x41, 0x7F, 0x40, 0x00, 0x7C, 0x04, 0x78, 0x04, 0x78, 0x7C, 0x08,
  0x04, 0x04, 0x78, 0x38, 0x44, 0x44, 0x44, 0x38, 0xFC, 0x18, 0x24, 0x24,
  0x18, 0x18, 0x24, 0x24, 0x18, 0xFC, 0x7C, 0x08, 0x04, 0x04, 0x08, 0x48,
  0x54, 0x54, 0x54, 0x24, 0x04, 0x04, 0x3F, 0x44, 0x24, 0x3C, 0x40, 0x40,
  0x20, 0x7C, 0x1C, 0x20, 0x40, 0x20, 0x1C, 0x3C, 0x40, 0x30, 0x40, 0x3C,
  0x44, 0x28, 0x10, 0x28, 0x44, 0x4C, 0x90, 0x90, 0x90, 0x7C, 0x44, 0x64,
  0x54, 0x4C, 0x44, 0x00, 0x08, 0x36, 0x41, 0x00, 0x00, 0x00, 0x77, 0x00,
  0x00, 0x00, 0x41, 0x36, 0x08, 0x00, 0x02, 0x01, 0x02, 0x04, 0x02, 0x3C,
  0x26, 0x23, 0x26, 0x3C, 0x1E, 0xA1, 0xA1, 0x61, 0x12, 0x3A, 0x40, 0x40,
  0x20, 0x7A, 0x38, 0x54, 0x54, 0x55, 0x59, 0x21, 0x55, 0x55, 0x79, 0x41,
  0x22, 0x54, 0x54, 0x78, 0x42, // a-umlaut
  0x21, 0x55, 0x54, 0x78, 0x40, 0x20, 0x54, 0x55, 0x79, 0x40, 0x0C, 0x1E,
  0x52, 0x72, 0x12, 0x39, 0x55, 0x55, 0x55, 0x59, 0x39, 0x54, 0x54, 0x54,
  0x59, 0x39, 0x55, 0x54, 0x54, 0x58, 0x00, 0x00, 0x45, 0x7C, 0x41, 0x00,
  0x02, 0x45, 0x7D, 0x42, 0x00, 0x01, 0x45, 0x7C, 0x40, 0x7D, 0x12, 0x11,
  0x12, 0x7D, // A-umlaut
  0xF0, 0x28, 0x25, 0x28, 0xF0, 0x7C, 0x54, 0x55, 0x45, 0x00, 0x20, 0x54,
  0x54, 0x7C, 0x54, 0x7C, 0x0A, 0x09, 0x7F, 0x49, 0x32, 0x49, 0x49, 0x49,
  0x32, 0x3A, 0x44, 0x44, 0x44, 0x3A, // o-umlaut
  0x32, 0x4A, 0x48, 0x48, 0x30, 0x3A, 0x41, 0x41, 0x21, 0x7A, 0x3A, 0x42,
  0x40, 0x20, 0x78, 0x00, 0x9D, 0xA0, 0xA0, 0x7D, 0x3D, 0x42, 0x42, 0x42,
  0x3D, // O-umlaut
  0x3D, 0x40, 0x40, 0x40, 0x3D, 0x3C, 0x24, 0xFF, 0x24, 0x24, 0x48, 0x7E,
  0x49, 0x43, 0x66, 0x2B, 0x2F, 0xFC, 0x2F, 0x2B, 0xFF, 0x09, 0x29, 0xF6,
  0x20, 0xC0, 0x88, 0x7E, 0x09, 0x03, 0x20, 0x54, 0x54, 0x79, 0x41, 0x00,
  0x00, 0x44, 0x7D, 0x41, 0x30, 0x48, 0x48, 0x4A, 0x32, 0x38, 0x40, 0x40,
  0x22, 0x7A, 0x00, 0x7A, 0x0A, 0x0A, 0x72, 0x7D, 0x0D, 0x19, 0x31, 0x7D,
  0x26, 0x29, 0x29, 0x2F, 0x28, 0x26, 0x29, 0x29, 0x29, 0x26, 0x30, 0x48,
  0x4D, 0x40, 0x20, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x38, 0x2F, 0x10, 0xC8, 0xAC, 0xBA, 0x2F, 0x10, 0x28, 0x34, 0xFA, 0x00,
  0x00, 0x7B, 0x00, 0x00, 0x08, 0x14, 0x2A, 0x14, 0x22, 0x22, 0x14, 0x2A,
  0x14, 0x08, 0x55, 0x00, 0x55, 0x00, 0x55, // #176 (25% block) missing in old code
  0xAA, 0x55, 0xAA, 0x55, 0xAA,             // 50% block
  0xFF, 0x55, 0xFF, 0x55, 0xFF,             // 75% block
  0x00, 0x00, 0x00, 0xFF, 0x00, 0x10, 0x10, 0x10, 0xFF, 0x00, 0x14, 0x14,
  0x14, 0xFF, 0x00, 0x10, 0x10, 0xFF, 0x00, 0xFF, 0x10, 0x10, 0xF0, 0x10,
  0xF0, 0x14, 0x14, 0x14, 0xFC, 0x00, 0x14, 0x14, 0xF7, 0x00, 0xFF, 0x00,
  0x00, 0xFF, 0x00, 0xFF, 0x14, 0x14, 0xF4, 0x04, 0xFC, 0x14, 0x14, 0x17,
  0x10, 0x1F, 0x10, 0x10, 0x1F, 0x10, 0x1F, 0x14, 0x14, 0x14, 0x1F, 0x00,
  0x10, 0x10, 0x10, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x10, 0x10, 0x10,
  0x10, 0x1F, 0x10, 0x10, 0x10, 0x10, 0xF0, 0x10, 0x00, 0x00, 0x00, 0xFF,
  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0xFF, 0x10, 0x00,
  0x00, 0x00, 0xFF, 0x14, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x1F,
  0x10, 0x17, 0x00, 0x00, 0xFC, 0x04, 0xF4, 0x14, 0x14, 0x17, 0x10, 0x17,
  0x14, 0x14, 0xF4, 0x04, 0xF4, 0x00, 0x00, 0xFF, 0x00, 0xF7, 0x14, 0x14,
  0x14, 0x14, 0x14, 0x14, 0x14, 0xF7, 0x00, 0xF7, 0x14, 0x14, 0x14, 0x17,
  0x14, 0x10, 0x10, 0x1F, 0x10, 0x1F, 0x14, 0x14, 0x14, 0xF4, 0x14, 0x10,
  0x10, 0xF0, 0x10, 0xF0, 0x00, 0x00, 0x1F, 0x10, 0x1F, 0x00, 0x00, 0x00,
  0x1F, 0x14, 0x00, 0x00, 0x00, 0xFC, 0x14, 0x00, 0x00, 0xF0, 0x10, 0xF0,
  0x10, 0x10, 0xFF, 0x10, 0xFF, 0x14, 0x14, 0x14, 0xFF, 0x14, 0x10, 0x10,
  0x10, 0x1F, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x10, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x38, 0x44, 0x44,
  0x38, 0x44, 0xFC, 0x4A, 0x4A, 0x4A, 0x34, // sharp-s or beta
  0x7E, 0x02, 0x02, 0x06, 0x06, 0x02, 0x7E, 0x02, 0x7E, 0x02, 0x63, 0x55,
  0x49, 0x41, 0x63, 0x38, 0x44, 0x44, 0x3C, 0x04, 0x40, 0x7E, 0x20, 0x1E,
  0x20, 0x06, 0x02, 0x7E, 0x02, 0x02, 0x99, 0xA5, 0xE7, 0xA5, 0x99, 0x1C,
  0x2A, 0x49, 0x2A, 0x1C, 0x4C, 0x72, 0x01, 0x72, 0x4C, 0x30, 0x4A, 0x4D,
  0x4D, 0x30, 0x30, 0x48, 0x78, 0x48, 0x30, 0xBC, 0x62, 0x5A, 0x46, 0x3D,
  0x3E, 0x49, 0x49, 0x49, 0x00, 0x7E, 0x01, 0x01, 0x01, 0x7E, 0x2A, 0x2A,
  0x2A, 0x2A, 0x2A, 0x44, 0x44, 0x5F, 0x44, 0x44, 0x40, 0x51, 0x4A, 0x44,
  0x40, 0x40, 0x44, 0x4A, 0x51, 0x40, 0x00, 0x00, 0xFF, 0x01, 0x03, 0xE0,
  0x80, 0xFF, 0x00, 0x00, 0x08, 0x08, 0x6B, 0x6B, 0x08, 0x36, 0x12, 0x36,
  0x24, 0x36, 0x06, 0x0F, 0x09, 0x0F, 0x06, 0x00, 0x00, 0x18, 0x18, 0x00,
  0x00, 0x00, 0x10, 0x10, 0x00, 0x30, 0x40, 0xFF, 0x01, 0x01, 0x00, 0x1F,
  0x01, 0x01, 0x1E, 0x00, 0x19, 0x1D, 0x17, 0x12, 0x00, 0x3C, 0x3C, 0x3C,
  0x3C, 0x00, 0x00, 0x00, 0x00, 0x00 // #255 NBSP
};

/*
  void initialize(void) executes initialization commands for ILI9341 chip.
*/
void ILI9341::initialize(void)
{
  spi.format(8, 3);
  spi.frequency(40000000);
  chipSelect = 1;
  dataCommand = 1;
  
  reset = 1;
  ThisThread::sleep_for(chrono::milliseconds(5));
  reset = 0;
  ThisThread::sleep_for(chrono::milliseconds(20));
  reset = 1;
  ThisThread::sleep_for(chrono::milliseconds(150));
  
  // Execute initialization commands for IL9341 chip
  uint8_t index = 0x00;
  uint8_t cmd = 0x00;
  uint8_t numArgs = 0x00;
  while((cmd = initCommands[index++]) > 0x00)
  {
    numArgs = initCommands[index++];

    writeCommand(cmd);

    if(numArgs == 0x00)
    {
      ThisThread::sleep_for(chrono::milliseconds(150));
    }
    else
    {
      uint8_t oldIndex = index;
      for(; index < oldIndex + numArgs; index++)
      {
        spi.write(initCommands[index]);
      }
    }

    chipSelect = 1; 
  }
}

/*
  void setAddrWindow(uint16_t, uint16_t, uint16_t, uint16_t) define an area to recieve a stream of pixels.
*/
void ILI9341::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint16_t x2 = (x + w - 1);
  uint16_t y2 = (y + h - 1);

  writeCommand(ILI9341_CASET);  // Column address set

  spi.format(16, 3);
  spi.write(x);
  spi.write(x2);
  spi.format(8, 3);
  chipSelect = 1;

  writeCommand(ILI9341_PASET);  // Row address set

  spi.format(16, 3);
  spi.write(y);
  spi.write(y2);
  spi.format(8, 3);
  chipSelect = 1;

  writeCommand(ILI9341_RAMWR);  // Write to RAM
}

/*
  void writeCommand(uint8_t) writes a ILI9341 command via SPI.
*/
void ILI9341::writeCommand(uint8_t cmd)
{
  dataCommand = 0;
  chipSelect = 0;
  spi.write(cmd);
  dataCommand = 1;
}

/*
  void drawPixel(uint16_t, uint16_t, uint16_t) draws a pixel with a specific color on the display.
*/
void ILI9341::drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  setAddrWindow(x, y, 1, 1);

  spi.format(16, 3);
  spi.write(color);
  spi.format(8, 3);

  chipSelect = 1;
}

/*
  void drawVLine(uint16_t, uint16_t, uint16_t, uint16_t) draws a vertical line on the display.
*/
void ILI9341::drawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color)
{
  setAddrWindow(x, y, 1, h);

  spi.format(16, 3);
  for(auto i = 0; i < h; i++)
  {
    spi.write(color);
  }
  spi.format(8, 3);

  chipSelect = 1;
}

/*
  void drawHLine(uint16_t, uint16_t, uint16_t, uint16_t) draws a horizontal line on the display.
*/
void ILI9341::drawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{
  setAddrWindow(x, y, w, 1);

  spi.format(16, 3);
  for(auto i = 0; i < w; i++)
  {
    spi.write(color);
  }
  spi.format(8, 3);

  chipSelect = 1;
}

/*
  void setRotation(uint8_t) sets rotation of display (input from 0-4)
*/
void ILI9341::setRotation(uint8_t rot)
{
  uint8_t rotation = rot % 4;

  writeCommand(ILI9341_MADCTL);
  switch(rotation)
  {
    case 0:
      spi.write(ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR);
      width = ILI9341_TFTWIDTH;
      height = ILI9341_TFTHEIGHT;
      break;
    case 1:
      spi.write(ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR);
      width = ILI9341_TFTHEIGHT;
      height = ILI9341_TFTWIDTH;
      break;
    case 2:
      spi.write(ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR);
      width = ILI9341_TFTWIDTH;
      height = ILI9341_TFTHEIGHT;
      break;
    case 3:
      spi.write(ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR);
      width = ILI9341_TFTHEIGHT;
      height = ILI9341_TFTWIDTH;
      break;
  }

  chipSelect = 1;
}

/*
  void drawRectangle(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t) draws a rectangle on the display.
*/
void ILI9341::drawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
  drawHLine(x, y, w, color);
  drawHLine(x, y + h - 1, w, color);
  drawVLine(x, y, h, color);
  drawVLine(x + w - 1, y, h, color);
}

/*
  void fillRectangle(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t) draws a filled rectangle on the display. 
*/
void ILI9341::fillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
  setAddrWindow(x, y, w, h);

  spi.format(16, 3);
  for(auto i = 0; i < h; i++)
  {
    for(auto j = 0; j < w; j++)
    {
      spi.write(color);
    }
  }
  spi.format(8, 3);

  chipSelect = 1;
}

/*
  void fillBackground(uint16_t) fills the background of the display with a specific color.
  While sending color pixels display will be turned off so that the long drawing animation 
  will not be shown.
*/
void ILI9341::fillBackground(uint16_t color)
{
  writeCommand(ILI9341_DISPOFF);
  fillRectangle(0, 0, width, height, color);
  writeCommand(ILI9341_DISPON);
}

/*
  void drawCircleHelper(uint16_t, uint16_t, uint16_t, uint8_t, uint16_t) draws a circle (corners are selectable)
  using Bresenham algorythm.

  Source: https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/
*/
void ILI9341::drawCircleHelper(uint16_t xc, uint16_t yc, uint16_t r, uint8_t corners, uint16_t color)
{
  int16_t x = 0;
  int16_t y = r;
  int16_t d = 3 - 2 * r;

  while(y >= x)
  {
    x++;

    if(d > 0)
    {
      y--;
      d = d + 4 * (x - y) + 10;
    }
    else
    {
      d = d + 4 * x + 6;
    }

    if(corners & 0x01)
    {
      drawPixel(xc - y, yc - x, color);
      drawPixel(xc - x, yc - y, color);
    }
    if(corners & 0x02)
    {
      drawPixel(xc + x, yc - y, color);
      drawPixel(xc + y, yc - x, color);
    }
    if(corners & 0x04)
    {
      drawPixel(xc + x, yc + y, color);
      drawPixel(xc + y, yc + x, color);
    }
    if(corners & 0x08)
    {
      drawPixel(xc - y, yc + x, color);
      drawPixel(xc - x, yc + y, color);
    }
  }
}

/*
  void drawCircle(uint16_t, uint16_t, uint16_t, uint16_t) draws a circle to the display.
*/
void ILI9341::drawCircle(uint16_t xc, uint16_t yc, uint16_t r, uint16_t color)
{
  drawCircleHelper(xc, yc, r, 0xF, color);
}

/*
  void fillCircleHelper(uint16_t, uint16_t, uint16_t, uint8_t, uint16_t) draws a filled circle 
  (top half and bottom half are selectable) using Bresenham algorythm.

  Source: https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/
*/
void ILI9341::fillCircleHelper(uint16_t xc, uint16_t yc, uint16_t r, uint8_t corners, uint16_t color)
{
  int16_t x = 0;
  int16_t y = r;
  int16_t d = 3 - 2 * r;

  while(y >= x)
  {
    x++;

    if(d > 0)
    {
      y--;
      d = d + 4 * (x - y) + 10;
    }
    else
    {
      d = d + 4 * x + 6;
    }

    if(corners & 0x01)
    {
      drawVLine(xc + x, yc - y, 2 * y, color);
      drawVLine(xc + y, yc - x, 2 * x, color);
    }
    if(corners & 0x02)
    {
      drawVLine(xc - x, yc - y, 2 * y, color);
      drawVLine(xc - y, yc - x, 2 * x, color);
    }
  }
}

/*
  void fillCircle(uint16_t, uint16_t, uint16_t, uint16_t) draws a filled circle to the display.
*/
void ILI9341::fillCircle(uint16_t xc, uint16_t yc, uint16_t r, uint16_t color)
{
  drawVLine(xc, yc - r, 2 * r, color);
  fillCircleHelper(xc, yc, r, 0x03, color);
}

/*
  int8_t signumFunc(int16_t) returns sign of a number.
*/
int8_t ILI9341::signumFunc(int16_t x)
{
  return (x > 0) ? 1 : (x < 0) ? -1 : 0;
}

/*
  void drawLine(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t) draws a line on the display using Bresenham algorythm.

  Source: https://de.wikipedia.org/wiki/Bresenham-Algorithmus
*/
void ILI9341::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
  int x, y;
  int dx, dy;     // Distance between points in both dimensions
  int incx, incy; // Sign of increment 
  int pdx, pdy;   // Parallelstep in x/y dimension
  int ddx, ddy;   // Diagonalstep ub x/y dimension
  int dsd;        // Delta-Slow-Direction
  int dfd;        // Delta-Fast-Direction
  int err;        // Error

  dx = x1 - x0;
  dy = y1 - y0;

  incx = signumFunc(dx);
  incy = signumFunc(dy);

  if(dx < 0)
  {
    dx = -dx;
  }

  if(dy < 0)
  {
    dy = -dy;
  }

  if(dx > dy)
  {
    pdx = incx; pdy = 0;
    ddx = incx; ddy = incy;
    dsd = dy;
    dfd = dx;
  }
  else
  {
    pdx = 0; pdy = incy;
    ddx = incx; ddy = incy;
    dsd = dx;
    dfd = dy;
  }

  x = x0;
  y = y0;
  err = dfd / 2;
  drawPixel(x, y, color);

  for(int t = 0; t < dfd; ++t)
  {
    err -= dsd;

    if(err < 0)
    {
      err += dfd;
      x += ddx;
      y += ddy;
    }
    else
    {
      x += pdx;
      y += pdy;
    }

    drawPixel(x, y, color);
  }
}

/*
  void drawTriangle(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t) draws a triangle on the display.
*/
void ILI9341::drawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

/*
  void fillTriangle(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t) draws a filled triangle on the display.
*/
void ILI9341::fillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if(y0 > y1)
  {
    swap(y0, y1);
    swap(x0, x1);
  }

  if(y1 > y2)
  {
    swap(y2, y1);
    swap(x2, x1);
  }

  if(y0 > y1)
  {
    swap(y0, y1);
    swap(x0, x1);
  }

  if(y0 == y2)
  {
    // Handle awkward all-on-same-line case as its own thing
    a = b = x0;

    if(x1 < a)
    {
      a = x1;
    }     
    else if(x1 > b)
    {
      b = x1;
    }
    if(x2 < a)
    {
      a = x2;
    }
    else if(x2 > b)
    {
      b = x2;
    }

    drawHLine(a, y0, b - a + 1, color);
    return;
  }

  int16_t dx01 = x1 - x0;
  int16_t dy01 = y1 - y0;
  int16_t dx02 = x2 - x0;
  int16_t dy02 = y2 - y0;
  int16_t dx12 = x2 - x1;
  int16_t dy12 = y2 - y1;
  int16_t sa   = 0;
  int16_t sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if(y1 == y2)
  {
    last = y1;  // Include y1 scanline
  }
  else
  {
    last = y1 - 1; // Skip it
  }

  for(y = y0; y <= last; y++)
  {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;

    if(a > b)
    {
      swap(a, b);
    }

    drawHLine(a, y, b - a + 1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for(; y <= y2; y++)
  {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;

    if(a > b)
    {
      swap(a, b);
    }

    drawHLine(a, y, b - a + 1, color);
  }
}

/*
  void drawChar(uint16_t, uint16_t, uint16_t, unsigned char, uint16_t, uint16_t, uint16_t) draws an ASCII character on the display.
*/
void ILI9341::drawChar(uint16_t x, uint16_t y, unsigned char c, uint16_t size, uint16_t foreColor, uint16_t backColor)
{
  if((x >= width) ||
    (y >= height) ||
    ((x + 6 * size - 1) < 0) ||
    ((y + 8 * size - 1) < 0))
  {
    return;
  }

  for(int8_t i = 0; i < 5; i++)
  {
    uint8_t line = font[c * 5 + i];

    for(int8_t j = 0; j < 8; j++)
    {
      if(line & 0x01)
      {
        if(size == 1)
        {
          drawPixel(x + i, y + j, foreColor);
        }
        else
        {
          fillRectangle(x + i * size, y + j * size, size, size, foreColor);
        }
      }
      else if(backColor != foreColor)
      {
        if(size == 1)
        {
          drawPixel(x + i, y + j, backColor);
        }
        else
        {
          fillRectangle(x + i * size, y + j * size, size, size, backColor);
        }
      }

      line >>= 1;
    }
  }
}

/*
  void drawString(uint16_t, uint16_t, const char*, uint16_t, uint16_t, uint16_t, uint16_t) draws a string on the display.
*/
void ILI9341::drawString(uint16_t x, uint16_t y, const char* str, uint16_t strSize, uint16_t charSize, uint16_t foreColor, uint16_t backColor)
{
  uint16_t xi = x;
  for(auto i = 0; i < strSize; i++)
  {
    drawChar(xi, y, str[i], charSize, foreColor, backColor);
    xi += 5 * charSize + 1; // 5 * charSize is the width of one drawn character + 1 for space between character.
  }
}