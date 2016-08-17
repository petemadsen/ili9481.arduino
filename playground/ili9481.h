extern void Lcd_Write_Cmd(unsigned char d);
extern void Lcd_Write_Data(unsigned char d);


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

