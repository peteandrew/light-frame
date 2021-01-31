#include <driver/timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <esp_log.h>
#include <cJSON.h>
#include "frame_base.h"

// Timer freq: 80 MHz
//
// isr freq = 1000 Hz
//          = (80000000 / 2) / 40000
//
// Timer divide = 2
// Timer count (alarm value) = 40000

#define TIMER_ALARM_VALUE 40000

volatile uint32_t millis = 0;
volatile bool tick = false;


void wifi_initialise();
void leds_initialise();
void leds_clear();
void fill_scene_update(uint32_t currMillis);
void fill_scene_stop();
void fill_scene_reset_millis(uint32_t currMillis);
void fill_scene_update_config(cJSON *json);
void snake_scene_update(uint32_t currMillis);
void snake_scene_stop();
void snake_scene_reset_millis(uint32_t currMillis);

void tg_timer_isr()
{
    /* Clear the interrupt */
    TIMERG0.int_clr_timers.t1 = 1;
    TIMERG0.hw_timer[1].config.alarm_en = TIMER_ALARM_EN;
    millis++;
    tick = true;
}

static void tg_timer_init()
{
    timer_config_t config;
    config.divider = 2;
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_EN;
    config.intr_type = TIMER_INTR_LEVEL;
    config.auto_reload = true;
    timer_init(TIMER_GROUP_0, TIMER_1, &config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_1, TIMER_ALARM_VALUE);
    timer_enable_intr(TIMER_GROUP_0, TIMER_1);
    timer_isr_register(TIMER_GROUP_0, TIMER_1, tg_timer_isr, NULL, 0, NULL);
    timer_start(TIMER_GROUP_0, TIMER_1);
}

void pause()
{
    timer_pause(TIMER_GROUP_0, TIMER_1);
}

void resume()
{
    timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0);
    timer_start(TIMER_GROUP_0, TIMER_1);
    // fill_scene_reset_millis(millis);
    snake_scene_reset_millis(millis);
}

void stop()
{
    pause();
    leds_clear();
    // fill_scene_stop();
    snake_scene_stop();
}



void setSceneConfig(int scene, cJSON *json)
{
    if (scene == 1) {
        fill_scene_update_config(json);
    }
}

static void leds_task(void *pvParameters) {
    leds_initialise();
    tg_timer_init();

    uint32_t localLastMillis = 0;
    uint32_t seconds = 0;
    uint32_t lastSeconds = 0;
    UBaseType_t uxHighWaterMark;

    // for (uint8_t row=0; row<NUM_ROWS; row++) {
    //     for (uint8_t col=0; col<PIXELS_PER_ROW; col++) {
    //         uint8_t pixel = pixelIdx(col, row);
    //         printf("col: %d, row: %d, pixel: %d\n", col, row, pixel);
    //     }
    // }

    // fill_scene_reset_millis(millis);
    snake_scene_reset_millis(millis);

    printf("LEDs task start\n");

    for (;;) {
        if (tick) {
            if (millis - localLastMillis >= 1000) {
                seconds++;
                printf("%d\n", seconds);
                localLastMillis = millis;
            }

            if (seconds - lastSeconds >= 5) {
                uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
                printf("Stack remaining: %d\n", uxHighWaterMark);
                lastSeconds = seconds;
            }

            // fill_scene_update(millis);
            snake_scene_update(millis);

            tick = false;
        }
    }
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_initialise();

    xTaskCreatePinnedToCore(
        leds_task,
        "leds_task",
        3000, // Stack size in bytes
        NULL, // Task input parameter
        0, // Priority of task
        NULL, // Task handle
        1); // Core
}
