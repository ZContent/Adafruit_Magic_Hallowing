/*
 * The Magic Hallowing Magic
 * Dan Cogliano, DanTheGeek.com
 * January 2, 2019
 */
 
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
//#include <Adafruit_ST7789.h>
//#include <Adafruit_ST77xx.h>

#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#define SHAKE_THRESHOLD 225 // about 1.5G

#define TFT_CS     39
#define TFT_RST    37
#define TFT_DC     38
#define TFT_BACKLIGHT 7

char *answers[] = {
  "It is certain",
  "It is decidedly so",
  "Without a doubt",
  "Yes - definitely",
  "You may rely on it",
  "As I see it, yes",
  "Most likely",
  "Outlook good",
  "Yes",
  "Signs point to yes",
  "Reply hazy, try again",
  "Ask again later",
  "Better not tell you now",
  "Cannot predict now",
  "Concentrate and ask again",
  "Don't count on it",
  "My reply is no",
  "My sources say no",
  "Outlook not so good",
  "Very doubtful"
};

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

int getStringLength(char *str, int strlen = 0)
{
  char buff[255];
  int16_t x, y;
  uint16_t w, h;
  if(strlen == 0)
  {
    strcpy(buff, str);
  }
  else
  {
    strncpy(buff, str, strlen);
    buff[strlen+1] = '\0';
  }
  tft.getTextBounds(buff, 0,0, &x, &y, &w, &h);
  return(w);  
}

char *wrapWord(char *str = "", int size = 114)
{
  static char buff[255];
  static int linestart = 0;
  static int lineend = 0;
  static int bufflen = 0;
  if(strcmp(str,"") == 0)
  {
    // additional line from original string
    linestart = lineend + 1;
    lineend = bufflen;
  }
  else
  {
    memset(buff,0,sizeof(buff));
    // new string to wrap
    linestart = 0;
    strcpy(buff,str);
    lineend = strlen(buff);
    bufflen = strlen(buff);
  }
  uint16_t w;
  w = getStringLength(&buff[linestart]);
  while(w > size || (buff[lineend] != ' ' && buff[lineend] != '\0') && lineend > linestart)
  {
    lineend--;
    w = getStringLength(&buff[linestart],lineend - linestart);
  }
  buff[lineend] = '\0';
  return &buff[linestart];
}

void showTitle(bool fadeout = true)
{
  if(fadeout)
  {
    for(int i= 255; i >= 8; i-=8)
    {
      analogWrite(TFT_BACKLIGHT, i);
      delay(50);   
    }
  }
  analogWrite(TFT_BACKLIGHT, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  analogWrite(TFT_BACKLIGHT, 0);
  delay(250);
  char *line1 = "Ask a question";
  char *line2 = "Shake for answer";
  char *qline = "?";
  tft.setTextSize(1);
  tft.setCursor((126 - getStringLength(line1))/2, 30);
  tft.print(line1);
  tft.setTextSize(3);
  tft.setCursor((126 - getStringLength(qline))/2, 50);
  tft.print(qline);
  tft.setTextSize(1);
  tft.setCursor((126 - getStringLength(line2))/2, 80);
  tft.print(line2);
  
  for(int i= 0; i <= 255; i+=8)
  {
    analogWrite(TFT_BACKLIGHT, i);
    delay(50);   
  }
  analogWrite(TFT_BACKLIGHT, 255);
}

void showAnswer(char *answer)
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_BLUE);
  analogWrite(TFT_BACKLIGHT, 0);
  delay(100);
  char *line = wrapWord(answer); // get first wrapped line
  int ypos = 60;
  while(strlen(line) > 0)
  {
    Serial.print("got line: '");Serial.print(line);Serial.println('"');
    tft.setCursor((126 - getStringLength(line))/2, ypos);
    tft.print(line);
    line = wrapWord(); // get next wrapped line
    ypos += 12;
  }
  for(int i= 0; i <= 255; i+=8)
  {
    analogWrite(TFT_BACKLIGHT, i);
    delay(50);   
  }
  analogWrite(TFT_BACKLIGHT, 255);
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  //while (!Serial);

  Serial.println("The Magic Hallowing");
  Serial.println("by DanTheGeek.com");
  
  tft.initR(INITR_144GREENTAB);
  tft.setRotation(2);

  if (! lis.begin(0x18) && ! lis.begin(0x19)) {
    Serial.println("Couldnt start lis3dh");
    while (1);
  }
  Serial.println("LIS3DH found!");

  // Initialize backlight backlight
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, HIGH);
  analogWrite(TFT_BACKLIGHT, 0);

  tft.setTextWrap(false);
  showTitle(false);
}

void loop() {
  static bool showtitle = true;
  static uint32_t shakecounter = 0;
  static uint32_t counter = 0;
  sensors_event_t event; 
  lis.getEvent(&event);
  //Serial.println("debug - shake vector: " + String((event.acceleration.x*event.acceleration.x + event.acceleration.y*event.acceleration.y + 
  //  event.acceleration.z*event.acceleration.z),2));
  if((event.acceleration.x*event.acceleration.x + event.acceleration.y*event.acceleration.y + 
    event.acceleration.z*event.acceleration.z) > SHAKE_THRESHOLD)
  {
    counter = 0;
    //use shake counter to prevent catching other movements
    shakecounter++;
    if(shakecounter > 2)
    {
      shakecounter = 0;
      int random_answer = random(sizeof(answers)/sizeof(char *));
      Serial.println("answer is " + String(random_answer) + ": " + answers[random_answer]);
      tft.setTextSize(1);
      showAnswer(answers[random_answer]);
      showtitle = false;
    }
  }
  else
  {
    shakecounter = 0;
  }
  counter++;
  if((counter%100) == 0 && !showtitle)
  {
    showTitle();
    showtitle = true;
  }
  delay(50);
}
