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


#define LCD_RED 0xf800
#define LCD_GREEN 0x07E0
#define LCD_BLUE 0x001F



#define WR_MASK B00000010
#define RS_MASK B00000100
void Lcd_Writ_Bus(unsigned char d)
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


void Lcd_Write_Com(unsigned char VH)  
{   
  PORTF &= ~RS_MASK;//RS=0
  Lcd_Writ_Bus(VH);
}


void Lcd_Write_Data(unsigned char VH)
{
  PORTF |= RS_MASK;//LCD_RS=1;
  Lcd_Writ_Bus(VH);
}


void Address_set(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
  Serial.print("address-set: ");
  Serial.print(x1);
  Serial.print(", ");
  Serial.print(y1);
  Serial.print(", ");
  Serial.print(x2);
  Serial.print(", ");
  Serial.println(y2);
  
  Lcd_Write_Com(LCD_CMD_SET_COL_ADDR);
	Lcd_Write_Data(x1>>8);
	Lcd_Write_Data(x1);
	Lcd_Write_Data(x2>>8);
	Lcd_Write_Data(x2);
  
  Lcd_Write_Com(LCD_CMD_SET_PAGE_ADDR);
	Lcd_Write_Data(y1>>8);
	Lcd_Write_Data(y1);
	Lcd_Write_Data(y2>>8);
	Lcd_Write_Data(y2);
 
	Lcd_Write_Com(LCD_CMD_WRITE_MEM_START);
}


// FIXME: does not work
void Lcd_Display_On(bool b)
{
  Lcd_Write_Com(b ? LCD_CMD_DISPLAY_ON : LCD_CMD_DISPLAY_OFF);
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
  
  Lcd_Write_Com(0x11); // exit sleep mode
  delay(20);
  
  Lcd_Write_Com(0xD0); // power settings
  Lcd_Write_Data(0x07);
  Lcd_Write_Data(0x42);
  Lcd_Write_Data(0x18);
  
  Lcd_Write_Com(0xD1); // vcom control
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x07);
  Lcd_Write_Data(0x10);
  
  Lcd_Write_Com(0xD2); // power settings for normal mode
  Lcd_Write_Data(0x01);
  Lcd_Write_Data(0x02);
  
  Lcd_Write_Com(0xC0); // panel driving setting
  Lcd_Write_Data(0x10);
  Lcd_Write_Data(0x3B);
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x02);
  Lcd_Write_Data(0x11);
  
  Lcd_Write_Com(0xC5); // frame rate & inversion control
  Lcd_Write_Data(0x03);
  
  Lcd_Write_Com(0x36); // set address mode
  Lcd_Write_Data(0x0A); // page/column-selection, horizontal flip
  
  Lcd_Write_Com(0x3A); // set pixel format
  Lcd_Write_Data(0x55);
  
  Lcd_Write_Com(LCD_CMD_SET_COL_ADDR);
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x01);
  Lcd_Write_Data(0x3F);
  
  Lcd_Write_Com(LCD_CMD_SET_PAGE_ADDR);
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x00);
  Lcd_Write_Data(0x01);
  Lcd_Write_Data(0xE0);
  
  delay(120);
  Lcd_Write_Com(LCD_CMD_DISPLAY_ON); // set display on
  
  //Lcd_Write_Com(0x002c); 
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


int RGB(int r,int g,int b)
{
  return r << 16 | g << 8 | b;
}


void LCD_Clear(unsigned int c)                   
{	
  unsigned int i,m;

  Serial.println("lcd-clear");

  digitalWrite(LCD_CS, LOW);
  
  Address_set(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

  for(i=0;i<LCD_HEIGHT; i++)
  {
    for(m=0;m<LCD_WIDTH;m++)
    {
      Lcd_Write_Data(c>>8);
      Lcd_Write_Data(c);
    }
  }
    
  digitalWrite(LCD_CS, HIGH);   
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
  
  pinMode(A0,OUTPUT);
  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
  pinMode(A4,OUTPUT);
  
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);
  
  Lcd_Init();
  LCD_Clear(0x0);

  for(int i=0; i<9; ++i) {
    Lcd_Set_Pixel(i, i, LCD_RED);
  }
  
  Serial.println("Ready");
}


unsigned int current_color = LCD_RED;

void loop()
{
  while(Serial.available()) {
    int ch = Serial.read();
    
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
          Serial.print("rect ");
          Serial.println(i);
          Rect(i*10, i*10, LCD_WIDTH-2*i*10, LCD_HEIGHT-2*i*10, current_color);
          //Rect(20, 20, LCD_WIDTH-40, LCD_HEIGHT-40);
        }
        break;

      case 'x':
        Rect(random(300),random(300),random(300),random(300),random(65535)); // rectangle at x, y, with, hight, color
        break;
    }
#if 0
  for(int i=0;i<1000;i++)
  {
    Rect(random(300),random(300),random(300),random(300),random(65535)); // rectangle at x, y, with, hight, color
  }
#endif
   
   
   
  }
}
