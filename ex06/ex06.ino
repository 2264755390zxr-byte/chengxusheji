// Ex06 - 警车双闪灯效（双通道PWM）
// 功能：两个LED呈现平滑交替的渐变闪烁效果，呈现反相亮度关系
// 关联：实验3(PWM呼吸灯)

const int ledPinA = 2;   // LED A引脚
const int ledPinB = 3;   // LED B引脚

unsigned long prevMillis = 0;
const int delayTime = 5;  // PWM更新延迟 5ms

int brightness = 0;
int direction = 1;  // 1: 增加, -1: 减少
const int stepSize = 3;  // 占空比递增步长

void setup() {
  pinMode(ledPinA, OUTPUT);
  pinMode(ledPinB, OUTPUT);
  analogWrite(ledPinA, 0);
  analogWrite(ledPinB, 255);
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - prevMillis >= delayTime) {
    prevMillis = currentMillis;
    
    // 更新亮度
    brightness += direction * stepSize;
    
    // 亮度范围控制
    if (brightness >= 255) {
      brightness = 255;
      direction = -1;
    } else if (brightness <= 0) {
      brightness = 0;
      direction = 1;
    }
    
    // LED A：从0增加到255
    // LED B：从255减少到0（反相关系）
    int brightnessB = 255 - brightness;
    
    analogWrite(ledPinA, brightness);
    analogWrite(ledPinB, brightnessB);
  }
}
