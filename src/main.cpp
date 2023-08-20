#include <M5Stack.h>
#include <WiFi.h>

#include "time.h"

const char* ssid = "yourSSID";
const char* password = "yourPASSWORD";

const char* ntp_server = "ntp.jst.mfeed.ad.jp";
const long gmt_offset_sec = 9 * 3600;
const int daylight_offset_sec = 0;

const int alarm_start_hour = 8;
const int alarm_end_hour = 21;

int beep_volume = 3;

int text_color = TFT_PURPLE;

// Forward declaration needed for IDE 1.6.x
static uint8_t conv2d(const char* p);

// Get H, M, S from compile time
uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3),
        ss = conv2d(__TIME__ + 6);

void drawTimeTextSet(int time) {
  if (time < 10) M5.Lcd.print('0');
  M5.Lcd.print(time);
}

void drawColon(int sec) {
  if (sec % 2) {  // Flash the colons on/off
    M5.Lcd.setTextColor(0x0001, TFT_BLACK);
    M5.Lcd.print(":");
    M5.Lcd.setTextColor(text_color, TFT_BLACK);
  } else {
    M5.Lcd.print(":");
  }
}

void drawTime(int hh, int mm, int ss) {
  M5.Lcd.setTextColor(text_color, TFT_BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(52, 80, 7);
  drawTimeTextSet(hh);  // Draw hours
  drawColon(ss);
  drawTimeTextSet(mm);  // Draw minutes
  drawColon(ss);
  drawTimeTextSet(ss);  // Draw seconds
}

void beep() {
  for (size_t i = 0; i <= 1; i++) {
    M5.Speaker.setVolume(beep_volume);
    M5.Speaker.beep();
    delay(100);
    M5.Speaker.mute();
    delay(200);
  }
}

void btnAdisplayText(String text) {
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextDatum(1);
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextSize(2);
  M5.Lcd.drawString(text, 68, 220);
}

void setup(void) {
  M5.begin();
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(text_color, TFT_BLACK);
  M5.Speaker.begin();
  M5.Speaker.setVolume(beep_volume);

  // connect to WiFi
  M5.Lcd.printf("Connecting to %s \n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print(".");
  }
  M5.Lcd.println("CONNECTED!");

  // init and get the time
  configTime(gmt_offset_sec, daylight_offset_sec, ntp_server);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    M5.Lcd.println("Failed to obtain time");
    return;
  }
  // disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  M5.Lcd.clearDisplay();
}

void loop() {
  M5.update();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    M5.Lcd.println("Failed to obtain time");
    return;
  }
  hh = timeinfo.tm_hour;
  mm = timeinfo.tm_min;
  ss = timeinfo.tm_sec;

  // Show on display
  drawTime(hh, mm, ss);
  btnAdisplayText("Vol:" + String(beep_volume));

  // Set alarm volume
  if (M5.BtnA.isPressed()) {
    ++beep_volume;
    if (beep_volume > 6) beep_volume = 0;
    btnAdisplayText("Vol:" + String(beep_volume));
    beep();
  }

  // Set time signal
  if (hh >= alarm_start_hour && hh <= alarm_end_hour) {
    if ((mm == 00 && ss == 00)) {
      beep();
    }
  }
}

// Function to extract numbers from compile time string
static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9') v = *p - '0';
  return 10 * v + *++p - '0';
}