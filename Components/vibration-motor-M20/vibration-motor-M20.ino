// 定义控制引脚
const int motorPin = 16; // GPIO 16 用于控制振动电机对应的MOSEFET引脚

void setup() {
  // 设置 motorPin 为输出
  pinMode(motorPin, OUTPUT);
}

void loop() {
  // 启动电机
  digitalWrite(motorPin, HIGH); 
  delay(1000); // 振动 1 秒
  
  // 停止电机
  digitalWrite(motorPin, LOW);
  delay(1000); // 停止 1 秒
}
