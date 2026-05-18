#include "global.h"
#include "shared_data.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "task_webserver.h"
// #include "mainserver.h"
//  #include "tinyml.h"
#include "coreiot.h"

// include task
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task_core_iot.h"
#include "lcd_display.h"
#include "tinyml.h"
#include "mock.h"
#include "task_keypad.h"
#include "task_actuator.h"
void setup()
{
  delay(3000);

  Serial.begin(115200);
  Serial.println("\n--- ESP32 ĐANG KHỞI ĐỘNG ---");

  Wire.begin(11, 12);

  // Tạo Mutex trước khi tạo Task
  xSerialMutex = xSemaphoreCreateMutex();

  init_shared_data();

  Serial.println(">> Dang kiem tra File System va WiFi...");
  check_info_File(0);

  // Task độc lập chỉ chuyên lo việc theo dõi và kết nối lại Wi-Fi
  xTaskCreate([](void *pvParameters)
              {
      while(1) {
          Wifi_reconnect();
          vTaskDelay(pdMS_TO_TICKS(1000));
      } }, "Task WiFi Monitor", 8192, NULL, 2, NULL);

  xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  //xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
  //xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, NULL, 2, NULL);

  xTaskCreate(mock_function, "Task Mock", 2048, NULL, 2, NULL);
 
  xTaskCreate(task_webserver, "Task Web", 8192, NULL, 3, NULL);
  // xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
  xTaskCreate(tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);

  xTaskCreate(lcd_display_task, "Task LCD", 2048, NULL, 2, NULL); // THÊM DÒNG NÀY
  // xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
   //xTaskCreate(task_keypad, "Task Keypad", 2048, NULL, 2, NULL);
  xTaskCreate(taskActuator_Execution, "Task Actuator", 2048, NULL, 2, NULL);

  Serial.println(">> Dang kiem tra File System...");
  check_info_File(0); // chuyển tạm xuống dưới cùng
}

void loop()
{
  // Wifi_reconnect();

  // vTaskDelay(pdMS_TO_TICKS(1000));
  vTaskDelete(NULL);
}