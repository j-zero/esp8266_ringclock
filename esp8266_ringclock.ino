#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <FastLED.h>      // https://github.com/FastLED/FastLED
#include <ESPNtpClient.h> // https://github.com/gmag11/ESPNtpClient

#include "WifiConfig.h"

#define LED_COUNT 60
#define LED_OFFSET 30
#define LED_DATA_PIN 2

#define LO(led) ((led + LED_OFFSET < LED_COUNT) ? led + LED_OFFSET : led + LED_OFFSET - LED_COUNT)
#define L2(led) (led == 0 ? (LED_COUNT - 1) : led - 1)

#define SHOW_TIME_PERIOD 1000

CRGB leds[LED_COUNT];
char printf_buffer[32];
long led_hours[12];

void setup() { 
  Serial.begin (115200);
  LEDS.addLeds<WS2812,LED_DATA_PIN,GRB>(leds,LED_COUNT).setCorrection(TypicalSMD5050);
  LEDS.setBrightness(32);

  // LEDs für "Zifferblatt" abhängig von der Anzahl der LEDs berechen.
  for(int i = 0; i < 12; i++){
    led_hours[i] = LO(map(i*5, 0, 59, 0, LED_COUNT - 1));
  }
  
  WiFi.begin (WIFI_SSID, WIFI_PASSWD);

  NTP.setTimeZone (TZ_Europe_Berlin);
  NTP.begin ();
}

void loop() { 
  static int last = 0;
  
  FastLED.clear();

  long last_synced = NTP.getFirstSyncUs().tv_sec;
  
  int64_t us    = NTP.micros();
  int64_t now   = us / 1000;
  time_t  now_s = now / 1000;
  tm* local_tm  = localtime(&now_s);

  uint8_t h = local_tm->tm_hour;
  uint8_t m = local_tm->tm_min;
  uint8_t s = local_tm->tm_sec;
  long ms = (now % 1000) ;

  long hm = (h > 12 ? h - 12 : h) * 60 + m; // 0-12h auf 0-720min umrechnen um die passende LED zu mappen.

  uint8_t h_led = map(hm, 0, 719, 0, LED_COUNT - 1);
  uint8_t m_led = map(m,  0, 59,  0, LED_COUNT - 1);
  uint8_t s_led = map(s,  0, 59,  0, LED_COUNT - 1);
  uint8_t u_led = map(ms, 0, 999, 0, LED_COUNT - 1);
  
  uint8_t h_val = map(m,  0, 59,  0, 255);
  uint8_t m_val = map(s,  0, 59,  0, 255);
  uint8_t s_val = map(ms, 0, 999, 0, 255);
  uint8_t u_val = 255;
 
  leds[led_hours[0]]  = CHSV(0, 0, 32);
  leds[led_hours[1]]  = CHSV(0, 0, 8);
  leds[led_hours[2]]  = CHSV(0, 0, 8);
  leds[led_hours[3]]  = CHSV(0, 0, 32);
  leds[led_hours[4]]  = CHSV(0, 0, 8);
  leds[led_hours[5]]  = CHSV(0, 0, 8);
  leds[led_hours[6]]  = CHSV(0, 0, 32);
  leds[led_hours[7]]  = CHSV(0, 0, 8);
  leds[led_hours[8]]  = CHSV(0, 0, 8);
  leds[led_hours[9]]  = CHSV(0, 0, 32);
  leds[led_hours[10]] = CHSV(0, 0, 8);
  leds[led_hours[11]] = CHSV(0, 0, 8);

  if(last_synced != 0){
    // Stunden
    leds[LO(h_led)]     = CHSV(64,  255, h_val);
    leds[LO(L2(h_led))] = CHSV(64,  255, 255 - h_val);

    // Minuten
    leds[LO(m_led)]     = CHSV(96,  255, m_val);
    leds[LO(L2(m_led))] = CHSV(96,  255, 255 - m_val);

    // Sekunden
    leds[LO(s_led)]     = CHSV(160, 255, s_val);
    leds[LO(L2(s_led))] = CHSV(160, 255, 255 - s_val);
  }

  // 1/100 Sekunden
  leds[LO(u_led)] = CHSV(0, 255, u_val);
  
  FastLED.show(); 

  if (true && (millis () - last) >= SHOW_TIME_PERIOD) { // DEBUG
      last = millis ();
      Serial.print(NTP.getTimeDateStringUs()); 
      Serial.print(" ");
      
      Serial.print("\tLEDs: "); 
      printf("%2d", h_led); printf("(%3d) ", h_val);
      printf("%2d", m_led); printf("(%3d) ", m_val);
      printf("%2d", s_led); printf("(%3d) ", s_val);
      Serial.println("");
  }
}

char* printf(char* format, int16_t num){
  sprintf(printf_buffer, format, num);
  Serial.print(printf_buffer);   
}
