#include "ili9481.h"

#include <gfxfont.h>
#include <Adafruit_GFX.h>

#include "bitstream.h"
#include <Fonts/FreeMono24pt7b.h>


// Product&Sources:
// http://www.banggood.com/3_5-Inch-TFT-Color-Screen-Module-320-X-480-Support-Arduino-UNO-Mega2560-p-1022298.html
//
// Breakout/Arduino UNO pin usage:
// LCD Data Bit :   7   6   5   4   3   2   1   0
// Uno dig. pin :   7   6   5   4   3   2   9   8
// Uno port/pin : PD7 PD6 PD5 PD4 PD3 PD2 PB1 PB0
// Mega dig. pin:  29  28  27  26  25  24  23  22
//              : PH4  PH3 PE3 PG5 PE5 PE4 PH6 PH5
//
// setDataOutputDirection()
// setDataInputDirection()



#define LCD_CMD_SET_COL_ADDR     0x2a
#define LCD_CMD_SET_PAGE_ADDR    0x2b
#define LCD_CMD_WRITE_MEM_START  0x2c

#define LCD_CMD_DISPLAY_ON       0x29
#define LCD_CMD_DISPLAY_OFF      0x28

#define LCD_CMD_SET_ADDRESS_MODE 0x36



#define RD_MASK B00000001
#define WR_MASK B00000010
#define RS_MASK B00000100
#define CS_MASK B00001000


#define LCD_RD  A0 // ? data read
#define LCD_WR  A1 // ? data write
#define LCD_RS  A2 // ? data-or-command ? low if command, DC/X
#define LCD_CS  A3 // chip select, low enable
#define LCD_RST A4 // reset


#define LCD_BUS_DATA  PORTF |= RS_MASK
#define LCD_BUS_CMD   PORTF &= ~RS_MASK
#define CS_ON     digitalWrite(LCD_CS, LOW)
#define CS_OFF    digitalWrite(LCD_CS, HIGH)






void LcdIli9481::begin()
{
  /**
   * Arduino ports
   */
  setDataOutputDirection();
  
  pinMode(LCD_RD, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RST, OUTPUT);
  
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RST, HIGH);

  /**
   * Bring up the LCD
   */
  digitalWrite(LCD_RST, HIGH);
  delay(5); 
  digitalWrite(LCD_RST, LOW);
  delay(15);
  digitalWrite(LCD_RST, HIGH);
  delay(15);

  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_CS, LOW);  //CS
  
  writeCmd(0x11); // exit sleep mode
  delay(20);
  
  writeCmd(0xD0); // power settings
  writeData(0x07);
  writeData(0x42);
  writeData(0x18);
  
  writeCmd(0xD1); // vcom control
  writeData(0x00);
  writeData(0x07);
  writeData(0x10);
  
  writeCmd(0xD2); // power settings for normal mode
  writeData(0x01);
  writeData(0x02);
  
  writeCmd(0xC0); // panel driving setting
  writeData(0x10);
  writeData(0x3B);
  writeData(0x00);
  writeData(0x02);
  writeData(0x11);
  
  writeCmd(0xC5); // frame rate & inversion control
  //writeData(0x03); // 72Hz
  writeData(0x02); // 85Hz, default
  
  writeCmd(LCD_CMD_SET_ADDRESS_MODE); // set address mode
  writeData(0x0A); // page-address-order | page/column-selection |  horizontal flip
  
  writeCmd(0x3A); // set pixel format
  writeData(0x55);
  
  writeCmd(LCD_CMD_SET_COL_ADDR);
  writeData(0x00);
  writeData(0x00);
  writeData(0x01);
  writeData(0x3F);
  
  writeCmd(LCD_CMD_SET_PAGE_ADDR);
  writeData(0x00);
  writeData(0x00);
  writeData(0x01);
  writeData(0xE0);
  
  delay(120);
  writeCmd(LCD_CMD_DISPLAY_ON); // set display on
}


void LcdIli9481::setDataOutputDirection()
{
  DDRH |= 0x78;
  DDRE |= 0x38;
  DDRG |= 0x20;
}

void LcdIli9481::setDataInputDirection()
{
  DDRH &= ~0x78;
  DDRE &= ~0x38;
  DDRG &= ~0x20;
}



/* Seem to move parts of the screen
void Lcd_Set_Scrolling_Area(uint16_t top, uint16_t height)
{
  digitalWrite(LCD_CS, LOW);
  writeCmd(0x33);
  writeData(top>>8);
  writeData(top);
  writeData(height>>8);
  writeData(height);
  writeData((top+height)>>8);
  writeData(top+height);
  digitalWrite(LCD_CS, HIGH);
}
void Lcd_Start_Scrolling(uint16_t line)
{
  digitalWrite(LCD_CS, LOW);
  writeCmd(0x37);
  writeData(line>>8);
  writeData(line);
  digitalWrite(LCD_CS, HIGH);
}
*/


void LcdIli9481::setInvert(bool doit)
{
  CS_ON;
  writeCmd(doit ? 0x21 : 0x20);
  CS_OFF;
}



uint8_t LcdIli9481::readFromBus()
{
  uint8_t d = 0;

  PORTF &= ~RD_MASK;
  delay(10);
  
  //PORTH &= ~(0x78);
  //PORTH |= ((d&0xC0) >> 3) | ((d&0x3) << 5);
  d |= (PINH & 0x18) << 3;
  d |= (PINH & 0x60) >> 5;
  
  //PORTE &= ~(0x38);
  //PORTE |= ((d & 0xC) << 2) | ((d & 0x20) >> 2);
  d |= (PINE & 0x30) >> 2;
  d |= (PINE & 0x08) << 2;
  
  //PORTG &= ~(0x20);
  //PORTG |= (d & 0x10) << 1;
  d |= (PING & 0x20) >> 1;

  PORTF |=  RD_MASK;

  return d;
}


uint16_t LcdIli9481::info()
{
  uint16_t device_id = 0;
  uint8_t d0, dh, dl;
  
  CS_ON;
  
  writeCmd(0xbf);
  
  setDataInputDirection();

  LCD_BUS_DATA;
  
  d0 = readFromBus(); //Serial.println(d0, HEX);
  d0 = readFromBus(); //Serial.println(d0, HEX);
  d0 = readFromBus(); //Serial.println(d0, HEX);
  dh = readFromBus(); //Serial.println(dh, HEX);
  dl = readFromBus(); //Serial.println(dl, HEX);
  d0 = readFromBus(); //Serial.println(d0, HEX);

  device_id = (dh << 8) | dl;

  setDataOutputDirection();

  CS_OFF;

  return device_id;
}


inline void LcdIli9481::writeToBus(uint8_t d)
{
  PORTH &= ~(0x78);
  PORTH |= ((d&0xC0) >> 3) | ((d&0x3) << 5);
  
  PORTE &= ~(0x38);
  PORTE |= ((d & 0xC) << 2) | ((d & 0x20) >> 2);
  
  PORTG &= ~(0x20);
  PORTG |= (d & 0x10) << 1;
  
  PORTF &= ~WR_MASK;
  PORTF |=  WR_MASK;
}


inline void LcdIli9481::writeCmd(uint8_t d)
{   
  LCD_BUS_CMD;
  writeToBus(d);
}


inline void LcdIli9481::writeData(uint8_t d)
{
  LCD_BUS_DATA;
  writeToBus(d);
}


void LcdIli9481::setAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  writeCmd(LCD_CMD_SET_COL_ADDR);
  writeData(x1>>8);
  writeData(x1);
  writeData(x2>>8);
  writeData(x2);
  
  writeCmd(LCD_CMD_SET_PAGE_ADDR);
  writeData(y1>>8);
  writeData(y1);
  writeData(y2>>8);
  writeData(y2);
 
  writeCmd(LCD_CMD_WRITE_MEM_START);
}


void LcdIli9481::setOn(bool b)
{
  CS_ON;
  writeCmd(b ? LCD_CMD_DISPLAY_ON : LCD_CMD_DISPLAY_OFF);
  CS_OFF;
}


void LcdIli9481::rotate(int degrees)
{
  Serial.print("rotate: ");
  Serial.println(degrees);
  uint8_t cfg = B00001010;
  LCD_WIDTH = 320;
  LCD_HEIGHT = 480;
  
  switch(degrees) {
  case 90:
    cfg = B00101000;
    LCD_WIDTH = 480;
    LCD_HEIGHT = 320;
    break;
  case 180:
    cfg = B00001001;
    LCD_WIDTH = 320;
    LCD_HEIGHT = 480;
    break;
  case 270:
    cfg = B00101011;
    LCD_WIDTH = 480;
    LCD_HEIGHT = 320;
    break;
  }

  CS_ON;
  writeCmd(LCD_CMD_SET_ADDRESS_MODE);
  writeData(cfg);
  CS_OFF;
}


void LcdIli9481::clear(uint16_t c)
{
  drawFilledRect(0, 0, LCD_WIDTH, LCD_HEIGHT, c);
}


void LcdIli9481::drawFilledRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c)
{
  uint16_t i, k;

  CS_ON;
  
  setAddress(x, y, x + w - 1, y + h - 1);

  PORTF |= RS_MASK; // LCD_RS=1;
  uint16_t c1 = c>>8;

  for(i=0; i<h; ++i)
  {
    for(k=0; k<w; ++k)
    {
      //writeData(c>>8);
      //PORTF |= RS_MASK; // LCD_RS=1;
      //Lcd_Writ_Bus(c>>8);
      //
      PORTH &= ~(0x78);
      PORTH |= ((c1&0xC0) >> 3) | ((c1&0x3) << 5);
      PORTE &= ~(0x38);
      PORTE |= ((c1 & 0xC) << 2) | ((c1 & 0x20) >> 2);
      PORTG &= ~(0x20);
      PORTG |= (c1 & 0x10) << 1;
      PORTF &= ~WR_MASK;
      PORTF |=  WR_MASK;
      
      //writeData(c);
      //PORTF |= RS_MASK; // LCD_RS=1;
      //Lcd_Writ_Bus(c);
      //
      PORTH &= ~(0x78);
      PORTH |= ((c&0xC0) >> 3) | ((c&0x3) << 5);
      PORTE &= ~(0x38);
      PORTE |= ((c & 0xC) << 2) | ((c & 0x20) >> 2);
      PORTG &= ~(0x20);
      PORTG |= (c & 0x10) << 1;
      PORTF &= ~WR_MASK;
      PORTF |=  WR_MASK;
    }
  }
    
  CS_OFF;
}


/*
void Lcd_Set_Pixel(uint16_t x, uint16_t y, uint16_t c)
{
  Serial.println("set-pixel");

  digitalWrite(LCD_CS, LOW);

  setAddress(x, y, x, y);
  writeData(c>>8);
  writeData(c);

  digitalWrite(LCD_CS,HIGH);
}
*/

/*
void Lcd_Print(uint16_t x, unsigned y, const char* t, uint16_t c)
{
  uint8_t i, k;
  uint8_t row;
  uint8_t mask;
  char* p;
  
  digitalWrite(LCD_CS, LOW);

  x = x / 8 * 8;
  
  while(*t)
  {
    if(*t < 32 || *t > 127)
      continue;
      
    p = my_8x8_font + (*t - ' ') * 8;
    
    setAddress(x, y, x + 7, y + 7);

    for(i=0; i<8; ++i) {
      row = p[i];
      mask = 0x80;
      for(k=0; k<8; ++k)
      {
        if(row & mask) {
          writeData(c>>8);
          writeData(c);
        } else {
          writeData(0);
          writeData(0);
        }
        mask >>= 1;
      }
    }

    x += 8;
    if(x >= LCD_WIDTH) {
      x = 0;
      y += 8;
    }
    ++t;
  }
  
  digitalWrite(LCD_CS, HIGH);
}
*/

void LcdIli9481::drawText(const char* t, uint16_t left, unsigned y, uint16_t c)
{
  uint8_t i;

  uint8_t h, w;
  GFXfont font;
  GFXglyph g;
  BitStream in;

  uint16_t x = left;

  memcpy_P(&font, &FreeMono24pt7b, sizeof(GFXfont));
  
  CS_ON;

  
  while(*t)
  {
    if(*t < font.first || *t > font.last)
      continue;
      
    memcpy_P(&g, &font.glyph[*t - font.first], sizeof(GFXglyph));
    
    uint8_t* bmp = font.bitmap + g.bitmapOffset;
    in.set_addr(bmp);
    
    setAddress(x, y, x + g.xAdvance - 1, y + font.yAdvance - 1);

//    writeData(c>>8);
//    writeData(c);


    // empty lines at top
    for(h=0; h<font.yAdvance-g.height; ++h)
    {
      for(i=0; i<g.xAdvance; ++i)
      {
        writeData(0);
        writeData(0);
      }
    }
    // the char, itself!
    for(h=0; h<g.height; ++h)
    {
      for(i=0; i<g.xOffset; ++i)
      {
        writeData(0);
        writeData(0);
      }

      for(w=0; w<g.width; ++w)
      {
        if(in.next())
        {
          writeData(c>>8);
          writeData(c);
        } else {
          writeData(0);
          writeData(0);          
        }
      }

      for(i=g.xOffset+g.width; i<g.xAdvance; ++i)
      {
        writeData(0);
        writeData(0);
      }
    }
    // FIXME: empty lines at bottom

    x += g.xAdvance;
    if((x+g.xAdvance) >= LCD_WIDTH) {
      x = left;
      y += font.yAdvance;
    }
    ++t;
  }
  
  CS_OFF;
}


void LcdIli9481::drawHorizontalLine(uint16_t x, uint16_t y, uint16_t l, uint16_t c)
{
  uint16_t i;
  
  CS_ON;
  
  setAddress(x, y, x+l,y);
  
  for(i=0; i<l; ++i)
  {
    writeData(c>>8);
    writeData(c);
  }
  
  CS_OFF;
}


void LcdIli9481::drawVerticalLine(uint16_t x, uint16_t y, uint16_t h, uint16_t c)
{  
  uint16_t i;
  
  CS_ON;
  
  setAddress(x, y, x, y+h);
  
  for(i=0; i < h; ++i)
  { 
    writeData(c>>8);
    writeData(c);
  }
  
  CS_OFF;
}


void LcdIli9481::drawRect(uint16_t x,uint16_t y, uint16_t w, uint16_t h, uint16_t c)
{
  Serial.println("rect");
  drawHorizontalLine(x  , y  , w, c);
  drawHorizontalLine(x  , y+h, w, c);
  drawVerticalLine(x  , y  , h, c);
  drawVerticalLine(x+w, y  , h, c);
}


/*
void Rectf(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t c)
{
  uint16_t i;
  for(i=0;i<h;i++)
  {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}
*/




void LcdIli9481::clearBlack()
{
  unsigned long m = millis();
  
  uint16_t ii, kk;

  CS_ON;
  
  setAddress(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

  PORTF |= RS_MASK; // LCD_RS=1;

  PORTH &= ~(0x78); // clear all data bits
  PORTE &= ~(0x38);
  PORTG &= ~(0x20);

  for(ii=0; ii<LCD_HEIGHT; ++ii)
  {
    for(kk=0; kk<LCD_WIDTH/4; ++kk)
    {
      PORTF &= ~WR_MASK; PORTF |=  WR_MASK;
      PORTF &= ~WR_MASK; PORTF |=  WR_MASK;

      PORTF &= ~WR_MASK; PORTF |=  WR_MASK;
      PORTF &= ~WR_MASK; PORTF |=  WR_MASK;
      
      PORTF &= ~WR_MASK; PORTF |=  WR_MASK;
      PORTF &= ~WR_MASK; PORTF |=  WR_MASK;
      
      PORTF &= ~WR_MASK; PORTF |=  WR_MASK;
      PORTF &= ~WR_MASK; PORTF |=  WR_MASK;
    }
  }
    
  CS_OFF;

  m = millis() - m;

  Serial.println("lcd-clear");
  Serial.print("m: ");
  Serial.println(m);
}

