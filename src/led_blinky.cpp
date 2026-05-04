#include "led_blinky.h"
#include "shared_data.h"
#include "WiFi.h"
void led_blinky(void *pvParameters)
{
    pinMode(LED_GPIO, OUTPUT);
    int blink_delay = 1000; // Mặc định nháy chậm

    while (1)
    {
        // Chờ tín hiệu từ Sensor Task báo có nhiệt độ mới.
        // Nếu sau 10 Tick không có cờ, lệnh này sẽ bỏ qua và tiếp tục giữ nguyên delay cũ để duy trì nhịp nháy.
        if (xSemaphoreTake(xTempUpdateSemaphore, pdMS_TO_TICKS(10)) == pdTRUE)
        {

            // Đã có cờ hiệu! Tiến hành lấy nhiệt độ an toàn
            float current_temp = get_temperature();
            uint8_t temp_state = get_state_temp();                                              // Lấy trạng thái nhiệt độ đã được Task Sensor đánh giá
            Serial.printf("\n>> Da vao mang STA! IP: %s\n", WiFi.localIP().toString().c_str()); // Trace IP

            // Định nghĩa 3 hành vi theo nhiệt độ
            switch (temp_state)
            {
            case TEMP_LOW:
                blink_delay = 1000; // Nhiệt độ thấp -> Nháy chậm
                Serial.println("LED: LOW TEMPERATURE -> SLOW BLINK");
                break;
            case TEMP_NORMAL:
                blink_delay = 300; // Nhiệt độ bình thường -> Nháy trung bình
                // Serial.println("LED: NORMAL TEMPERATURE -> MEDIUM BLINK");
                break;
            case TEMP_HIGH:
                blink_delay = 100; // Nhiệt độ cao -> Nháy nhanh
                Serial.println("LED: HIGH TEMPERATURE -> FAST BLINK");
                break;
            default:
                blink_delay = 1000; // Mặc định nếu có lỗi trạng thái
                Serial.println("LED: UNKNOWN TEMPERATURE STATE -> DEFAULT BLINK");
            }
        }

        // Thực thi việc nháy LED với delay đã được tính toán
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(blink_delay));
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(blink_delay));
    }
}