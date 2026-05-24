// Ex08 - 物联网安防报警器
// 功能：通过Web网页控制布防/撤防，触摸引脚触发报警LED闪烁
// 关联：基础IO、触摸引脚、Web服务器

#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "your_ssid";
const char* password = "your_password";

const int ledPin = 2;         // LED引脚
const int touchPin = T0;      // 触摸引脚(GPIO4)

// 系统状态
bool systemArmed = false;     // 布防状态
bool alarmTriggered = false;  // 报警状态
unsigned long lastBlinkTime = 0;
const int blinkInterval = 100; // 闪烁间隔 100ms
bool ledState = false;

WebServer server(80);

// HTML网页 - 包含布防/撤防按钮
const char htmlPage[] = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>安防报警器</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
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
        .status {
            font-size: 24px;
            font-weight: bold;
            margin: 30px 0;
            padding: 20px;
            border-radius: 5px;
            transition: all 0.3s;
        }
        .status.armed {
            background-color: #f5576c;
            color: white;
        }
        .status.disarmed {
            background-color: #4caf50;
            color: white;
        }
        .button-group {
            display: flex;
            gap: 15px;
            margin: 30px 0;
            justify-content: center;
        }
        button {
            padding: 15px 30px;
            font-size: 16px;
            font-weight: bold;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            transition: transform 0.2s, opacity 0.2s;
            min-width: 120px;
        }
        button:active {
            transform: scale(0.98);
        }
        .arm-btn {
            background-color: #f5576c;
            color: white;
        }
        .arm-btn:hover {
            opacity: 0.9;
        }
        .arm-btn:disabled {
            background-color: #cccccc;
            cursor: not-allowed;
        }
        .disarm-btn {
            background-color: #4caf50;
            color: white;
        }
        .disarm-btn:hover {
            opacity: 0.9;
        }
        .disarm-btn:disabled {
            background-color: #cccccc;
            cursor: not-allowed;
        }
        .info-text {
            color: #666;
            font-size: 14px;
            margin-top: 20px;
            line-height: 1.6;
        }
        .alarm-indicator {
            width: 80px;
            height: 80px;
            background-color: #e0e0e0;
            border-radius: 50%;
            margin: 20px auto;
            box-shadow: 0 0 15px rgba(0, 0, 0, 0.2);
        }
        .alarm-indicator.alarm {
            animation: blink 0.1s infinite;
            background-color: #f5576c;
        }
        @keyframes blink {
            0%, 50% {
                background-color: #f5576c;
                box-shadow: 0 0 30px rgba(245, 87, 108, 0.8);
            }
            51%, 100% {
                background-color: #000;
                box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚨 安防报警器</h1>
        <div class="status disarmed" id="status">未布防</div>
        <div class="alarm-indicator" id="alarmIndicator"></div>
        <div class="button-group">
            <button class="arm-btn" id="armBtn" onclick="armSystem()">🔒 布防(Arm)</button>
            <button class="disarm-btn" id="disarmBtn" onclick="disarmSystem()" disabled>🔓 撤防(Disarm)</button>
        </div>
        <div class="info-text">
            <p>✓ 点击"布防"启动安防系统</p>
            <p>✓ 触摸传感器会触发报警</p>
            <p>⚠ 只有点击"撤防"才能停止报警</p>
        </div>
    </div>

    <script>
        function armSystem() {
            fetch('/arm')
                .then(response => response.text())
                .then(data => {
                    updateStatus();
                })
                .catch(error => console.error('Error:', error));
        }

        function disarmSystem() {
            fetch('/disarm')
                .then(response => response.text())
                .then(data => {
                    updateStatus();
                })
                .catch(error => console.error('Error:', error));
        }

        function updateStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    const statusDiv = document.getElementById('status');
                    const armBtn = document.getElementById('armBtn');
                    const disarmBtn = document.getElementById('disarmBtn');
                    const alarmIndicator = document.getElementById('alarmIndicator');

                    if (data.armed) {
                        statusDiv.textContent = '已布防 ⚠️';
                        statusDiv.className = 'status armed';
                        armBtn.disabled = true;
                        disarmBtn.disabled = false;
                    } else {
                        statusDiv.textContent = '未布防 ✓';
                        statusDiv.className = 'status disarmed';
                        armBtn.disabled = false;
                        disarmBtn.disabled = true;
                    }

                    if (data.alarm) {
                        alarmIndicator.classList.add('alarm');
                    } else {
                        alarmIndicator.classList.remove('alarm');
                    }
                })
                .catch(error => console.error('Error:', error));
        }

        // 定时更新状态
        setInterval(updateStatus, 500);
        updateStatus();
    </script>
</body>
</html>
)";

void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
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
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
  
  // 检测触摸引脚（当布防时）
  if (systemArmed && !alarmTriggered) {
    int touchValue = touchRead(touchPin);
    if (touchValue < 40) {  // 触摸阈值
      alarmTriggered = true;
      Serial.println("ALARM TRIGGERED!");
    }
  }
  
  // LED闪烁逻辑（当报警时）
  if (alarmTriggered) {
    unsigned long currentTime = millis();
    if (currentTime - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = currentTime;
      ledState = !ledState;
      digitalWrite(ledPin, ledState ? HIGH : LOW);
    }
  } else {
    digitalWrite(ledPin, LOW);
  }
}

// 处理根路由 - 返回HTML页面
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", htmlPage);
}

// 处理布防请求
void handleArm() {
  systemArmed = true;
  alarmTriggered = false;
  Serial.println("System ARMED");
  server.send(200, "text/plain", "Armed");
}

// 处理撤防请求
void handleDisarm() {
  systemArmed = false;
  alarmTriggered = false;
  digitalWrite(ledPin, LOW);
  Serial.println("System DISARMED");
  server.send(200, "text/plain", "Disarmed");
}

// 返回系统状态
void handleStatus() {
  String json = "{\"armed\": " + String(systemArmed ? "true" : "false") + 
                ", \"alarm\": " + String(alarmTriggered ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}
