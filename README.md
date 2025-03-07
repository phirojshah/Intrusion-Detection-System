# Home Intrusion Detection System

A DIY security system that detects motion and instantly emails snapshots of potential intruders to your inbox. Built with ESP32-CAM and PIR motion sensor for reliable home monitoring.

![Home Security System](https://github.com/phirojshah/repo-name/raw/main/images/system-photo.jpg)

## 🌟 Features

- **Motion Detection**: Passive infrared (PIR) sensor for reliable human presence detection
- **Instant Alerts**: Captures and emails photos immediately when motion is detected
- **Easy Setup**: Simple configuration with minimal wiring
- **Cost-Effective**: Affordable alternative to commercial security systems
- **Customizable**: Adjustable sensitivity and notification settings

## 📋 Components Required

- ESP32-CAM module with OV2640 camera
- PIR motion sensor (HC-SR501)
- LED indicator (optional)
- Power supply (5V)
- Jumper wires
- Micro USB cable (for programming)
- FTDI programmer (for ESP32-CAM)

## 📝 Prerequisites

Before building this project, you'll need:

1. [Arduino IDE](https://www.arduino.cc/en/software) installed with ESP32 board support
2. Required libraries:
   - ESP32 Mail Client library
   - ESP32 Camera library

## 📋 Installation

1. **Hardware Setup**:
   - Connect the PIR sensor to GPIO 13 on the ESP32
   - Connect an LED indicator to GPIO 14 (optional)
   - Ensure proper power connections for both modules

2. **Software Setup**:
   - Clone this repository:
     ```
     git clone https://github.com/phirojshah/intrusion-detection-system.git
     ```
   - Open the Arduino IDE
   - Install required libraries through the Library Manager
   - Configure your WiFi and email credentials in the code

3. **Configure Email Settings**:
   - You'll need to create an app-specific password for your Gmail account
   - Update the following in the `esp32cam.ino` file:
     ```cpp
     #define SMTP_HOST "smtp.gmail.com"
     #define SMTP_PORT 465
     #define AUTHOR_EMAIL "your_email@gmail.com"
     #define AUTHOR_PASSWORD "your_app_password"
     #define RECIPIENT_EMAIL "recipient_email@gmail.com"
     ```

4. **Upload the Code**:
   - Upload `motion_detection_pir.ino` to your ESP32 or Arduino (connected to PIR)
   - Upload `esp32cam.ino` to the ESP32-CAM module

## 📷 How It Works

1. The PIR sensor continuously monitors for motion
2. When motion is detected, the system triggers the ESP32-CAM
3. The ESP32-CAM captures an image and sends it via email
4. An LED indicator lights up during the detection event
5. The system resets after a short cooldown period

## ⚙️ Configuration Options

You can customize the system by modifying these parameters:

- **Debounce Delay**: Adjust the cooldown period between detections
  ```cpp
  const unsigned long debounceDelay = 500; // 500ms cooldown
  ```

- **Camera Resolution**: Change the image quality in `esp32cam.ino`
  ```cpp
  config.frame_size = FRAMESIZE_SVGA;  // 800x600
  config.jpeg_quality = 10;            // 0-63 (lower = better quality)
  ```

## 🔧 Troubleshooting

- **Camera Not Initializing**: Check power supply stability, ESP32-CAM requires consistent power
- **Email Not Sending**: Verify your app password and email configuration
- **False Positives**: Adjust PIR sensor sensitivity using the onboard potentiometers
- **Connection Issues**: Ensure WiFi credentials are correct and signal is strong

## 🛠️ Future Improvements

- [ ] Add a web interface for remote monitoring
- [ ] Implement cloud storage for captured images
- [ ] Add battery backup for power outages
- [ ] Create a mobile app for notifications
- [ ] Add support for multiple sensors/cameras

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [ESP32-CAM Documentation](https://github.com/espressif/esp32-camera)
- [ESP Mail Client](https://github.com/mobizt/ESP-Mail-Client)

## 📞 Contact

For questions or suggestions, please open an issue on this repository or contact [your-email@example.com](mailto:your-email@example.com).

---

*Note: This security system is intended for educational purposes and as a DIY project. For critical security applications, consider professional security solutions.*
