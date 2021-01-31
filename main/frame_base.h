#ifndef FRAME_BASE_H
#define FRAME_BASE_H

#include <cJSON.h>

#define PIXELS_PER_ROW 8
#define NUM_ROWS 6
#define NUM_PIXELS (PIXELS_PER_ROW * NUM_ROWS)

#define HSV_MAX_VALUE 1

typedef struct colourConfig {
    float hue;
    float sat;
    float value;
    float hueChange;
    float valueChange;
    float maxValue;
} colourConfig;

uint8_t pixelIdx(uint8_t col, uint8_t row);
void updateBaseColour();
void setBaseColourConfig(cJSON *json);
colourConfig getBaseColourConfig();

#endif /* FRAME_BASE_H */