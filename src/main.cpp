#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- PINS ----------------
#define BUTTON_PIN 18
#define EXT_LED_PIN 4
#define BUILTIN_LED_PIN 2

// ---------------- STATE ----------------
String song = "";
String artist = "";
bool isPlaying = false;

unsigned long progressMs = 0;
unsigned long durationMs = 1;
bool lastButtonState = HIGH;

// ---------------- VISUALIZER ----------------
int visValue = 0;
float rotPhase = 0.0;

// ---------------- BPM LED ----------------
int bpm = 120;
unsigned long beatInterval = 500;
unsigned long lastBeatMillis = 0;
bool ledState = false;

// ---------------- TEXT SCROLL ----------------
const int TEXT_X = 0;
const int TEXT_WIDTH = 78;     // HARD LIMIT (visualizer starts at 80)
const int CHAR_W = 6;
const int SCROLL_GAP = 20;
const int SCROLL_DELAY = 25;

int songPixelOffset = 0;
int artistPixelOffset = 0;
unsigned long songTimer = 0;
unsigned long artistTimer = 0;

// ---------------- UTILS ----------------
String formatTime(unsigned long ms) {
  unsigned long sec = ms / 1000;
  char buf[8];
  sprintf(buf, "%lu:%02lu", sec / 60, sec % 60);
  return String(buf);
}

// ---------------- HARD-CLIPPED MARQUEE (PERFECT, NO OVERLAP) ----------------
void drawMarquee(
  const String &text,
  int y,
  int &pixelOffset,
  unsigned long &timer
) {
  int textPixels = text.length() * CHAR_W;
  int totalPixels = textPixels + SCROLL_GAP;

  // Clear ONLY the text row (prevents ghosting + overlap)
  display.fillRect(TEXT_X, y, TEXT_WIDTH, 8, SSD1306_BLACK);

  // No scrolling needed
  if (textPixels <= TEXT_WIDTH) {
    display.setCursor(TEXT_X, y);
    display.print(text);
    pixelOffset = 0;
    return;
  }

  // Draw TWO copies for seamless loop
  for (int copy = 0; copy < 2; copy++) {
    int baseX = TEXT_X - pixelOffset + copy * totalPixels;

    for (int i = 0; i < text.length(); i++) {
      int charX = baseX + i * CHAR_W;

      // HARD CLIP: never enter visualizer area
      if (charX >= TEXT_WIDTH || charX + CHAR_W <= 0) continue;

      display.setCursor(charX, y);
      display.write(text[i]);
    }
  }

  // Smooth pixel scroll
  if (millis() - timer >= SCROLL_DELAY) {
    pixelOffset++;
    if (pixelOffset >= totalPixels) {
      pixelOffset = 0;
    }
    timer = millis();
  }
}

// ---------------- CIRCULAR VISUALIZER ----------------
void drawCircularVisualizer() {
  int cx = 100;
  int cy = 24;
  int baseR = 14;
  int maxR = 24;

  int radius = map(visValue, 0, 100, baseR, maxR);
  if (!isPlaying) radius = baseR;

  for (int a = 0; a < 360; a += 14) {
    float rad = radians(a) + rotPhase;
    int x = cx + cos(rad) * radius;
    int y = cy + sin(rad) * radius;

    if (x >= 80 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
      display.drawPixel(x, y, SSD1306_WHITE);
    }
  }

  display.fillCircle(cx, cy, 2, SSD1306_WHITE);

  if (isPlaying) {
    rotPhase += 0.06;
    if (rotPhase > TWO_PI) rotPhase -= TWO_PI;
  }
}

// ---------------- UI ----------------
void drawUI() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print(isPlaying ? "NOW PLAYING" : "PAUSED");

  drawMarquee(song,   14, songPixelOffset,   songTimer);
  drawMarquee(artist, 24, artistPixelOffset, artistTimer);

  // Timeline
  int barY = 42;
  display.drawRect(0, barY, 128, 6, SSD1306_WHITE);
  int fillW = map(progressMs, 0, durationMs, 0, 126);
  display.fillRect(1, barY + 1, fillW, 4, SSD1306_WHITE);

  // Time
  display.setCursor(0, 52);
  display.print(formatTime(progressMs));
  display.setCursor(90, 52);
  display.print(formatTime(durationMs));

  drawCircularVisualizer();
  display.display();
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  Wire.begin();

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(EXT_LED_PIN, OUTPUT);
  pinMode(BUILTIN_LED_PIN, OUTPUT);
  digitalWrite(BUILTIN_LED_PIN, HIGH);
}

// ---------------- LOOP ----------------
void loop() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.startsWith("SONG:")) {
      song = line.substring(5);
      songPixelOffset = 0;
    }
    else if (line.startsWith("ARTIST:")) {
      artist = line.substring(7);
      artistPixelOffset = 0;
    }
    else if (line.startsWith("STATE:")) isPlaying = (line.substring(6) == "PLAY");
    else if (line.startsWith("PROGRESS:")) progressMs = line.substring(9).toInt();
    else if (line.startsWith("DURATION:")) durationMs = line.substring(9).toInt();
    else if (line.startsWith("VIS:")) visValue = line.substring(4).toInt();
    else if (line.startsWith("BPM:")) {
      bpm = line.substring(4).toInt();
      if (bpm > 30 && bpm < 300) beatInterval = 60000 / bpm;
    }
  }

  // BPM LED
  if (isPlaying && millis() - lastBeatMillis >= beatInterval) {
    lastBeatMillis = millis();
    ledState = !ledState;
    digitalWrite(EXT_LED_PIN, ledState);
  }
  if (!isPlaying) digitalWrite(EXT_LED_PIN, LOW);

  // Button
  bool btn = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && btn == LOW) {
    Serial.println("TOGGLE");
    delay(200);
  }
  lastButtonState = btn;

  drawUI();
  delay(20);
}
