#ifndef LCD_ILI_9481_H
#define LCD_ILI_9481_H

#include <Arduino.h>


class LcdIli9481
{
public:
  void begin();

  // returns device ID
  uint16_t info();

  uint16_t width() const { return LCD_WIDTH; }
  uint16_t height() const { return LCD_HEIGHT; }

  void setInvert(bool b);

  void rotate(int degrees);

  void setOn(bool b);

  void clear(uint16_t c);
  void clearBlack();

  void drawText(const char* t, uint16_t x, unsigned y, uint16_t c);
  void drawHorizontalLine(uint16_t x, uint16_t y, uint16_t l, uint16_t c);
  void drawVerticalLine(uint16_t x, uint16_t y, uint16_t l, uint16_t c);
  void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c);
  void drawFilledRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c);

private:
  void setDataOutputDirection();
  void setDataInputDirection();

  void writeToBus(uint8_t d);
  uint8_t readFromBus();
  
  void writeCmd(uint8_t d);
  void writeData(uint8_t d);

  void setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

private:
  uint16_t LCD_WIDTH = 320;
  uint16_t LCD_HEIGHT = 480;
};


#endif

