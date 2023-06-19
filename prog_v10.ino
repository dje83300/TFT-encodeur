#include <RTClib.h>

#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

RTC_DS1307 rtc;

//define boutons paramètres
#define button_param_heure 5
//define encodeur
#define KEY 2         // KEY pin
#define S2 3          // S2 pin
#define S1 4          // S1 pin
#define CCW 0b010010  // knob turn CCW pattern
#define CW 0b100001   // knob turn CW pattern
#define DEF 0b11      // default / base value of encoder signal
//define TFT
#define TFT_CS 10
#define TFT_RST 9
#define TFT_DC 8
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
//define couleurs TFT HEX
#define BLACK 0x0000
#define RED 0x001F
#define BLUE 0xF800
#define GREEN 0x07E0
#define YELLOW 0x07FF
#define PURPLE 0xF81F
#define CYAN 0xFFE0
#define WHITE 0xFFFF

//variables encodeur
byte prevPattern = 0;    // previous signal pattern
byte pattern = 0;        // actual signal pattern
bool keyPushed = false;  // button push status
bool mode_parametre_heure = false;
bool heure_debut_atteinte = false;
//variables étapes paramètre
int compteur_parametre = 0;

char text[3];
char Time[] = "  :  :  ";
char heure_debut[] = "  :  :  ";
char heure_debut_avec_duree[] = "  :  :  ";
//variables RCT
int new_second, new_minute, new_hour;
int min_intervalle;
int min_duree;
//String heure_debut ;
void setup(void) {

  //encodeur
  pinMode(KEY, INPUT);
  pinMode(S2, INPUT);
  pinMode(S1, INPUT);
  //BOUTONS
  pinMode(button_param_heure, INPUT_PULLUP);
  //led paramètre
  pinMode(12, OUTPUT);
  //RTC
  Wire.begin();  // Initialise la communication I2C
  rtc.begin();   // Initialise le RTC
  Serial.begin(9600);
  //TFT
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  //initialise le mode paramètre
  compteur_parametre = 0;
  min_intervalle = 0;
  min_duree = 0;
  mode_parametre_heure = false;
  heure_debut_atteinte = false;
  //mode_parametre_intervalle = false;
  delay(1000);
}
void calcule_heure_debut() {  // calcule l'heure de début du cycle (heure actuelle + minutes intervalle)
  if (heure_debut_atteinte = true && min_intervalle >= 1) {
    DateTime now = rtc.now();   // Obtient la date et l'heure actuelles à partir du RTC
    int hour = now.hour();      // Obtient l'heure
    int minute = now.minute();  // Obtient les minutes
    int second = now.second();  // Obtient les secondes
    minute += min_intervalle;   //additionne l'intervalle souhaitée aux minutes actuelles de l'heure
    if (minute >= 60) {
      hour += minute / 60;  // Ajoute les heures supplémentaires
      minute %= 60;         // Met à jour les minutes restantes
    }
    heure_debut[7] = second % 10 + 48;
    heure_debut[6] = second / 10 + 48;
    heure_debut[4] = minute % 10 + 48;
    heure_debut[3] = minute / 10 + 48;
    heure_debut[1] = hour % 10 + 48;
    heure_debut[0] = hour / 10 + 48;
    heure_debut_atteinte = false;
  }
}

// void calcule_heure_debut_et_duree(){
// heure_debut[4] += 1;//min_duree ;//min_intervalle ;A DECOMMENTER -> PHASE DE TEST
// if (heure_debut[4] >= 60) {
//     heure_debut[1] += heure_debut[4] / 60; // Ajoute les heures supplémentaires
//     heure_debut[4] %= 60;        // Met à jour les minutes restantes
//   }
// heure_debut_avec_duree[7] = heure_debut[7];
//   heure_debut_avec_duree[6] = heure_debut[6];
//   heure_debut_avec_duree[4] = heure_debut[4];
//   heure_debut_avec_duree[3] = heure_debut[3];
//   heure_debut_avec_duree[1] = heure_debut[1];
//   heure_debut_avec_duree[0] = heure_debut[0];
// }

void affich_heure() {  // Affiche l'heure sur l'écran TFT

  DateTime now = rtc.now();  // Obtient la date et l'heure actuelles à partir du RTC

  int hour = now.hour();      // Obtient l'heure
  int minute = now.minute();  // Obtient les minutes
  int second = now.second();  // Obtient les secondes

  tft.fillRect(5, 5, 120, 120, BLACK);  //efface la zone heure
  tft.setTextColor(WHITE);              // Couleur du texte: blanc
  tft.setTextSize(1);                   // Taille du texte: 2

  tft.setCursor(5, 5);
  if (hour < 10) {
    tft.print("0");
  }
  tft.print(hour);
  tft.print(":");

  if (minute < 10) {
    tft.print("0");
  }
  tft.print(minute);
  tft.print(":");

  if (second < 10) {
    tft.print("0");
  }
  tft.print(second);

  //AFFICHE INTERVALLE

  tft.setCursor(5, 15);
  tft.setTextColor(YELLOW);

  if (min_intervalle < 100) {
    tft.print("0");
    tft.print(min_intervalle);
  } else {
    tft.print(min_intervalle);
  }
  tft.print(" Min intervalle");
  //AFFICHE durée

  tft.setCursor(5, 35);
  tft.setTextColor(GREEN);
  if (min_duree < 100) {
    tft.print("0");
    tft.print(min_duree);
  } else {
    tft.print(min_duree);
  }
  tft.print(" ");
  tft.print("Min duree");


  //affiche l'heure de début
  tft.setCursor(5, 25);
  tft.setTextColor(RED);

  tft.print(heure_debut);
  tft.print(" eche. suiv.");
  delay(1000);  // Attente d'une seconde
}


void loop() {
  mode_param();               // un appui sur le bouton arrête l'heure et boucle dans les différents pramètres
  test_heure_alarme();        // teste si l'heure + minutes intervalle est atteinte
  if (!mode_parametre_heure)  //si pas en mode parametre heure alors affiche l'heure normalement
  {
    affich_heure();

  } else {

    //fonction lit encodeur si en mode parametre
    readEncoder();
  }
}
///////////////
void test_heure_alarme() {
  DateTime now = rtc.now();  // Obtient la date et l'heure actuelles à partir du RTC

  int hour = now.hour();      // Obtient l'heure
  int minute = now.minute();  // Obtient les minutes
  int second = now.second();  // Obtient les secondes

  Time[7] = second % 10 + 48;
  Time[6] = second / 10 + 48;
  Time[4] = minute % 10 + 48;
  Time[3] = minute / 10 + 48;
  Time[1] = hour % 10 + 48;
  Time[0] = hour / 10 + 48;

  if (String(Time) == String(heure_debut)) {
    heure_debut_atteinte = true;
    calcule_heure_debut();
    Serial.begin(9600);
    Serial.println("alarme");
    Serial.print("heure : ");
    Serial.print(String(Time));
    Serial.print(" ech. suiv : ");
    Serial.print(String(heure_debut));
  }
}

//////////////
void mode_param() {

  int buttonstate = digitalRead(button_param_heure);
  while (compteur_parametre >= 0 && digitalRead(button_param_heure) == LOW) {  
    {
      message();
      mode_parametre_heure = true;
      if (compteur_parametre == 0) {  //HEURES
        digitalWrite(12, HIGH);
        delay(300);
        //
        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(RED);
        tft.setCursor(5, 5);
        tft.print("00");
        tft.setTextColor(BLUE);
        tft.print(":00:00");
        //
        Serial.println(compteur_parametre);
        compteur_parametre = 1;
        break;
      }
    }

    if (compteur_parametre == 1) {  //MINUTES

      digitalWrite(12, HIGH);
      delay(300);
      //
      tft.fillScreen(ST7735_BLACK);
      tft.setTextColor(BLUE);
      tft.setCursor(5, 5);
      tft.print("00:");
      tft.setTextColor(RED);
      tft.print("00");
      tft.setTextColor(BLUE);
      tft.print(":00");
      //
      Serial.println(compteur_parametre);

      compteur_parametre = 2;
      break;
    }
    if (compteur_parametre == 2) {  //intervalle
      digitalWrite(12, HIGH);
      delay(300);
      tft.fillScreen(ST7735_BLACK);
      tft.setTextColor(RED);
      tft.setCursor(5, 15);
      tft.print("000");
      tft.setTextColor(BLUE);
      tft.print(" Min intervalle");
      Serial.println(compteur_parametre);
      compteur_parametre = 3;
      break;
    }
    if (compteur_parametre == 3) {  //duréer
      digitalWrite(12, HIGH);
      delay(300);
      tft.fillScreen(ST7735_BLACK);
      tft.setTextColor(RED);
      tft.setCursor(5, 35);
      tft.print("000");
      tft.setTextColor(BLUE);
      tft.print(" ");
      tft.print("Min dur\x82e");
      Serial.println(compteur_parametre);
      compteur_parametre = 4;
      break;
    }
  }
  if (compteur_parametre > 4) {//tous les paramètres sont renseignés
    
    compteur_parametre = 0;
    digitalWrite(12, LOW);
    
  }
}
//////MESSAGE
void message() {
  tft.fillScreen(ST7735_BLACK);
  if (compteur_parametre == 0) {
    ecris_tft(1, GREEN, BLACK, 5, 5, "R\x82glage de l'heure");
    delay(1000);
  }
  if (compteur_parametre == 1) {
    ecris_tft(1, GREEN, BLACK, 5, 5, "R\x82glage minutes");
    delay(1000);
  }
  if (compteur_parametre == 2) {
    ecris_tft(1, GREEN, BLACK, 5, 5, "R\x82glage intervalle");
    delay(1000);
  }
  if (compteur_parametre == 3) {
    ecris_tft(1, GREEN, BLACK, 5, 5, "R\x82glage dur\x82e");
    delay(1000);
  }
}
//////////////////////// fonction affiche sur TFT
void ecris_tft(byte txt_size, char* txt_color, char* bck_color, byte pos_txt_x, byte pos_txt_y, String txt) {
  tft.setTextSize(txt_size);
  tft.setTextColor(txt_color, bck_color);
  tft.setCursor(pos_txt_x, pos_txt_y);
  tft.println(txt);
}


//fonction regle l'heure
void regle_heure(int etape, int valeur_etape) {
  etape = compteur_parametre - 1;
  tft.fillScreen(ST7735_BLACK);  // Efface l'écran pour afficher la nouvelle heure
  tft.setTextColor(RED);         // Couleur du texte: rouge
  tft.setTextSize(1);            // Taille du texte: 1
  ////choix de la position
  switch (etape) {
    case 0:  //heure
      tft.fillScreen(ST7735_BLACK);
      tft.setTextColor(RED);
      tft.setCursor(5, 5);
      if (valeur_etape < 10) {
        tft.print("0");
      }
      tft.print(valeur_etape);
      tft.setTextColor(BLUE);
      tft.print(":00:00");
      break;
    case 1:  //minutes
      tft.fillScreen(ST7735_BLACK);
      tft.setCursor(5, 5);

      tft.setTextColor(BLUE);
      if (new_hour < 10) {
        tft.print("0");
      }
      tft.print(new_hour);
      tft.print(":");
      tft.setTextColor(RED);
      if (valeur_etape < 10) {
        tft.print("0");
      }
      tft.print(valeur_etape);
      tft.setTextColor(BLUE);
      tft.print(":00");
      break;
    case 2:  //intervalles
      tft.fillScreen(ST7735_BLACK);
      tft.setCursor(5, 15);
      if (valeur_etape < 100) {
        tft.print("0");
      }
      tft.setTextColor(RED);
      tft.print(min_intervalle);

      tft.setTextColor(BLUE);
      tft.print(" Min intervalle");
      break;
    case 3:  //durée
      tft.fillScreen(ST7735_BLACK);
      tft.setCursor(5, 35);
      if (valeur_etape < 100) {
        tft.print("0");
      }
      tft.setTextColor(RED);
      tft.print(min_duree);

      tft.setTextColor(BLUE);
      tft.print(" Min dur\x82e");
      break;
  }
}
//fonction lit l'encodeur
void readEncoder() {

  byte aktS1 = digitalRead(S1);
  byte aktS2 = digitalRead(S2);
  byte aktKey = !digitalRead(KEY);

  if (keyPushed != aktKey) {
    keyPushed = aktKey;
    Serial.println(keyPushed ? "Key push" : "Key release");
    digitalWrite(12, LOW);
    mode_parametre_heure = false;
    compteur_parametre = 0;
    rtc.adjust(DateTime(2000, 1, 1, new_hour, new_minute, new_second));
    calcule_heure_debut();
    affich_heure();
  }

  byte aktPattern = (aktS1 << 1) | aktS2;
  if (aktPattern != prevPattern) {
    prevPattern = aktPattern;
    if (aktPattern != DEF) {
      pattern = (pattern << 2) | aktPattern;
    } else {
      /////HEURE
      if (compteur_parametre == 1) {
        if (pattern == CW) {  // sens +
          new_hour = new_hour + 1;
          if (new_hour > 23) {
            new_hour = 0;
          }

        } else if (pattern == CCW) {  //sens -
          new_hour = new_hour - 1;
          if (new_hour < 0) {
            new_hour = 23;
          }
        }
        sprintf(text, "%02u", new_hour);
        Serial.println(text);
        regle_heure(0, new_hour);
        pattern = 0;
      }
      //MINUTES
      if (compteur_parametre == 2) {
        if (pattern == CW) {  // sens +
          new_minute = new_minute + 1;
          if (new_minute > 59) {
            new_minute = 0;
          }

        } else if (pattern == CCW) {  //sens -
          new_minute = new_minute - 1;
          if (new_minute < 0) {
            new_minute = 59;
          }
        }
        sprintf(text, "%02u", new_minute);
        Serial.println(text);
        regle_heure(0, new_minute);
        pattern = 0;
      }
      //INTERVALLE
      if (compteur_parametre == 3) {              
        if (pattern == CW) {                      // sens +
          min_intervalle = (min_intervalle) + 1;  //  * 10;; A DECOMMENTER POUR INCREMETER DE 10 EN 10
          if (min_intervalle > 120) {
            min_intervalle = 0;
          }

        } else if (pattern == CCW) {            //sens -
          min_intervalle = min_intervalle - 1;  // (min_intervalle )- 1  * 10; A DECOMMENTER POUR INCREMETER DE 10 EN 10
          if (min_intervalle < 0) {
            min_intervalle = 120;
          }
        }
        sprintf(text, "%03u", min_intervalle);
        Serial.println(text);
        regle_heure(2, min_intervalle);
        pattern = 0;
      }
      //durée
      if (compteur_parametre == 4) {  
        if (pattern == CW) {          // sens +
          min_duree = (min_duree) + 1 * 10;
          if (min_duree > 120) {
            min_duree = 0;
          }

        } else if (pattern == CCW) {  //sens -
          min_duree = (min_duree)-1 * 10;
          if (min_duree < 0) {
            min_duree = 120;
          }
        }
        sprintf(text, "%03u", min_duree);
        Serial.println(text);
        regle_heure(3, min_duree);
        pattern = 0;
      }
    }
  }
}
