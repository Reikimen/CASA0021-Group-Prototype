#include "Arduino.h"

void Task1(void *pvParameters);
void Task2(void *pvParameters);

void setup() {
    Serial.begin(115200);

    // 创建 Task1，在核心 0 运行
    xTaskCreatePinnedToCore(
        Task1,          // 任务函数
        "Task 1",       // 任务名称
        1000,           // 栈大小（字节）
        NULL,           // 任务参数
        1,              // 优先级（1 是低优先级，5 是高优先级）
        NULL,           // 任务句柄
        0               // 绑定到 CPU 核心 0
    );

    // 创建 Task2，在核心 1 运行
    xTaskCreatePinnedToCore(
        Task2,          // 任务函数
        "Task 2",       // 任务名称
        1000,           // 栈大小（字节）
        NULL,           // 任务参数
        1,              // 优先级
        NULL,           // 任务句柄
        1               // 绑定到 CPU 核心 1
    );
}

void loop() {
    // loop 运行在 setup() 之后，不做额外操作
}

void Task1(void *pvParameters) {
    while (1) {
        Serial.println("Task 1 running on Core 0");
        delay(1000);
    }
}

void Task2(void *pvParameters) {
    while (1) {
        Serial.println("Task 2 running on Core 1");
        delay(2000);
    }
}
