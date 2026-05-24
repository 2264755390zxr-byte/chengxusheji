// Ex05 - 多档位触摸调速呼吸灯
// 功能：结合呼吸灯和触摸自锁开关，每次触摸改变呼吸灯的速度（1、2、3档）
// 关联：实验3(PWM呼吸灯)、实验4(触摸引脚)

const int ledPin = 2;
const int touchPin = T0;  // 触摸引脚

bool lastTouchState = false;
unsigned long lastDebounceTime = 0;
const long debounceDelay = 50;

int speedLevel = 1;  // 速度档位：1(慢)、2(中)、3(快)

// 各档位的延迟时间
const int delayFast[] = {1, 2, 3};      // 快速档
const int delaySlow[] = {8, 6, 4};      // 对应各档的延迟
const int stepSize[] = {3, 5, 8};       // 占空比递增步长

unsigned long prevMillis = 0;
int brightness = 0;
int direction = 1;  // 1: 增加, -1: 减少

void setup() {
  pinMode(ledPin, OUTPUT);
  analogWrite(ledPin, 0);
}

void loop() {
  // 读取触摸状态
  int touchValue = touchRead(touchPin);
  bool currentTouchState = (touchValue < 30) ? false : true;
  
  // 软件防抖和边缘检测
  unsigned long currentMillis = millis();
  
  if (currentTouchState && !lastTouchState && 
      (currentMillis - lastDebounceTime >= debounceDelay)) {
    
    // 档位循环切换：1 -> 2 -> 3 -> 1
    speedLevel++;
    if (speedLevel > 3) speedLevel = 1;
    lastDebounceTime = currentMillis;
  }
  
  lastTouchState = currentTouchState;
  
  // 根据档位选择延迟
  int delayTime = 0;
  if (speedLevel == 1) delayTime = 8;
  else if (speedLevel == 2) delayTime = 5;
  else if (speedLevel == 3) delayTime = 2;
  
  // 呼吸灯逻辑：根据档位改变步长
  int step = 0;
  if (speedLevel == 1) step = 2;
  else if (speedLevel == 2) step = 4;
  else if (speedLevel == 3) step = 6;
  
  // PWM呼吸灯核心逻辑
  if (currentMillis - prevMillis >= delayTime) {
    prevMillis = currentMillis;
    
    brightness += direction * step;
    
    // 亮度反向控制
    if (brightness >= 255) {
      brightness = 255;
      direction = -1;
    } else if (brightness <= 0) {
      brightness = 0;
      direction = 1;
    }
    
    analogWrite(ledPin, brightness);
  }
}
