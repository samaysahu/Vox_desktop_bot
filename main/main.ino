/*********************************************************************
  Touch-OLED + DHT11 + NTP (scroll-in animation)
  ESP8266 (NodeMCU) – SSD1306 128×64
*********************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "face.h"

/* --------------------- OLED --------------------- */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA D5
#define OLED_SCL D6
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


/* --------------------- Touch --------------------- */
#define TOUCH_PIN D2                     // TTP223B SIG
bool displayState = false;               // false = blank
bool lastTouchState = LOW;
unsigned long lastTouchTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;

/* --------------------- DHT11 --------------------- */
#define DHTPIN D7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
unsigned long lastTempRead = 0;
const unsigned long TEMP_READ_INTERVAL = 2000;
float currentTemp = 0.0;
float currentHumidity = 0.0;
bool tempReadSuccess = false;

/* --------------------- Wi-Fi / NTP --------------------- */
const char* ssid = "Nisha 4g";
const char* password = "khush292009";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);   // UTC+5:30

/* --------------------- Animation --------------------- */
bool firstRunAfterTouch = true;
unsigned long screenOnTime = 0;
const unsigned long SCREEN_ON_DURATION = 8000;   // 8 s total

/* --------------------- Icons (PROGMEM) --------------------- */
static const unsigned char PROGMEM temp_icon_dt[] = {
  0b00000100, 0b00001010, 0b00001010, 0b00001010,
  0b00001110, 0b00011111, 0b00011111, 0b00001110
};

static const unsigned char PROGMEM humidity_icon_dt[] = {
  0b00000100, 0b00000100, 0b00001010, 0b00001010,
  0b00010001, 0b00010001, 0b00010001, 0b00001110
};

const char *months_dt[] = {"Jan","Feb","Mar","Apr","May","Jun",
                           "Jul","Aug","Sep","Oct","Nov","Dec"};
const char *daysOfWeek_dt[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

/* -------------------------------------------------------------
   Helper: centre a string at the given Y-offset
   ------------------------------------------------------------- */
void centerText(const char *txt, uint8_t yOffset, uint8_t textSize)
{
  display.setTextSize(textSize);
  int16_t x1, y1;
  uint16_t w, th;                     // th = text height (uint16_t!)
  display.getTextBounds(txt, 0, 0, &x1, &y1, &w, &th);
  int16_t x = (SCREEN_WIDTH - w) / 2;
  display.setCursor(x, yOffset);
  display.print(txt);
}

/* -------------------------------------------------------------
   Draw the whole screen (date, time, temp, humi) at a Y-offset
   ------------------------------------------------------------- */
void drawDateTimeScreen(int yOffset)
{
  /* ---- current time from NTP ---- */
  String formatted = timeClient.getFormattedTime();      // HH:MM:SS
  int hour   = formatted.substring(0,2).toInt();
  int minute = formatted.substring(3,5).toInt();
  int second = formatted.substring(6,8).toInt();

  /* ---- date string DD MMM YYYY WWW ---- */
  time_t raw = timeClient.getEpochTime();
  struct tm *ti = localtime(&raw);

  char dateStr[20];
  sprintf(dateStr, "%02d %s %04d %s",
          ti->tm_mday,
          months_dt[ti->tm_mon],
          ti->tm_year + 1900,
          daysOfWeek_dt[ti->tm_wday]);

  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);

  /* ---- draw ---- */
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  /* ----- DATE (centred) ----- */
  centerText(dateStr, 3 + yOffset, 1);
  display.drawFastHLine(0, 15 + yOffset, SCREEN_WIDTH, SSD1306_WHITE);

  /* ----- TIME (centred) ----- */
  centerText(timeStr, 18 + yOffset, 2);
  display.drawFastHLine(0, 35 + yOffset, SCREEN_WIDTH, SSD1306_WHITE);

  /* ----- TEMP ----- */
  display.setTextSize(1);
  display.setCursor(5, 40 + yOffset);
  display.print("TEMP");
  display.drawBitmap(55, 40 + yOffset, temp_icon_dt, 8, 8, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(5, 50 + yOffset);
  if (tempReadSuccess) {
    char buf[6];
    dtostrf(currentTemp, 4, 1, buf);
    display.print(buf);
  } else {
    display.print("N/A");
  }
  display.setTextSize(1);
  display.print("C");

  /* ----- HUMI ----- */
  display.setTextSize(1);
  display.setCursor(70, 40 + yOffset);
  display.print("HUMI");
  display.drawBitmap(115, 40 + yOffset, humidity_icon_dt, 8, 8, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(70, 50 + yOffset);
  if (tempReadSuccess) {
    char buf[6];
    dtostrf(currentHumidity, 4, 1, buf);
    display.print(buf);
  } else {
    display.print("N/A");
  }
  display.setTextSize(1);
  display.print("%");

  display.display();
}

/* ------------------------------------------------------------- */
void setup()
{
  Serial.begin(115200);
  dht.begin();
  pinMode(TOUCH_PIN, INPUT);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED init failed!");
    for (;;);
  }
  Serial.println("OLED OK");
  center_eyes();

  /* ---- Wi-Fi ---- */
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500); Serial.print("."); attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK");
    timeClient.begin();
    timeClient.update();

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi Connected!");
    display.println("Time synced");
    display.display();
    delay(1500);
  } else {
    Serial.println("\nWiFi FAILED");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi Failed!");
    display.println("Time unavailable");
    display.display();
    delay(1500);
  }

  display.clearDisplay();
  display.display();
}

/* ------------------------------------------------------------- */
void loop()
{
  /* ---- keep NTP fresh ---- */
  if (WiFi.status() == WL_CONNECTED) timeClient.update();

  /* ---- touch handling ---- */
  bool curTouch = digitalRead(TOUCH_PIN);
  unsigned long now = millis();

  if (curTouch == HIGH && lastTouchState == LOW &&
      (now - lastTouchTime) > DEBOUNCE_DELAY) {
    displayState = true;                 // turn ON
    Serial.println("Touch → Display ON (8 s)");
    lastTouchTime = now;
  }
  lastTouchState = curTouch;

  /* ---- DISPLAY ON ---- */
  if (displayState) {
    /* start timer on first entry */
    if (!screenOnTime) {
      screenOnTime = millis();
      firstRunAfterTouch = true;
    }

    /* ---- read DHT every 2 s ---- */
    if (now - lastTempRead >= TEMP_READ_INTERVAL) {
      lastTempRead = now;
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      if (isnan(h) || isnan(t)) {
        tempReadSuccess = false;
      } else {
        currentTemp = t;
        currentHumidity = h;
        tempReadSuccess = true;
      }
    }

    /* ---- scroll-in animation (once) ---- */
    if (firstRunAfterTouch) {
      for (int y = -64; y <= 0; y += 4) {
        drawDateTimeScreen(y);
        delay(10);                     // Faster: 20ms per frame (~340ms total)
        if (millis() - screenOnTime >= SCREEN_ON_DURATION) break;
      }
      firstRunAfterTouch = false;
    }

    /* ---- static update (every 500 ms) ---- */
    static unsigned long lastStatic = 0;
    if (now - lastStatic >= 500) {
      drawDateTimeScreen(0);
      lastStatic = now;
    }

    /* ---- auto-off after 8 s ---- */
    if (now - screenOnTime >= SCREEN_ON_DURATION) {
      displayState = false;
      display.clearDisplay();
      display.display();
      screenOnTime = 0;
      Serial.println("Screen timed-out → OFF");
    }
  }
  else {
    if (screenOnTime) screenOnTime = 0;   // reset when OFF
    face_loop();
  }

  delay(10);
}
