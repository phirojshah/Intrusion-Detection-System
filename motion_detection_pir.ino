const int pirSensorPin = 13;
const int ledPin = 14;
bool motionDetected = false;
unsigned long lastMotionTime = 0;
const unsigned long debounceDelay = 500; // 500ms cooldown

void setup() {
  pinMode(pirSensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  // delay(60 * 1000); // Warm-up time for PIR sensor (1 minute)
}

void loop() {
  int pirValue = digitalRead(pirSensorPin);

  if (pirValue == HIGH) {
    if (!motionDetected && (millis() - lastMotionTime > debounceDelay)) {
      motionDetected = true;
      lastMotionTime = millis(); // Update cooldown timer
      // Send command to ESP32 CAM to capture an image
    
      digitalWrite(ledPin, HIGH);
      Serial.println("CAPTURE");  // Command to ESP32-CAM
      Serial.println("ALERT: Intrusion detected!");
      delay(3000);
    }
  } else {
    if (motionDetected) {
      motionDetected = false;
      digitalWrite(ledPin, LOW);
      Serial.println("ALERT: Situation normal");
      delay(3000);
    }
  }

  delay(10); // Smaller delay for responsiveness
}