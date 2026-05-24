// Ex04 - 基于触摸传感器的"自锁"开关
// 功能：触摸一次LED亮起并保持，再触摸一次LED熄灭
// 关联：实验2(基础IO)、实验4(触摸引脚)

const int ledPin = 2;
const int touchPin = T0;  // 触摸引脚 (GPIO4 on ESP32)

bool ledState = false;           // LED当前状态
bool lastTouchState = false;     // 上一次触摸状态
unsigned long lastDebounceTime = 0;
const long debounceDelay = 50;   // 软件防抖延迟 50ms

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  // 读取当前触摸状态
  // 触摸传感器阈值通常在30以下（未触摸），>30为触摸
  int touchValue = touchRead(touchPin);
  bool currentTouchState = (touchValue < 30) ? false : true;
  
  // 软件防抖：判断是否超过防抖延迟
  unsigned long currentMillis = millis();
  
  // 边缘检测：检测从未触摸到被触摸的瞬间
  if (currentTouchState && !lastTouchState && 
      (currentMillis - lastDebounceTime >= debounceDelay)) {
    
    // 翻转LED状态
    ledState = !ledState;
    digitalWrite(ledPin, ledState ? HIGH : LOW);
    lastDebounceTime = currentMillis;
  }
  
  // 更新上一次触摸状态
  lastTouchState = currentTouchState;
  
  delay(10);  // 最小循环延迟
}
