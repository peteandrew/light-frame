#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "frame_base.h"

#include "esp32_digital_led_lib.h"

#define LED_GPIO 16


static strand_t STRANDS[] = {
    {
        .rmtChannel = 0,
        .ledType = LED_WS2812B_V1,
        .brightLimit = 22,
        .pixels = 0,
        ._stateVars = 0,
    }
};

static float my_fmod(float arg1, float arg2)
{
    int full = (int)(arg1/arg2);
    return arg1 - full*arg2;
}

static pixelColor_t pixel_from_hsv(float hue, float sat, float value)
{
    float pr,  pg, pb, avg;    pr=pg=pb=avg=0;
    short ora, og, ob;         ora=og=ob=0;

    float ro = my_fmod( hue * 6, 6. );
    ro = my_fmod( ro + 6 + 1, 6 ); //Hue was 60* off...

    //yellow->red
    if     ( ro < 1 ) { pr = 1;         pg = 1. - ro; }
    else if( ro < 2 ) { pr = 1;         pb = ro - 1.; }
    else if( ro < 3 ) { pr = 3. - ro;   pb = 1;       }
    else if( ro < 4 ) { pb = 1;         pg = ro - 3;  }
    else if( ro < 5 ) { pb = 5  - ro;   pg = 1;       }
    else              { pg = 1;         pr = ro - 5;  }

    //Actually, above math is backwards, oops!
    pr *= value;   pg *= value;   pb *= value;
    avg += pr;     avg += pg;     avg += pb;

    pr = pr * sat + avg * (1.-sat);
    pg = pg * sat + avg * (1.-sat);
    pb = pb * sat + avg * (1.-sat);

    ora = pr*255;  og = pb*255;   ob = pg*255;

    if( ora < 0 ) ora = 0;
    if( ora > 255 ) ora = 255;
    if( og  < 0 ) og = 0;
    if( og  > 255 ) og  = 255;
    if( ob  < 0 ) ob = 0;
    if( ob > 255 )  ob  = 255;

    pixelColor_t v;
    v.r = ora;
    v.g = og;
    v.b = ob;
    v.w = 0;
    return v;
}

void leds_initialise()
{
    STRANDS[0].gpioNum = LED_GPIO;
    STRANDS[0].numPixels = NUM_PIXELS;

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<16);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    gpio_set_level(16, 0);

    if (digitalLeds_initStrands(STRANDS, 1)) {
        printf("Init FAILURE: halting\n");
        while (true) {};
    }
    strand_t * pStrand = &STRANDS[0];
    digitalLeds_resetPixels(pStrand);
}

void leds_clear()
{
    strand_t * pStrand = &STRANDS[0];
    digitalLeds_resetPixels(pStrand);    
}

void leds_set_pixel(int pixel, float hue, float sat, float value)
{
    strand_t * strand = &STRANDS[0];
    pixelColor_t colour = pixel_from_hsv(hue, sat, value);
    strand->pixels[pixel] = colour;
}

void leds_update()
{
    strand_t * strand = &STRANDS[0];
    digitalLeds_updatePixels(strand);
}

