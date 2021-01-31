#include <stdio.h>
#include <esp_log.h>
#include <esp_system.h>
#include "frame_base.h"

#define LENGTH 5

static uint32_t lastMillis = 0;

typedef struct segment {
    uint8_t col;
    uint8_t row;
    float hue;
    float sat;
    float value;
} segment;

static segment segments[LENGTH];
static uint8_t movesSinceDirChange = 0;
static uint8_t dir = 2;


void leds_set_pixel(int pixel, float hue, float sat, float value);
void leds_clear();
void leds_update();

void snake_scene_update(uint32_t currMillis)
{
    uint32_t elapsedMillis = currMillis - lastMillis;

    if (elapsedMillis >= 200) {
        leds_clear();

        for (uint8_t i = 0; i < LENGTH; i++) {
            leds_set_pixel(
                pixelIdx(segments[i].col, segments[i].row),
                segments[i].hue,
                segments[i].sat,
                segments[i].value
            );
        }
        leds_update();

        if (++movesSinceDirChange > 5) {
            // There are only two possible new directions for each current direction
            // current dir: up; new dirs: left, right
            // current dir: right; new dirs: up, down
            // current dir: down; new dirs: right, left
            // current dir: left: down, up
            uint8_t newDir = esp_random() & 0x01;
            if (dir == 0) {
                // Up
                if (newDir == 1) {
                    dir = 3;
                } else {
                    dir = 1;
                }
            } else if (dir == 1) {
                // Right
                if (newDir == 1) {
                    dir = 2;
                } else {
                    dir = 0;
                }
            } else if (dir == 2) {
                // Down
                if (newDir == 1) {
                    dir = 1;
                } else {
                    dir = 3;
                }
            } else if (dir == 3) {
                // Left
                if (newDir == 1) {
                    dir = 2;
                } else {
                    dir = 0;
                }
            }
            movesSinceDirChange = 0;
        }

        for (uint8_t i = LENGTH - 1; i > 0; i--) {
            segments[i].col = segments[i - 1].col;
            segments[i].row = segments[i - 1].row;
        }

        bool moved = false;
        while (!moved) {
            if (dir == 0) {
                // Up
                if (--segments[0].row == 255) {
                    segments[0].row = 0;
                    dir = 1;
                    continue;
                }
                moved = true;
            } else if (dir == 1) {
                // Right
                if (++segments[0].col == PIXELS_PER_ROW) {
                    segments[0].col = PIXELS_PER_ROW - 1;
                    dir = 2;
                    continue;
                }
                moved = true;
            } else if (dir == 2) {
                // Down
                if (++segments[0].row == NUM_ROWS) {
                    segments[0].row = NUM_ROWS - 1;
                    dir = 3;
                    continue;
                }
                moved = true;
            } else if (dir == 3) {
                // Left
                if (--segments[0].col == 255) {
                    segments[0].col = 0;
                    dir = 0;
                    continue;
                }
                moved = true;
            }
        }

        colourConfig baseColourConfig = getBaseColourConfig();
        for (uint8_t i = 0; i < LENGTH; i++) {
            segments[i].hue += baseColourConfig.hueChange;
            segments[i].value += baseColourConfig.valueChange;
            if (segments[i].value > baseColourConfig.maxValue) {
                segments[i].value = 0;
            }
        }

        lastMillis = currMillis;
    }
}

void snake_scene_stop() {}

void snake_scene_reset_millis(uint32_t currMillis)
{
    lastMillis = currMillis;
    colourConfig baseColourConfig = getBaseColourConfig();
    for (uint8_t i = 0; i < LENGTH; i++) {
        segments[i].col = 0;
        segments[i].row = 0;
        segments[i].hue = baseColourConfig.hue;
        segments[i].sat = baseColourConfig.sat;
        segments[i].value = baseColourConfig.value;
    }
}