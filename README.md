# Fire-Fighting-Robot-with-Alert-System

## Overview
An Arduino-powered robot that autonomously detects and extinguishes fires and alerts users via SMS. Also supports manual Bluetooth control.

## Hardware Components
- Arduino UNO
- Ultrasonic Sensor
- Flame Sensor
- MQ2 Gas Sensor
- L298N Motor Driver
- Servo Motor + Water Pump
- HC-05 Bluetooth Module
- ESP32 Module (for SMS alerts)

## Modes of Operation
- Autonomous Mode: Detects and fights fire automatically
- Manual Mode: Bluetooth remote control via mobile app

## Key Features
- Real-time fire, smoke, and obstacle detection
- Auto-targeted fire suppression using water spray
- Sends emergency SMS via ESP32
- Obstacle avoidance and path correction

## Results
- Flame detection accuracy: 94%
- Smoke detection accuracy: 92%
- Obstacle avoidance: 96%
- Fire extinguishing success: 90%
- Response time: 2.5 seconds

## Conclusion
A robust firefighting solution for hazardous environments, improving safety through automation and alert systems.

## Note:
Create a twilio account and get twilio number and load the twilio code into ESP32.
