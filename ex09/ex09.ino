// Ex09 - 实时传感器Web仪表盘
// 功能：通过Web实时显示触摸传感器数值，支持动态阈值和上电自校准
// 关联：Web服务器、触摸引脚、AJAX技术

#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "111";
const char* password = "54188sb.";

const int touchPin = T0;      // 触摸引脚(GPIO4)

// 传感器数据
int sensorValue = 0;
int baselineValue = 750;       // 基线值（初始化时校准）
int calibratedThreshold = 600; // 动态阈值（基线值的80%）

// 校准相关
bool isCalibrated = false;
unsigned long calibrationStartTime = 0;
const int calibrationDuration = 2000; // 校准持续2秒

WebServer server(80);

// HTML网页 - 实时传感器仪表盘
const char htmlPage[] = R"(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>实时传感器仪表盘</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        .container {
            background: white;
            padding: 40px;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            text-align: center;
            width: 90%;
            max-width: 500px;
        }
        h1 {
            color: #333;
            margin-bottom: 30px;
            font-size: 28px;
        }
        .gauge-container {
            position: relative;
            width: 280px;
            height: 280px;
            margin: 20px auto;
            background: radial-gradient(circle at center, #f5f5f5 0%, #e0e0e0 100%);
            border-radius: 50%;
            display: flex;
            justify-content: center;
            align-items: center;
            box-shadow: inset 0 2px 10px rgba(0, 0, 0, 0.1), 0 10px 30px rgba(0, 0, 0, 0.2);
        }
        .gauge-value {
            font-size: 64px;
            font-weight: bold;
            color: #667eea;
            text-align: center;
            line-height: 1;
        }
        .gauge-unit {
            font-size: 16px;
            color: #999;
            margin-top: 5px;
        }
        .stats {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin: 30px 0;
        }
        .stat-box {
            background: #f5f5f5;
            padding: 15px;
            border-radius: 8px;
            border-left: 4px solid #667eea;
        }
        .stat-label {
            font-size: 12px;
            color: #999;
            text-transform: uppercase;
            margin-bottom: 5px;
        }
        .stat-value {
            font-size: 20px;
            font-weight: bold;
            color: #333;
        }
        .calibration-status {
            padding: 10px;
            border-radius: 5px;
            font-size: 14px;
            margin: 15px 0;
            font-weight: bold;
        }
        .calibration-status.calibrating {
            background-color: #fff3cd;
            color: #856404;
        }
        .calibration-status.calibrated {
            background-color: #d4edda;
            color: #155724;
        }
        .chart {
            margin: 30px 0;
            padding: 20px;
            background: #f9f9f9;
            border-radius: 8px;
        }
        .chart-title {
            font-size: 14px;
            color: #666;
            margin-bottom: 10px;
            text-align: left;
        }
        .chart-bar {
            width: 100%;
            height: 30px;
            background: #e0e0e0;
            border-radius: 15px;
            overflow: hidden;
            position: relative;
        }
        .chart-fill {
            height: 100%;
            background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
            transition: width 0.2s ease-out;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-size: 12px;
            font-weight: bold;
        }
        .threshold-indicator {
            position: absolute;
            height: 100%;
            width: 2px;
            background: red;
            opacity: 0.7;
        }
        .info-text {
            color: #666;
            font-size: 13px;
            margin-top: 20px;
            line-height: 1.6;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>📊 实时传感器仪表盘</h1>
        
        <div class="calibration-status calibrating" id="calibrationStatus">
            📡 正在校准... 请勿接近传感器
        </div>

        <div class="gauge-container">
            <div>
                <div class="gauge-value" id="sensorValue">0</div>
                <div class="gauge-unit">传感器数值</div>
            </div>
        </div>

        <div class="stats">
            <div class="stat-box">
                <div class="stat-label">基线值</div>
                <div class="stat-value" id="baseline">0</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">触摸阈值</div>
                <div class="stat-value" id="threshold">0</div>
            </div>
        </div>

        <div class="chart">
            <div class="chart-title">📈 传感器信号强度</div>
            <div class="chart-bar">
                <div class="chart-fill" id="chartFill" style="width: 0%;">
                    <span id="chartPercent">0%</span>
                </div>
                <div class="threshold-indicator" id="thresholdLine"></div>
            </div>
        </div>

        <div class="info-text">
            <p>✓ 系统自动校准中</p>
            <p>✓ 手指逐渐靠近时数值会减小</p>
            <p>⚠️ 触摸距离很近时会触发阈值</p>
        </div>
    </div>

    <script>
        let baselineValue = 50;
        let thresholdValue = 40;
        let isCalibrated = false;

        async function fetchSensorData() {
            try {
                const response = await fetch('/data');
                const data = await response.json();
                
                // 更新传感器值
                document.getElementById('sensorValue').textContent = data.value;
                document.getElementById('baseline').textContent = data.baseline;
                document.getElementById('threshold').textContent = data.threshold;
                
                baselineValue = data.baseline;
                thresholdValue = data.threshold;
                isCalibrated = data.calibrated;

                // 更新校准状态
                const statusDiv = document.getElementById('calibrationStatus');
                if (isCalibrated) {
                    statusDiv.className = 'calibration-status calibrated';
                    statusDiv.textContent = '✓ 校准完成 | 系统就绪';
                } else {
                    statusDiv.className = 'calibration-status calibrating';
                    statusDiv.textContent = '📡 正在校准... 请勿接近传感器';
                }

                // 更新进度条
                const maxValue = baselineValue;
                const percentage = Math.max(0, Math.min(100, (data.value / maxValue) * 100));
                const chartFill = document.getElementById('chartFill');
                chartFill.style.width = percentage + '%';
                document.getElementById('chartPercent').textContent = Math.round(percentage) + '%';

                // 更新阈值指示线位置
                const thresholdPercent = (thresholdValue / maxValue) * 100;
                document.getElementById('thresholdLine').style.left = thresholdPercent + '%';

                // 根据是否接近阈值改变颜色
                if (data.value < thresholdValue) {
                    chartFill.style.background = 'linear-gradient(90deg, #ff6b6b 0%, #ee5a6f 100%)';
                } else {
                    chartFill.style.background = 'linear-gradient(90deg, #667eea 0%, #764ba2 100%)';
                }

            } catch (error) {
                console.error('Error fetching sensor data:', error);
            }
        }

        // 初始化 - 立即获取一次数据
        fetchSensorData();

        // 每100ms更新一次数据
        setInterval(fetchSensorData, 100);
    </script>
</body>
</html>
)";

void setup() {
  Serial.begin(115200);
  delay(100);
  
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
  server.on("/data", handleData);
  
  server.begin();
  Serial.println("Server started");
  
  // 开始校准
  calibrationStartTime = millis();
  Serial.println("Calibration started - do not touch the sensor!");
}

void loop() {
  server.handleClient();
  
  // 读取传感器值
  sensorValue = touchRead(touchPin);
  
  // 校准逻辑 - 前2秒收集基线值
  if (!isCalibrated) {
    unsigned long elapsedTime = millis() - calibrationStartTime;
    
    if (elapsedTime < calibrationDuration) {
      // 校准期间：取最大值作为基线（无手指时的最大值）
      if (sensorValue > baselineValue) {
        baselineValue = sensorValue;
      }
    } else {
      // 校准完成
      isCalibrated = true;
      // 动态阈值设为基线值的80%
      calibratedThreshold = baselineValue * 0.8;
      Serial.print("Calibration complete. Baseline: ");
      Serial.print(baselineValue);
      Serial.print(", Threshold: ");
      Serial.println(calibratedThreshold);
    }
  }
  
  delay(10);
}

// 处理根路由 - 返回HTML页面
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", htmlPage);
}

// 处理数据请求 - 返回JSON格式的传感器数据
void handleData() {
  String json = "{\"value\": " + String(sensorValue) +
                ", \"baseline\": " + String(baselineValue) +
                ", \"threshold\": " + String(calibratedThreshold) +
                ", \"calibrated\": " + String(isCalibrated ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}
