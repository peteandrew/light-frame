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
    currentSceneResetMillis(millis);
}

void stop()
{
    pause();
    leds_clear();
    currentSceneStop();
}

static void leds_task(void *pvParameters) {
    leds_initialise();
    tg_timer_init();

    uint32_t localLastMillis = 0;
    uint32_t seconds = 0;
    uint32_t lastSeconds = 0;
    UBaseType_t uxHighWaterMark;

    currentSceneResetMillis(millis);

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

            currentSceneUpdate(millis);

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
