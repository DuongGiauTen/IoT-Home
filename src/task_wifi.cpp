#include "task_wifi.h"
#include "shared_data.h"
#include "task_check_info.h"

// Biến cờ để theo dõi trạng thái mạng
static unsigned long last_wifi_check = 0;
static bool is_ap_fallback_active = false; //neu bang true thi mach dang mat wifi, phat AP cuu ho, va khong can check wifi nua

void startAP()
{
    Serial.println("\n[WIFI] Khong co data -> Dang khoi tao AP Mode...");
    WiFi.mode(WIFI_AP);
    bool success = WiFi.softAP("YOLO_UNO_SETUP", "12345678"); 
    
    if (success) {
        Serial.print("[WIFI] Phat mang THANH CONG! AP IP: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("[WIFI] LOI: Khong the phat mang AP!");
    }
}

void startSTA()
{
    String ssid = "";
    String pass = "";
    
    // Lấy thông tin từ két sắt
    if (xSemaphoreTake(xConfigMutex, portMAX_DELAY)) {
        ssid = String(sysConfig.wifi_ssid);
        pass = String(sysConfig.wifi_pass);
        xSemaphoreGive(xConfigMutex);
    }

    if (ssid.isEmpty()) {
        startAP();
        return;
    }

    Serial.print(">> Dang ket noi den WiFi: ");
    Serial.println(ssid);

    // Chỉ set chế độ STA nếu không đang phát AP cứu hộ
    if (!is_ap_fallback_active) {
        WiFi.mode(WIFI_STA); 
        WiFi.disconnect();
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (pass.isEmpty()) {
        WiFi.begin(ssid.c_str());
    } else {
        WiFi.begin(ssid.c_str(), pass.c_str());
    }

    // === RÚT NGẮN THỜI GIAN QUÉT ===
    int max_wait = is_ap_fallback_active ? 50 : 150; 
    int timeout_counter = 0;
    while (WiFi.status() != WL_CONNECTED && timeout_counter < max_wait)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        timeout_counter++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n>> Da vao mang STA! IP: %s\n", WiFi.localIP().toString().c_str());// Trace IP
        xSemaphoreGive(xBinarySemaphoreInternet);
        
        // Nếu AP đang bật mà bắt được mạng nhà -> Tắt AP đi
        if (is_ap_fallback_active) {
            WiFi.mode(WIFI_STA);
            is_ap_fallback_active = false;
        }
    } else {
        // Chỉ gọi lệnh khởi tạo AP nếu nó CHƯA được bật
        if (!is_ap_fallback_active) {
            Serial.println(">> [SMART MODE] Khong the vao mang! Tu dong phat AP de cuu ho...");

            // ===  CHUYỂN HẲN SANG WIFI_AP VÀ ÉP KÊNH 1 ===
            WiFi.mode(WIFI_AP); 
            WiFi.softAP("YOLO_UNO_SETUP_SERVER", "12345678", 1);
            
            Serial.print(">> AP IP de cuu ho: ");
            Serial.println(WiFi.softAPIP());
            is_ap_fallback_active = true;
        } else {
            Serial.println(">> [SMART MODE] Dang o che do AP. Cho nguoi dung cai dat...");
        }
    }
}

bool Wifi_reconnect()
{
    // === NẾU ĐANG PHÁT AP CỨU HỘ THÌ KHÓA CHẾT, KHÔNG ĐI DÒ MẠNG ===
    // Việc này giúp Laptop quét mạng cực nhanh và kết nối không bao giờ rớt
    if (is_ap_fallback_active) {
        return false; 
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        return true;
    }

    // Nếu rớt mạng và CHƯA bật AP, thì mới gọi startSTA() để xử lý
    // Rút ngắn thời gian kiểm tra rớt mạng xuống 30 giây cho nhạy
    if (millis() - last_wifi_check > 30000 || last_wifi_check == 0) {
        last_wifi_check = millis();
        startSTA(); 
    }
    return false;
}