extern void Lcd_Write_Cmd(unsigned char d);
extern void Lcd_Write_Data(unsigned char d);


#define RD_MASK B00000001
#define WR_MASK B00000010
#define RS_MASK B00000100
#define CS_MASK B00001000



#define LCD_DATA  PORTF |= RS_MASK
#define CS_ON     digitalWrite(LCD_CS, LOW)
#define CS_OFF    digitalWrite(LCD_CS, HIGH)




// Seem to move parts of the screen
void Lcd_Set_Scrolling_Area(unsigned int top, unsigned int height)
{
  digitalWrite(LCD_CS, LOW);
  Lcd_Write_Cmd(0x33);
  Lcd_Write_Data(top>>8);
  Lcd_Write_Data(top);
  Lcd_Write_Data(height>>8);
  Lcd_Write_Data(height);
  Lcd_Write_Data((top+height)>>8);
  Lcd_Write_Data(top+height);
  digitalWrite(LCD_CS, HIGH);
}
void Lcd_Start_Scrolling(unsigned int line)
{
  digitalWrite(LCD_CS, LOW);
  Lcd_Write_Cmd(0x37);
  Lcd_Write_Data(line>>8);
  Lcd_Write_Data(line);
  digitalWrite(LCD_CS, HIGH);
}


void Lcd_Set_Invert(bool doit)
{
  digitalWrite(LCD_CS, LOW);
  Lcd_Write_Cmd(doit ? 0x21 : 0x20);
  digitalWrite(LCD_CS, HIGH);
}



uint8_t Lcd_Read_From_Bus()
{
  uint8_t d = 0;

  //PORTF |= RS_MASK; // LCD_RS=1; data

  //PORTF |=  RD_MASK;
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

  //PORTF &= ~RD_MASK;
  PORTF |=  RD_MASK;

  return d;
}


void Lcd_Device_Info()
{
  CS_ON;

  //PORTF |=  RD_MASK;
  
  Lcd_Write_Cmd(0xbf);

  //PORTF &= ~WR_MASK;
  //PORTF |=  WR_MASK;
  

  // input mode
  DDRH &= ~(0x78);
  DDRE &= ~(0x38);
  DDRG &= ~(0x20);

  LCD_DATA;
  
  Serial.println("DEVICEINFO.BEGIN");
  Serial.println(Lcd_Read_From_Bus(), HEX);
  Serial.println(Lcd_Read_From_Bus(), HEX);
  Serial.println(Lcd_Read_From_Bus(), HEX);
  Serial.println(Lcd_Read_From_Bus(), HEX);
  Serial.println(Lcd_Read_From_Bus(), HEX);
  Serial.println(Lcd_Read_From_Bus(), HEX);
  Serial.println("DEVICEINFO.END");

  // pin mode restored
  DDRH |= 0x78;
  DDRE |= 0x38;
  DDRG |= 0x20;

  CS_OFF;
}

