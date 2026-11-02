#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
//screen pins
const int PIN_LCD_DC = 14;     // 43 on the real board
const int PIN_LCD_BL = 10;
const int title = 3;
const int text = 2;
const int textSmall = 1;
const int answer = 5;
int w, h;
// Basic colours (RGB565 format, see below)
const uint16_t BLACK    = 0x0000;
const uint16_t WHITE    = 0xFFFF;
const uint16_t RED      = 0xF800;
const uint16_t GREEN    = 0x07E0;
const uint16_t BLUE     = 0x001F;
const uint16_t YELLOW   = 0xFFE0;
const uint16_t CYAN     = 0x07FF;
const uint16_t MAGENTA  = 0xF81F;
const uint16_t ORANGE   = 0xFD20;
const uint16_t PINK     = 0xFB56;
const uint16_t PURPLE   = 0x8010;
const uint16_t TEAL     = 0x0410;
const uint16_t NAVY     = 0x0010;
const uint16_t GREY     = 0x8410;
const uint16_t DARKGREY = 0x4208;
bool home_screen = true;
Adafruit_ILI9341 tft(&SPI, PIN_LCD_DC, -1, -1);   // -1 = CS and RST not used


unsigned long lastBat = 0;
const int PIN_TRIGGER = 2;
const int PIN_LASER = 7;
const int PIN_BUZZER = 13;
int light_value = LOW;
bool wasPressed_but = false;
//flashing
unsigned long lastBlink = 0;
int blinkValue = LOW;
unsigned long lastChange = 0;
//sound
int  noteIndex = -1;            
unsigned long noteStart = 0;

int  melodyFreq[3] = {2000, 2500, 3000};
int  melodyMs[3]   = {80, 80, 150};

const int clickFreq[] = {2500};            const int clickMs[] = {40};
const int lockFreq[]  = {2000, 3000};      const int lockMs[]  = {70, 120};
const int errorFreq[] = {800};             const int errorMs[] = {400};

const int* curFreq = nullptr;   
const int* curMs   = nullptr;   
int curLen = 0;     
//rotary encoder
const int PIN_ENC_A = 4, PIN_ENC_B = 5, PIN_ENC_SW = 3;
volatile int wheelCount = 0;            
int lastWheelCount = 0;
void IRAM_ATTR onWheel() {
  if (digitalRead(PIN_ENC_B) == HIGH) wheelCount++;
  else                                wheelCount--;
}
float noise_reducer(float (*readFn)(),int count){
  float sum = 0;
  for (int i=0;i<count;i++){
    sum += readFn();
  }
  return sum / count;
}

void playMelody(const int freq[], const int ms[], int len) {
  curFreq = freq;
  curMs = ms;
  curLen = len;
  noteIndex = 0;
  noteStart = millis();
  tone(PIN_BUZZER, curFreq[0], curMs[0]);
}

void updateMelody(){
  if (noteIndex==-1) return;
  if (millis() - noteStart >= curMs[noteIndex]) {
    noteIndex++;
    if (noteIndex < curLen) {
      noteStart = millis();
      tone(PIN_BUZZER, curFreq[noteIndex], curMs[noteIndex]);
    } else {
      noteIndex = -1;
    }
  }
}
float readPinVolts(){
  return analogReadMilliVolts(1) * 2 / 1000.0;
}
float readBatteryVolts(){
  return noise_reducer(readPinVolts,16);
}
int batteryPercent(float volts){
  volts = constrain(volts,3.5,4.2);
  return round((volts-3.5)/(4.2-3.5)*100);
}
int menuIndex (int options, int current){
  return ((current % options) + options) % options;
}
void textSizePx(int textSize, int length, int &w, int &h) {
  w = length * textSize * 6;
  h = textSize * 8;
}

void homeScreen () {
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(title);
  tft.setCursor(20, 20);
  tft.print("Tools:");
 
  tft.setCursor(20, 80);
  tft.setTextSize(text);
  int16_t x = tft.getCursorX();
  int16_t y = tft.getCursorY();
  tft.print("Motion");
  textSizePx(text, 6, w, h);
  tft.drawRect(x - 4, y - 4, w + 8, h + 8, WHITE); 
 
  tft.setCursor(20, 120);
  x = tft.getCursorX();
  y = tft.getCursorY();
  tft.print("Measure");
  textSizePx(text, 7, w, h);
  tft.drawRect(x - 4, y - 4, w + 8, h + 8, WHITE); 
 
  tft.setCursor(20, 160);
  x = tft.getCursorX();
  y = tft.getCursorY();
  tft.print("Room Info");
  textSizePx(text, 9, w, h);
  tft.drawRect(x - 4, y - 4, w + 8, h + 8, WHITE); 
 
  tft.setCursor(20, 200); 
  x = tft.getCursorX();
  y = tft.getCursorY();
  tft.print("Thermometor");
  textSizePx(text, 11, w, h);
  tft.drawRect(x - 4, y - 4, w + 8, h + 8, WHITE); 
 
  tft.setCursor(20, 240);
  x = tft.getCursorX();
  y = tft.getCursorY();
  tft.print("Light");
  textSizePx(text, 5, w, h);
  tft.drawRect(x - 4, y - 4, w + 8, h + 8, WHITE); 
 
  tft.setCursor(20, 280);
  x = tft.getCursorX();
  y = tft.getCursorY();
  tft.print("Settings");
  textSizePx(text, 8, w, h);
  tft.drawRect(x - 4, y - 4, w + 8, h + 8, WHITE); 
}
void motion(){
  tft.print("Motion");
}
void measure(){
  tft.print("Measure");
}
void roomInfo(){
  tft.print("Room Info");
}
void thermometer(){
  tft.print("Thermometer");
}
void light(){
  tft.print("Light");
}
void settings(){
  tft.print("Settings");
}
void menu (int index) {
  tft.setTextSize(title);
  tft.setCursor(20, 60);
  switch (index) {
  case 0: motion(); break;
  case 1: measure(); break;
  case 2: roomInfo(); break;
  case 3: thermometer(); break;
  case 4: light(); break;
  case 5: settings(); break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIGGER, INPUT_PULLUP);
  pinMode(PIN_LASER, OUTPUT);
  lastBlink = millis();
  blinkValue = LOW;
  //rotary encoder
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), onWheel, FALLING);
  //screen
  SPI.begin(12, -1, 11);        // SCK, MISO (none), MOSI
  Serial.println("3 setup running");
  tft.begin();
  Serial .println("3.1 setup running");
  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, HIGH);  // backlight on
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(title);
  tft.setCursor(20, 20);
  homeScreen();
}
void loop() {
  static bool once = false;
if (!once) { Serial.println("4 loop running"); once = true; }
  updateMelody();
  if (digitalRead(PIN_TRIGGER) == LOW) {
    homeScreen();
  }
  if (digitalRead(PIN_ENC_SW) == LOW) {
    menu(menuIndex(6, wheelCount));
  }
  if (wheelCount != lastWheelCount) {
    Serial.printf("Wheel: %d\n\r", menuIndex(6, wheelCount));
    lastWheelCount = wheelCount;
  }
  
  if (millis()-lastBat >= 1000) {
    float v = readBatteryVolts();
    int pct = batteryPercent(v);
    lastBat = millis();
    Serial.printf("Battery: %.2f V  %d\n\r", v, pct);
  }

  bool isPressed_but = (digitalRead(PIN_TRIGGER) == LOW);

  if (isPressed_but != wasPressed_but && millis() - lastChange >= 50) {
  
    lastChange = millis();
    wasPressed_but = isPressed_but;

    if (isPressed_but) {                      
      light_value = !light_value;
      digitalWrite(PIN_LASER, light_value);
      lastBlink = millis();             
      blinkValue = LOW;
      if (light_value == HIGH) {
        playMelody(lockFreq, lockMs, 2);
      }
      else if (light_value == LOW) {
        playMelody(errorFreq, errorMs, 1);
      }
    }
  }

  if (light_value == HIGH && millis() - lastBlink >= 500) {
    digitalWrite(PIN_LASER, blinkValue);
    blinkValue = !blinkValue;
    lastBlink = millis();
  }

}