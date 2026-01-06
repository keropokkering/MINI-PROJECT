# wiring digram insturction for ESP-32 , BME sensor , Ultrsonic Sensor

This diagram shows how an ESP32 development board is wired to two sensors:

BME280 sensor (temperature, humidity, pressure)

HC-SR04 ultrasonic sensor (distance measurement)

I’ll explain each part and the connections step by step.

1️⃣ ESP32 (main controller)

This is the microcontroller in the middle.

It powers the sensors, reads their data, and can send results over Wi-Fi/Bluetooth.

2️⃣ BME280 sensor (top-right, purple board)

This sensor uses I²C communication.

Pins shown:

VIN → Power

GND → Ground

SCL → I²C Clock

SDA → I²C Data

Connections to ESP32:

VIN → 3.3V on ESP32

GND → GND

SCL → ESP32 SCL pin (commonly GPIO 22)

SDA → ESP32 SDA pin (commonly GPIO 21)

🔹 Purpose: Measures temperature, humidity, and air pressure

3️⃣ HC-SR04 Ultrasonic Sensor (right, blue board)

This sensor measures distance using sound waves.

Pins:

VCC → Power

Trig → Trigger signal (sent from ESP32)

Echo → Echo signal (received by ESP32)

GND → Ground

Connections to ESP32:

VCC → 5V (or VIN) on ESP32

GND → GND

Trig → GPIO pin (any digital pin)

Echo → GPIO pin (any digital pin)

⚠️ Important note:
The Echo pin outputs 5V, but ESP32 GPIOs are 3.3V only.
In real projects, you should use a voltage divider on Echo to protect the ESP32.

🔹 Purpose: Measures distance (e.g., obstacle detection)

4️⃣ Wire colors (just visual help)

Red → Power (3.3V or 5V)

Black → Ground

Yellow / Blue → Data or signal lines

Colors don’t change function electrically—they just help humans read the diagram.

🧠 What this setup does overall

The ESP32:

Reads environment data from the BME280

Measures distance using the ultrasonic sensor

Can display, log, or send data wirelessly
