#include "task_actuator.h"
#include "shared_data.h"
#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>

#define NEO_PIN 45
#define LED_COUNT 1

#define TEMP_CRITICAL_LOW 25
#define TEMP_TARGET_MIN 30
#define TEMP_TARGET_MAX 35

Servo eggServo;
Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

void taskActuator_Execution(void *pvParameters)
{

    strip.begin();
    strip.clear();
    strip.show();

    eggServo.attach(SERVO_PIN);

    unsigned long last_servo_turn = 0;
    int auto_servo_angle = 0;

    Serial.println(">> [ACTUATOR] Task Thuc Thi (Servo + NeoPixel) da san sang!");

    while (1)
    {

        bool is_auto = get_auto_mode();
        float current_temp = get_temperature();
        // AUTO MODE
        if (is_auto)
        {
            if (current_temp > 0)
            {
                int brightness = 0;

                if (current_temp < TEMP_CRITICAL_LOW)
                {
                    brightness = 50; // Max power
                }
                else if (current_temp >= TEMP_CRITICAL_LOW && current_temp < TEMP_TARGET_MIN)
                {
                    brightness = 30; // 60% power
                }
                else if (current_temp >= TEMP_TARGET_MIN && current_temp < TEMP_TARGET_MAX)
                {
                    brightness = 10; // 20% power
                }
                else if (current_temp >= TEMP_TARGET_MAX)
                {
                    brightness = 0; 
                }

                
                if (brightness > 0)
                {
                    strip.setPixelColor(0, strip.Color(255, 147, 41)); 
                }
                else
                {
                    strip.clear();
                }
                strip.setBrightness(brightness);
                strip.show();

                
                set_heater_pwm(brightness);
            }

            
            if (millis() - last_servo_turn > 30000)
            {
                last_servo_turn = millis();
                
                auto_servo_angle = (auto_servo_angle == 0) ? 180 : 0;
                eggServo.write(auto_servo_angle);
                set_servo_angle(auto_servo_angle); 

                if (xSemaphoreTake(xSerialMutex, portMAX_DELAY))
                {
                    Serial.printf(">> [AUTO] He thong tu dong dao trung sang goc: %d\n", auto_servo_angle);
                    xSemaphoreGive(xSerialMutex);
                }
            }
        }
        // MANUAL MODE
        else
        {
            
            int target_pwm = get_heater_pwm();
            int target_angle = get_servo_angle();

            if (target_pwm > 0)
            {
                strip.setPixelColor(0, strip.Color(255, 147, 41)); 
            }
            else
            {
                strip.clear();
            }
            strip.setBrightness(target_pwm);
            strip.show();

            eggServo.write(target_angle);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}