#include <gfxfont.h>
#include <Adafruit_GFX.h>

// Product&Sources:
// http://www.banggood.com/3_5-Inch-TFT-Color-Screen-Module-320-X-480-Support-Arduino-UNO-Mega2560-p-1022298.html
//
// Breakout/Arduino UNO pin usage:
// LCD Data Bit :   7   6   5   4   3   2   1   0
// Uno dig. pin :   7   6   5   4   3   2   9   8
// Uno port/pin : PD7 PD6 PD5 PD4 PD3 PD2 PB1 PB0
// Mega dig. pin:  29  28  27  26  25  24  23  22
//              : PH4  PH3 PE3 PG5 PE5 PE4 PH6 PH5
#define LCD_RD  A0 // ? data read
#define LCD_WR  A1 // ? data write
#define LCD_RS  A2 // ? data-or-command ? low if command
#define LCD_CS  A3 // chip select, low enable
#define LCD_RST A4 // reset


#define LCD_WIDTH 320
#define LCD_HEIGHT 480

#define LCD_CMD_SET_COL_ADDR 0x2a
#define LCD_CMD_SET_PAGE_ADDR 0x2b
#define LCD_CMD_WRITE_MEM_START 0x2c

#define LCD_CMD_DISPLAY_ON 0x29
#define LCD_CMD_DISPLAY_OFF 0x28


#define RGB2LCD(r, g, b) (r << 16 | g << 8 | b)

#define LCD_RED    0xf800
#define LCD_GREEN  0x07E0
#define LCD_BLUE   0x001F
#define LCD_YELLOW 0x07ff



#include "bitstream.h"

#include <Fonts/FreeMono24pt7b.h>
extern const char my_8x8_font[];



#define WR_MASK B00000010
#define RS_MASK B00000100
#define CS_MASK B00001000
inline void Lcd_Writ_Bus(unsigned char d)
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


inline void Lcd_Write_Cmd(unsigned char d)  
{   
  PORTF &= ~RS_MASK; // LCD_RS=0
  Lcd_Writ_Bus(d);
}


inline void Lcd_Write_Data(unsigned char d)
{
  PORTF |= RS_MASK; // LCD_RS=1;
  Lcd_Writ_Bus(d);
}


void Address_set(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
  Lcd_Write_Cmd(LCD_CMD_SET_COL_ADDR);
	Lcd_Write_Data(x1>>8);
	Lcd_Write_Data(x1);
	Lcd_Write_Data(x2>>8);
	Lcd_Write_Data(x2);
  
  Lcd_Write_Cmd(LCD_CMD_SET_PAGE_ADDR);
	Lcd_Write_Data(y1>>8);
	Lcd_Write_Data(y1);
	Lcd_Write_Data(y2>>8);
	Lcd_Write_Data(y2);
 
	Lcd_Write_Cmd(LCD_CMD_WRITE_MEM_START);
}


// FIXME: does not work
void Lcd_Display_On(bool b)
{
  digitalWrite(LCD_CS, LOW);
  Lcd_Write_Cmd(b ? LCD_CMD_DISPLAY_ON : LCD_CMD_DISPLAY_OFF);
  digitalWrite(LCD_CS, HIGH);
}


void Lcd_Init(void)
{
  digitalWrite(LCD_RST, HIGH);
  delay(5); 
  digitalWrite(LCD_RST, LOW);
  delay(15);
  digitalWrite(LCD_RST, HIGH);
  delay(15);

  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_CS, LOW);  //CS
  
  Lcd_Write_Cmd(0x11); // exit sleep mode
  delay(20);
  
  Lcd_Write_Cmd(0xD0); // power settings
  Lcd_Write_Data(0x07);
  Lcd_Write_Data(0x42);
  Lcd_Write_Data(0x18);
  
  Lcd_Write_Cmd(0xD1); // vcom control
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x07);
  Lcd_Write_Data(0x10);
  
  Lcd_Write_Cmd(0xD2); // power settings for normal mode
  Lcd_Write_Data(0x01);
  Lcd_Write_Data(0x02);
  
  Lcd_Write_Cmd(0xC0); // panel driving setting
  Lcd_Write_Data(0x10);
  Lcd_Write_Data(0x3B);
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x02);
  Lcd_Write_Data(0x11);
  
  Lcd_Write_Cmd(0xC5); // frame rate & inversion control
  //Lcd_Write_Data(0x03); // 72Hz
  Lcd_Write_Data(0x02); // 85Hz, default
  
  Lcd_Write_Cmd(0x36); // set address mode
  Lcd_Write_Data(0x0A); // page/column-selection, horizontal flip
  
  Lcd_Write_Cmd(0x3A); // set pixel format
  Lcd_Write_Data(0x55);
  
  Lcd_Write_Cmd(LCD_CMD_SET_COL_ADDR);
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x01);
  Lcd_Write_Data(0x3F);
  
  Lcd_Write_Cmd(LCD_CMD_SET_PAGE_ADDR);
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x01);
  Lcd_Write_Data(0xE0);
  
  delay(120);
  Lcd_Write_Cmd(LCD_CMD_DISPLAY_ON); // set display on
}


void Lcd_Set_Pixel(unsigned int x, unsigned int y, unsigned int c)
{
  Serial.println("set-pixel");

  digitalWrite(LCD_CS, LOW);

  Address_set(x, y, x, y);
  Lcd_Write_Data(c>>8);
  Lcd_Write_Data(c);

  digitalWrite(LCD_CS,HIGH);
}


void Lcd_Print(unsigned int x, unsigned y, const char* t, unsigned int c)
{
  unsigned char i, k;
  unsigned char row;
  unsigned char mask;
  char* p;
  
  digitalWrite(LCD_CS, LOW);

  x = x / 8 * 8;
  
  while(*t)
  {
    if(*t < 32 || *t > 127)
      continue;
      
    p = my_8x8_font + (*t - ' ') * 8;
    
    Address_set(x, y, x + 7, y + 7);

    for(i=0; i<8; ++i) {
      row = p[i];
      mask = 0x80;
      for(k=0; k<8; ++k)
      {
        if(row & mask) {
          Lcd_Write_Data(c>>8);
          Lcd_Write_Data(c);
        } else {
          Lcd_Write_Data(0);
          Lcd_Write_Data(0);
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


void Lcd_Print_New(unsigned int x, unsigned y, const char* t, unsigned int c)
{
  uint8_t i;

  uint8_t h, w;
  GFXfont font;
  GFXglyph g;
  BitStream in;

  memcpy_P(&font, &FreeMono24pt7b, sizeof(GFXfont));
  
  digitalWrite(LCD_CS, LOW);

  
  while(*t)
  {
    if(*t < font.first || *t > font.last)
      continue;
      
    memcpy_P(&g, &font.glyph[*t - font.first], sizeof(GFXglyph));
    
    uint8_t* bmp = font.bitmap + g.bitmapOffset;
    in.set_addr(bmp);
    
    Address_set(x, y, x + g.xAdvance - 1, y + font.yAdvance - 1);

//    Lcd_Write_Data(c>>8);
//    Lcd_Write_Data(c);


    // empty lines at top
    for(h=0; h<font.yAdvance-g.height; ++h)
    {
      for(i=0; i<g.xAdvance; ++i)
      {
        Lcd_Write_Data(0);
        Lcd_Write_Data(0);
      }
    }
    // the char, itself!
    for(h=0; h<g.height; ++h)
    {
      for(i=0; i<g.xOffset; ++i)
      {
        Lcd_Write_Data(0);
        Lcd_Write_Data(0);
      }

      for(w=0; w<g.width; ++w)
      {
        if(in.next())
        {
          Lcd_Write_Data(c>>8);
          Lcd_Write_Data(c);
        } else {
          Lcd_Write_Data(0);
          Lcd_Write_Data(0);          
        }
      }

      for(i=g.xOffset+g.width; i<g.xAdvance; ++i)
      {
        Lcd_Write_Data(0);
        Lcd_Write_Data(0);
      }
    }
    // FIXME: empty lines at bottom

    x += g.xAdvance;
    if((x+g.xAdvance) >= LCD_WIDTH) {
      x = 0;
      y += font.yAdvance;
    }
    ++t;
  }
  
  digitalWrite(LCD_CS, HIGH);
}


void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c)
{
  unsigned int i;
  
  digitalWrite(LCD_CS, LOW);
  
  Address_set(x, y, x+l,y);
  
  for(i=0; i<l; ++i)
  {
    Lcd_Write_Data(c>>8);
    Lcd_Write_Data(c);
  }
  
  digitalWrite(LCD_CS,HIGH);   
}


void V_line(unsigned int x, unsigned int y, unsigned int h, unsigned int c)                   
{	
  unsigned int i;
  
  digitalWrite(LCD_CS,LOW);
  
  Address_set(x, y, x, y+h);
  
  for(i=0; i < h; ++i)
  { 
    Lcd_Write_Data(c>>8);
    Lcd_Write_Data(c);
  }
  
  digitalWrite(LCD_CS, HIGH);   
}


void Rect(unsigned int x,unsigned int y, unsigned int w, unsigned int h, unsigned int c)
{
  Serial.println("rect");
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}


void Rectf(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c)
{
  unsigned int i;
  for(i=0;i<h;i++)
  {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}


void LCD_Clear(unsigned int c)                   
{
  unsigned long m = millis();
  
  unsigned int i, k;

  digitalWrite(LCD_CS, LOW);
  //PORTF &= ~CS_MASK; // LOW
  
  Address_set(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

  PORTF |= RS_MASK; // LCD_RS=1;
  unsigned int c1 = c>>8;

  for(i=0; i<LCD_HEIGHT; ++i)
  {
    for(k=0; k<LCD_WIDTH; ++k)
    {
      //Lcd_Write_Data(c>>8);
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
      
      //Lcd_Write_Data(c);
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
    
  digitalWrite(LCD_CS, HIGH);
  //PORTF |= CS_MASK; // HIGH

  m = millis() - m;

  Serial.println("lcd-clear");
  Serial.print("m: ");
  Serial.println(m);
}


void LCD_ClearBlack()
{
  unsigned long m = millis();
  
  unsigned int ii, kk;

  PORTF &= ~CS_MASK; // LOW
  
  Address_set(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

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
    
  PORTF |= CS_MASK; // HIGH

  m = millis() - m;

  Serial.println("lcd-clear");
  Serial.print("m: ");
  Serial.println(m);
}


void setup()
{
  Serial.begin(9600);
  /*
  for(int p=0;p<10;p++)
  {
    pinMode(p,OUTPUT);
  }*/
  DDRH |= 0x78;
  DDRE |= 0x38;
  DDRG |= 0x20;
  
  pinMode(LCD_RD,OUTPUT);
  pinMode(LCD_WR,OUTPUT);
  pinMode(LCD_RS,OUTPUT);
  pinMode(LCD_CS,OUTPUT);
  pinMode(LCD_RST,OUTPUT);
  
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RST, HIGH);
  
  Lcd_Init();

  LCD_ClearBlack();


  for(int i=0; i<5; ++i) {
    Lcd_Set_Pixel(5, i, LCD_RED);
    Lcd_Set_Pixel(10, 5+i, LCD_RED);
    Lcd_Set_Pixel(5, 10+i, LCD_RED);
  }
  
  Serial.println("Ready");

  //Lcd_Print(0, 100, " !                              @ABCDEFGHIJKLMNOPQRSTUVWXYZ       abcdefghijklmnopqrstuvwxyz", LCD_RED);

  Lcd_Print_New(0, 100, "Hello, Peter!", LCD_RED);
  //Lcd_Print_New(0, 100, " ! ", LCD_RED);
}


unsigned int current_color = LCD_YELLOW;
bool demo_mode = false;
char* demo_mode_steps = "rcS bcS gcS zcSS b 1S 2S 3SS rcS bcS OSSS oS t";
unsigned char demo_mode_pos = 0;

void loop()
{
  int ch = -1;
  
  if(demo_mode) {
    if(demo_mode_steps[demo_mode_pos]) {
      ch = demo_mode_steps[demo_mode_pos++];
    } else {
      demo_mode = false;
    }
    
  } else {
    if(Serial.available()) {
      ch = Serial.read();
    }
  }

  //Serial.print("ch: ");
  //Serial.println(ch, HEX);
      
  switch(ch) {
    case 'c':
      LCD_Clear(current_color);
      break;
        
    case 'r':
      Serial.println("red");
      current_color = LCD_RED;
      break;
    case 'g':
      Serial.println("green");
      current_color = LCD_GREEN;
      break;
    case 'b':
      Serial.println("blue");
      current_color = LCD_BLUE;
      break;
    case 'z':
      Serial.println('black');
      current_color = 0;
      break;

    case 'o':
      Serial.println("display-on");
      Lcd_Display_On(true);
      break;
    case 'O':
    Serial.println("display-off");
      Lcd_Display_On(false);
      break;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      {
        int i = ch - '1' + 1;
        Rect(i*10, i*10, LCD_WIDTH-2*i*10, LCD_HEIGHT-2*i*10, current_color);
        //Rect(20, 20, LCD_WIDTH-40, LCD_HEIGHT-40);
      }
      break;

    case 'x':
      Rect(random(300),random(300),random(300),random(300),random(65535)); // rectangle at x, y, with, hight, color
      break;

    case 'd':
      demo_mode = true;
      demo_mode_pos = 0;
      break;

    case 'S':
      delay(1000);
      break;
  }
#if 0
  for(int i=0;i<1000;i++)
  {
    Rect(random(300),random(300),random(300),random(300),random(65535)); // rectangle at x, y, with, hight, color
  }
#endif
   
   
   
}
