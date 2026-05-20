#include "lcd_display.h"
#include "shared_data.h"
#include "LiquidCrystal_I2C.h"

void lcd_display_task(void *pvParameters) {
    // Khởi tạo LCD. Lưu ý: 33 trong hệ thập phân chính là địa chỉ 0x21 (Hex)
    LiquidCrystal_I2C lcd(0x27, 16, 2); 
    lcd.init();
    lcd.backlight();
    
    while(1) {
        // Lấy dữ liệu
        float current_temp = get_temperature();
        float current_humi = get_humidity();
        uint8_t temp_state = get_state_temp();
        uint8_t humi_state = get_state_humi();
        bool is_auto = get_auto_mode();  //added
        
        // ==========================================
        // DÒNG 1: HIỂN THỊ GIÁ TRỊ (Ví dụ: T:25.5 H:60.2)
        // ==========================================
        
        // Nhiệt độ (Góc trên bên trái)
        lcd.setCursor(0, 0);
        lcd.print("T:");
        lcd.print(current_temp, 1); // Nên để 1 số thập phân cho gọn trên màn 16x2
        lcd.print(" ");             // Xóa khoảng trắng thừa nếu số bị tụt chữ số
        
        // Độ ẩm (Góc trên bên phải, bắt đầu từ cột 8)
        lcd.setCursor(8, 0);
        lcd.print("H:");
        lcd.print(current_humi, 1);
        lcd.print(" ");
        
        // ==========================================
        // DÒNG 2: HIỂN THỊ TRẠNG THÁI (Ví dụ: T:NORM  H:HIGH)
        // ==========================================
        
        lcd.setCursor(0, 1);
        if (is_auto) {
            lcd.print("MODE:AUTO     "); // Thêm khoảng trắng để xóa chữ thừa cũ
        } else {
            lcd.print("MODE:MANUAL   ");
        }

        // Trạng thái Nhiệt độ (Góc dưới bên trái, tối đa 8 ký tự)
        // lcd.setCursor(0, 1);
        // switch (temp_state) {
        //     case TEMP_LOW:    lcd.print("T:LOW   "); break;
        //     case TEMP_NORMAL: lcd.print("T:NORM  "); break;
        //     case TEMP_HIGH:   lcd.print("T:HIGH  "); break;
        //     default:          lcd.print("T:UNK   "); break;
        // }

        // // Trạng thái Độ ẩm (Góc dưới bên phải, bắt đầu từ cột 8)
        // lcd.setCursor(8, 1);
        // switch (humi_state) {
        //     case HUMI_LOW:    lcd.print("H:LOW   "); break;
        //     case HUMI_NORMAL: lcd.print("H:NORM  "); break;
        //     case HUMI_HIGH:   lcd.print("H:HIGH  "); break;
        //     default:          lcd.print("H:UNK   "); break;
        // }
        
        // Nghỉ ngơi 100ms trước khi cập nhật lại khung hình
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}