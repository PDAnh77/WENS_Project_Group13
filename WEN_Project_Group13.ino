#include <WiFi.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP_Mail_Client.h>

// Thông tin SMTP
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 587
#define EMAIL_SENDER "" // Địa chỉ email của bạn
#define EMAIL_PASSWORD "password"
#define EMAIL_RECIPIENT "22520067@gm.uit.edu.vn" // Email nhận

SMTPSession smtp; // Đối tượng SMTP

const char* ssid = "";
const char* password = "";

const char* udpAddress = "192.168.1.123"; // Địa chỉ máy đích
const int udpPort = 12345; // Cổng gửi dữ liệu

const int oneWireBus = 4; // Cổng GPIO cảm biến kết nối tới

OneWire oneWire(oneWireBus);

DallasTemperature sensors(&oneWire);

WiFiUDP udp;

int readCount = 0;

float temperature_threshold = 20.0; // Ngưỡng nhiệt độ

void setup() {
  Serial.begin(115200);

  sensors.begin();

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password); // Kết nối tới Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  Serial.print("Local ESP32S3 IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("[+] Gateway IP : ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("[+] Subnet Mask : ");
  Serial.println(WiFi.subnetMask());
}

void sendEmail(float temperature) {
    smtp.debug(1);

    // Cấu hình phiên SMTP
    Session_Config config;
    config.server.host_name = SMTP_HOST;
    config.server.port = SMTP_PORT;
    config.login.email = EMAIL_SENDER;
    config.login.password = EMAIL_PASSWORD;

    SMTP_Message message;

    // Cấu hình email
    message.sender.name = "ESP32S3";
    message.sender.email = EMAIL_SENDER;
    message.subject = "Temperature Alert!";
    message.addRecipient("Recipient", EMAIL_RECIPIENT);
    message.text.content = "The temperature has exceeded the threshold! Current temperature: " + String(temperature) + " °C";

    // Đăng nhập vào server SMTP
    smtp.callback([](SMTP_Status status) {
      Serial.println(status.info());
    });

    if (!smtp.connect(&config)) {
        Serial.println("Failed to connect to SMTP server!");
        return;
    }

    // Gửi email
    if (!MailClient.sendMail(&smtp, &message)) {
        Serial.print("Error sending Email: ");
        Serial.println(smtp.errorReason());
    } else {
        Serial.println("Email sent successfully!");
        delay(600000);    
    }

    smtp.closeSession();
}

void loop() {
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);

  if (readCount < 3) {
    if (temperatureC != DEVICE_DISCONNECTED_C) {
      Serial.print("Current temperature is: ");
      Serial.println(temperatureC);

      String tempStr = "Current temperature: " + String(temperatureC) + "°C";

      // Gửi gói tin UDP
      udp.beginPacket(udpAddress, udpPort);
      udp.print(tempStr);
      if (udp.endPacket()) {
        Serial.print("Packet sent to ");
        Serial.print(udpAddress);
        Serial.println(" successfully.");
      } else {
        Serial.println("Failed to send packet.");
      }

      readCount++;
    } else {
      Serial.println("Error: Could not read temperature data");
    }
  }
  else {
    if(temperatureC > temperature_threshold) {
      sendEmail(temperatureC);
    }
    Serial.println("Preparing for the next temperature reading...");
    readCount = 0;
    delay(2000);
  }
  delay(3000);
}