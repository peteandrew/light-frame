#include <stdio.h>
#include <esp_log.h>
#include <cJSON.h>
#include "frame_base.h"

static const char *TAG = "scene fill";

static uint32_t fill_scene_lastMillis = 0;
static uint8_t fill_scene_pixel = 0;
// 0 - filling, 1 - pausing, 2 - clearing, 3 - pausing
static uint8_t fill_scene_mode = 0;
// 0 - change at end of fill, 1 - change after each pixel, 2 - change after each row
static uint8_t fill_scene_colour_mode = 0;
static bool fill_scene_clear_mode = true;
// true - down, false - up
static bool fill_scene_fill_direction = true;
static bool fill_scene_clear_direction = true;
static uint16_t fill_scene_fill_pixel_millis = 50;
static uint16_t fill_scene_fill_pause_millis = 200;
static uint16_t fill_scene_clear_pixel_millis = 30;
static uint16_t fill_scene_clear_pause_millis = 100;


void leds_set_pixel(int pixel, float hue, float sat, float value);
void leds_update();


static void init_fill_pixel()
{
    if (fill_scene_fill_direction) {
        fill_scene_pixel = 0;
    } else {
        fill_scene_pixel = NUM_PIXELS - 1;
    }
}


static void init_clear_pixel()
{
    if (fill_scene_clear_direction) {
        fill_scene_pixel = 0;
    } else {
        fill_scene_pixel = NUM_PIXELS - 1;
    }
}


void fill_scene_update(uint32_t currMillis)
{
    uint32_t elapsedMillis = currMillis - fill_scene_lastMillis;
    // printf("mode %d\n", mode);
    // printf("elapsed millis %d\n", elapsedMillis);

    if (fill_scene_mode == 0 && elapsedMillis >= fill_scene_fill_pixel_millis) {
        colourConfig baseColourConfig = getBaseColourConfig();
        leds_set_pixel(
            fill_scene_pixel,
            baseColourConfig.hue,
            baseColourConfig.sat,
            baseColourConfig.value
        );
        leds_update();
        
        if (fill_scene_fill_direction) {
            fill_scene_pixel++;
        } else {
            fill_scene_pixel--;
        }
        
        if (fill_scene_colour_mode == 1) {
            updateBaseColour();
        } else if (fill_scene_colour_mode == 2) {
            // If fill direction is down we change colour when we are on the first pixel of the next row
            if (fill_scene_fill_direction && fill_scene_pixel % PIXELS_PER_ROW == 0) {
                updateBaseColour();
            // If fill direction is up we change colour when we are on the last pixel of the next row
            } else if (!fill_scene_fill_direction && (fill_scene_pixel + 1) % PIXELS_PER_ROW == 0) {
                updateBaseColour();
            }
        }

        bool fillDone = false;
        if (fill_scene_fill_direction && fill_scene_pixel == NUM_PIXELS) {
            fillDone = true;
        } else if (!fill_scene_fill_direction && fill_scene_pixel == 255) {
            // When fill direction is up we need to check whether fill is done when fill_scene_pixel has rolled over i.e. pixel 0 has been set
            fillDone = true;
        }
        if (fillDone) {
            fill_scene_mode++;
            if (fill_scene_colour_mode == 0) {
                updateBaseColour();
            }
        }

        fill_scene_lastMillis = currMillis;
    } else if (fill_scene_mode == 1 && elapsedMillis >= fill_scene_fill_pause_millis) {
        if (!fill_scene_clear_mode) {
            fill_scene_mode = 0;
            init_fill_pixel();
        } else {
            fill_scene_mode++;
            init_clear_pixel();
        }
        fill_scene_lastMillis = currMillis;
    } else if (fill_scene_mode == 2 && elapsedMillis >= fill_scene_clear_pixel_millis) {
        leds_set_pixel(fill_scene_pixel, 0, 0, 0);
        leds_update();

        if (fill_scene_clear_direction) {
            fill_scene_pixel++;
        } else {
            fill_scene_pixel--;
        }

        bool clearDone = false;
        if (fill_scene_clear_direction && fill_scene_pixel == NUM_PIXELS) {
            clearDone = true;
        } else if (!fill_scene_clear_direction && fill_scene_pixel == 255) {
            // We want to check whether fill_scene_pixel has rolled over - pixel 0 has been set
            clearDone = true;
        }
        if (clearDone) {
            fill_scene_mode++;
        }

        fill_scene_lastMillis = currMillis;
    } else if (fill_scene_mode == 3 && elapsedMillis >= fill_scene_clear_pause_millis) {
        fill_scene_mode = 0;
        init_fill_pixel();
        fill_scene_lastMillis = currMillis;
    }
}

void fill_scene_stop()
{
    fill_scene_pixel = 0;
    fill_scene_mode = 0;
}

void fill_scene_reset_millis(uint32_t currMillis)
{
    fill_scene_lastMillis = currMillis;
}

void fill_scene_update_config(cJSON *json)
{
    const cJSON *colourModelJson = cJSON_GetObjectItem(json, "colourMode");
    if (cJSON_IsNumber(colourModelJson)) {
        fill_scene_colour_mode = (uint8_t) colourModelJson->valueint;
        if (fill_scene_colour_mode > 2) {
           fill_scene_colour_mode = 0;
        }
    }
    const cJSON *clearModeJson = cJSON_GetObjectItem(json, "clearMode");
    if (cJSON_IsBool(clearModeJson)) {
        fill_scene_clear_mode = cJSON_IsTrue(clearModeJson);
    }
    const cJSON *fillDirectionJson = cJSON_GetObjectItem(json, "fillDirection");
    if (cJSON_IsBool(fillDirectionJson)) {
        fill_scene_fill_direction = cJSON_IsTrue(fillDirectionJson);
    }
    const cJSON *clearDirectionJson = cJSON_GetObjectItem(json, "clearDirection");
    if (cJSON_IsBool(clearDirectionJson)) {
        fill_scene_clear_direction = cJSON_IsTrue(clearDirectionJson);
    }
    const cJSON *fillPixelMillisJson = cJSON_GetObjectItem(json, "fillPixelMillis");
    if (cJSON_IsNumber(fillPixelMillisJson)) {
        fill_scene_fill_pixel_millis = (uint16_t) fillPixelMillisJson->valueint;
    }
    const cJSON *fillPauseMillisJson = cJSON_GetObjectItem(json, "fillPauseMillis");
    if (cJSON_IsNumber(fillPauseMillisJson)) {
        fill_scene_fill_pause_millis = (uint16_t) fillPauseMillisJson->valueint;
    }
    const cJSON *clearPixelMillisJson = cJSON_GetObjectItem(json, "clearPixelMillis");
    if (cJSON_IsNumber(clearPixelMillisJson)) {
        fill_scene_clear_pixel_millis = (uint16_t) clearPixelMillisJson->valueint;
    }
    const cJSON *clearPauseMillisJson = cJSON_GetObjectItem(json, "clearPauseMillis");
    if (cJSON_IsNumber(clearPauseMillisJson)) {
        fill_scene_clear_pause_millis = (uint16_t) clearPauseMillisJson->valueint;
    }
    ESP_LOGI(TAG, "Updated config: colour mode = %d, clear mode = %d, fill direction = %d, clear direction = %d, fill pixel millis = %d, fill pause millis = %d, clear pixel millis = %d, clear pause millis = %d\n", fill_scene_colour_mode, fill_scene_clear_mode, fill_scene_fill_direction, fill_scene_clear_direction, fill_scene_fill_pixel_millis, fill_scene_fill_pause_millis, fill_scene_clear_pixel_millis, fill_scene_clear_pause_millis);
}