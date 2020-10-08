#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "weather.h"
#include "connect.h"
#include "esp_wifi.h"

#define TAG "NVS_WIFI"

TaskHandle_t taskHandle;

const uint32_t WIFI_ON = BIT1;
const uint32_t WIFI_OFF = BIT2;

void nvs_wifi_task(void *params)
{
    uint32_t command = 0;
    
    nvs_handle_t handle;

    char weatherKey[20];
    Weather weather;
    size_t weatherSize = sizeof(Weather);

    while (true)
    {
        xTaskNotifyWait(0, 0, &command, portMAX_DELAY);
        
        switch (command)
        {
            case WIFI_ON:
                ESP_ERROR_CHECK(nvs_flash_init_partition("MyNvs"));

                ESP_ERROR_CHECK(nvs_open_from_partition("MyNvs", "weather_store", NVS_READWRITE, &handle));


                
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
                
                nvs_close(handle);

                ESP_LOGI(TAG, "WIFI TA ON!\n");
                vTaskDelay(1000 / portTICK_PERIOD_MS);

            break;
            case WIFI_OFF:
                ESP_ERROR_CHECK(nvs_flash_init_partition("MyNvs"));

                ESP_ERROR_CHECK(nvs_open_from_partition("MyNvs", "weather_store", NVS_READWRITE, &handle));


                for (int i = 0; i < 5; i++)
                {
                    sprintf(weatherKey, "weather_%d", i);
                    Weather newWeather;

                    newWeather.temp = 28.9 + i;
                    newWeather.humidity = 89.1 + i;
                    newWeather.id = i;

                    ESP_ERROR_CHECK(nvs_set_blob(handle, weatherKey, (void *) &newWeather, weatherSize));
                    ESP_ERROR_CHECK(nvs_commit(handle));        
                }

                nvs_close(handle);

                ESP_LOGI(TAG, "WIFI TA OFF!\n");
                vTaskDelay(1000 / portTICK_PERIOD_MS);

            break;                
            default:
            break;
        }
    }
}

void app_main()
{
    wifiInit();
    ESP_ERROR_CHECK(esp_wifi_start());

    xTaskCreate(nvs_wifi_task, "nvs_wifi_task", 1024 * 5, NULL, 5, &taskHandle);

}