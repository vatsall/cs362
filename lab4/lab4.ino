//Heng Li
//hli212

// include the library code:
#include <LiquidCrystal.h>


LiquidCrystal lcd(12,11,5,4,3,2);
int lightSensor = A0;
int analogValue = 0;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  
}

void loop()
{
  analogValue = analogRead(lightSensor);
  lcd.setCursor(0,0);
  if(analogValue <= 25)
  {
    lcd.print("Dark");
  }
  else if(analogValue <= 50)
  {
    lcd.print("partially dark");
  }
  else if(analogValue <= 75)
  {
    lcd.print("medium");
  }
  else if(analogValue <= 100)
  {
    lcd.print("partially light");
  }
  else
  {
    lcd.print("fully lit");
  }
  

  // delay at the end of the full loop:
  delay(500);
  lcd.clear();


}

  // scroll 13 positions (string length) to the left
  // to move it offscreen left:
  
  //String quote = "abcdefghijklmnopqrstuvwx";
  //int n = quote.length();
  /*
  for(int i = 15; i >= 0;i--)
  {
    lcd.setCursor(0,0);
    lcd.print("Heng Li");
    
    lcd.setCursor(i,1);
    lcd.print(quote);
    delay(200);
    lcd.clear();
  }

  for(int i = 1 ; i < n; i++)
  {
    lcd.setCursor(0,0);
    lcd.print("Heng Li");
    
    String quote1 = &quote[i];
    lcd.setCursor(0,1);
    lcd.print(quote1);
    delay(200);
    lcd.clear();
  }
  */



