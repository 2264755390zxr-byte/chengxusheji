// Ex07 - Web网页端无极调光器
// 功能：通过Web网页上的滑动条实时调节LED亮度
// 关联：实验3(PWM呼吸灯)、Web服务器

#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "111";
const char* password = "54188sb.";

const int ledPin = 2;    // LED引脚
int brightness = 128;    // 初始亮度

WebServer server(80);

// HTML网页，包含滑动条和实时调光功能
const char htmlPage[] = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 无极调光器</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        .container {
            background: white;
            padding: 40px;
            border-radius: 10px;
            box-shadow: 0 10px 25px rgba(0, 0, 0, 0.2);
            text-align: center;
            width: 90%;
            max-width: 400px;
        }
        h1 {
            color: #333;
            margin-top: 0;
        }
        .slider-group {
            margin: 30px 0;
        }
        input[type="range"] {
            width: 100%;
            height: 10px;
            cursor: pointer;
            accent-color: #667eea;
        }
        .brightness-display {
            font-size: 32px;
            font-weight: bold;
            color: #667eea;
            margin: 20px 0;
        }
        .led-indicator {
            width: 100px;
            height: 100px;
            background-color: rgb(128, 128, 128);
            border-radius: 50%;
            margin: 20px auto;
            box-shadow: 0 0 20px rgba(0, 0, 0, 0.3);
            transition: background-color 0.1s;
        }
        .info-text {
            color: #666;
            font-size: 14px;
            margin-top: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Brightness Controller</h1>
        <div class="slider-group">
            <label for="brightnessSlider">亮度控制：</label><br><br>
            <input type="range" id="brightnessSlider" min="0" max="255" value="128">
        </div>
        <div class="brightness-display" id="brightnessValue">128</div>
        <div class="led-indicator" id="ledIndicator"></div>
        <div class="info-text">
            拖动滑动条实时调节LED亮度
        </div>
    </div>

    <script>
        const slider = document.getElementById('brightnessSlider');
        const brightnessValue = document.getElementById('brightnessValue');
        const ledIndicator = document.getElementById('ledIndicator');

        // 监听滑动条变动
        slider.addEventListener('input', function() {
            const value = this.value;
            brightnessValue.textContent = value;
            
            // 更新LED指示器的亮度
            const rgb = Math.round(value);
            ledIndicator.style.backgroundColor = `rgb(${rgb}, ${rgb}, ${rgb})`;
            
            // 通过GET请求发送亮度值给ESP32
            fetch(`/set?brightness=${value}`)
                .catch(error => console.error('Error:', error));
        });

        // 初始化LED指示器
        ledIndicator.style.backgroundColor = `rgb(128, 128, 128)`;
    </script>
</body>
</html>
)";

void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(ledPin, OUTPUT);
  analogWrite(ledPin, brightness);
  
  Serial.println("\n\nStarting WiFi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
  }
  
  // 设置路由处理
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}

// 处理根路由 - 返回HTML页面
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", htmlPage);
}

// 处理调光请求 - /set?brightness=value
void handleSet() {
  if (server.hasArg("brightness")) {
    String brightnessStr = server.arg("brightness");
    brightness = constrain(brightnessStr.toInt(), 0, 255);
    analogWrite(ledPin, brightness);
    
    Serial.print("Brightness set to: ");
    Serial.println(brightness);
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing brightness parameter");
  }
}
