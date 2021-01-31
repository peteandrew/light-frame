#include <esp_log.h>
#include "frame_base.h"

static colourConfig baseColourConfig = {
    .hue = 0.01,
    .sat = 1,
    .value = 0.07,
    .hueChange = 0.01,
    .valueChange = 0,
    .maxValue = HSV_MAX_VALUE
};

static const char *TAG = "light frame base";


uint8_t pixelIdx(uint8_t col, uint8_t row)
{
    if (row % 2 == 0) {
        return (row * PIXELS_PER_ROW) + col;
    } else {
        return (row * PIXELS_PER_ROW) + (PIXELS_PER_ROW - col - 1);
    }
}


void updateBaseColour()
{
    baseColourConfig.hue += baseColourConfig.hueChange;
    baseColourConfig.value += baseColourConfig.valueChange;
    if (baseColourConfig.value > baseColourConfig.maxValue) {
        baseColourConfig.value = 0;
    }
}


void setBaseColourConfig(cJSON *json)
{
    const cJSON *hueJson = cJSON_GetObjectItem(json, "hue");
    if (cJSON_IsNumber(hueJson)) {
        baseColourConfig.hue = (float) hueJson->valuedouble;
    }
    const cJSON *satJson = cJSON_GetObjectItem(json, "sat");
    if (cJSON_IsNumber(satJson)) {
        baseColourConfig.sat = (float) satJson->valuedouble;
    }
    const cJSON *valueJson = cJSON_GetObjectItem(json, "value");
    if (cJSON_IsNumber(valueJson)) {
        baseColourConfig.value = (float) valueJson->valuedouble;
    }
    const cJSON *hueChangeJson = cJSON_GetObjectItem(json, "hueChange");
    if (cJSON_IsNumber(hueChangeJson)) {
        baseColourConfig.hueChange = (float) hueChangeJson->valuedouble;
    }
    const cJSON *valueChangeJson = cJSON_GetObjectItem(json, "valueChange");
    if (cJSON_IsNumber(valueChangeJson)) {
        baseColourConfig.valueChange = (float) valueChangeJson->valuedouble;
    }
    const cJSON *maxValueJson = cJSON_GetObjectItem(json, "maxValue");
    if (cJSON_IsNumber(maxValueJson)) {
        baseColourConfig.maxValue = (float) maxValueJson->valuedouble;
        if (baseColourConfig.maxValue > HSV_MAX_VALUE) {
            baseColourConfig.maxValue = HSV_MAX_VALUE;
        }
    }

    ESP_LOGI(TAG, "Colour config: hue = %f, sat = %f, value = %f, hue change = %f, value change = %f, max value = %f", baseColourConfig.hue, baseColourConfig.sat, baseColourConfig.value, baseColourConfig.hueChange, baseColourConfig.valueChange, baseColourConfig.maxValue);
}


colourConfig getBaseColourConfig()
{
    return baseColourConfig;
}