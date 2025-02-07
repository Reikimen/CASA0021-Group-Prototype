#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define DEVICE_NAME "ESP32_BLE"  // 设备名称

void setup() {
  Serial.begin(115200);
  
  // 初始化 BLE 设备
  BLEDevice::init(DEVICE_NAME);
  
  // 创建 BLE 服务器
  BLEServer *pServer = BLEDevice::createServer();

  // 开始 BLE 广播
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLEUUID((uint16_t)0x180F));  // 添加电池服务 UUID
  BLEDevice::startAdvertising();

  Serial.println("ESP32 BLE broadcasting...");
}

void loop() {
  delay(1000);
}
