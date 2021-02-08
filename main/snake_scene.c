#include <stdio.h>
#include <esp_log.h>
#include <esp_system.h>
#include "frame_base.h"

#define LENGTH 5

typedef struct segment {
    uint8_t col;
    uint8_t row;
    float hue;
    float sat;
    float value;
} segment;
typedef enum {UP, RIGHT, DOWN, LEFT} direction;

static segment segments[LENGTH];
static uint8_t movesBeforeDirChange = 2;
static uint8_t millisBeforeMove = 100;
static uint8_t movesSinceDirChange = 0;
static direction currentDirection = DOWN;
static hsvColour colour = {
    .hue = 0.01,
    .sat = 1,
    .value = 0.07
};
static hsvColourChangeConfig colourChange = {
    .hueChange = 0.01,
    .valueChange = 0,
    .maxValue = HSV_MAX_VALUE
};

static uint32_t lastMillis = 0;

void leds_set_pixel(int pixel, float hue, float sat, float value);
void leds_clear();
void leds_update();

static direction newDirection(direction currentDir, uint8_t col, uint8_t row)
{
    uint8_t dirChoice = esp_random() & 0x01;
    if (currentDirection == UP) {
        if (col == 0) {
            if (row == 0) {
                // Top left corner, can only go right
                return RIGHT;
            }
            // Left hand column, can only go right or continue up
            if (dirChoice == 1) {
                return RIGHT;
            } else {
                return UP;
            }
        } else if (col == PIXELS_PER_ROW - 1) {
            if (row == 0) {
                // Top right corner, can only go left
                return LEFT;
            }
            // Right hand column, can only go left or continue up
            if (dirChoice == 1) {
                return LEFT;
            } else {
                return UP;
            }
        } else {
            if (dirChoice == 1) {
                return LEFT;
            } else {
                return RIGHT;
            }
        }
    } else if (currentDir == RIGHT) {
        if (row == 0) {
            if (col == PIXELS_PER_ROW - 1) {
                // Top right corner, can only go down
                return DOWN;
            }
            // Top row, can only go down or continue right
            if (dirChoice == 1) {
                return DOWN;
            } else {
                return RIGHT;
            }
        } else if (row == NUM_ROWS - 1) {
            if (col == PIXELS_PER_ROW - 1) {
                // Bottom right corner, can only go up
                return UP;
            }
            // Bottom row, can only go up or continue right
            if (dirChoice == 1) {
                return UP;
            } else {
                return RIGHT;
            }
        } else {
            if (dirChoice == 1) {
                return UP;
            } else {
                return DOWN;
            }
        }
    } else if (currentDir == DOWN) {
        if (col == 0) {
            if (row == NUM_ROWS - 1) {
                // Bottom left corner, can only go right
                return RIGHT;
            }
            // Left hand column, can only go right or continue down
            if (dirChoice == 1) {
                return RIGHT;
            } else {
                return DOWN;
            }
        } else if (col == PIXELS_PER_ROW - 1) {
            if (row == NUM_ROWS - 1) {
                // Bottom right corner, can only go left
                return LEFT;
            }
            // Right hand column, can only go left or continue down
            if (dirChoice == 1) {
                return LEFT;
            } else {
                return DOWN;
            }
        } else {
            if (dirChoice == 1) {
                return RIGHT;
            } else {
                return LEFT;
            }
        }
    } else if (currentDir == LEFT) {
        if (row == 0) {
            if (col == 0) {
                // Top left corner, can only go down
                return DOWN;
            }
            // Top row, can only go down or continue left
            if (dirChoice == 1) {
                return DOWN;
            } else {
                return LEFT;
            }
        } else if (row == NUM_ROWS - 1) {
            if (col == 0) {
                // Bottom left corner, can only go up
                return UP;
            }
            // Bottom row, can only go up or continue left
            if (dirChoice == 1) {
                return UP;
            } else {
                return LEFT;
            }
        } else {
            if (dirChoice == 1) {
                return DOWN;
            } else {
                return UP;
            }
        }
    }

    // Default to DOWN, this shouldn't be reached
    return DOWN;
}

void snake_scene_update(uint32_t currMillis)
{
    uint32_t elapsedMillis = currMillis - lastMillis;

    if (elapsedMillis >= millisBeforeMove) {
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

        if (++movesSinceDirChange > movesBeforeDirChange) {
            currentDirection = newDirection(currentDirection, segments[0].col, segments[0].row);
            movesSinceDirChange = 0;
        }

        // Move tail segments to preceding segment position
        for (uint8_t i = LENGTH - 1; i > 0; i--) {
            segments[i].col = segments[i - 1].col;
            segments[i].row = segments[i - 1].row;
        }

        // Attempt to move head
        // If move hits side find new direction
        bool moved = false;
        while (!moved) {
            if (currentDirection == UP) {
                if (--segments[0].row == 255) {
                    segments[0].row = 0;
                    currentDirection = newDirection(currentDirection, segments[0].col, segments[0].row);
                    movesSinceDirChange = 0;
                    continue;
                }
                moved = true;
            } else if (currentDirection == RIGHT) {
                if (++segments[0].col == PIXELS_PER_ROW) {
                    segments[0].col = PIXELS_PER_ROW - 1;
                    currentDirection = newDirection(currentDirection, segments[0].col, segments[0].row);
                    movesSinceDirChange = 0;
                    continue;
                }
                moved = true;
            } else if (currentDirection == DOWN) {
                if (++segments[0].row == NUM_ROWS) {
                    segments[0].row = NUM_ROWS - 1;
                    currentDirection = newDirection(currentDirection, segments[0].col, segments[0].row);
                    movesSinceDirChange = 0;
                    continue;
                }
                moved = true;
            } else if (currentDirection == LEFT) {
                if (--segments[0].col == 255) {
                    segments[0].col = 0;
                    currentDirection = newDirection(currentDirection, segments[0].col, segments[0].row);
                    movesSinceDirChange = 0;
                    continue;
                }
                moved = true;
            }
        }

        for (uint8_t i = 0; i < LENGTH; i++) {
            segments[i].hue += colourChange.hueChange;
            segments[i].value += colourChange.valueChange;
            if (segments[i].value > colourChange.maxValue) {
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
    for (uint8_t i = 0; i < LENGTH; i++) {
        segments[i].col = 0;
        segments[i].row = 0;
        segments[i].hue = colour.hue;
        segments[i].sat = colour.sat;
        segments[i].value = colour.value;
    }
}