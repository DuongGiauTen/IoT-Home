# BTL IoT - Trạm giám sát trung tâm và Điều khiển từ xa (Node 2) cho Yolo UNO

## 1. Giới thiệu

Đây là thành phần mã nguồn thuộc dự án BTL IoT phát triển trên nền tảng ESP32 Yolo UNO sử dụng PlatformIO và framework Arduino. Dự án hiện thực hóa **Node 2 (Phòng điều khiển / Trạm giám sát trung tâm)**, đóng vai trò tiếp nhận dữ liệu viễn trắc từ lò ấp trứng (Node 1) chuyển tiếp qua đám mây, trực quan hóa ra thiết bị hiển thị tại chỗ, đồng thời gửi kịch bản điều khiển ngược lại thiết bị từ xa.

Kiến trúc luồng truyền thông giao tiếp chéo (Cross-Communication Loop):
- **Luồng dữ liệu**: `Node 1 (Telemetry) ---> Cloud (ThingsBoard) ---> Node 2 (RPC Receive)`
- **Luồng lệnh điều khiển**: `Node 2 (Telemetry Command) ---> Cloud (ThingsBoard) ---> Node 1 (RPC Execute)`

## 2. Kiến trúc phần mềm và tính năng chính

### 2.1. Hệ điều hành thực thi
- Vận hành trên nền tảng hệ điều hành thời gian thực **FreeRTOS** nhằm phân bổ tài nguyên phi tập trung dựa trên cơ chế lập lịch ưu tiên độc chiếm (Preemptive Scheduling).
- Cô lập hoàn toàn tiến trình kết nối mạng và hiển thị ngoại vi thành các tác vụ chạy song song để đảm bảo tính thời gian thực và tránh hiện tượng nghẽn luồng xử lý chính.

### 2.2. Các task chính
- `task_wifi.cpp`: Giám sát hạ tầng mạng STA. Nếu mất tín hiệu, tự động khởi tạo mạng Access Point (AP) cứu hộ cấu hình độc lập cho phòng trực.
- `task_webserver.cpp`: Khởi tạo máy chủ Web bất đồng bộ phục vụ Local Dashboard giám sát từ xa và hỗ trợ nâng cấp phần mềm qua ElegantOTA.
- `task_core_iot.cpp`: Đóng vai trò trung tâm xử lý truyền thông chéo. Tiếp nhận bản tin RPC môi trường từ Cloud và xuất bản (Publish) các cờ hiệu lệnh điều khiển ngược lại hệ thống.
- `lcd_display.cpp`: Quản lý hiển thị thông số nhiệt độ, độ ẩm nhận được từ Node 1 ra màn hình LCD I2C cục bộ tại phòng trực.
- `temp_humi_monitor.cpp`: Đánh giá các ngưỡng cảnh báo từ dữ liệu nhận được để đưa ra trạng thái hệ thống (Bình thường, Cảnh báo, Nguy hiểm).
- `led_blinky.cpp` / `neo_blinky.cpp`: Thực thi chớp LED đơn và dải LED NeoPixel cảnh báo trực quan tại phòng trực dựa trên trạng thái môi trường lò ấp.

### 2.3. Cấu trúc dữ liệu chung
- Tuân thủ nghiêm ngặt nguyên lý thiết kế **"Zero Global Variables"** (Không biến toàn cục) để bảo vệ an toàn đa luồng.
- Toàn bộ dữ liệu được đóng gói đóng kín trong vùng nhớ ẩn của `shared_data.cpp` và chỉ cho phép tương tác thông qua các hàm Thread-Safe Wrapper (Getter/Setter) được che chắn bởi Mutex (`xSensorDataMutex`, `xConfigMutex`).

### 2.4. Cổng Web và cấu hình
- Tích hợp giao diện Web quản trị tiêu chuẩn được lưu trữ trên hệ thống tệp tin LittleFS nội bộ.
- Giao diện thiết bị tập trung độc quyền vào việc quản lý thiết bị phụ trợ phòng điều khiển (Bật/tắt đèn hành lang GPIO 18, kéo thanh trượt Slider băm xung PWM điều chỉnh dải LED điều hướng), cô lập hoàn toàn để tránh kỹ thuật viên vô tình can thiệp sai lệch luồng sinh học tự động của Node 1.
- Hỗ trợ biểu mẫu cấu hình động lưu thông số WiFi, Token, Server Broker dưới dạng tệp mã hóa JSON lưu vào bộ nhớ Flash.

### 2.5. Kết nối Cloud (ThingsBoard)
- Sử dụng thư viện `ThingsBoard` và `Arduino_MQTT_Client`.
- **Nhận dữ liệu (RPC Callbacks)**: Đăng ký lắng nghe trực tiếp từ Cloud thông qua các hàm `processUpdateTemp` và `processUpdateHumi` để bắt gói tin môi trường từ Node 1 dội về.


## 3. Phần cứng và môi trường

### 3.1. Phần cứng chính
- Board mạch: **ESP32 Yolo UNO**
- Thiết bị hiển thị: **Màn hình LCD I2C** (Hiển thị thông số lò ấp từ xa)
- Thiết bị báo hiệu: **LED NeoPixel** cục bộ (Hiển thị trạng thái cảnh báo phân cấp màu sắc)
- Ngoại vi chấp hành: Hệ thống rơ-le/đèn phụ trợ khu vực phòng trực kết nối qua **GPIO 18** và **GPIO 45**

### 3.2. Môi trường phát triển
- **PlatformIO Embedded Development Environment**
- Cấu hình tệp `platformio.ini`:
  - `platform = espressif32`
  - `board = yolo_uno`
  - `framework = arduino`
  - `monitor_speed = 115200`
  - `board_build.filesystem = littlefs`
- Các thư viện phụ thuộc chính (`lib_deps`):
  - LiquidCrystal_I2C (Hoặc thư viện điều khiển LCD chuyên biệt cho Yolo UNO)
  - Adafruit NeoPixel
  - ThingsBoard
  - PubSubClient
  - ArduinoJson
  - ESPAsyncWebServer

## 4. Cấu trúc thư mục

- `platformio.ini`: Định nghĩa cấu hình biên dịch, tốc độ nạp và quản lý các thư viện phụ thuộc.
- `src/`: Thư mục chứa toàn bộ mã nguồn thực thi chính của trạm giám sát.
  - `main.cpp`: Khởi tạo hệ thống, thiết lập khóa Mutex toàn cục và cấp phát tài nguyên tạo tác vụ FreeRTOS.
  - `shared_data.cpp` / `shared_data.h`: Hiện thực hóa mô hình đóng gói dữ liệu an toàn Thread-Safe.
  - `task_core_iot.cpp`: Lắng nghe sự kiện RPC cập nhật cảm biến từ mây và gửi gói viễn trắc lệnh điều khiển.
  - `task_wifi.cpp`: Thuật toán khôi phục kết nối thông minh và quản lý màng lọc radio AP Fallback.
  - `task_webserver.cpp`: Vận hành máy chủ điều phối WebSocket kết nối Local Dashboard phòng trực.
  - `temp_humi_monitor.cpp`: Đảm nhận việc phân cấp và đánh giá trạng thái an toàn môi trường từ xa.
- `data/`: Chứa mã nguồn giao diện Web (HTML/CSS/JS) nạp trực tiếp vào phân vùng LittleFS của bộ nhớ Flash.

## 5. Hướng dẫn cài đặt và chạy

### 5.1. Build và nạp firmware
1. Mở thư mục mã nguồn Node 2 bằng phần mở rộng PlatformIO trên VS Code.
2. Kết nối bo mạch ESP32 Yolo UNO vào máy tính thông qua cáp USB-C.
3. Tiến hành thực hiện lệnh `PlatformIO: Build` để biên dịch toàn bộ hệ thống thư viện.
4. Chọn `PlatformIO: Upload Filesystem Image` để nạp tài nguyên giao diện Web vào LittleFS trước, sau đó nhấn `PlatformIO: Upload` để nạp mã nguồn chương trình.

### 5.2. Cấu hình kết nối Trạm trung tâm
- Tại lần đầu tiên khởi động, nếu thiết bị chưa được nạp thông tin mạng, bo mạch sẽ tự động cấu hình mạng cứu hộ Access Point nội bộ:
  - SSID: `YOLO_UNO_SETUP` (Hoặc định danh riêng cấu hình cho Node 2)
  - Mật khẩu mặc định: `12345678`
- Sử dụng máy tính hoặc điện thoại kết nối vào điểm truy cập này, truy cập vào địa chỉ Gateway mặc định hiển thị trên Serial Monitor để vào giao diện Cài đặt.
- Nhập thông tin WiFi của trang trại và mã định danh Token, IP Server Broker của hệ thống ThingsBoard Cloud cá nhân để thiết bị ghi tệp `info.dat` vào Flash và tự động tái khởi động áp dụng cấu hình.

### 5.3. Vận hành giám sát
- Khi hệ thống chuyển sang chế độ kết nối STA thành công, màn hình LCD cục bộ tại trạm trực sẽ bắt đầu hiển thị các thông số Nhiệt độ và Độ ẩm thực tế liên tục từ lò ấp trứng dội về qua mây.
- Kỹ thuật viên có thể truy cập IP nội bộ của Node 2 để điều khiển hệ thống chiếu sáng phụ trợ phòng trực hoặc nhấn các nút nhấn chức năng trên giao diện để ra lệnh điều khiển chéo từ xa cho lò ấp.

## 6. Ghi chú quan trọng

- Luồng gửi lệnh điều khiển (`cmd_`) phát ra từ các sự kiện tương tác Web trên Node 2 được thiết kế chạy theo cơ chế rẽ nhánh ưu tiên (Bypass Mechanism). Lệnh sẽ được đóng gói và bắn trực tiếp lên Broker ngay trong chu kỳ lặp mà không cần phải thông qua bộ lọc kiểm định Semaphore của TinyML như luồng dữ liệu môi trường, đảm bảo tốc độ đáp ứng tức thời (Low Latency).
- Hệ thống tệp tin LittleFS yêu cầu phải được nạp đúng cấu trúc dữ liệu trang Web để máy chủ `AsyncWebServer` không gặp lỗi khi Client yêu cầu tải trang.

## 7. Mở rộng và phát triển

- Tích hợp thêm kịch bản đồng bộ cảnh báo còi hú cấu hình qua chân GPIO cục bộ khi nhận được trạng thái nguy hiểm từ lò ấp.
- Phát triển tính năng chuyển đổi giao tiếp dự phòng sang giao thức không dây tầm ngắn trực tiếp (như ESP-NOW) kết nối thẳng với Node 1 khi phát hiện mất mạng Internet diện rộng (Mất kết nối với MQTT Broker trung tâm).

---
*Tài liệu README này mô tả chi tiết kiến trúc phân tán hướng sự kiện và luồng xử lý tương tác chéo của Node 2 trạm giám sát trung tâm thuộc đề tài Smart Egg Incubator.*
