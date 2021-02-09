#include <esp_log.h>
#include <string.h>
#include "frame_base.h"

static scene currentScene = SCENE_FILL;

static const char *TAG = "light frame base";


void fill_scene_update(uint32_t currMillis);
void fill_scene_init();
void fill_scene_update_config(cJSON *json);
void snake_scene_update(uint32_t currMillis);
void snake_scene_init();
void blocks_scene_update(uint32_t currMillis);
void blocks_scene_init();
void blocks_scene_update_config(cJSON *json);
void leds_clear(bool updateLeds);

uint8_t pixelIdx(uint8_t col, uint8_t row)
{
    if (row % 2 == 0) {
        return (row * PIXELS_PER_ROW) + col;
    } else {
        return (row * PIXELS_PER_ROW) + (PIXELS_PER_ROW - col - 1);
    }
}

void setSceneConfig(char *scene, cJSON *json)
{
    if (strncmp(scene, "fill", 4) == 0)
    {
        fill_scene_update_config(json);
    }
    else if (strncmp(scene, "blocks", 6) == 0)
    {
        blocks_scene_update_config(json);
    }
}

void setCurrentScene(char *scene)
{
    if (strncmp(scene, "fill", 4) == 0)
    {
        currentScene = SCENE_FILL;
    }
    else if (strncmp(scene, "snake", 5) == 0)
    {
        currentScene = SCENE_SNAKE;
    }
    else if (strncmp(scene, "blocks", 6) == 0)
    {
        currentScene = SCENE_BLOCKS;
    }
    else
    {
        return;
    }
    
    leds_clear(true);
    currentSceneInit();
    ESP_LOGI(TAG, "New scene: %s", scene); 
}

void currentSceneUpdate(uint32_t millis)
{
    switch (currentScene)
    {
        case SCENE_FILL:
            fill_scene_update(millis);
            break;
        case SCENE_SNAKE:
            snake_scene_update(millis);
            break;
        case SCENE_BLOCKS:
            blocks_scene_update(millis);
            break;
    }
}

void currentSceneInit()
{
    switch (currentScene)
    {
        case SCENE_FILL:
            fill_scene_init();
            break;
        case SCENE_SNAKE:
            snake_scene_init();
            break;
        case SCENE_BLOCKS:
            blocks_scene_init();
            break;
    }
}