#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "weather.h"

#define TAG "NVS"




void app_main()
{

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(nvs_flash_init_partition("MyNvs"));

    nvs_handle_t handle;
    ESP_ERROR_CHECK(nvs_open_from_partition("MyNvs", "weather_store", NVS_READWRITE, &handle));

    char weatherKey[20];
    Weather weather;
    size_t weatherSize = sizeof(Weather);

    for (int i = 0; i < 5; i++)
    {
        sprintf(weatherKey, "weather_%d", i);
        esp_err_t result = nvs_get_blob(handle, weatherKey, (void *) &weather, &weatherSize);
        switch(result)
        {
            case ESP_ERR_NOT_FOUND:
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGE(TAG, "Value not set yet");
                break;
            case ESP_OK:
                ESP_LOGI(TAG, "Weather temperature: %.2f, humidity: %.2f, id: %d\n", weather.temp, weather.humidity, weather.id);
                break;
            default:
                ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(result));
                break;
        }
    }

    for (int i = 0; i < 5; i++)
    {
        sprintf(weatherKey, "weather_%d", i);
        Weather newWeather;

        newWeather.temp = 28.9;
        newWeather.humidity = 89.1;
        newWeather.id = i;

        ESP_ERROR_CHECK(nvs_set_blob(handle, weatherKey, (void *) &newWeather, weatherSize));
        ESP_ERROR_CHECK(nvs_commit(handle));        
    }

    nvs_close(handle);
}