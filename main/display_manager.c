// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Free RTOS includes
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// OLED includes
#include "ssd1306.h"
#include "font8x8_basic.h"
#include "esp_log.h"

// Local includes
#include "sensor_manager.h"

#define FLOAT_TO_INT(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

void display_sensor_data_task(void *pvParameters) {
    /*
    /This task displays sensor data to the OLED display
    */

    // Gets sensor values from the sensor queue and displays them to ssd1306
    SSD1306_t dev;
    sensor_data_t sensor_data;
    char* log_tag = "DISPLAY";
    QueueHandle_t sensor_queue = (QueueHandle_t) pvParameters;

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
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}