#ifndef FRAME_BASE_H
#define FRAME_BASE_H

#include <cJSON.h>

#define PIXELS_PER_ROW 8
#define NUM_ROWS 6
#define NUM_PIXELS (PIXELS_PER_ROW * NUM_ROWS)

#define HSV_MAX_VALUE 1

typedef struct hsvColour {
    float hue;
    float sat;
    float value;
} hsvColour;

typedef struct hsvColourChangeConfig {
    float hueChange;
    float valueChange;
    float maxValue;
} hsvColourChangeConfig;

typedef enum {SCENE_FILL, SCENE_SNAKE, SCENE_BLOCKS} scene;

uint8_t pixelIdx(uint8_t col, uint8_t row);
void setSceneConfig(char *scene, cJSON *json);
void setCurrentScene(char *newScene);
void currentSceneUpdate(uint32_t millis);
void currentSceneInit();

#endif /* FRAME_BASE_H */