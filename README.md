# PillSync: IoT-Based Smart Medical Dispenser

PillSync is an autonomous, IoT-enabled smart medication dispenser powered by the ESP32 microcontroller. It ensures medication adherence through a physical "Hardware Interlock" (Double Validation) system and securely logs intake data to a cloud-based Google Sheets database in real-time.

## Key Features
* **Hardware Interlock System:** Prevents "false logging". Users must physically open the lid and press the button to successfully confirm medication intake.
* **Zero-Latency Tamper Alarm:** Utilizes hardware interrupts (ISR) to detect unauthorized lid openings instantly, triggering a continuous buzzer and aggressive RGB flashing.
* **Cloud Integration:** Secure TLS/SSL encrypted HTTPS GET requests transmit data directly to Google Apps Script Webhooks.
* **Interactive UI:** A 0.96" OLED display provides real-time system status, prompts, and connection feedback.
* **Fully Portable:** Operates independently via a 5V USB Power Bank without triggering auto-shutoff circuits due to continuous active states.

## Hardware Components
* ESP32 NodeMCU-32S
* 0.96" I2C OLED Display (SSD1306)
* Common Cathode RGB LED
* Active Buzzer
* Mechanical Micro-Switch (Lid State Sensor)
* Tactile Push Button

## Software Architecture
* **Environment:** C++ (Arduino IDE)
* **Libraries:** WiFi.h, WiFiClientSecure.h, HTTPClient.h, Wire.h, Adafruit_GFX.h, Adafruit_SSD1306.h
* **Core Logic:** Non-blocking Finite State Machine (FSM) utilizing millis() for timers and attachInterrupt() for critical safety responses, ensuring the CPU never hangs and remains strictly responsive.
