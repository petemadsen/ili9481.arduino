/***********************************************************************************
 * 
 * For debugging purposes, you can send a number of commands using the terminal.
 * You can put multiple commands in one line. Check out the loop() for full command
 * list.
 * 
 * A few commands explained:
 * 
 * r to set current color to red
 * g to set current color to green
 * b to set current color to blue
 * z to set current color to black
 * 
 * 1..9 to draw a rectangle using the current color
 *  
 * R rotate the screen by 90 degrees and draw some text on it.
 * 
 * S sleep for 1 second
 * 
 * o turn the display on
 * O turn the display off
 * 
 * 
 */


#define RGB2LCD(r, g, b) (r << 16 | g << 8 | b)


#define LCD_RED    0xf800
#define LCD_GREEN  0x07E0
#define LCD_BLUE   0x001F
#define LCD_YELLOW 0x07ff



#include "ili9481.h"
LcdIli9481 lcd;


void setup()
{
  Serial.begin(9600);


  lcd.begin();
  Serial.print("device-id: 0x"); Serial.println(lcd.info(), HEX);
  lcd.rotate(0);
  lcd.clearBlack();


/*
  for(int i=0; i<5; ++i) {
    Lcd_Set_Pixel(5, i, LCD_RED);
    Lcd_Set_Pixel(10, 5+i, LCD_RED);
    Lcd_Set_Pixel(5, 10+i, LCD_RED);
  }
  */
  Serial.println("Ready");

  //Lcd_Print(0, 100, " !                              @ABCDEFGHIJKLMNOPQRSTUVWXYZ       abcdefghijklmnopqrstuvwxyz", LCD_RED);

  //lcd.drawText("Hello, Peter! How r u?", 0, 15, LCD_RED);
  lcd.rotate(90);
  lcd.clear(LCD_BLUE);
  lcd.drawText("abcdefghijklmnopqrstuvwxyz", 15, 15, LCD_RED);
  lcd.drawText("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 15, 150, LCD_RED);
}


unsigned int current_color = LCD_YELLOW;
int orientation = 0;

void loop()
{
  int ch = -1;
  
  if(Serial.available()) {
    ch = Serial.read();
  }

  //Serial.print("ch: ");
  //Serial.println(ch, HEX);
      
  switch(ch) {
    case 'c':
      lcd.clear(current_color);
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
      Serial.println("black");
      current_color = 0;
      break;

    case 'o':
      Serial.println("display-on");
      lcd.setOn(true);
      break;
    case 'O':
    Serial.println("display-off");
      lcd.setOn(false);
      break;

    case 'f':
      lcd.drawFilledRect(40, 40, lcd.width()-2*40, lcd.height()-2*40, current_color);
      break;
    case 'F':
      lcd.drawFilledRect(0, 0, lcd.width(), lcd.height(), current_color);
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
        lcd.drawRect(i*10, i*10, lcd.width()-2*i*10, lcd.height()-2*i*10, current_color);
        //Rect(20, 20, LCD_WIDTH-40, LCD_HEIGHT-40);
      }
      break;

    case 'x':
      lcd.drawRect(random(300),random(300),random(300),random(300),random(65535)); // rectangle at x, y, with, hight, color
      break;

    case 'S':
      delay(1000);
      break;

    case 'R':
      orientation = (orientation + 90) % 360;
      lcd.rotate(orientation);
      lcd.clearBlack();
      lcd.drawText("Hello, Peter! How do you do?", 50, 50, LCD_GREEN);
      break;

#if 0
    case 'N':
      Serial.println("scrolling?");
      Lcd_Set_Scrolling_Area(50, 150);
      Lcd_Start_Scrolling(75);
      break;
    case 'M':
      break;
#endif

    case 'i':
      lcd.setInvert(false);
      break;
    case 'I':
      lcd.setInvert(true);
      break;

    case '?':
      lcd.info();
      break;
      
  }
#if 0
  for(int i=0;i<1000;i++)
  {
    Rect(random(300),random(300),random(300),random(300),random(65535)); // rectangle at x, y, with, hight, color
  }
#endif
   
   
   
}
