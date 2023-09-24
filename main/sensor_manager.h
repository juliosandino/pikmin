#include <freertos/queue.h>

// struct all sensor data
typedef struct {
    float temperature;
    int moisture;
    float humidity;
} sensor_data_t;

// functions declared
int read_temp_sensor(float* temperature, float* humidity);
void read_moisture_sensor(int*  moisture);
void sensor_task(void *pvParameters);