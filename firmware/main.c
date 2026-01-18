#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_adc/adc_oneshot.h"

// =======================
// Configuration
// =======================
#define SAMPLE_RATE        75
#define ADC_CHANNEL        ADC_CHANNEL_6   // GPIO 34
#define ADC_UNIT           ADC_UNIT_1
#define ADC_VREF           3.3f
#define ADC_MAX            4095.0f

// Blink detection
#define BLINK_THRESHOLD    1000    // validated by you
#define RESET_THRESHOLD    150

// Morse timing (ms)
#define DOT_MAX_MS         300
#define DASH_MIN_MS        400
#define LOCKOUT_MS         200

// =======================
// Blink FSM
// =======================
typedef enum {
    STATE_IDLE,
    STATE_BLINKING,
    STATE_LOCKOUT
} blink_state_t;

// =======================
// Band-pass Filter
// =======================
float EOGFilter(float input)
{
    float output = input;

    {
        static float z1, z2;
        float x = output - 0.02977423f*z1 - 0.04296318f*z2;
        output = 0.09797471f*x + 0.19594942f*z1 + 0.09797471f*z2;
        z2 = z1;
        z1 = x;
    }
    {
        static float z1, z2;
        float x = output - 0.08383952f*z1 - 0.46067709f*z2;
        output = x + 2.0f*z1 + z2;
        z2 = z1;
        z1 = x;
    }
    {
        static float z1, z2;
        float x = output + 1.92167271f*z1 - 0.92347975f*z2;
        output = x - 2.0f*z1 + z2;
        z2 = z1;
        z1 = x;
    }
    {
        static float z1, z2;
        float x = output + 1.96758891f*z1 - 0.96933514f*z2;
        output = x - 2.0f*z1 + z2;
        z2 = z1;
        z1 = x;
    }

    return output;
}

// =======================
// EOG Task
// =======================
void eog_task(void *arg)
{
    // ADC setup
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT
    };
    adc_oneshot_new_unit(&init_config, &adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12
    };
    adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &chan_cfg);

    int raw;
    float mean = 0;

    int64_t last_time = esp_timer_get_time();
    int64_t timer = 0;
    const int64_t interval = 1000000 / SAMPLE_RATE;

    // Blink FSM variables
    blink_state_t state = STATE_IDLE;
    int64_t blink_start_ms = 0;
    int64_t lockout_start_ms = 0;

    while (1)
    {
        int64_t now = esp_timer_get_time();
        int64_t dt = now - last_time;
        last_time = now;
        timer -= dt;

        if (timer <= 0)
        {
            timer += interval;

            adc_oneshot_read(adc_handle, ADC_CHANNEL, &raw);

            // ADC -> voltage
            float voltage = (raw / ADC_MAX) * ADC_VREF;

            // DC removal
            mean = 0.999f * mean + 0.001f * voltage;
            voltage -= mean;

            // Filter
            float signal = EOGFilter(voltage);
            int sig_i = (int)(signal * 1000);

            int64_t now_ms = esp_timer_get_time() / 1000;

            // =======================
            // Blink FSM
            // =======================
            if (state == STATE_IDLE)
            {
                if (abs(sig_i) > BLINK_THRESHOLD)
                {
                    blink_start_ms = now_ms;
                    state = STATE_BLINKING;
                }
            }
            else if (state == STATE_BLINKING)
            {
                if (abs(sig_i) < RESET_THRESHOLD)
                {
                    int64_t duration = now_ms - blink_start_ms;

                    // DOT or DASH
                    if (duration <= DOT_MAX_MS)
                    {
                        printf(".\n");
                    }
                    else if (duration >= DASH_MIN_MS)
                    {
                        printf("-\n");
                    }

                    lockout_start_ms = now_ms;
                    state = STATE_LOCKOUT;
                }
            }
            else if (state == STATE_LOCKOUT)
            {
                if ((now_ms - lockout_start_ms) > LOCKOUT_MS)
                {
                    state = STATE_IDLE;
                }
            }
        }

        vTaskDelay(1);   // feed watchdog
    }
}

// =======================
// Main
// =======================
void app_main(void)
{
    xTaskCreatePinnedToCore(
        eog_task,
        "eog_task",
        4096,
        NULL,
        5,
        NULL,
        1
    );
}
