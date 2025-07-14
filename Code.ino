#include <lvgl.h>

#include <dummy.h>

#ifndef LV_USE_GESTURE
#define LV_USE_GESTURE 1
#endif
#include <FS.h>
using fs::FS; 

#include <lvgl.h>
#include "BitAxe_Logosml.h"
#include "bitcoin_logo.h"
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
 
#include <WiFiManager.h> 


/* ----------  Touch-Pins  ---------- */
#define XPT2046_IRQ   36
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33

SPIClass touchscreenSPI(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

/* ----------  Display / LVGL  ---------- */
TFT_eSPI  tft;
lv_disp_t* disp = nullptr;
lv_indev_t* indev_drv;

/* ----------  WLAN & API  ---------- */
// WiFi and API configuration with placeholder values
const char* ssid                         = "YOUR_WIFI_SSID"; 
const char* pwd                          = "YOUR_WIFI_PASSWORD"; 
const char* bitaxe_url                   = "http://YOUR_BITAXE_IP/api/system/info"; 
// Custom parameter for WiFiManager to store Bitaxe URL
char custom_bitaxe_url[100] = "http://YOUR_BITAXE_IP/api/system/info"; 
// Custom parameter for display rotation
char custom_display_rotation[2] = "2"; // Default rotation value (0-3)
char custom_temp_target[4] = "65";
char custom_target_freq[5] = "300";
char custom_normal_freq[5] = "250";

/* ----------  UI-Objects (Site-1)  ---------- */
lv_obj_t* label_status;
lv_obj_t* label_hashrate;
lv_obj_t* label_shares;
lv_obj_t* label_best;
lv_obj_t* label_session;
    lv_obj_t* label_temp;
    lv_obj_t* label_vr_temp;
    lv_obj_t* label_pwr;

    lv_obj_t* hashrate_bar;
    lv_obj_t* temp_bar;
    lv_obj_t* vr_temp_bar;
    lv_obj_t* pwr_bar;
    lv_obj_t* bitcoin_logo_img;

/* ----------  Screens  ---------- */
lv_obj_t* screen1      = nullptr;   
lv_obj_t* screen2      = nullptr;   
lv_obj_t* screen3      = nullptr;   
bool      on_screen2   = false;
bool      on_screen3   = false;

bool  api_update_requested  = false;
SemaphoreHandle_t api_data_mutex;

/* ----------  UI-Objects (Site-2)  ---------- */
lv_obj_t* label_hashrate_screen2;
lv_obj_t* label_shares_screen2;
lv_obj_t* label_best_screen2;
lv_obj_t* label_session_screen2;
lv_obj_t* label_temp_screen2;

/* ----------  UI-Objects (Site-3)  ---------- */
lv_obj_t* label_hashrate_screen3;
lv_obj_t* label_shares_screen3;
lv_obj_t* label_best_screen3;
lv_obj_t* label_session_screen3;
lv_obj_t* label_temp_screen3;
lv_obj_t* label_pwr_screen3;
lv_obj_t* label_status_screen3;

// Preferences for NVS
Preferences prefs;


/********************************************************************
 *  LVGL Display Flush
 *******************************************************************/
void flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* color_p)
{
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t*)color_p, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/********************************************************************
 *  Touch → LVGL Callback
 *******************************************************************/
void touchscreen_read(lv_indev_t* indev, lv_indev_data_t* data)
{
    if (touchscreen.tirqTouched() && touchscreen.touched()) {
        TS_Point p = touchscreen.getPoint();

        
        data->point.x = map(p.x, 200, 3800, 0, 319); 
        data->point.y = map(p.y, 200, 4000, 0, 239); 

        data->point.x = constrain(data->point.x, 0, 319);
        data->point.y = constrain(data->point.y, 0, 239);
        data->state   = LV_INDEV_STATE_PR;
    } else {
        data->state   = LV_INDEV_STATE_REL;
    }
}


/********************************************************************
 *  Screen-Changing
 *******************************************************************/
static void switch_to_screen1()
{
    if (on_screen2 || on_screen3) {
        lv_scr_load(screen1);
        on_screen2 = false;
        on_screen3 = false;
    }
}
static void switch_to_screen2()
{
    if (!on_screen2 && !on_screen3) {
        lv_scr_load(screen2);
        on_screen2 = true;
        on_screen3 = false;
    } else if (on_screen3) {
        lv_scr_load(screen2);
        on_screen2 = true;
        on_screen3 = false;
    }
}
static void switch_to_screen3()
{
    if (on_screen2) {
        lv_scr_load(screen3);
        on_screen2 = false;
        on_screen3 = true;
    }
}


/*  Gesture-Callbacks  */
static void screen1_gesture_event(lv_event_t* e)
{
    if (lv_event_get_code(e) == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_RIGHT) switch_to_screen2();
    }
}

static void screen2_gesture_event(lv_event_t* e)
{
    if (lv_event_get_code(e) == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_LEFT) switch_to_screen1();
        if (dir == LV_DIR_RIGHT) switch_to_screen3();
    }
}

static void screen3_gesture_event(lv_event_t* e)
{
    if (lv_event_get_code(e) == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_LEFT) switch_to_screen2();
    }
}


/********************************************************************
 *  UI  Site 1 (Original)
 *******************************************************************/
void create_ui()
{
    screen1 = lv_scr_act();                      
    lv_obj_add_event_cb(screen1, screen1_gesture_event, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(screen1, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // Create an image for the logo
    LV_IMG_DECLARE(BitAxe_Logosml);
    lv_obj_t * logo = lv_img_create(screen1);
    lv_img_set_src(logo, &BitAxe_Logosml);
    lv_obj_align(logo, LV_ALIGN_TOP_MID, 0, 5);

    /*  Labels  ---------------------------------------------------- */
    label_status = lv_label_create(screen1);
    lv_obj_set_pos(label_status, 20, 220);
    lv_obj_set_style_text_color(label_status, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_status, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_status, "");

    hashrate_bar = lv_bar_create(screen1);
    lv_obj_set_size(hashrate_bar, 280, 10);
    lv_obj_align(hashrate_bar, LV_ALIGN_TOP_MID, 0, 85);
    lv_bar_set_range(hashrate_bar, 0, 2000);
    lv_obj_set_style_bg_color(hashrate_bar, lv_color_hex(0x404040), LV_PART_MAIN); // Dark grey background for hashrate bar
    lv_obj_set_style_bg_color(hashrate_bar, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR); // White indicator for hashrate bar

    temp_bar = lv_bar_create(screen1);
    lv_obj_set_size(temp_bar, 280, 10);
    lv_obj_align(temp_bar, LV_ALIGN_TOP_MID, 0, 115);
    lv_bar_set_range(temp_bar, 0, 100);
    lv_obj_set_style_bg_color(temp_bar, lv_color_hex(0x404040), LV_PART_MAIN); // Dark grey background for temp bar
    lv_obj_set_style_bg_color(temp_bar, lv_color_hex(0xFF0000), LV_PART_INDICATOR); // Red indicator for temp bar

    vr_temp_bar = lv_bar_create(screen1);
    lv_obj_set_size(vr_temp_bar, 280, 10);
    lv_obj_align(vr_temp_bar, LV_ALIGN_TOP_MID, 0, 145);
    lv_bar_set_range(vr_temp_bar, 0, 100);
    lv_obj_set_style_bg_color(vr_temp_bar, lv_color_hex(0x404040), LV_PART_MAIN); // Dark grey background for temp bar
    lv_obj_set_style_bg_color(vr_temp_bar, lv_color_hex(0xFF0000), LV_PART_INDICATOR); // Red indicator for temp bar

    pwr_bar = lv_bar_create(screen1);
    lv_obj_set_size(pwr_bar, 280, 10);
    lv_obj_align(pwr_bar, LV_ALIGN_TOP_MID, 0, 175);
    lv_bar_set_range(pwr_bar, 0, 100);
    lv_obj_set_style_bg_color(pwr_bar, lv_color_hex(0x404040), LV_PART_MAIN); // Dark grey background for pwr bar
    lv_obj_set_style_bg_color(pwr_bar, lv_color_hex(0x00FF00), LV_PART_INDICATOR); // Green indicator for pwr bar

    label_hashrate = lv_label_create(screen1);
    lv_obj_set_pos(label_hashrate, 20, 70);
    lv_obj_set_style_text_color(label_hashrate, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_hashrate, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_hashrate, "HASHRATE: --");

    label_shares = lv_label_create(screen1);
    lv_obj_set_pos(label_shares, 20, 190);
    lv_obj_set_style_text_color(label_shares, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_shares, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_shares, "SHARES: --");

    label_best = lv_label_create(screen1);
    lv_obj_set_pos(label_best, 170, 190);
    lv_obj_set_style_text_color(label_best, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_best, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_best, "BEST: --");

    label_session = lv_label_create(screen1);
    lv_obj_set_pos(label_session, 170, 205);
    lv_obj_set_style_text_color(label_session, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_session, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_session, "SESSION: --");

    label_temp = lv_label_create(screen1);
    lv_obj_set_pos(label_temp, 20, 100);
    lv_obj_set_style_text_color(label_temp, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_temp, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_temp, "TEMP: --");

    label_vr_temp = lv_label_create(screen1);
    lv_obj_set_pos(label_vr_temp, 20, 130);
    lv_obj_set_style_text_color(label_vr_temp, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_vr_temp, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_vr_temp, "VR TEMP: --");

    label_pwr = lv_label_create(screen1);
    lv_obj_set_pos(label_pwr, 20, 160);
    lv_obj_set_style_text_color(label_pwr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(label_pwr, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_label_set_text(label_pwr, "PWR: --");

    bitcoin_logo_img = lv_img_create(screen1);
    lv_img_set_src(bitcoin_logo_img, &bitcoin_logo_16x16);
    lv_obj_align_to(bitcoin_logo_img, label_shares, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_obj_add_flag(bitcoin_logo_img, LV_OBJ_FLAG_HIDDEN);
}

/********************************************************************
 *  UI  Site 2
 *******************************************************************/
void create_screen2()
{
    screen2 = lv_obj_create(NULL);

    lv_obj_add_event_cb(screen2, screen2_gesture_event, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(screen2, LV_OBJ_FLAG_GESTURE_BUBBLE);

    
    label_hashrate_screen2 = lv_label_create(screen2);
    lv_obj_set_pos(label_hashrate_screen2, 13, 25); 
    lv_obj_set_style_text_color(label_hashrate_screen2, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_hashrate_screen2, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_label_set_text(label_hashrate_screen2, "HASHRATE: --");

    
    label_shares_screen2 = lv_label_create(screen2);
    lv_obj_set_pos(label_shares_screen2, 13, 38); 
    lv_obj_set_style_text_color(label_shares_screen2, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_shares_screen2, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_label_set_text(label_shares_screen2, "SHARES: --");

    
    label_best_screen2 = lv_label_create(screen2);
    lv_obj_set_pos(label_best_screen2, 13, 51); 
    lv_obj_set_style_text_color(label_best_screen2, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_best_screen2, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_label_set_text(label_best_screen2, "BEST: --");

    
    label_temp_screen2 = lv_label_create(screen2);
    lv_obj_set_pos(label_temp_screen2, 13, 64); 
    lv_obj_set_style_text_color(label_temp_screen2, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_temp_screen2, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_label_set_text(label_temp_screen2, "TEMP: --");

    
    label_session_screen2 = lv_label_create(screen2);
    lv_obj_set_pos(label_session_screen2, 13, 77); 
    lv_obj_set_style_text_color(label_session_screen2, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_session_screen2, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_label_set_text(label_session_screen2, "SESSION: --");
    lv_obj_add_flag(label_session_screen2, LV_OBJ_FLAG_HIDDEN); 
}

/********************************************************************
 *  UI  Site 3
 *******************************************************************/
void create_screen3()
{
    screen3 = lv_obj_create(NULL);

    lv_obj_add_event_cb(screen3, screen3_gesture_event, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(screen3, LV_OBJ_FLAG_GESTURE_BUBBLE);

    
    label_hashrate_screen3 = lv_label_create(screen3);
    lv_obj_set_pos(label_hashrate_screen3, 50, 10); 
    lv_obj_set_style_text_color(label_hashrate_screen3, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_hashrate_screen3, &lv_font_montserrat_24, LV_PART_MAIN); 
    lv_label_set_text(label_hashrate_screen3, "HASHRATE: --");

    
    label_shares_screen3 = lv_label_create(screen3);
    lv_obj_set_pos(label_shares_screen3, 8, 45); 
    lv_obj_set_style_text_color(label_shares_screen3, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_shares_screen3, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_label_set_text(label_shares_screen3, "SHARES: --");

    
    label_best_screen3 = lv_label_create(screen3);
    lv_obj_set_pos(label_best_screen3, 8, 65); 
    lv_obj_set_style_text_color(label_best_screen3, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_best_screen3, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_label_set_text(label_best_screen3, "BEST: --");

    
    label_session_screen3 = lv_label_create(screen3);
    lv_obj_set_pos(label_session_screen3, 8, 85); 
    lv_obj_set_style_text_color(label_session_screen3, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_session_screen3, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_label_set_text(label_session_screen3, "SESSION: --");

    
    label_temp_screen3 = lv_label_create(screen3);
    lv_obj_set_pos(label_temp_screen3, 8, 105); 
    lv_obj_set_style_text_color(label_temp_screen3, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_temp_screen3, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_label_set_text(label_temp_screen3, "TEMP: --");

    
    label_pwr_screen3 = lv_label_create(screen3);
    lv_obj_set_pos(label_pwr_screen3, 8, 125); 
    lv_obj_set_style_text_color(label_pwr_screen3, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_pwr_screen3, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_label_set_text(label_pwr_screen3, "PWR: --");

    
    label_status_screen3 = lv_label_create(screen3);
    lv_obj_set_pos(label_status_screen3, 8, 145); 
    lv_obj_set_style_text_color(label_status_screen3, lv_color_hex(0xFFFFFF), LV_PART_MAIN); 
    lv_obj_set_style_text_font(label_status_screen3, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_label_set_text(label_status_screen3, "");
}

void show_wifi_instructions() {
    
    lv_obj_clean(lv_scr_act());
    
    
    lv_obj_t* wifi_overlay = lv_obj_create(lv_scr_act());
    lv_obj_set_size(wifi_overlay, 320, 240); 
    lv_obj_align(wifi_overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(wifi_overlay, lv_color_hex(0x000000), LV_PART_MAIN); 
    lv_obj_set_style_bg_opa(wifi_overlay, LV_OPA_100, LV_PART_MAIN); 
    lv_obj_set_style_border_width(wifi_overlay, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(wifi_overlay, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(wifi_overlay, LV_OBJ_FLAG_SCROLLABLE);

    
    lv_obj_t* title = lv_label_create(wifi_overlay);
    lv_obj_set_pos(title, 10, 10);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFF00), LV_PART_MAIN); // Yellow text
    lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, LV_PART_MAIN);
    lv_label_set_text(title, "WiFi Setup Required");

    // Instructions text
    lv_obj_t* instructions = lv_label_create(wifi_overlay);
    lv_obj_set_pos(instructions, 10, 40);
    lv_obj_set_style_text_color(instructions, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // White text
    lv_obj_set_style_text_font(instructions, LV_FONT_DEFAULT, LV_PART_MAIN);
    lv_obj_set_width(instructions, 300); 
    lv_label_set_long_mode(instructions, LV_LABEL_LONG_WRAP);
    lv_label_set_text(instructions, 
        "1. Connect to WiFi 'BitaxeMonitorAP'\n"
        "   Password: password\n"
        "2. Open browser, go to 192.168.4.1\n"
        "3. Enter your WiFi details & Bitaxe URL\n"
        "4. Save to connect. Device will restart.");

    
    lv_obj_t* hint = lv_label_create(wifi_overlay);
    lv_obj_set_pos(hint, 10, 200);
    lv_obj_set_style_text_color(hint, lv_color_hex(0x808080), LV_PART_MAIN); 
    lv_obj_set_style_text_font(hint, LV_FONT_DEFAULT, LV_PART_MAIN);
    lv_label_set_text(hint, "Waiting for configuration...");

    
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(disp);
}
// Function to handle long press for WiFi reset (10 seconds)
void long_press_reset_handler(lv_event_t* e) {
    static int long_press_duration = 0;
    if (lv_event_get_code(e) == LV_EVENT_LONG_PRESSED_REPEAT) {
        long_press_duration += 750; 
        Serial.print("Long press duration: ");
        Serial.println(long_press_duration);
        if (long_press_duration >= 10000) { 
            Serial.println("Long press detected (10s), resetting WiFi settings...");
            WiFi.disconnect(true); 
            
            WiFiManager wm;
            wm.resetSettings(); 
            Serial.println("WiFiManager settings reset complete.");
            delay(1000);
            ESP.restart(); 
        }
    } else if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
        long_press_duration = 0; 
        Serial.println("Touch released, resetting long press duration");
    }
}




int prev_shares_accepted = 0;

void update_bitaxe_data()
{
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.reconnect();
        lv_label_set_text(label_status, "WIFI DISCONNECTED");
        lv_obj_set_style_text_color(label_status, lv_color_hex(0xFF0000), LV_PART_MAIN);
        if (label_status_screen3) {
            lv_label_set_text(label_status_screen3, "WIFI DISCONNECTED");
            lv_obj_set_style_text_color(label_status_screen3, lv_color_hex(0xFF0000), LV_PART_MAIN);
        }
        return;
    }

    HTTPClient http;
    http.setTimeout(3000);
    Serial.print("Connecting to: ");
    Serial.println(bitaxe_url);
    http.begin(bitaxe_url);

    int httpCode = http.GET();
    if (httpCode == 200) {
        String jsonResponse = http.getString();
        Serial.println("JSON Response: " + jsonResponse);
        StaticJsonDocument<2048> doc;
        DeserializationError error = deserializeJson(doc, jsonResponse);
        if (error) {
            lv_label_set_text(label_status, "JSON ERROR");
            lv_obj_set_style_text_color(label_status, lv_color_hex(0xFF0000), LV_PART_MAIN);
            if (label_status_screen3) {
                lv_label_set_text(label_status_screen3, "JSON ERROR");
                lv_obj_set_style_text_color(label_status_screen3, lv_color_hex(0xFF0000), LV_PART_MAIN);
            }
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
        } else {
            float hashRateGHs = doc["hashRate"].as<float>();
            float hashRate = hashRateGHs / 1000.0;
            int sharesAccepted = doc["sharesAccepted"].as<int>();
            int sharesRejected = doc["sharesRejected"].as<int>();
            String bestDiff = doc["bestDiff"].as<String>();
            String sessionDiff = doc["bestSessionDiff"].as<String>();
            float temp = doc["temp"].as<float>();
            float vr_temp = doc["vrTemp"].as<float>();
            float power = doc["power"].as<float>();
            Serial.print("Parsed Temp: ");
            Serial.println(temp);
            Serial.print("Parsed VR Temp: ");
            Serial.println(vr_temp);

            char hashrate_str[10];
            char temp_str[10];
            char vr_temp_str[10];
            char power_str[10];
            snprintf(hashrate_str, sizeof(hashrate_str), "%d.%02d", (int)hashRate, (int)(hashRate * 100) % 100);
            snprintf(temp_str, sizeof(temp_str), "%.1f", temp);
            snprintf(vr_temp_str, sizeof(vr_temp_str), "%.1f", vr_temp);
            snprintf(power_str, sizeof(power_str), "%.1f", power);

            String label_hashrate_text = String("HASHRATE: ") + hashrate_str + " GH/s";
            String label_temp_text = String("TEMP: ") + temp_str + "°C";
            String label_vr_temp_text = String("VR TEMP: ") + vr_temp_str + "°C";
            String label_power_text = String("PWR: ") + power_str + "W";

            lv_bar_set_value(hashrate_bar, (int)(hashRateGHs), LV_ANIM_ON);
            lv_bar_set_value(temp_bar, (int)temp, LV_ANIM_ON); 
            lv_bar_set_value(vr_temp_bar, (int)vr_temp, LV_ANIM_ON);
            lv_bar_set_value(pwr_bar, (int)power, LV_ANIM_ON); 

            // Aktualisierung auf Seite 1
            lv_label_set_text(label_hashrate, label_hashrate_text.c_str());
            lv_label_set_text_fmt(label_shares, "SHARES: %d / %d", sharesAccepted, sharesRejected);
            lv_label_set_text_fmt(label_best, "BEST: %s", bestDiff.c_str());
            lv_label_set_text_fmt(label_session, "SESSION: %s", sessionDiff.c_str());
            lv_label_set_text(label_temp, label_temp_text.c_str());
            lv_label_set_text(label_vr_temp, label_vr_temp_text.c_str());
            lv_label_set_text(label_pwr, label_power_text.c_str());

            if (sharesAccepted > prev_shares_accepted) {
                lv_obj_clear_flag(bitcoin_logo_img, LV_OBJ_FLAG_HIDDEN);
                lv_timer_t * timer = lv_timer_create([](lv_timer_t * t) {
                    lv_obj_add_flag(bitcoin_logo_img, LV_OBJ_FLAG_HIDDEN);
                    lv_timer_del(t);
                }, 2000, NULL);
            }
            prev_shares_accepted = sharesAccepted;

            // Aktualisierung auf Seite 2
            lv_label_set_text(label_hashrate_screen2, label_hashrate_text.c_str());
            lv_label_set_text_fmt(label_shares_screen2, "SHARES: %d / %d", sharesAccepted, sharesRejected);
            lv_label_set_text_fmt(label_best_screen2, "BEST: %s", bestDiff.c_str());
            lv_label_set_text(label_temp_screen2, label_temp_text.c_str()); 

            // Aktualisierung auf Seite 3
            lv_label_set_text(label_hashrate_screen3, label_hashrate_text.c_str());
            lv_label_set_text_fmt(label_shares_screen3, "SHARES: %d / %d", sharesAccepted, sharesRejected);
            lv_label_set_text_fmt(label_best_screen3, "BEST: %s", bestDiff.c_str());
            lv_label_set_text_fmt(label_session_screen3, "SESSION: %s", sessionDiff.c_str());
            lv_label_set_text(label_temp_screen3, label_temp_text.c_str());
            lv_label_set_text(label_pwr_screen3, label_power_text.c_str()); 

            if (temp < 60) {
                lv_obj_set_style_text_color(label_temp, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_color(label_temp_screen2, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_color(label_temp_screen3, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            } else {
                lv_obj_set_style_text_color(label_temp, lv_color_hex(0xFF0000), LV_PART_MAIN);
                lv_obj_set_style_text_color(label_temp_screen2, lv_color_hex(0xFF0000), LV_PART_MAIN);
                lv_obj_set_style_text_color(label_temp_screen3, lv_color_hex(0xFF0000), LV_PART_MAIN);
            }

            lv_label_set_text(label_status, "");
            if (label_status_screen3) {
                lv_label_set_text(label_status_screen3, "");
            }
        }
    } else {
        lv_label_set_text(label_status, "HTTP ERROR");
        lv_obj_set_style_text_color(label_status, lv_color_hex(0xFF0000), LV_PART_MAIN);
        if (label_status_screen3) {
            lv_label_set_text(label_status_screen3, "HTTP ERROR");
            lv_obj_set_style_text_color(label_status_screen3, lv_color_hex(0xFF0000), LV_PART_MAIN);
        }
        Serial.printf("HTTP Error: %s, Code: %d\n", http.errorToString(httpCode).c_str(), httpCode);
    }
    http.end();
}


/********************************************************************
 *  Setup
 *******************************************************************/
void setup() {
    Serial.begin(115200);
    delay(200);

    api_data_mutex = xSemaphoreCreateMutex();

    delay(200);

    /*  TFT  ---------------------------------------------------------- */
    tft.init();
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);
    tft.setRotation(2);

    /*  Touch  -------------------------------------------------------- */
    touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touchscreen.begin(touchscreenSPI);
    touchscreen.setRotation(1);

    /*  LVGL  --------------------------------------------------------- */
    lv_init();

    
    static lv_color_t* buf1 = nullptr;
    static lv_color_t* buf2 = nullptr;
    if (ESP.getPsramSize() > 0) {
        buf1 = (lv_color_t*)ps_malloc(320 * 60 * sizeof(lv_color_t));
        buf2 = (lv_color_t*)ps_malloc(320 * 60 * sizeof(lv_color_t));
    } else {
        static lv_color_t buf1_fallback[320 * 20];
        static lv_color_t buf2_fallback[320 * 20];
        buf1 = buf1_fallback;
        buf2 = buf2_fallback;
    }
    disp = lv_display_create(320, 240); // Create a display
    lv_display_set_flush_cb(disp, flush_cb); // Set the flush callback
    lv_display_set_user_data(disp, &tft); // Pass tft object to the display
    lv_display_set_buffers(disp, buf1, buf2, (ESP.getPsramSize() > 0) ? 320 * 60 * sizeof(lv_color_t) : 320 * 20 * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);

    
    indev_drv = lv_indev_create(); // Create an input device
    lv_indev_set_type(indev_drv, LV_INDEV_TYPE_POINTER); // Set the type
    lv_indev_set_read_cb(indev_drv, touchscreen_read); // Set the read callback

    
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), LV_PART_MAIN);

    
    create_ui();      
    create_screen2(); 
    create_screen3();

// Initialize WiFiManager
WiFiManager wm;

// Enable debug output for WiFiManager to track issues
wm.setDebugOutput(true);

// HTML for Input Field for Bitaxe IP Address
String html = R"(
  <br/>
  <label for='bitaxe_ip'>Bitaxe IP Address</label>
  <input type='text' name='bitaxe_ip' id='bitaxe_ip' value='' placeholder='IP (e.g., 192.168.1.100)' size='40'>
  <br/>
  <label for='rotation'>Display Rotation (0-3)</label>
  <input type='text' name='rotation' id='rotation' value='2' size='2'>
)";

WiFiManagerParameter custom_html_param(html.c_str());

// Add custom parameters to WiFiManager
wm.addParameter(&custom_html_param);

// Set callback to save custom parameters to NVS
wm.setSaveConfigCallback([&wm]() {
  Serial.println("Saving custom configuration...");
  Preferences prefs;
  prefs.begin("config", false); // Open Preferences in read-write mode
  String bitaxe_ip = wm.server->arg("bitaxe_ip");
  String bitaxe_url_full = "";
  if (bitaxe_ip != "") {
    bitaxe_url_full = "http://" + bitaxe_ip + "/api/system/info";
  }
  String rotationValue = wm.server->arg("rotation");
  prefs.putString("bitaxe_url", bitaxe_url_full);
  prefs.putInt("rotation", atoi(rotationValue.c_str()));
  prefs.end();
  Serial.println("Custom configuration saved to NVS");
});

// Set a longer timeout for the configuration portal (in seconds)
wm.setConfigPortalTimeout(300); // Increased to 5 minutes to ensure users have enough time

// Show WiFi instructions on the display when starting the configuration portal
Serial.println("Starting WiFiManager AutoConnect...");
show_wifi_instructions(); // Display instructions on the ESP32 screen

// Try to connect to WiFi or start configuration portal if connection fails
if (!wm.autoConnect("BitaxeMonitorAP", "password")) {
  Serial.println("Failed to connect and hit timeout, restarting ESP32...");
  ESP.restart(); // Restart ESP32 if connection fails after timeout
} else {
  Serial.println("WiFi connected successfully!");
  // Clean the screen completely and recreate the UI
  lv_obj_clean(lv_scr_act()); // Remove all objects including the instructions overlay
  create_ui(); // Recreate the main UI
  create_screen2(); // Recreate screen 2
  create_screen3(); // Recreate screen 3
  lv_scr_load(screen1); // Load the first screen
  lv_obj_invalidate(lv_scr_act()); // Invalidate to force redraw
  lv_refr_now(disp); // Force immediate refresh
}

// Load saved custom parameters after connection
prefs.begin("config", true); // Open Preferences in read-only mode
String saved_bitaxe_url = prefs.getString("bitaxe_url", "http://YOUR_BITAXE_IP/api/system/info");
int saved_rotation = prefs.getInt("rotation", 2); // Default to 2 if not set
prefs.end();

// Update global variables with saved values
strncpy(custom_bitaxe_url, saved_bitaxe_url.c_str(), sizeof(custom_bitaxe_url) - 1);
custom_bitaxe_url[sizeof(custom_bitaxe_url) - 1] = '\0'; // Ensure null termination
bitaxe_url = custom_bitaxe_url; // Update the global bitaxe_url pointer with the saved value

// Apply saved display rotation and corresponding touch rotation
tft.setRotation(saved_rotation);
// Define a mapping for touch rotation based on display rotation
// This mapping can be adjusted based on user feedback
int touch_rotation;
switch (saved_rotation) {
  case 0: touch_rotation = 3; break; // Example mapping, adjust based on feedback
  case 1: touch_rotation = 0; break;
  case 2: touch_rotation = 1; break; // Matches your setup (display 2 -> touch 1)
  case 3: touch_rotation = 2; break;
  default: touch_rotation = 1; // Fallback to your default
}
touchscreen.setRotation(touch_rotation);
Serial.print("Applied display rotation: ");
Serial.println(saved_rotation);
Serial.print("Applied touch rotation: ");
Serial.println(touch_rotation);


    
    lv_label_set_text(label_status, "");
}


/********************************************************************
 *  Loop
 *******************************************************************/
void loop() {
    static uint32_t last_ms = millis();
    uint32_t now_ms = millis();
    lv_tick_inc(now_ms - last_ms);   
    last_ms = now_ms;

    lv_timer_handler();              
    update_bitaxe_data();                   

    delay(5);
}
