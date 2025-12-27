#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define BUTTON_PIN 18
bool lastButtonState = HIGH;
#define EXT_LED_PIN 4      // external LED
#define BUILTIN_LED_PIN 2  // ESP32 onboard LED
bool isPlaying = false;
unsigned long lastBlink = 0;
bool ledState = false;



Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// -------- DATA --------
String song = "";
String artist = "";
String state = "PLAY";

unsigned long progressMs = 0;
unsigned long durationMs = 1;

// Scrolling
int scrollX = 0;

// -------- UTILS --------
String formatTime(unsigned long ms) {
  unsigned long sec = ms / 1000;
  unsigned int m = sec / 60;
  unsigned int s = sec % 60;

  char buf[8];
  sprintf(buf, "%u:%02u", m, s);
  return String(buf);
}

void drawUI() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Header
  display.setCursor(0, 0);
  display.print(state == "PLAY" ? "NOW PLAYING" : "PAUSED");

  // Song
  display.setCursor(0, 14);
  display.print(song);

  // Artist
  display.setCursor(0, 24);
  display.print(artist);

  // Progress bar
  int barY = 42;
  display.drawRect(0, barY, 128, 6, SSD1306_WHITE);

  int fillW = map(progressMs, 0, durationMs, 0, 126);
  fillW = constrain(fillW, 0, 126);

  display.fillRect(1, barY + 1, fillW, 4, SSD1306_WHITE);

  // Time
  display.setCursor(0, 52);
  display.print(formatTime(progressMs));

  display.setCursor(90, 52);
  display.print(formatTime(durationMs));

  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(EXT_LED_PIN, OUTPUT);
  pinMode(BUILTIN_LED_PIN, OUTPUT);

  digitalWrite(BUILTIN_LED_PIN, HIGH); // ✅ always ON


}

void loop() {
  // ---- Read Serial ----
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');

    if (line.startsWith("SONG:")) song = line.substring(5);
    if (line.startsWith("ARTIST:")) artist = line.substring(7);
    if (line.startsWith("STATE:")) state = line.substring(6);
    if (line.startsWith("PROGRESS:")) progressMs = line.substring(9).toInt();
    if (line.startsWith("DURATION:")) durationMs = line.substring(9).toInt();
  }

  // ---- Scroll logic ----
  if (song.length() > 16) {
    scrollX++;
    if (scrollX > song.length() * 6) scrollX = 0;
  } else {
    scrollX = 0;
  }

  drawUI();
  delay(120);
  bool buttonState = digitalRead(BUTTON_PIN);
  if (Serial.available()) {
  String line = Serial.readStringUntil('\n');
  line.trim();

  if (line.startsWith("SONG:")) song = line.substring(5);
  else if (line.startsWith("ARTIST:")) artist = line.substring(7);
  else if (line.startsWith("STATE:")) {
    state = line.substring(6);
    isPlaying = (state == "PLAY");
  }
  else if (line.startsWith("PROGRESS:")) progressMs = line.substring(9).toInt();
  else if (line.startsWith("DURATION:")) durationMs = line.substring(9).toInt();
}

if (isPlaying) {
  if (millis() - lastBlink >= 500) {   // blink speed (ms)
    lastBlink = millis();
    ledState = !ledState;
    digitalWrite(EXT_LED_PIN, ledState);
  }
} else {
  digitalWrite(EXT_LED_PIN, LOW);      // OFF when paused
}


// Detect button press (falling edge)
if (lastButtonState == HIGH && buttonState == LOW) {
  Serial.println("TOGGLE");   // send command to PC
  delay(200);                 // debounce
}

lastButtonState = buttonState;

}
