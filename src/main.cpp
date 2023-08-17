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

static uint8_t conv2d(
    const char* p);  // Forward declaration needed for IDE 1.6.x

uint8_t hh = conv2d(__TIME__), mm = conv2d(__TIME__ + 3),
        ss = conv2d(__TIME__ + 6);  // Get H, M, S from compile time

byte xcolon = 0, xsecs = 0;

void setup(void) {
  Serial.begin(115200);
  M5.begin();

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_PURPLE, TFT_BLACK);
  M5.Speaker.begin();
  M5.Speaker.setVolume(3);

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

void beep() {
  for (size_t i = 0; i <= 1; i++) {
    M5.Speaker.beep();
    delay(100);
    M5.Speaker.mute();
    delay(200);
  }
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

  // alarm set
  if (hh >= alarm_start_hour && hh <= alarm_end_hour) {
    if ((mm == 00 && ss == 00)) {
      beep();
    }
  }

  // alarm test
  if (M5.BtnA.isPressed()) {
    beep();
  }

  // Update digital time
  int xpos = 0;
  int ypos = 85;  // Top left corner ot clock text, about half way down
  int ysecs = ypos + 24;

  // Draw hours and minutes
  if (hh < 10)
    xpos += M5.Lcd.drawChar('0', xpos, ypos,
                            8);  // Add hours leading zero for 24 hr clock
  xpos += M5.Lcd.drawNumber(hh, xpos, ypos, 8);  // Draw hours
  xcolon = xpos;  // Save colon coord for later to flash on/off later
  xpos += 29;
  if (mm < 10)
    xpos += M5.Lcd.drawChar('0', xpos, ypos, 8);  // Add minutes leading zero
  xpos += M5.Lcd.drawNumber(mm, xpos, ypos, 8);   // Draw minutes
  xsecs = xpos;  // Sae seconds 'x' position for later display updates

  if (ss % 2) {  // Flash the colons on/off
    M5.Lcd.setTextColor(0x0001, TFT_BLACK);
    M5.Lcd.drawChar(':', xcolon, ypos - 8, 8);      // Hour:minute colon
    xpos += M5.Lcd.drawChar(':', xsecs, ysecs, 6);  // Seconds colon
    M5.Lcd.setTextColor(TFT_PURPLE, TFT_BLACK);

  } else {
    M5.Lcd.drawChar(':', xcolon, ypos - 8, 8);      // Hour:minute colon
    xpos += M5.Lcd.drawChar(':', xsecs, ysecs, 6);  // Seconds colon
  }

  // Draw seconds
  if (ss < 10)
    xpos += M5.Lcd.drawChar('0', xpos, ysecs, 6);  // Add leading zero
  M5.Lcd.drawNumber(ss, xpos, ysecs, 6);           // Draw seconds
}

// Function to extract numbers from compile time string
static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9') v = *p - '0';
  return 10 * v + *++p - '0';
}