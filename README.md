# BTL IoT - Hệ thống điều khiển và giám sát nhiệt độ/humidity cho Yolo UNO

## 1. Giới thiệu

Đây là dự án BTL IoT phát triển trên nền tảng ESP32 Yolo UNO với PlatformIO và framework Arduino. Dự án xây dựng một hệ thống thu thập dữ liệu cảm biến nhiệt độ, độ ẩm, hiển thị và điều khiển thiết bị, đồng thời kết nối lên nền tảng IoT Cloud (ThingsBoard) và cung cấp giao diện web quản trị.

Dự án gồm hai node chính:
- **Node1**: node thu dữ liệu và điều khiển. Đây là node đang thực thi trong repository này.
- **Node2**: node nhận dữ liệu, nằm ở phía đầu cuối/kế thừa của hệ thống.

Kiến trúc chung: `Node1 ---> coreIoT_Server ---> Node2`

## 2. Kiến trúc phần mềm và tính năng chính

### 2.1. Hệ điều hành thực thi
- Sử dụng **FreeRTOS** trên ESP32.
- Chia nhiều task riêng biệt để xử lý WiFi, webserver, IOT cloud, hiển thị LCD, cảm biến, actuator, và các hiệu ứng LED.

### 2.2. Các task chính
- `task_wifi.cpp`: quản lý kết nối WiFi, tự động chuyển sang chế độ AP cứu hộ nếu không thể truy cập mạng.
- `task_webserver.cpp`: chạy web server với **WebSocket** và **ElegantOTA**.
- `task_core_iot.cpp`: kết nối tới ThingsBoard qua MQTT, gửi dữ liệu telemetry và nhận lệnh RPC từ Cloud.
- `tinyml.cpp`: nhiệm vụ TinyML (nếu có bổ sung) liên quan đến điều kiện gửi dữ liệu đáng tin cậy.
- `lcd_display.cpp`: hiển thị thông tin lên màn hình LCD.
- `task_actuator.cpp`: điều khiển actuator như servo và PWM cho đèn sưởi.
- `led_blinky.cpp` / `neo_blinky.cpp`: hiệu ứng LED báo trạng thái.
- `temp_humi_monitor.cpp`: đọc dữ liệu nhiệt độ và độ ẩm từ cảm biến DHT20.

### 2.3. Cấu trúc dữ liệu chung
- Dùng `shared_data.h` và `shared_data.cpp` để lưu trữ cấu hình, dữ liệu cảm biến và trạng thái điều khiển.
- Sử dụng **Semaphore / Mutex** để đồng bộ giữa các task và tránh tranh chấp khi truy cập dữ liệu chung.

### 2.4. Cổng Web và cấu hình
- Web server phục vụ giao diện trên `index.html`, `script.js`, `styles.css` cùng thư viện `raphael.min.js` và `justgage.js`.
- Hỗ trợ lưu cấu hình WiFi, device ID, token, server và port qua giao diện web.
- Điều chỉnh độ sáng đèn sười, động cơ để đảo trứng
- Hỗ trợ cập nhật firmware OTA bằng **ElegantOTA**.

### 2.5. Kết nối Cloud (ThingsBoard)
- Sử dụng `ThingsBoard` library và `Arduino_MQTT_Client`.
- Gửi telemetry:
  - `temperature`
  - `humidity`
  - `heater_pwm`
  - `servo_angle`


## 3. Phần cứng và môi trường

### 3.1. Phần cứng chính
- Board: **ESP32 Yolo UNO**
- Sensor: **DHT20** (nhiệt độ/độ ẩm)
- Servo và thiết bị điều khiển giả lập
- LED NeoPixel và LCD hiển thị
- WiFi nội bộ hoặc AP cứu hộ

### 3.2. Môi trường phát triển
- **PlatformIO**
- `platformio.ini` cấu hình:
  - `platform = espressif32`
  - `board = yolo_uno`
  - `framework = arduino`
  - `monitor_speed = 115200`
  - `board_build.filesystem = littlefs`
- Thư viện phụ thuộc được khai báo trong `lib_deps`:
  - TensorFlowLite_ESP32
  - Adafruit NeoPixel
  - DHT20
  - LCD
  - PubSubClient
  - Keypad
  - ESP32Servo
  - ESPAsyncWebServer

## 4. Cấu trúc thư mục

- `platformio.ini`: cấu hình build và thư viện.
- `src/`: mã nguồn chính của dự án.
  - `main.cpp`: khởi tạo task và cấu hình hệ thống.
  - `coreiot.cpp`, `task_core_iot.cpp`: xử lý kết nối Cloud.
  - `task_wifi.cpp`: quản lý WiFi và AP fallback.
  - `task_webserver.cpp`: webserver, WebSocket và OTA.
  - `shared_data.cpp`, `shared_data.h`: dữ liệu dùng chung và đồng bộ.
  - `temp_humi_monitor.cpp`: đọc cảm biến nhiệt độ/độ ẩm.
  - `task_actuator.cpp`: điều khiển thiết bị.
- `include/`: header, cấu hình, và định nghĩa interface.
- `data/`: tài nguyên web (HTML/CSS/JS) được nạp lên LittleFS.
- `lib/`: các thư viện bên thứ ba và custom.
- `boards/`: định nghĩa board Yolo UNO nếu cần.

## 5. Hướng dẫn cài đặt và chạy

### 5.1. Build và nạp firmware
1. Mở dự án trong PlatformIO.
2. Kết nối thiết bị ESP32.
3. Chọn môi trường `env:yolo_uno`.
4. Chạy lệnh `PlatformIO: Upload`.

### 5.2. Cấu hình WiFi và Cloud
- Sau khi bật thiết bị, nếu không có cấu hình WiFi hợp lệ thì board sẽ vào **chế độ Access Point** với:
  - SSID: `YOLO_UNO_SETUP_NODE1`
  - Password: `12345678`
- Kết nối vào AP này và mở trình duyệt để truy cập giao diện cấu hình.
- Nhập:
  - SSID/WiFi password
  - Device ID
  - IoT Token
  - Server URL
  - Port
- Thiết bị sẽ lưu cấu hình và khởi động lại để áp dụng.

### 5.3. Truy cập web control
- Khi WiFi hoạt động, mở trình duyệt đến IP do ESP32 cấp phát hoặc server cung cấp.
- Web app cho phép:
  - Xem trạng thái cảm biến.
  - Điều khiển bật tắt GPIO.
  - Điều chỉnh độ sáng NeoPixel.
  - Cập nhật cấu hình và OTA.

## 6. Ghi chú quan trọng

- `main.cpp` dùng `xSemaphoreCreateMutex()` để bảo vệ in Serial và dữ liệu chung.
- `task_core_iot` chỉ gửi dữ liệu lên Cloud khi `xDataReliableSemaphore` cho phép.
- `task_wifi` thiết kế sao cho khi AP cứu hộ đang bật thì không tiếp tục dò WiFi, giảm xung đột.
- Toàn bộ dữ liệu cấu hình và trạng thái được bảo vệ bởi `xConfigMutex` và các semaphore.

## 7. Mở rộng và phát triển

- Thêm `task_keypad.cpp` nếu cần điều khiển trực tiếp từ bàn phím ma trận.
- Hoàn thiện `task_rs485.cpp` nếu cần giao tiếp RS485.
- Mở rộng `tinyml.cpp` để xử lý dự đoán hoặc cảnh báo tự động.
- Tích hợp thêm dashboard trên ThingsBoard cho phép hiển thị biểu đồ và từ xa điều khiển.

---

*README này mô tả đầy đủ tính năng, kiến trúc và cách chạy dự án BTL IoT trên ESP32 Yolo UNO.*
