// Sensor includes
#include "dht.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include <time.h>

// Free RTOS includes
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// Log includes
#include "esp_log.h"

// Local includes
#include "sensor_manager.h"

#define SENSOR_TYPE DHT_TYPE_DHT11
#define BUFFER_TIME 5

// TIME PERIODS USED FOR EASY LONG PERIOD BUFFER TIMES
#define HOUR 3600
#define DAY 86400

#define SENSOR_TYPE DHT_TYPE_DHT11
#define MOISTURE_SENSOR_ADC ADC1_CHANNEL_7
#define TEMP_SENSOR_PIN GPIO_NUM_4
#define CONFIG_SDA_GPIO 23
#define CONFIG_SCL_GPIO 22

void sensor_task(void *pvParameters) {
    /*
    /This task reads sensor data and adds it to the sensor queue
    */
    QueueHandle_t sensor_queue = (QueueHandle_t) pvParameters;

    // Configure temperature and humidity sensor
    gpio_set_direction(TEMP_SENSOR_PIN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_pull_mode(TEMP_SENSOR_PIN, GPIO_PULLUP_ONLY);
    // Configure soil moisture sensor
    adc1_config_width(ADC_WIDTH_BIT_10);

    // declare sensor values and set to -1 to indicate no data
    float temperature = -1;
    int moisture = -1;
    float humidity = -1;

    while (1) {
        // Read sensor data
        read_temp_sensor(&temperature, &humidity);
        read_moisture_sensor(&moisture);

        // Add the data to the sensor queue
        sensor_data_t sensor_data = {
            .temperature = temperature,
            .moisture = moisture,
            .humidity = humidity
        };
        xQueueSend(sensor_queue, &sensor_data, 0);

        // Send data to display task
        // xQueueSend(LedControlQueue, &led_state, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int read_temp_sensor(float* temperature, float* humidity) {
    /*
    Reads temperature and humidity from a DHT11 sensor

    param temperature: pointer to float to store temperature
    param humidity: pointer to float to store humidity
    */
    // function_tag
    char* log_tag = "DHT11";

    // Read sensor data
    int ret = dht_read_float_data(SENSOR_TYPE, TEMP_SENSOR_PIN, humidity, temperature);
    if (ret == ESP_OK) {
        ESP_LOGI(log_tag, "Humidity: %.1f%% Temp: %.1fC", *humidity, *temperature);
    } else {
        ESP_LOGI(log_tag, "Could not read data from sensor");
    }

    return ret;
}

void read_moisture_sensor(int* moisture) {
    /*
    Reads moisture from a capacitive soil moisture sensor

    param moisture: pointer to int to store moisture
    */
    // function_tag (Capactive Soil Moisture Sensor v1.2)
    char* log_tag = "CSMSv1.2";

    // Read sensor data
    *moisture = adc1_get_raw(MOISTURE_SENSOR_ADC);
    ESP_LOGI(log_tag, "Moisture Sensor raw value: %d", *moisture);
}