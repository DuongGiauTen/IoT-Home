#include "temp_humi_monitor.h"
#include "shared_data.h"

DHT20 dht20;
// Đã CHUYỂN phần khai báo LCD sang file của Task LCD để code gọn gàng hơn
int counter = 0;
void temp_humi_monitor(void *pvParameters){
    //Wire.begin(11, 12);
    dht20.begin();

    while (1){
        dht20.read();
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();

        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
        } else {
            // 1. Cất dữ liệu an toàn và phất cờ cho Task 1 (LED), Task 2 (Neo)
            set_sensor_data(temperature, humidity);

            // 2. ĐÁNH GIÁ ĐIỀU KIỆN ĐỂ PHẤT CỜ CHO LCD (Yêu cầu của Task 3)
            // Giả sử: 
            // - Bình thường: Nhiệt độ < 28 VÀ Độ ẩm >= 40
            // - Nguy hiểm: Nhiệt độ > 32 HOẶC Độ ẩm < 30
            // - Cảnh báo: Các trường hợp còn lại
            
            if (temperature > 30 ){
                set_state_temp(TEMP_HIGH);
            } else if (temperature < 25){
                set_state_temp(TEMP_LOW);
            } else {
                set_state_temp(TEMP_NORMAL);
            }


            if (humidity < 60){
                set_state_humi(HUMI_LOW);
            } else if (humidity > 80){
                set_state_humi(HUMI_HIGH);
            } else {
                set_state_humi(HUMI_NORMAL);
            }

            // Serial.print("Humidity: ");
            // Serial.print(humidity);
            // Serial.print("%  Temperature: ");
            // Serial.print(temperature);
            // Serial.println("°C");

            // Bọc đoạn in thông số định kỳ
            if (xSemaphoreTake(xSerialMutex, portMAX_DELAY)) {
                Serial.print("Humidity: ");
                Serial.print(humidity);
                Serial.print("%  Temperature: ");
                Serial.print(temperature);
                Serial.println("°C");
                xSemaphoreGive(xSerialMutex); // Nhả khóa
            }

        }

        counter++;
        if (counter >= 15) {
            // Simulate anomaly every 15 readings
            set_sensor_data(100.0, 100.0); // Simulate anomaly
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait a bit before resetting the counter
            counter = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}