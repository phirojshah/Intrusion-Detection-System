#include "esp_camera.h"
#include <WiFi.h>
#include <ESP_Mail_Client.h>

// Camera Configuration (AI Thinker Model)
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

// WiFi Credentials
const char* WIFI_SSID = "your_ssid";
const char* WIFI_PASSWORD = "Your_password";

// Email Configuration
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "test@gmail.com"
#define AUTHOR_PASSWORD "Your_app_password"
#define RECIPIENT_EMAIL "test2@gmail.com"

SMTPSession smtp;
bool captureFlag = false;

void setup() {
  Serial.begin(115200);
  initializeCamera();
  connectToWiFi();
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
    captureAndSendEmail();
    captureFlag = false;
  }
}

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
  config.frame_size = FRAMESIZE_SVGA;  // 800x600 (was likely UXGA before)
  config.jpeg_quality = 10;            // 0-63 (lower = better quality)
  config.fb_count = 1;   
                // Number of frame buffers (use 1)
  // Init camera
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
    WiFi.reconnect();
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void captureAndSendEmail() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Configure email
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  SMTP_Message message;
  message.sender.name = "Security System";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "INTRUSION ALERT";
  message.addRecipient("Recipient", RECIPIENT_EMAIL);

  // Text content
  message.text.content = "Motion detected!\n\nTimestamp: ";
  message.text.content += String(millis() / 1000);
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  // Image attachment
  SMTP_Attachment img;
  img.descr.filename = "intrusion.jpg";
  img.descr.mime = "image/jpeg";
  img.blob.data = fb->buf;
  img.blob.size = fb->len;
  img.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64; 

  message.addAttachment(img);

  // Connect and send
  if (!smtp.connect(&session)) return;
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Email failed: " + smtp.errorReason());
  }

  esp_camera_fb_return(fb);
}
