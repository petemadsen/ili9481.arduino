#ifndef LCD_ILI_9481_H
#define LCD_ILI_9481_H

#include <Arduino.h>


class LcdIli9481
{
public:
  void begin();

  void info();

  unsigned int width() const { return LCD_WIDTH; }
  unsigned int height() const { return LCD_HEIGHT; }

  void setInvert(bool b);

  void rotate(int degrees);

  void setOn(bool b);

  void clear(unsigned int c);
  void clearBlack();

  void drawText(const char* t, unsigned int x, unsigned y, unsigned int c);
  void drawHorizontalLine(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
  void drawVerticalLine(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
  void drawRect(unsigned int x,unsigned int y, unsigned int w, unsigned int h, unsigned int c);

private:
  void setDataOutputDirection();
  void setDataInputDirection();

  void write2bus(unsigned char d);
  void writeCmd(unsigned char d);
  void writeData(unsigned char d);

  void setAddress(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

private:
  unsigned int LCD_WIDTH = 320;
  unsigned int LCD_HEIGHT = 480;
};


#endif

