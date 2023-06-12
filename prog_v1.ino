#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_ST7735.h> // Hardware-specific library

/// define encodeur
#define KEY 2         // KEY pin
#define S2 3          // S2 pin
#define S1 4          // S1 pin
#define CCW 0b010010  // knob turn CCW pattern
#define CW 0b100001   // knob turn CW pattern
#define DEF 0b11      // default / base value of encoder signal

/////define TFT
#define TFT_CS 10
#define TFT_RST 9
#define TFT_DC 8

#define BLACK 0x0000
#define RED 0x001F
#define BLUE 0xF800
#define GREEN 0x07E0
#define YELLOW 0x07FF
#define PURPLE 0xF81F
#define CYAN 0xFFE0
#define WHITE 0xFFFF

// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Option 2: use any pins but a little slower!
#define TFT_SCLK 13  // set these to be whatever pins you like!
#define TFT_MOSI 11  // set these to be whatever pins you like!

///////////////////////////////

//variables encodeur
byte prevPattern = 0;    // previous signal pattern
byte pattern = 0;        // actual signal pattern
bool keyPushed = false;  // button push status

int compteur = 0;

void setup() {

  ////encodeur
  pinMode(KEY, INPUT);
  pinMode(S2, INPUT);
  pinMode(S1, INPUT);
  Serial.begin(9600);
  ////TFT
  // Use this initializer if you're using TFT
  tft.initR(INITR_BLACKTAB);  // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);

  print_labels();
  
}

void loop() {
//fonction lit encodeur
  readEncoder();
 //fonction affiche sur TFT
  print_labels();
}

//////////////////////// fonction affiche sur TFT
void ecris_tft(byte txt_size, char* txt_color, char* bck_color, byte pos_txt_x, byte pos_txt_y, char* txt) {
  tft.setTextSize(txt_size);
  tft.setTextColor(txt_color, bck_color);
  tft.setCursor(pos_txt_x, pos_txt_y);
  tft.println(txt);
}

////////////affiche sur TFT
void print_labels() {
  //sensors_event_t humidity, temp;
  ecris_tft(1, BLUE, BLACK, 5, 5, "test");
  ecris_tft(1, BLUE, BLACK, 5, 15, "test2");
  
}
///////////
 //fonction lit encodeur
void readEncoder() {
  byte aktS1 = digitalRead(S1);
  byte aktS2 = digitalRead(S2);
  byte aktKey = !digitalRead(KEY);

  if (keyPushed != aktKey) {
    keyPushed = aktKey;
    Serial.println(keyPushed ? "Key push" : "Key release");
    compteur = 0;
  }

  byte aktPattern = (aktS1 << 1) | aktS2;
  if (aktPattern != prevPattern) {
    prevPattern = aktPattern;
    if (aktPattern != DEF) {
      pattern = (pattern << 2) | aktPattern;
    } else {
      if (pattern == CW) {
        compteur = compteur + 1;
        Serial.println("Turn right");
        Serial.print(compteur);
      } else if (pattern == CCW) {
        compteur = compteur - 1;
        Serial.println("Turn left");
        Serial.print(compteur);
      }
      pattern = 0;
    }
  }
}
