#include <stdio.h>
#include <esp_log.h>
#include <esp_system.h>
#include "frame_base.h"

static const char *TAG = "scene blocks";

#define BLOCK_SIZE 2
#define NUM_BLOCK_COLS (PIXELS_PER_ROW / BLOCK_SIZE)
#define NUM_BLOCK_ROWS (NUM_ROWS / BLOCK_SIZE)
#define NUM_BLOCKS (NUM_BLOCK_COLS * NUM_BLOCK_ROWS)

typedef struct block {
    uint8_t row;
    bool halfRow;
    float hue;
    float sat;
    float value;
    bool falling;
} block;

static block blocks[NUM_BLOCK_COLS][NUM_BLOCK_ROWS];
static bool colsComplete[NUM_BLOCK_COLS];
static uint8_t activeCol = 0;
static uint16_t millisBeforeMove = 300;
static uint8_t movesBeforeColsCompleteReset = 3;
static uint8_t movesSinceComplete = 0;

static uint32_t lastMillis = 0;

void leds_set_pixel(int pixel, float hue, float sat, float value);
void leds_clear(bool updateLeds);
void leds_update();

static void reset_blocks()
{
    float hue = 0.01;
    for (uint8_t col = 0; col < NUM_BLOCK_COLS; col++) {
        for (uint8_t colBlock = 0; colBlock < NUM_BLOCK_ROWS; colBlock++) {
            blocks[col][colBlock].row = 0;
            blocks[col][colBlock].halfRow = false;
            blocks[col][colBlock].hue = hue;
            blocks[col][colBlock].sat = 1;
            blocks[col][colBlock].value = 0.1;
            blocks[col][colBlock].falling = false;
            switch (esp_random() % 5) {
                case 0:
                    hue += 0.01;
                    break;
                case 1:
                    hue += 0.25;
                    break;
                case 2:
                    hue += 0.05;
                    break;
                case 3:
                    hue += 0.11;
                    break;
                case 4:
                    hue += 0.33;
                    break;
            }
        }
        colsComplete[col] = false;
    }
}

void blocks_scene_update(uint32_t currMillis)
{
    uint32_t elapsedMillis = currMillis - lastMillis;

    if (elapsedMillis >= millisBeforeMove) {
        leds_clear(false);

        for (uint8_t col = 0; col < NUM_BLOCK_COLS; col++) {
            for (uint8_t colBlock = 0; colBlock < NUM_BLOCK_ROWS; colBlock++) {
                if (blocks[col][colBlock].row == 0 && !blocks[col][colBlock].halfRow) {
                    continue;
                }
                for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
                    if (blocks[col][colBlock].row == 0 && !blocks[col][colBlock].halfRow) {
                        continue;
                    }
                    uint8_t x = (col * BLOCK_SIZE);
                    uint8_t y = ((blocks[col][colBlock].row - 1) * BLOCK_SIZE) + i;
                    if (blocks[col][colBlock].halfRow) {
                        y++;
                    }
                    leds_set_pixel(
                        pixelIdx(x, y),
                        blocks[col][colBlock].hue,
                        blocks[col][colBlock].sat,
                        blocks[col][colBlock].value
                    );
                    leds_set_pixel(
                        pixelIdx(x + 1, y),
                        blocks[col][colBlock].hue,
                        blocks[col][colBlock].sat,
                        blocks[col][colBlock].value
                    );
                }
            }
        }

        leds_update();

        uint8_t numColsRemaining = 0;
        uint8_t remainingCols[NUM_BLOCK_COLS];
        for (uint8_t i = 0; i < NUM_BLOCK_COLS; i++) {
            if (!colsComplete[i]) {
                remainingCols[numColsRemaining] = i + 1;
                numColsRemaining++;
            }
        }
        // Select next column
        if (activeCol == 0 && numColsRemaining > 0) {
            uint8_t selectedColIdx = esp_random() % numColsRemaining;
            activeCol = remainingCols[selectedColIdx];
            printf("active col: %d\n", activeCol);
        }

        if (activeCol > 0) {
            uint8_t colIdx = activeCol - 1;

            for (uint8_t colBlock = 0; colBlock < NUM_BLOCK_ROWS; colBlock++) {
                // Skip blocks that have finished falling
                if (blocks[colIdx][colBlock].row > 0 && !blocks[colIdx][colBlock].falling) continue;

                // Find the current top of the column and find whether there is already a falling block
                bool canMove = true;
                uint8_t columnTop = NUM_BLOCK_ROWS;
                for (uint8_t checkColBlock = 0; checkColBlock < NUM_BLOCK_ROWS; checkColBlock++) {
                    if (checkColBlock == colBlock) continue;
                    if (blocks[colIdx][checkColBlock].falling) {
                        canMove = false;
                    } else if (blocks[colIdx][checkColBlock].row > 0) {
                        if (blocks[colIdx][checkColBlock].row == columnTop) {
                            columnTop = blocks[colIdx][checkColBlock].row - 1;
                        }
                    }
                }

                if (canMove) {
                    blocks[colIdx][colBlock].falling = true;

                    if (blocks[colIdx][colBlock].row < columnTop) {
                        if (!blocks[colIdx][colBlock].halfRow) {
                            blocks[colIdx][colBlock].halfRow = true;
                        } else {
                            blocks[colIdx][colBlock].halfRow = false;
                            if (++blocks[colIdx][colBlock].row == columnTop) {
                                blocks[colIdx][colBlock].falling = false;
                            }
                            if (columnTop == 1) {
                                colsComplete[colIdx] = true;
                                activeCol = 0;
                            }
                        }
                    }

                    break;
                }
            }
        }

        if (numColsRemaining == 0) {
            if (++movesSinceComplete > movesBeforeColsCompleteReset) {
                movesSinceComplete = 0;
                reset_blocks();
            }
        }

        lastMillis = currMillis;
    }
}

void blocks_scene_init()
{
    lastMillis = 0;
    reset_blocks();
}

void blocks_scene_update_config(cJSON *json)
{
    const cJSON *moveMillisJson = cJSON_GetObjectItem(json, "moveMillis");
    if (cJSON_IsNumber(moveMillisJson)) {
        millisBeforeMove = (uint16_t) moveMillisJson->valueint;
    }
    const cJSON *movesBeforeColsResetJson = cJSON_GetObjectItem(json, "movesBeforeColsReset");
    if (cJSON_IsNumber(movesBeforeColsResetJson)) {
        movesBeforeColsCompleteReset = (uint8_t) movesBeforeColsResetJson->valueint;
    }

    ESP_LOGI(TAG, "Blocks config: move millis = %d, moves before reset = %d", millisBeforeMove, movesBeforeColsCompleteReset);
}