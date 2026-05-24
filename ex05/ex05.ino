// Ex05 - 多档位触摸调速呼吸灯
// 功能：结合呼吸灯和触摸自锁开关，每次触摸改变呼吸灯的速度（1、2、3档）
// 关联：实验3(PWM呼吸灯)、实验4(触摸引脚)
// 预期效果：LED呈现呼吸灯效果，触摸改变呼吸速度（三个明显的速度级别）

const int ledPin = 2;
const int touchPin = T0;  // 触摸引脚
const int touchThreshold = 600; // 触摸阈值，根据实际情况调整

bool lastTouchState = false;
unsigned long lastDebounceTime = 0;
const long debounceDelay = 50;

int speedLevel = 1;  // 速度档位：1(缓慢呼吸)、2(正常呼吸)、3(急促呼吸)

// 各档位的PWM参数
const int delayMS[4] = {0, 10, 6, 2};   // 0: 占位符, 1档(ms), 2档(ms), 3档(ms)
const int stepSize[4] = {0, 2, 4, 8};   // 0: 占位符, 1档步长, 2档步长, 3档步长

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
  bool currentTouchState = (touchValue < touchThreshold ) ? false : true;
  
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
  
  // PWM呼吸灯核心逻辑：根据当前档位更新
  if (currentMillis - prevMillis >= delayMS[speedLevel]) {
    prevMillis = currentMillis;
    
    brightness += direction * stepSize[speedLevel];
    
    // 亮度范围约束：0-255
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
