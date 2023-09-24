// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Free RTOS includes
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// Sensor includes
#include "dht.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include <time.h>

// OLED includes
#include "ssd1306.h"
#include "font8x8_basic.h"
#include "esp_log.h"


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

// This queue handles communication between sensor tasks and the display task
QueueHandle_t sensor_queue;

// functions declared
int read_temp_sensor(float* temperature, float* humidity);
void read_moisture_sensor(int*  moisture);

#define FLOAT_TO_INT(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

// struct all sensor data
typedef struct {
    float temperature;
    int moisture;
    float humidity;
} sensor_data_t;

void sensor_task(void *pvParamaters) {
    /*
    /This task reads sensor data and adds it to the sensor queue
    */

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
        vTaskDelay(pdMS_TO_TICKS(2000));
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

void display_sensor_data_task(void *pvParameters) {
    /*
    /This task displays sensor data to the OLED display
    */

    // Gets sensor values from the sensor queue and displays them to ssd1306
    SSD1306_t dev;
    sensor_data_t sensor_data;
    char* log_tag = "DISPLAY";

    ESP_LOGI(log_tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
    ESP_LOGI(log_tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
    ESP_LOGI(log_tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

    ESP_LOGI(log_tag, "Panel is 128x64");
	ssd1306_init(&dev, 128, 64);

    char* temperature_string;
    char* humidity_string;
    char* moisture_string;

    char DISPLAY_BLANK_LINE[] = "                \n";

    // grabs sensor data from queue
    while (1) {
        if (xQueueReceive(sensor_queue, &sensor_data, 0) == pdTRUE) {
            // Display sensor data
            ESP_LOGI(log_tag, "Temperature: %.1f%% Moisture: %.1fC Humidity: %d", sensor_data.humidity, sensor_data.temperature, sensor_data.moisture);


            int temp;
            int hum;
            temp = FLOAT_TO_INT(sensor_data.temperature);
            hum = FLOAT_TO_INT(sensor_data.humidity);
            // build strings to display
            if(0 > asprintf(&temperature_string, "Temp:   %dC  \n", temp)) assert("Error creating temperature string sensor_data.temperatures");
            if(0 > asprintf(&moisture_string, "Moisture:   %d\n", sensor_data.moisture)) assert("Error creating moisture string sensor_data.moisture");
            if(0 > asprintf(&humidity_string, "Humidity: %d%%\n", hum)) assert("Error creating humidity string sensor_data.humidity");

            // display strings
            ssd1306_display_text(&dev, 0, temperature_string, strlen(temperature_string), false);
            ssd1306_display_text(&dev, 1, &DISPLAY_BLANK_LINE, strlen(&DISPLAY_BLANK_LINE), false);
            ssd1306_display_text(&dev, 2, moisture_string, strlen(moisture_string), false);
            ssd1306_display_text(&dev, 3, &DISPLAY_BLANK_LINE, strlen(&DISPLAY_BLANK_LINE), false);
            ssd1306_display_text(&dev, 4, humidity_string, strlen(humidity_string), false);
            ssd1306_display_text(&dev, 5, &DISPLAY_BLANK_LINE, strlen(&DISPLAY_BLANK_LINE), false);
            ssd1306_display_text(&dev, 6, &DISPLAY_BLANK_LINE, strlen(&DISPLAY_BLANK_LINE), false);
            ssd1306_display_text(&dev, 7, &DISPLAY_BLANK_LINE, strlen(&DISPLAY_BLANK_LINE), false);

            // Clean up the allocated strings since asprintf uses malloc
            free(temperature_string);
            free(humidity_string);
            free(moisture_string);
            
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main()
{
    // Initialize sensor queue
    sensor_queue = xQueueCreate(10, sizeof(sensor_data_t));

    xTaskCreate(sensor_task, "sensor_task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
    xTaskCreate(display_sensor_data_task, "display_sensor_data_task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}
