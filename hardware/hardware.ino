  //##################################################################################################################
  //##                                      WEATHER STATION DISPLAY SYSTEM CODE                                     ##
  //##                                                                                                              ##
  //##################################################################################################################

  // LIBRARY IMPORTS
  #include <rom/rtc.h>
  #include <math.h>
  #include <ctype.h>

  // ADD YOUR IMPORTS HERE
  #include <PubSubClient.h>
  #include <SPI.h>
  #include <Wire.h>
  #include <string.h>

  #include <Adafruit_GFX.h>
  #include <Adafruit_BMP280.h>
  #include <Adafruit_ILI9341.h>
  #include <Fonts/FreeSansBold12pt7b.h>
  #include <Fonts/FreeSansBold18pt7b.h>
  #include <Fonts/FreeSansBold9pt7b.h>
  #include "DHT.h"

  #ifndef _WIFI_H
  #include <WiFi.h>
  #endif

  #include "icons.h"

  #ifndef STDLIB_H
  #include <stdlib.h>
  #endif

  #ifndef STDIO_H
  #include <stdio.h>
  #endif

  #ifndef ARDUINO_H
  #include <Arduino.h>
  #endif

  #ifndef ARDUINOJSON_H
  #include <ArduinoJson.h>
  #endif


  // DEFINE VARIABLES
  #define ARDUINOJSON_USE_DOUBLE      1

  // DISPLAY PIN DEFINITIONS
  #define TFT_DC      17
  #define TFT_CS      5
  #define TFT_RST     16
  #define TFT_CLK     18
  #define TFT_MOSI    23
  #define TFT_MISO    19
  
  // BMP280 PIN / ADDRESS DEFINITIONS (I2C)
  // NOTE: CSB → VCC (selects I2C mode), SDO → GND (sets address to 0x76)
  #define I2C_SDA      21
  #define I2C_SCL      22
  #define BMP280_ADDR  0x76
  
  // DHT22 PIN DEFINITIONS
  #define DHTPIN       4
  #define DHTTYPE      DHT22
  
  // SOIL MOISTURE SENSOR PIN DEFINITIONS
  #define SOIL_PIN     33   // Capacitive soil moisture sensor analog output pin
  
  // SOIL SENSOR CALIBRATION (adjust these after testing with your sensor)
  #define SOIL_DRY_VAL  320  // Raw ADC value in dry air / dry soil
  #define SOIL_WET_VAL  600  // Raw ADC value in water / saturated soil
  
  // BUTTON PIN DEFINITIONS
  #define BTN_NEXT_PIN  25   // Next page button
  #define BTN_PREV_PIN  26   // Previous page button
  #define BTN_DEBOUNCE  200  // Debounce time in ms
  
  // DISPLAY LAYOUT DEFINITIONS
  #define SCREEN_W    240
  #define SCREEN_H    320
  #define HEADER_H    28
  #define FOOTER_H    28
  #define MARGIN      6
  #define GUTTER      4
  #define CARD_W      ((SCREEN_W - (MARGIN * 2) - GUTTER) / 2)
  #define CARD_H      ((SCREEN_H - HEADER_H - FOOTER_H - (MARGIN * 2) - GUTTER) / 2)
  #define NUM_PAGES   2
  
  // TIMING DEFINITIONS
  #define REFRESH_MS      1000UL
  #define WIFI_RETRY_MS   10000UL
  #define DHT_WAIT_MS     2000UL
  
  // SEA LEVEL PRESSURE FOR ALTITUDE CALCULATION (hPa)
  #define SEA_LEVEL_HPA   1013.25
  
  
  // MQTT CLIENT CONFIG
  static const char* pubtopic      = "620172829";                    // Add your ID number here
  static const char* subtopic[]    = {"620172829_sub", "/elet2415"}; // Array of Topics(Strings) to subscribe to
  static const char* mqtt_server   = "www.yanacreations.com";         // Broker IP address or Domain name as a String
  static uint16_t    mqtt_port     = 1883;
  
  // WIFI CREDENTIALS
  const char* ssid     = "MonaConnect";  // Add your Wi-Fi ssid
  const char* password = "";             // Add your Wi-Fi password
  
  
  // TASK HANDLES
  TaskHandle_t xMQTT_Connect       = NULL;
  TaskHandle_t xNTPHandle          = NULL;
  TaskHandle_t xLOOPHandle         = NULL;
  TaskHandle_t xUpdateHandle       = NULL;
  TaskHandle_t xButtonCheckeHandle = NULL;
  
  // CARD / LAYOUT STRUCT
  typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
  } Rect;
  
  // FUNCTION DECLARATION
  void checkHEAP(const char* Name);   // RETURN REMAINING HEAP SIZE FOR A TASK
  void initMQTT(void);                // CONFIG AND INITIALIZE MQTT PROTOCOL
  unsigned long getTimeStamp(void);   // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
  void callback(char* topic, byte* payload, unsigned int length);
  void initialize(void);
  bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
  void vButtonCheck( void * pvParameters );
  void vUpdate( void * pvParameters );
  bool isNumber(double number);
  
  /* Declare your functions below */
  double convert_Celsius_to_fahrenheit(double c);
  double convert_fahrenheit_to_Celsius(double f);
  double calcHeatIndex(double Temp, double Humid);
  
  /* Forward declarations for display functions */
  static int  bmp280_init(void);
  static void draw_shell(void);
  static void draw_label(Rect r, const char *text);
  static void draw_card(Rect r, const char *value, const char *unit, float raw_val, uint16_t val_color, const unsigned char* icon = nullptr);
  static void draw_diff(Rect r, const char *old_text, const char *new_text, uint16_t fg, uint16_t bg);
  static void draw_page_indicator(void);
  static void render_values(void);
  static void log_values(void);
  
  
  /* Init class Instances for the DHT22, TFT Display, BMP280 etc */
  DHT dht(DHTPIN, DHTTYPE);
  static Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
  static Adafruit_BMP280  bmp;
  
  
  // DISPLAY COLOR VARIABLES
  static uint16_t bg_color;
  static uint16_t card_color;
  static uint16_t label_color;
  static uint16_t value_color;
  static uint16_t good_color;
  static uint16_t bad_color;
  static uint16_t accent_color;
  static uint16_t color_cold;
  static uint16_t color_comf;
  static uint16_t color_warm;
  static uint16_t color_hot;
  
  // SENSOR STATE VARIABLES
  static int            bmp_ready        = 0;
  static float          current_temp     = NAN;
  static float          current_hum      = NAN;
  static float          current_pressure = NAN;
  static float          current_altitude = NAN;
  static float          current_heat_idx = NAN;
  static float          current_soil     = NAN;
  static int            current_online   = 0;
  static int            current_rssi     = 0;
  static unsigned long  current_uptime   = 0;
  
  // PAGE STATE
  static int current_page = 0;  // 0 = page 1, 1 = page 2
  
  // DISPLAY MUTEX (prevents simultaneous SPI access from multiple tasks)
  static SemaphoreHandle_t tft_mutex = NULL;
  
  // DISPLAY DIFF TRACKING
  static char shown_temp[16]     = "";
  static char shown_hum[16]      = "";
  static char shown_press[16]    = "";
  static char shown_soil[16]     = "";
  static char shown_altitude[16] = "";
  static char shown_heat_idx[16] = "";
  static char shown_rssi[16]     = "";
  static char shown_uptime[16]   = "";
  static char shown_clock[16]    = "";
  static int  shown_online       = -1;
  static int  first_frame        = 1;
  
  // BUTTON DEBOUNCE TRACKING
  static unsigned long last_btn_next = 0;
  static unsigned long last_btn_prev = 0;
  
  // CARD POSITIONS (same grid layout for both pages)
  static const Rect CARD_TL    = {MARGIN, HEADER_H + MARGIN, CARD_W, CARD_H};
  static const Rect CARD_TR    = {MARGIN + CARD_W + GUTTER, HEADER_H + MARGIN, CARD_W, CARD_H};
  static const Rect CARD_BL    = {MARGIN, HEADER_H + MARGIN + CARD_H + GUTTER, CARD_W, CARD_H};
  static const Rect CARD_BR    = {MARGIN + CARD_W + GUTTER, HEADER_H + MARGIN + CARD_H + GUTTER, CARD_W, CARD_H};
  
  // HEADER AREA
  static const Rect CLOCK_AREA = {SCREEN_W - 72, 0, 66, HEADER_H};
  
  
  //############### IMPORT HEADER FILES ##################
  #ifndef NTP_H
  #include "NTP.h"
  #endif
  
  #ifndef MQTT_H
  #include "mqtt.h"
  #endif
  
  
  void setup() {
    Serial.begin(115200);  // INIT SERIAL
  
    // INITIALIZE ALL SENSORS AND DEVICES
    Wire.begin(I2C_SDA, I2C_SCL);
  
    Serial.println(F("DHT22 + BMP280 + ILI9341 Weather Display"));
    dht.begin();
  
    // CREATE DISPLAY MUTEX BEFORE ANY DRAWING BEGINS
    tft_mutex = xSemaphoreCreateMutex();
  
    tft.begin();
    tft.setRotation(0);
    tft.setTextWrap(false);
  
    bg_color     = tft.color565(18,  18,  18);
    card_color   = tft.color565(37,  37,  37);
    label_color  = tft.color565(160, 160, 160);
    value_color  = tft.color565(255, 255, 255);
    good_color   = tft.color565(0,   230, 118);
    bad_color    = tft.color565(255, 82,  82);
    accent_color = tft.color565(100, 180, 255);
    color_cold   = tft.color565(0,   191, 255);  // Deep Sky Blue
    color_comf   = tft.color565(0,   230, 118);  // Match good_color
    color_warm   = tft.color565(255, 165, 0);    // Orange
    color_hot    = tft.color565(255, 69,  0);    // Red-Orange
  
    /* Add all other necessary sensor Initializations and Configurations here */
    pinMode(SOIL_PIN,     INPUT);
    pinMode(BTN_NEXT_PIN, INPUT_PULLUP);
    pinMode(BTN_PREV_PIN, INPUT_PULLUP);
    bmp280_init();
    if (xSemaphoreTake(tft_mutex, portMAX_DELAY) == pdTRUE) {
      draw_shell();
      xSemaphoreGive(tft_mutex);
    }
  
    initialize();           // INIT WIFI, MQTT & NTP
    vButtonCheckFunction(); // BUTTON TASK HANDLES PAGE SWITCHING
  }
  
  
  void loop() {
    // put your main code here, to run repeatedly:
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  
  
  
  //####################################################################
  //#                          UTIL FUNCTIONS                          #
  //####################################################################
  void vButtonCheck( void * pvParameters ) {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
  
    for( ;; ) {
      unsigned long now = millis();
  
      // NEXT PAGE BUTTON (pin 25) - active LOW with INPUT_PULLUP
      if (digitalRead(BTN_NEXT_PIN) == LOW && now - last_btn_next > BTN_DEBOUNCE) {
        last_btn_next = now;
        current_page  = (current_page + 1) % NUM_PAGES;
        first_frame   = 1;
        if (xSemaphoreTake(tft_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
          draw_shell();
          xSemaphoreGive(tft_mutex);
        }
      }
  
      // PREVIOUS PAGE BUTTON (pin 26) - active LOW with INPUT_PULLUP
      if (digitalRead(BTN_PREV_PIN) == LOW && now - last_btn_prev > BTN_DEBOUNCE) {
        last_btn_prev = now;
        current_page  = (current_page - 1 + NUM_PAGES) % NUM_PAGES;
        first_frame   = 1;
        if (xSemaphoreTake(tft_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
          draw_shell();
          xSemaphoreGive(tft_mutex);
        }
      }
  
      vTaskDelay(20 / portTICK_PERIOD_MS);
    }
  }
  
  void vUpdate( void * pvParameters ) {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
  
    for( ;; ) {
      // #######################################################
      // ## This function must PUBLISH to topic every second. ##
      // #######################################################
  
      // 1. Read Humidity and save in variable below
      double h = dht.readHumidity();
  
      // 2. Read temperature as Celsius and save in variable below
      double t = dht.readTemperature();
  
      // 3. Read pressure and altitude from BMP280
      double p   = NAN;
      double alt = NAN;
      if (bmp_ready) {
        p = bmp.readPressure() / 100.0f;
        if (p <= 0.0f) {
          p = NAN;
        } else {
          alt = bmp.readAltitude(SEA_LEVEL_HPA);
        }
      }
  
      // 4. Read soil moisture from capacitive sensor
      int    soil_raw = analogRead(SOIL_PIN);
      double soil     = (double)(soil_raw - SOIL_DRY_VAL) / (double)(SOIL_WET_VAL - SOIL_DRY_VAL) * 100.0;
      soil            = constrain(soil, 0.0, 100.0);
  
      // 5. Calculate heat index (requires valid temp and humidity)
      double hi = NAN;
      if (isNumber(t) && isNumber(h)) {
        hi = convert_fahrenheit_to_Celsius(calcHeatIndex(convert_Celsius_to_fahrenheit(t), h));
      }
      Serial.printf("t=%.1f h=%.1f p=%.1f\n", t, h, (float)p);
      if (isNumber(t)) {
        // ## Publish update according to:
        // {"id":"student_id","timestamp":1702212234,"temperature":30,"humidity":90,
        //  "heatindex":30,"pressure":1013.25,"altitude":5.2,"soil":62.0}
  
        // 1. Create JSON object
        JsonDocument doc;
  
        // 2. Create message buffer/array to store serialized JSON object
        char message[1100] = {0};
  
        // 3. Add key:value pairs to JSON object based on above schema
        doc["id"]          = "620172829";
        doc["timestamp"]   = getTimeStamp();
        doc["temperature"] = t;
        doc["humidity"]    = h;
        doc["heatindex"]   = isNumber(hi)  ? hi  : 0;
        doc["pressure"]    = isNumber(p)   ? p   : 0;
        doc["altitude"]    = isNumber(alt) ? alt : 0;
        doc["soil"]        = soil;
  
        // 4. Serialize / Convert JSON object to JSON string and store in message array
        serializeJson(doc, message);
  
        // 5. Publish message to a topic subscribed to by both backend and frontend
        if (mqtt.connected()) {
          publish(pubtopic, message);
        }
  
        // 6. Update display values and re-render
        current_temp     = (float)t;
        current_hum      = (float)h;
        current_pressure = isNumber(p)   ? (float)p   : NAN;
        current_altitude = isNumber(alt) ? (float)alt : NAN;
        current_heat_idx = isNumber(hi)  ? (float)hi  : NAN;
        current_soil     = (float)soil;
        current_online   = WiFi.status() == WL_CONNECTED;
        current_rssi     = current_online ? WiFi.RSSI() : 0;
        current_uptime   = millis() / 1000UL;
        if (xSemaphoreTake(tft_mutex, pdMS_TO_TICKS(200)) == pdTRUE) {
          render_values();
          xSemaphoreGive(tft_mutex);
        }
  
        // 7. Log to serial
        log_values();
      }
  
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
  
  
  unsigned long getTimeStamp(void) {
    // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
    time_t now;
    time(&now); // Retrieve time[Timestamp] from system and save to &now variable
    return now;
  }
  
  
  void callback(char* topic, byte* payload, unsigned int length) {
    // ############## MQTT CALLBACK  ######################################
    // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  
    Serial.printf("\nMessage received : ( topic: %s ) \n", topic);
    char *received = new char[length + 1] {0};
  
    for (int i = 0; i < length; i++) {
      received[i] = (char)payload[i];
    }
  
    // PRINT RECEIVED MESSAGE
    Serial.printf("Payload : %s \n", received);
  
    // CONVERT MESSAGE TO JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, received);
  
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
  
    // PROCESS MESSAGE
    const char* type = doc["type"];
  
    if (strcmp(type, "controls") == 0) {
      // 1. EXTRACT BRIGHTNESS LEVEL FROM JSON OBJECT
      const int brightness = doc["brightness"];
      Serial.printf("Brightness control received: %d\n", brightness);
      // Additional display brightness or LED control logic can go here
    }
  }
  
  
  bool publish(const char *topic, const char *payload) {
    bool res = false;
    try {
      res = mqtt.publish(topic, payload);
      if (!res) {
        res = false;
        throw false;
      }
    }
    catch(...) {
      Serial.printf("\nError (%d) >> Unable to publish message\n", res);
    }
    return res;
  }
  
  
  
  //***** Complete the util functions below ******
  
  double convert_Celsius_to_fahrenheit(double c) {
    // CONVERTS INPUT FROM °C TO °F. RETURN RESULTS
    return (c * 9) / 5 + 32;
  }
  
  double convert_fahrenheit_to_Celsius(double f) {
    // CONVERTS INPUT FROM °F TO °C. RETURN RESULT
    return (f - 32) * 5 / 9;
  }
  
  double calcHeatIndex(double Temp, double Humid) {
    // CALCULATE AND RETURN HEAT INDEX USING EQUATION FOUND AT
    // https://byjus.com/heat-index-formula/
    return -42.379
        + (2.04901523   * Temp)
        + (10.14333127  * Humid)
        - (0.22475541   * Temp  * Humid)
        - (0.00683783   * Temp  * Temp)
        - (0.05481717   * Humid * Humid)
        + (0.00122874   * Temp  * Temp  * Humid)
        + (0.00085282   * Temp  * Humid * Humid)
        - (0.00000199   * Temp  * Temp  * Humid * Humid);
  }
  
  bool isNumber(double number) {
    char item[20];
    snprintf(item, sizeof(item), "%f\n", number);
    if (isdigit(item[0])) {
      return true;
    }
    return false;
  }
  
  uint16_t get_temp_color(float t) {
    if (isnan(t)) return value_color;
    if (t < 18.0) return color_cold;
    if (t < 26.0) return color_comf;
    if (t < 32.0) return color_warm;
    return color_hot;
  }

  uint16_t get_hum_color(float h) {
    if (isnan(h)) return value_color;
    if (h < 30.0) return color_warm; // Too dry
    if (h <= 60.0) return color_comf; // Comfortable
    return color_cold; // Humid/Wet
  }

  uint16_t get_soil_color(float s) {
    if (isnan(s)) return value_color;
    if (s < 30.0) return color_hot; // Dangerously dry
    if (s <= 70.0) return color_comf; // Good
    return color_cold; // Very wet
  }


  //####################################################################
  //#                       DISPLAY FUNCTIONS                          #
  //####################################################################
  
  static int bmp280_init(void) {
    bmp_ready = 0;
    if (!bmp.begin(BMP280_ADDR)) {
      Serial.println("bmp280 fail");
      return 0;
    }
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X1,
                    Adafruit_BMP280::SAMPLING_X4,
                    Adafruit_BMP280::FILTER_OFF,
                    Adafruit_BMP280::STANDBY_MS_500);
    bmp_ready = 1;
    return 1;
  }
  
  static void draw_shell(void) {
    // CLEAR SCREEN AND DRAW CARD BACKGROUNDS
    tft.fillScreen(bg_color);
    tft.fillRoundRect(CARD_TL.x, CARD_TL.y, CARD_TL.w, CARD_TL.h, 6, card_color);
    tft.fillRoundRect(CARD_TR.x, CARD_TR.y, CARD_TR.w, CARD_TR.h, 6, card_color);
    tft.fillRoundRect(CARD_BL.x, CARD_BL.y, CARD_BL.w, CARD_BL.h, 6, card_color);
    tft.fillRoundRect(CARD_BR.x, CARD_BR.y, CARD_BR.w, CARD_BR.h, 6, card_color);
  
    // DRAW CARD LABELS FOR CURRENT PAGE
    if (current_page == 0) {
      draw_label(CARD_TL, "TEMP");
      draw_label(CARD_TR, "HUMIDITY");
      draw_label(CARD_BL, "HEAT INDEX");
      draw_label(CARD_BR, "PRESSURE");
    } else {
      draw_label(CARD_TL, "ALTITUDE");
      draw_label(CARD_TR, "SOIL MOISTURE");
      draw_label(CARD_BL, "WIFI STRENGTH");
      draw_label(CARD_BR, "UPTIME");
    }
  
    // DRAW WIFI LABEL IN HEADER
    tft.setFont(NULL);
    tft.setTextColor(label_color);
    tft.setTextSize(1);
    tft.setCursor(MARGIN + 12, (HEADER_H - 8) / 2);
    tft.print("WIFI");
  
    // DRAW PAGE INDICATOR IN FOOTER
    draw_page_indicator();
  
    // RESET DIFF TRACKING TO FORCE FULL REDRAW ON NEXT RENDER
    shown_temp[0]     = 0;
    shown_hum[0]      = 0;
    shown_press[0]    = 0;
    shown_soil[0]     = 0;
    shown_altitude[0] = 0;
    shown_heat_idx[0] = 0;
    shown_rssi[0]     = 0;
    shown_uptime[0]   = 0;
    shown_clock[0]    = 0;
    shown_online      = -1;
  }
  
  static void draw_label(Rect r, const char *text) {
    tft.setFont(NULL);
    tft.setTextColor(label_color);
    tft.setTextSize(1);
    tft.setCursor(r.x + 8, r.y + 8);
    tft.print(text);
  }
  
  static void draw_card(Rect r, const char *value, const char *unit, float raw_val, uint16_t val_color, const unsigned char* icon) {
    // Define drawing area inside the card margins
    int16_t area_x = r.x + 4;
    int16_t area_y = r.y + 20;
    int16_t area_w = r.w - 8;
    int16_t area_h = r.h - 24;
    size_t  len    = strlen(value);
  
    GFXcanvas16 canvas(area_w, area_h);
    canvas.fillScreen(card_color);
  
    // 1. Draw a sleek left-edge accent line
    canvas.fillRoundRect(0, 4, 4, area_h - 16, 2, val_color);

    // 2. Draw the icon in the top-right corner (24x24, tinted with val_color)
    //    Requires PROGMEM bitmaps declared in icons.h
    if (icon != nullptr) {
      canvas.drawBitmap(area_w - 28, 4, icon, 24, 24, val_color);
    }
  
    // 3. Select Font Size
    if (len <= 5) {
      canvas.setFont(&FreeSansBold18pt7b);
    } else if (len <= 7) {
      canvas.setFont(&FreeSansBold12pt7b);
    } else {
      canvas.setFont(&FreeSansBold9pt7b);
    }
  
    // 4. Apply the dynamic color to the text
    canvas.setTextColor(val_color);
    canvas.setTextSize(1);
  
    // 5. Center text, shifting it slightly right to clear the new accent line
    int16_t  x1, y1;
    uint16_t w, h;
    canvas.getTextBounds(value, 0, 0, &x1, &y1, &w, &h);
    
    // Shift Y up slightly if we need room for the progress bar
    int16_t y_offset = (strcmp(unit, "%") == 0) ? 8 : 4;
    canvas.setCursor(((area_w - (int16_t)w) / 2) - x1 + 2,
                    ((area_h - (int16_t)h) / 2) - y1 - y_offset);
    canvas.print(value);
  
    // 6. Draw the Unit Label
    canvas.setFont(NULL);
    canvas.setTextColor(label_color);
    canvas.setTextSize(1);
    canvas.setCursor(area_w - ((int16_t)strlen(unit) * 6) - 4, area_h - 12);
    canvas.print(unit);

    // 7. Draw Mini Progress Gauge (Only for percentage metrics)
    if (strcmp(unit, "%") == 0 && !isnan(raw_val)) {
      int bar_max_w = area_w - 20;
      int bar_w = bar_max_w * (constrain(raw_val, 0.0, 100.0) / 100.0);
      // Dark track background
      canvas.fillRoundRect(10, area_h - 6, bar_max_w, 4, 2, tft.color565(50, 50, 50));
      // Colored fill
      canvas.fillRoundRect(10, area_h - 6, bar_w, 4, 2, val_color); 
    }
  
    // Push the completed canvas to the display
    tft.drawRGBBitmap(area_x, area_y, canvas.getBuffer(), canvas.width(), canvas.height());
  }
  
  static void draw_diff(Rect r, const char *old_text, const char *new_text, uint16_t fg, uint16_t bg) {
    size_t old_len = strlen(old_text);
    size_t new_len = strlen(new_text);
    size_t n       = old_len > new_len ? old_len : new_len;
  
    tft.setFont(NULL);
    tft.setTextSize(1);
  
    for (size_t i = 0; i < n && i < (size_t)(r.w / 6); i++) {
      char a = i < old_len ? old_text[i] : ' ';
      char b = i < new_len ? new_text[i] : ' ';
      if (a == b) continue;
      tft.drawChar(r.x + (int16_t)(i * 6), r.y + ((r.h - 8) / 2), b, fg, bg, 1);
    }
  }
  
  static void draw_page_indicator(void) {
    // CLEAR FOOTER AREA
    tft.fillRect(0, SCREEN_H - FOOTER_H, SCREEN_W, FOOTER_H, bg_color);
  
    int16_t footer_y = SCREEN_H - FOOTER_H + (FOOTER_H - 8) / 2;
  
    // DRAW PREV LABEL ON LEFT (only if not on first page)
    if (current_page > 0) {
      tft.setFont(NULL);
      tft.setTextColor(label_color);
      tft.setTextSize(1);
      tft.setCursor(MARGIN + 4, footer_y);
      tft.print("< PREV");
    }
  
    // DRAW PAGE NUMBER CENTERED e.g. "1/2"
    char     indicator[8];
    int16_t  x1, y1;
    uint16_t w, h;
    snprintf(indicator, sizeof(indicator), "%d/%d", current_page + 1, NUM_PAGES);
    tft.setFont(NULL);
    tft.setTextSize(1);
    tft.getTextBounds(indicator, 0, 0, &x1, &y1, &w, &h);
    tft.setTextColor(accent_color);
    tft.setCursor((SCREEN_W - w) / 2, footer_y);
    tft.print(indicator);
  
    // DRAW NEXT LABEL ON RIGHT (only if not on last page)
    if (current_page < NUM_PAGES - 1) {
      tft.setFont(NULL);
      tft.setTextColor(label_color);
      tft.setTextSize(1);
      tft.setCursor(SCREEN_W - 40, footer_y);
      tft.print("NEXT >");
    }
  }
  
  static void render_values(void) {
    char temp[16], hum[16], press[16], soil[16], altitude[16], heat_idx[16], clock_text[16], rssi[16], uptime[16];
  
    // FORMAT ALL VALUES
    if (isnan(current_temp)) {
      snprintf(temp, sizeof(temp), "--");
    } else {
      snprintf(temp, sizeof(temp), "%.1f", current_temp);
    }
  
    if (isnan(current_hum)) {
      snprintf(hum, sizeof(hum), "--");
    } else {
      snprintf(hum, sizeof(hum), "%.1f", current_hum);
    }
  
    if (isnan(current_pressure)) {
      snprintf(press, sizeof(press), "--");
    } else {
      snprintf(press, sizeof(press), "%.1f", current_pressure);
    }
  
    if (isnan(current_soil)) {
      snprintf(soil, sizeof(soil), "--");
    } else {
      snprintf(soil, sizeof(soil), "%.1f", current_soil);
    }
  
    if (isnan(current_altitude)) {
      snprintf(altitude, sizeof(altitude), "--");
    } else {
      snprintf(altitude, sizeof(altitude), "%.1f", current_altitude);
    }
  
    if (isnan(current_heat_idx)) {
      snprintf(heat_idx, sizeof(heat_idx), "--");
    } else {
      snprintf(heat_idx, sizeof(heat_idx), "%.1f", current_heat_idx);
    }

    if (!current_online) {
      snprintf(rssi, sizeof(rssi), "--");
    } else {
      snprintf(rssi, sizeof(rssi), "%d", current_rssi);
    }

    unsigned long s = current_uptime;
    snprintf(uptime, sizeof(uptime), "%02lu:%02lu:%02lu", (s / 3600UL) % 24UL, (s / 60UL) % 60UL, s % 60UL);
  
    // GET REAL TIME FROM NTP FOR CLOCK
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      snprintf(clock_text, sizeof(clock_text), "%02d:%02d:%02d",
              timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
      snprintf(clock_text, sizeof(clock_text), "--:--:--");
    }
  
    // DRAW WIFI INDICATOR DOT
    if (first_frame || current_online != shown_online) {
      tft.fillCircle(MARGIN + 4, HEADER_H / 2, 3, current_online ? good_color : bad_color);
      shown_online = current_online;
    }
  
    // DRAW CLOCK (top right)
    if (first_frame || strcmp(clock_text, shown_clock) != 0) {
      draw_diff(CLOCK_AREA, shown_clock, clock_text, value_color, bg_color);
      strncpy(shown_clock, clock_text, sizeof(shown_clock) - 1);
      shown_clock[sizeof(shown_clock) - 1] = 0;
    }
  
    // DRAW CARDS FOR CURRENT PAGE
    if (current_page == 0) {
      // PAGE 1: TEMP | HUMIDITY
      //         HEAT INDEX | PRESSURE
      if (first_frame || strcmp(temp, shown_temp) != 0) {
        draw_card(CARD_TL, temp, "C", current_temp, get_temp_color(current_temp), icon_temp);
        strncpy(shown_temp, temp, sizeof(shown_temp) - 1);
        shown_temp[sizeof(shown_temp) - 1] = 0;
      }
      if (first_frame || strcmp(hum, shown_hum) != 0) {
        draw_card(CARD_TR, hum, "%", current_hum, get_hum_color(current_hum), icon_hum);
        strncpy(shown_hum, hum, sizeof(shown_hum) - 1);
        shown_hum[sizeof(shown_hum) - 1] = 0;
      }
      if (first_frame || strcmp(heat_idx, shown_heat_idx) != 0) {
        draw_card(CARD_BL, heat_idx, "C", current_heat_idx, get_temp_color(current_heat_idx), icon_heat);
        strncpy(shown_heat_idx, heat_idx, sizeof(shown_heat_idx) - 1);
        shown_heat_idx[sizeof(shown_heat_idx) - 1] = 0;
      }
      if (first_frame || strcmp(press, shown_press) != 0) {
        // Pressure uses the standard accent color since it doesn't have a rigid "good/bad" scale here
        draw_card(CARD_BR, press, "hPa", current_pressure, accent_color, icon_press);
        strncpy(shown_press, press, sizeof(shown_press) - 1);
        shown_press[sizeof(shown_press) - 1] = 0;
      }
    } else {
      // PAGE 2:      ALTITUDE | SOIL MOISTURE
      //         WIFI STRENGTH | UPTIME
      if (first_frame || strcmp(altitude, shown_altitude) != 0) {
        draw_card(CARD_TL, altitude, "m", current_altitude, accent_color, icon_alt);
        strncpy(shown_altitude, altitude, sizeof(shown_altitude) - 1);
        shown_altitude[sizeof(shown_altitude) - 1] = 0;
      }
      if (first_frame || strcmp(soil, shown_soil) != 0) {
        draw_card(CARD_TR, soil, "%", current_soil, get_soil_color(current_soil), icon_sprout);
        strncpy(shown_soil, soil, sizeof(shown_soil) - 1);
        shown_soil[sizeof(shown_soil) - 1] = 0;
      }
      if (first_frame || strcmp(rssi, shown_rssi) != 0) {
        // WiFi RSSI scale (-50 is great, -90 is bad)
        uint16_t rssi_color = (current_rssi > -70) ? good_color : ((current_rssi > -85) ? color_warm : bad_color);
        draw_card(CARD_BL, rssi, "dBm", current_rssi, rssi_color, icon_wifi);
        strncpy(shown_rssi, rssi, sizeof(shown_rssi) - 1);
        shown_rssi[sizeof(shown_rssi) - 1] = 0;
      }
      if (first_frame || strcmp(uptime, shown_uptime) != 0) {
        draw_card(CARD_BR, uptime, "", 0, accent_color, icon_uptime);
        strncpy(shown_uptime, uptime, sizeof(shown_uptime) - 1);
        shown_uptime[sizeof(shown_uptime) - 1] = 0;
      }
    }
  
    first_frame = 0;
  }
  
  static void log_values(void) {
    Serial.print("Temp: ");
    if (isnan(current_temp)) {
      Serial.print("--");
    } else {
      Serial.print(current_temp, 1);
    }

    Serial.print(" C  Humidity: ");
    if (isnan(current_hum)) {
      Serial.print("--");
    } else {
      Serial.print(current_hum, 1);
    }

    Serial.print(" %  Pressure: ");
    if (isnan(current_pressure)) {
      Serial.print("--");
    } else {
      Serial.print(current_pressure, 1);
    }

    Serial.print(" hPa  Altitude: ");
    if (isnan(current_altitude)) {
      Serial.print("--");
    } else {
      Serial.print(current_altitude, 1);
    }

    Serial.print(" m  Heat Index: ");
    if (isnan(current_heat_idx)) {
      Serial.print("--");
    } else {
      Serial.print(current_heat_idx, 1);
    }

    Serial.print(" C  Soil: ");
    if (isnan(current_soil)) {
      Serial.print("--");
    } else {
      Serial.print(current_soil, 1);
    }

    Serial.print(" %  RSSI: ");
    Serial.print(current_rssi);

    Serial.print(" dBm  Uptime: ");
    Serial.print(current_uptime / 3600UL);
    Serial.print("h ");
    Serial.print((current_uptime / 60UL) % 60UL);
    Serial.print("m ");
    Serial.print(current_uptime % 60UL);
    Serial.println("s");
  }
