#include "esp_camera.h"
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

// ===== WiFi Credentials =====
const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASSWORD = "WIFI_PASSWORD";

// ===== SMTP Email Config =====
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "test@gmail.com"
#define AUTHOR_PASSWORD "APP_PASSWORD"
#define RECIPIENT_EMAIL "test20@gmail.com"

// ===== API Gateway for S3 Presigned URL =====
const char* apiGatewayUrl = "PRESIGNEDURL";

SMTPSession smtp;
bool captureFlag = false;

// ===== Camera Configuration (AI Thinker) =====
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
  Serial.begin(115200);
  initializeCamera();
  connectToWiFi();
  syncTime();
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    Serial.println("[DEBUG] Received: " + command);
    if (command == "CAPTURE") {
      captureFlag = true;
    }
  }

  if (captureFlag) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      captureFlag = false;
      return;
    }

    // Email send
    sendEmail(fb);

    // S3 Upload
    String url = getPresignedUrl();
    if (!url.isEmpty()) {
      uploadImage(url, fb);
    } else {
      Serial.println("Skipping upload - invalid presigned URL");
    }

    esp_camera_fb_return(fb);
    captureFlag = false;
  }
}

// ====== Helpers ======
void initializeCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init FAILED!");
  } else {
    Serial.println("Camera init SUCCESS!");
  }
}

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());
}

void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Syncing time");
  time_t now = time(nullptr);
  while (now < 1000000000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.printf("\nTime synced: %ld\n", now);
}

void sendEmail(camera_fb_t *fb) {
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;

  SMTP_Message message;
  message.sender.name = "Security System";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "INTRUSION ALERT";
  message.addRecipient("Recipient", RECIPIENT_EMAIL);

  message.text.content = "Motion detected!\n\nTimestamp: " + String(millis() / 1000);
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  SMTP_Attachment img;
  img.descr.filename = "intrusion.jpg";
  img.descr.mime = "image/jpeg";
  img.blob.data = fb->buf;
  img.blob.size = fb->len;
  img.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

  message.addAttachment(img);

  if (!smtp.connect(&session)) return;

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Email failed: " + smtp.errorReason());
  } else {
    Serial.println("Email sent successfully");
  }
}

String getPresignedUrl() {
  HTTPClient http;
  String url = "";

  http.begin(apiGatewayUrl);
  http.setTimeout(10000);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    url = http.getString();
    url.trim();
    url.replace("\"", "");
    url.replace("\\/", "/");

    if (!url.startsWith("http")) {
      Serial.println("Invalid URL format");
      url = "";
    }
  } else {
    Serial.printf("Failed to get URL. HTTP Code: %d\n", httpCode);
  }

  http.end();
  return url;
}

bool uploadImage(String url, camera_fb_t *fb) {
  if (url.isEmpty() || !url.startsWith("https://")) {
    Serial.println("Invalid URL for upload");
    return false;
  }

  WiFiClientSecure *client = new WiFiClientSecure;
  client->setInsecure(); // WARNING: Insecure for production

  HTTPClient http;
  http.begin(*client, url);
  http.addHeader("Content-Type", "image/jpeg");

  int httpCode = http.sendRequest("PUT", fb->buf, fb->len);
  Serial.printf("Upload status: %d\n", httpCode);

  http.end();
  delete client;

  return (httpCode == 200);
}
