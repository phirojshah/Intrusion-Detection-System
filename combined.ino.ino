// === Sensor Pin Definitions ===
const int pirPin        = 27;  // PIR motion sensor
const int vibrationPin  = 13;  // Vibration sensor
const int irPin         = 14;  // IR sensor
const int reedPin       = 26;  // Reed switch
const int soundSensor   = 25;  // Analog sound sensor (AO pin)

// === LED for PIR Indicator ===
const int ledPin        = 33;  // PIR LED output

// === Debounce Settings ===
unsigned long lastTrigger = 0;
const unsigned long debounceDelay = 2000; // ms

// === Sound Variables ===
int rawValue = 0;
int normalizedValue = 0;

void setup() {
  Serial.begin(115200);

  pinMode(pirPin, INPUT);
  pinMode(vibrationPin, INPUT);
  pinMode(irPin, INPUT);
  pinMode(reedPin, INPUT_PULLUP);  // Reed switch needs pull-up
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, LOW);  // Ensure LED is off initially

  analogReadResolution(12);   // 12-bit ADC: values 0–4095

  Serial.println("All sensors initialized...");
}

void loop() {
  bool triggered = false;

  // === PIR Motion Detection ===
  if (digitalRead(pirPin) == HIGH) {
    digitalWrite(ledPin, HIGH);
    Serial.println("PIR: Motion detected");
    triggered = true;
  } else {
    digitalWrite(ledPin, LOW);
  }

  // === Vibration Detection ===
  if (digitalRead(vibrationPin) == LOW) {
    Serial.println("Vibration: Detected");
    triggered = true;
  }

  // === IR Obstacle Sensor ===
  if (digitalRead(irPin) == LOW) {
    Serial.println("IR Sensor: Object detected");
    triggered = true;
  }

  // === Reed Switch (Magnet Removed) ===
  if (digitalRead(reedPin) == HIGH) {
    Serial.println("Reed Switch: Door/Window opened");
    triggered = true;
  }

  // === Analog Sound Detection ===
  rawValue = analogRead(soundSensor);
  normalizedValue = map(rawValue, 0, 4095, 0, 100);
  Serial.print("Sound Raw: ");
  Serial.print(rawValue);
  Serial.print("\tNormalized: ");
  Serial.println(normalizedValue);

  if (normalizedValue > 50) { // Tune threshold as needed
    Serial.println("Sound: Detected");
    triggered = true;
  }

  // === Send CAPTURE Signal to ESP32-CAM ===
  if (triggered && (millis() - lastTrigger > debounceDelay)) {
    Serial.println("CAPTURE");
    lastTrigger = millis();
  }

  delay(3000);
}
