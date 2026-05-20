#include "mock.h"
#include "shared_data.h"
void mock_function(void *pvParameters)
{
    int counter = 0;
    int temp = 200;
    int humi = 450;
    while (1)
    {

        temp = temp + 2;
        humi = humi + 5;
        set_sensor_data(temp / 10.0, humi / 10.0);
        // Serial.print("[MOCK] Temperature: ");
        // Serial.print(get_temperature());
        // Serial.print("°C, Humidity: ");
        // Serial.print(get_humidity());
        // Serial.println("%");

        if (temp >350) temp = 200;
        if (humi > 800) humi = 450;

        counter++;
        Serial.print("[MOCK] Counter: ");
        Serial.println(counter);
        if (counter >= 15)
        {
            set_sensor_data(100.0, 100.0); // Simulate anomaly
            vTaskDelay(1000);            // Wait a bit before resetting the counter
            counter = 0;
            // Serial.println("[MOCK] Simulating Anomaly: High Temperature and Humidity!");
        }

        if (get_temperature() > 30)
        {
            set_state_temp(TEMP_HIGH);
        }
        else if (get_temperature() < 25)
        {
            set_state_temp(TEMP_LOW);
        }
        else
        {
            set_state_temp(TEMP_NORMAL);
        }

        if (get_humidity() < 60)
        {
            set_state_humi(HUMI_LOW);
        }
        else if (get_humidity() > 80)
        {
            set_state_humi(HUMI_HIGH);
        }
        else
        {
            set_state_humi(HUMI_NORMAL);
        }
        vTaskDelay(1000);
    }
}