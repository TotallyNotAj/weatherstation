// ####################################################################
// #                    STARTUP / STARTSCREEN SCREEN                       #
// ####################################################################
//

// Startup splash screen — shown while WiFi, MQTT, and NTP come online.
// Displays a progress bar with three steps and animated status text.
// Drop this file in the sketch folder alongside the main .ino.


#define STARTSCREEN_STEPS      3       // WiFi = 1, MQTT = 2, NTP = 3
#define STARTSCREEN_BAR_X      18
#define STARTSCREEN_BAR_Y      172
#define STARTSCREEN_BAR_W      204     // SCREEN_W - STARTSCREEN_BAR_X*2
#define STARTSCREEN_BAR_H      10
#define STARTSCREEN_STATUS_Y   190


// Draws the static parts of the splash screen — title, subtitle, divider,
// step labels, the empty progress bar, and the footer text.
// Call this once right after the display is initialised. The caller
// should hold tft_mutex before calling.
static void draw_startup_screen(void) {

  tft.fillScreen(bg_color);

  // Title
  tft.setFont(&FreeSansBold18pt7b);
  tft.setTextColor(value_color);
  tft.setTextWrap(false);
  {
    int16_t  x1, y1;
    uint16_t w,  h;
    const char *line1 = "Weather";
    tft.getTextBounds(line1, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((SCREEN_W - (int16_t)w) / 2 - x1, 60);
    tft.print(line1);

    const char *line2 = "Station";
    tft.getTextBounds(line2, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((SCREEN_W - (int16_t)w) / 2 - x1, 88);
    tft.print(line2);
  }

  // Subtitle — version and student ID
  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(label_color);
  {
    int16_t  x1, y1;
    uint16_t w,  h;
    const char *sub = "v6.7  |  620172829";
    tft.getTextBounds(sub, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((SCREEN_W - (int16_t)w) / 2 - x1, 114);
    tft.print(sub);
  }

  // Horizontal divider between the title block and the progress section
  tft.drawFastHLine(30, 128, SCREEN_W - 60, label_color);

  // Step labels and the small tick marks that sit between them
  tft.setFont(NULL);
  tft.setTextSize(1);
  tft.setTextColor(label_color);
  {
    const char *names[STARTSCREEN_STEPS] = { "WiFi", "MQTT", "NTP" };
    int seg = STARTSCREEN_BAR_W / STARTSCREEN_STEPS;
    for (int i = 0; i < STARTSCREEN_STEPS; i++) {
      int16_t  x1, y1;
      uint16_t w, h;
      tft.getTextBounds(names[i], 0, 0, &x1, &y1, &w, &h);
      int cx = STARTSCREEN_BAR_X + seg * i + seg / 2;
      tft.setCursor(cx - (int16_t)w / 2, 154);
      tft.print(names[i]);

      // tick mark at the segment boundary (skip after the last label)
      if (i < STARTSCREEN_STEPS - 1) {
        int tx = STARTSCREEN_BAR_X + seg * (i + 1);
        tft.drawFastVLine(tx, 164, 7, label_color);
      }
    }
  }

  // Empty progress bar — startup_step_complete() fills this in later
  tft.drawRoundRect(STARTSCREEN_BAR_X, STARTSCREEN_BAR_Y, STARTSCREEN_BAR_W, STARTSCREEN_BAR_H, 4, label_color);

  // Footer text at the very bottom of the screen
  tft.setFont(NULL);
  tft.setTextSize(1);
  tft.setTextColor(tft.color565(64, 64, 64));
  {
    int16_t  x1, y1;
    uint16_t w, h;
    const char *hint = "Starting weather station";
    tft.getTextBounds(hint, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((SCREEN_W - (int16_t)w) / 2, 310);
    tft.print(hint);
  }
}


// Called once a step finishes — fills the progress bar up to the current
// step and prints a status message in green (success) or red (failure).
//   step    : which step just finished (1, 2, or 3)
//   total   : total number of steps, always STARTSCREEN_STEPS
//   label   : short message to show, e.g. "WiFi connected"
//   success : drives the colour and whether to show green or red
static void startup_step_complete(int step, int total,
                                  const char *label, bool success) {

  uint16_t fill_col = success ? good_color : bad_color;

  // Fill the bar proportionally to how many steps are done
  int fill_w = (STARTSCREEN_BAR_W - 4) * step / total;
  tft.fillRoundRect(STARTSCREEN_BAR_X + 2, STARTSCREEN_BAR_Y + 2,
                    fill_w, STARTSCREEN_BAR_H - 4, 3, fill_col);

  // Small dot at the division point between this step and the next
  int seg  = STARTSCREEN_BAR_W / total;
  int dotx = STARTSCREEN_BAR_X + seg * step;
  if (step < total) {
    tft.fillCircle(dotx, STARTSCREEN_BAR_Y + STARTSCREEN_BAR_H / 2, 5, fill_col);
  }

  // Clear the status line and print the new message centred
  tft.fillRect(0, STARTSCREEN_STATUS_Y, SCREEN_W, 14, bg_color);
  tft.setFont(NULL);
  tft.setTextSize(1);
  tft.setTextColor(success ? good_color : bad_color);
  {
    int16_t  x1, y1;
    uint16_t w, h;
    tft.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((SCREEN_W - (int16_t)w) / 2, STARTSCREEN_STATUS_Y + 2);
    tft.print(label);
  }
}


// Called repeatedly while waiting for a step to complete.
// Animates a trailing ellipsis to show the device is still working.
//   label : the base message, e.g. "Connecting to WiFi"
//   dots  : pass (millis() / 400) % 4 to get 0-3 dots cycling
static void startup_step_waiting(const char *label, int dots) {
  // Trailing spaces overwrite any leftover characters from the previous frame
  char buf[40];
  snprintf(buf, sizeof(buf), "%s%.*s   ", label, dots, "...");

  tft.fillRect(0, STARTSCREEN_STATUS_Y, SCREEN_W, 14, bg_color);
  tft.setFont(NULL);
  tft.setTextSize(1);
  tft.setTextColor(accent_color);
  {
    int16_t  x1, y1;
    uint16_t w, h;
    tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((SCREEN_W - (int16_t)w) / 2, STARTSCREEN_STATUS_Y + 2);
    tft.print(buf);
  }
}


//   - the splash screen goes up immediately after the display is ready
//   - WiFi is connected here (with live dot animation) before initialize() runs
//   - after initialize() we poll for MQTT and NTP so each gets its own
void setup() {

  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println(F("DHT22 + BMP280 + ILI9341 Weather Display"));
  dht.begin();

  // Mutex must exist before anything touches the display
  tft_mutex = xSemaphoreCreateMutex();

  tft.begin();
  tft.setRotation(0);
  tft.setTextWrap(false);

  // Colour palette — identical to the original setup()
  bg_color     = tft.color565(18,  18,  18);
  card_color   = tft.color565(37,  37,  37);
  label_color  = tft.color565(160, 160, 160);
  value_color  = tft.color565(255, 255, 255);
  good_color   = tft.color565(0,   230, 118);
  bad_color    = tft.color565(255, 82,  82);
  accent_color = tft.color565(100, 180, 255);
  color_cold   = tft.color565(0,   191, 255);
  color_comf   = tft.color565(0,   230, 118);
  color_warm   = tft.color565(255, 165,  0);
  color_hot    = tft.color565(255,  69,  0);

  pinMode(SOIL_PIN,     INPUT);
  pinMode(BTN_NEXT_PIN, INPUT_PULLUP);
  pinMode(BTN_PREV_PIN, INPUT_PULLUP);
  bmp280_init();

  // Show the splash screen before anything else happens
  if (xSemaphoreTake(tft_mutex, portMAX_DELAY) == pdTRUE) {
    draw_startup_screen();
    xSemaphoreGive(tft_mutex);
  }
  delay(300);


  // Step 1 — WiFi
  // Connect here so we can show animated dots while waiting.
  // initialize() below will see WiFi is already up and skip its own connect.
  WiFi.begin(ssid, password);
  {
    const unsigned long WIFI_TIMEOUT_MS = 30000UL;
    unsigned long t0 = millis();
    bool          ok = false;

    while (millis() - t0 < WIFI_TIMEOUT_MS) {
      if (WiFi.status() == WL_CONNECTED) { ok = true; break; }

      if (xSemaphoreTake(tft_mutex, pdMS_TO_TICKS(80)) == pdTRUE) {
        startup_step_waiting("Connecting to WiFi", (millis() / 400) % 4);
        xSemaphoreGive(tft_mutex);
      }
      delay(150);
    }

    if (xSemaphoreTake(tft_mutex, portMAX_DELAY) == pdTRUE) {
      startup_step_complete(1, STARTSCREEN_STEPS,
                            ok ? "WiFi connected"
                               : "WiFi timed out — offline mode",
                            ok);
      xSemaphoreGive(tft_mutex);
    }
    delay(350);
  }


  // Step 2 — MQTT
  // initialize() spins up the MQTT and NTP FreeRTOS tasks then returns.
  // We poll mqtt.connected() until the broker handshake completes or
  // we hit the 12 s timeout — the task will keep retrying either way.
  if (xSemaphoreTake(tft_mutex, pdMS_TO_TICKS(80)) == pdTRUE) {
    startup_step_waiting("Starting services", 1);
    xSemaphoreGive(tft_mutex);
  }

  initialize();

  {
    const unsigned long MQTT_TIMEOUT_MS = 12000UL;
    unsigned long t0 = millis();
    bool          ok = false;

    while (millis() - t0 < MQTT_TIMEOUT_MS) {
      if (mqtt.connected()) { ok = true; break; }

      if (xSemaphoreTake(tft_mutex, pdMS_TO_TICKS(80)) == pdTRUE) {
        startup_step_waiting("Connecting to MQTT", (millis() / 400) % 4);
        xSemaphoreGive(tft_mutex);
      }
      delay(150);
    }

    if (xSemaphoreTake(tft_mutex, portMAX_DELAY) == pdTRUE) {
      startup_step_complete(2, STARTSCREEN_STEPS,
                            ok ? "MQTT connected"
                               : "MQTT pending — will retry",
                            ok);
      xSemaphoreGive(tft_mutex);
    }
    delay(350);
  }


  // Step 3 — NTP
  {
    const unsigned long NTP_TIMEOUT_MS = 15000UL;
    unsigned long t0 = millis();
    bool          ok = false;
    struct tm     ti;

    while (millis() - t0 < NTP_TIMEOUT_MS) {
      if (getLocalTime(&ti) && ti.tm_year > 120) {
        ok = true;
        break;
      }

      if (xSemaphoreTake(tft_mutex, pdMS_TO_TICKS(80)) == pdTRUE) {
        startup_step_waiting("Syncing time (NTP)", (millis() / 400) % 4);
        xSemaphoreGive(tft_mutex);
      }
      delay(200);
    }

    if (xSemaphoreTake(tft_mutex, portMAX_DELAY) == pdTRUE) {
      startup_step_complete(3, STARTSCREEN_STEPS,
                            ok ? "Ready!"
                               : "NTP pending — no clock yet",
                            ok);
      xSemaphoreGive(tft_mutex);
    }
    delay(900);   // give the user a moment to read "Ready!" before the main screen appears
  }


  // Hand off to the main display
  if (xSemaphoreTake(tft_mutex, portMAX_DELAY) == pdTRUE) {
    draw_shell();
    xSemaphoreGive(tft_mutex);
  }

  vButtonCheckFunction();
  vUpdateFunction();
}