#include "tinyml.h"
#include "shared_data.h"


namespace {
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    constexpr int kTensorArenaSize = 8 * 1024;
    uint8_t tensor_arena[kTensorArenaSize];
}

void setupTinyML() {
    Serial.println("TensorFlow Lite Init....");
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(dht_anomaly_model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report("Lỗi phiên bản mô hình!");
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        error_reporter->Report("AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);
    Serial.println("TensorFlow Lite Micro initialized on ESP32.");
}

void tiny_ml_task(void *pvParameters) {
    setupTinyML();

    // --- CÁC BIẾN CHO BỘ ĐỆM VÒNG (TIME-SERIES) ---
    float temp_history[5] = {0};
    float humi_history[5] = {0};
    int history_index = 0;
    bool is_first_run = true;

    while (1) {
        // 1. Lấy dữ liệu thực tế
        float current_temp = get_temperature();
        float current_humi = get_humidity();

        // 2. Xử lý lưu trữ vào bộ đệm
        if (is_first_run) {
            // Lần chạy đầu tiên: Copy số liệu hiện tại cho cả 5 ô
            vTaskDelay(pdMS_TO_TICKS(2000)); 
            for (int i = 0; i < 5; i++) {
                temp_history[i] = current_temp;
                humi_history[i] = current_humi;
            }
            is_first_run = false;
        } else {
            // Các lần chạy sau: Ghi đè vào vị trí cũ nhất
            temp_history[history_index] = current_temp;
            humi_history[history_index] = current_humi;
            history_index = (history_index + 1) % 5; // Tăng index, quay vòng về 0 nếu đạt 5
        }

        // 3. Nạp 10 dữ liệu vào AI (Từ cũ nhất đến mới nhất)
        for (int i = 0; i < 5; i++) {
            int idx = (history_index + i) % 5; 
            input->data.f[i*2]     = temp_history[idx];
            input->data.f[i*2 + 1] = humi_history[idx];
        }

        // 4. Chạy suy luận (Inference)
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
            Serial.println("AI Invoke failed!");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        // 5. Đọc kết quả (0: Tin cậy, 1: Lỗi/Rác)
        float prob_reliable = output->data.f[0];   
        float prob_error = output->data.f[1]; 

        Serial.print("[TinyML] T_Hiện_tại: "); Serial.print(current_temp);
        Serial.print("C | H_Hiện_tại: "); Serial.print(current_humi);
        Serial.print("% ===> AI KẾT LUẬN: ");

        if (prob_reliable > prob_error) {
            Serial.println("DỮ LIỆU TIN CẬY (Cho phép gửi Cloud)");
            // Phất cờ cho phép Task CoreIOT lấy dữ liệu gửi đi
            xSemaphoreGive(xDataReliableSemaphore); 
        } else {
            Serial.println("RÁC / NHIỄU CẢM BIẾN (Chặn, không gửi Cloud)");
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // AI chạy mỗi 2 giây
    }
}