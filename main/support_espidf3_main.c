#include "mjd.h"

/*
 * Logging
 */
static const char *TAG = "support";

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * SENSOR settings
 */
static const int MY_TRIGGER_GPIO_NUM = 23;
// Spinlock for protecting concurrent access
static portMUX_TYPE jsnsr04t_spinlock = portMUX_INITIALIZER_UNLOCKED;

/*
 * TASKS
 */
void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*
     * MAIN
     */
    gpio_config_t io_config;
    io_config.pin_bit_mask = (1ULL << MY_TRIGGER_GPIO_NUM);
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pull_down_en = GPIO_PULLDOWN_ENABLE; // @important
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.intr_type = GPIO_PIN_INTR_DISABLE;
    f_retval = gpio_config(&io_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. gpio_config() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  vTaskDelay(); ...)");
    portENTER_CRITICAL(&jsnsr04t_spinlock);
    gpio_set_level(MY_TRIGGER_GPIO_NUM, 1);
    vTaskDelay(RTOS_DELAY_1SEC);
    gpio_set_level(MY_TRIGGER_GPIO_NUM, 0);
    portEXIT_CRITICAL(&jsnsr04t_spinlock);

    vTaskDelay(RTOS_DELAY_2SEC);

    ESP_LOGI(TAG, "  ets_delay_us(); ...)");
    portENTER_CRITICAL(&jsnsr04t_spinlock);
    gpio_set_level(MY_TRIGGER_GPIO_NUM, 1);
    ets_delay_us(1000 * 1000);
    gpio_set_level(MY_TRIGGER_GPIO_NUM, 0);
    portEXIT_CRITICAL(&jsnsr04t_spinlock);

    /*
     * END
     */
    cleanup:
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
    mjd_rtos_wait_forever();
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer on GPIO#, ...)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /*
     * CREATE TASK:
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    /*
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
