#include "task_webserver.h"
#include "shared_data.h"
#include "task_check_info.h" // để dùng save info file

// BỔ SUNG 2 BIẾN NÀY ĐỂ TRÌ HOÃN RESTART
static bool need_restart = false;
static unsigned long restart_time = 0;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool webserver_isrunning = false;

void Webserver_sendata(String data)
{
    if (ws.count() > 0)
    {
        ws.textAll(data); // Gửi đến tất cả client đang kết nối
    };
    // else
    // {
    //     Serial.println("Không có client WebSocket nào đang kết nối!");
    // }
}

// --- HÀM XỬ LÝ DỮ LIỆU TỪ WEB GỬI XUỐNG ---
void handleWebSocketMessage(String message)
{
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    if (error)
        return;

    String page = doc["page"] | "";

    // NẾU LÀ GÓI TIN CÀI ĐẶT
    if (page == "setting")
    {
        JsonObject value = doc["value"];
        String ssid = value["ssid"] | "";
        String pass = value["password"] | "";
        String dev_id = value["deviceId"] | "YOLO_01"; // <--- Hứng Device ID
        String token = value["token"] | "";            // <--- Hứng Token
        String server_url = value["server"] | "";
        String port = value["port"] | "";

        Serial.println(">> Nhan cau hinh moi tu Web. Dang luu...");
        // Gọi hàm lưu với đủ 6 tham số
        Save_info_File(ssid, pass, dev_id, token, server_url, port);

        // Sau khi lưu xong, gửi phản hồi về Web và khởi động lại để áp dụng cấu hình mới
        Serial.println(">> Da luu xong! Dang khoi dong lai de ap dung mang moi...");
        vTaskDelay(pdMS_TO_TICKS(1000)); // Đợi 1 giây để chữ "Thành công" kịp bay về Web
        // BẬT CỜ BÁO HIỆU ĐÒI RESTART SAU 1.5 GIÂY (Không restart ngay lập tức)
        need_restart = true;
        restart_time = millis();
    }

    // NẾU LÀ GÓI TIN ĐIỀU KHIỂN THIẾT BỊ
    else if (page == "device_control")
    {
        String type = doc["type"] | "";
        int value = doc["value"] | 0;

        if (type == "mode")
        {
            set_auto_mode(value == 1); // 1: Auto, 0: Manual
            if (xSemaphoreTake(xSerialMutex, portMAX_DELAY)) {
                Serial.printf(">> [WEB] Chuyen che do: %s\n", value == 1 ? "AUTO" : "MANUAL");
                xSemaphoreGive(xSerialMutex);
            }
        }
        else if (type == "neo_pwm")
        {
            set_heater_pwm(value);
            if (xSemaphoreTake(xSerialMutex, portMAX_DELAY)) {
                Serial.printf(">> [WEB] Chinh do sang NeoPixel (Heater): %d\n", value);
                xSemaphoreGive(xSerialMutex);
            }
        }
        else if (type == "servo_angle")
        {
            set_servo_angle(value);
            if (xSemaphoreTake(xSerialMutex, portMAX_DELAY)) {
                Serial.printf(">> [WEB] Chinh goc Servo: %d do\n", value);
                xSemaphoreGive(xSerialMutex);
            }
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;

        if (info->opcode == WS_TEXT)
        {
            String message;
            message += String((char *)data).substring(0, len);
            // parseJson(message, true);
            handleWebSocketMessage(message);
        }
    }
}

void connnectWSV()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "application/javascript"); });
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles.css", "text/css"); });

    server.on("/raphael.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/raphael.min.js", "application/javascript"); });
    server.on("/justgage.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/justgage.js", "application/javascript"); });

    server.begin();
    ElegantOTA.begin(&server);
    webserver_isrunning = true;
    Serial.println(">> Web Server Da Khoi Chay!");
}

void Webserver_stop()
{
    if (webserver_isrunning)
    {
        ws.closeAll();
        server.end();
        webserver_isrunning = false;
        Serial.println(">> Web Server Da Dung!");
    }
}

void Webserver_reconnect()
{
    if (!webserver_isrunning)
    {
        connnectWSV();
    }
    ElegantOTA.loop();
}

// === TASK CHÍNH CỦA WEBSERVER ĐÃ ĐƯỢC LÀM GỌN LẠI ===
// void task_webserver(void *pvParameters) {
//     Serial.println(">> Web Server Task Started! Cho mang 1 giay...");

//     //khởi tạo đèn led
//     pinMode(18, OUTPUT);
//     digitalWrite(18, LOW);

//     // KHOÁ CHẶT Ở ĐÂY: Cho phép cả chế độ WIFI_AP_STA (Smart Mode) đi qua
//     while (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) {
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }

//     Serial.println(">> Mang da san sang! Tien hanh mo cong Web Server...");
//     Serial.println(">> Web Server Task Started!");

//     unsigned long last_send = 0;

//     while (1) {
//         // 1. Luôn gọi reconnect để đảm bảo trạng thái Web và OTA hoạt động
//         Webserver_reconnect();

//         // 2. Bơm dữ liệu cảm biến lên Web mỗi 2 giây (CHỈ KHI WEB ĐANG CHẠY)
//         if (webserver_isrunning && (millis() - last_send > 2000)) {
//             last_send = millis();

//             // LẤY DỮ LIỆU AN TOÀN QUA MUTEX
//             float t = get_temperature();
//             float h = get_humidity();

//             if (t != -1 && h != -1) {
//                 String json = "{\"temperature\":" + String(t, 1) + ",\"humidity\":" + String(h, 1) + "}";
//                 Webserver_sendata(json);
//             }
//         }

//         vTaskDelay(pdMS_TO_TICKS(50)); // Delay nhỏ để nhường CPU
//     }
// }

// === TASK CHÍNH CỦA WEBSERVER ĐÃ ĐƯỢC LÀM GỌN LẠI ===
void task_webserver(void *pvParameters)
{
    Serial.println(">> Web Server Task Started! Cho mang 1 giay...");

    

    // KHOÁ CHẶT Ở ĐÂY: Cho phép cả chế độ WIFI_AP_STA (Smart Mode) đi qua
    while (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    Serial.println(">> Mang da san sang! Tien hanh mo cong Web Server & Task Started!");

    unsigned long last_send = 0;
    unsigned long last_send_info = 0; // BỔ SUNG: Biến đếm nhịp cho Tab Thông tin

    while (1)
    {
        // === KHỐI ĐẾM NGƯỢC RESTART ===
        if (need_restart && (millis() - restart_time > 1500))
        {
            Serial.println("\n>> Da luu xong! Khoi dong lai ESP32 de ap dung mang...");
            ESP.restart();
        }

        // 1. Luôn gọi reconnect để đảm bảo trạng thái Web và OTA hoạt động
        Webserver_reconnect();

        // 2. Bơm dữ liệu cảm biến lên Web mỗi 2 giây (CHỈ KHI WEB ĐANG CHẠY)
        if (webserver_isrunning && (millis() - last_send > 2000))
        {
            last_send = millis();

            // LẤY DỮ LIỆU AN TOÀN QUA MUTEX
            float t = get_temperature();
            float h = get_humidity();

            if (t != -1 && h != -1)
            {
                DynamicJsonDocument doc(128); // Tạo document JSON nhỏ
                doc["temperature"] = t;       // ArduinoJson tự động xử lý float
                doc["humidity"] = h;

                String json;
                serializeJson(doc, json); // Chuyển thành chuỗi JSON
                Webserver_sendata(json);
            }
        }

        // 3. BỔ SUNG: Bơm Thông tin hệ thống lên Web mỗi 5 giây
        if (webserver_isrunning && (millis() - last_send_info > 5000))
        {
            last_send_info = millis();

            DynamicJsonDocument doc(512);
            doc["page"] = "info";
            doc["ssid"] = WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "AP Mode (YOLO_UNO_SETUP)";
            doc["ip"] = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
            doc["mac"] = WiFi.macAddress();
            doc["ram"] = ESP.getFreeHeap() / 1024; // Tính bằng KB
            doc["uptime"] = millis() / 1000;       // Tính bằng Giây

            String infoJson;
            serializeJson(doc, infoJson);
            Webserver_sendata(infoJson);
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Delay nhỏ để nhường CPU
    }
}