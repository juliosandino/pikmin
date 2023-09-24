// Free RTOS includes
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// Local includes
#include "sensor_manager.h"
#include "display_manager.h"

// This queue handles communication between sensor tasks and the display task
QueueHandle_t sensor_queue;

void app_main()
{
    sensor_queue = xQueueCreate(10, sizeof(sensor_data_t));

    xTaskCreate(sensor_task, "sensor_task", configMINIMAL_STACK_SIZE * 3, sensor_queue, 5, NULL);
    xTaskCreate(display_sensor_data_task, "display_sensor_data_task", configMINIMAL_STACK_SIZE * 3, sensor_queue, 5, NULL);
}
