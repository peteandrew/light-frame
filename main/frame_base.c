#include <esp_log.h>
#include <string.h>
#include "frame_base.h"

static scene currentScene = SCENE_FILL;

static const char *TAG = "light frame base";


void fill_scene_update(uint32_t currMillis);
void fill_scene_stop();
void fill_scene_reset_millis(uint32_t currMillis);
void fill_scene_update_config(cJSON *json);
void snake_scene_update(uint32_t currMillis);
void snake_scene_stop();
void snake_scene_reset_millis(uint32_t currMillis);
void leds_clear();

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
}

void setCurrentScene(char *newScene)
{
    if (strncmp(newScene, "fill", 4) == 0)
    {
        currentScene = SCENE_FILL;
    }
    else if (strncmp(newScene, "snake", 5) == 0)
    {
        currentScene = SCENE_SNAKE;
    }
    else
    {
        return;
    }
    
    leds_clear();
    currentSceneResetMillis(0);
    ESP_LOGI(TAG, "New scene: %s", newScene); 
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
    }
}

void currentSceneResetMillis(uint32_t millis)
{
    switch (currentScene)
    {
        case SCENE_FILL:
            fill_scene_reset_millis(millis);
            break;
        case SCENE_SNAKE:
            snake_scene_reset_millis(millis);
            break;
    }
}

void currentSceneStop()
{
    switch (currentScene)
    {
        case SCENE_FILL:
            fill_scene_stop();
            break;
        case SCENE_SNAKE:
            snake_scene_stop();
            break;
    }
}