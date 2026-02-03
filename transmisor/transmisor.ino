#include <Wire.h>
#include <SPI.h>
#include <RF24.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// --- Config ---
// NRF24L01 Pins (CE, CSN)
RF24 radio(15, 14); 
const byte address[6] = "node1";

Adafruit_MPU6050 mpu;

struct Payload {
  float timestamp;
  float ax, ay, az;
  float gx, gy, gz;
};

void setup() {
  Serial.begin(115200);
  
  // Inicializa MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) { delay(10); }
  }
  Serial.println("MPU6050 Found!");

  // Inicializa NRF24L01
  if (!radio.begin()) {
    Serial.println("NRF24L01 hardware not responding!");
    while (1) {}
  }
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();

  Serial.println("Transmitter starting...");
}

void loop() {
  unsigned long start_time = millis();
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  Payload data;
  data.timestamp = start_time / 1000.0;
  data.ax = a.acceleration.x;
  data.ay = a.acceleration.y;
  data.az = a.acceleration.z;
  data.gx = g.gyro.x;
  data.gy = g.gyro.y;
  data.gz = g.gyro.z;

  bool report = radio.write(&data, sizeof(data));

  if (report) {
    Serial.print("Sent at ");
    Serial.print(start_time);
    Serial.print("ms: Ax=");
    Serial.println(data.ax);
  } else {
    Serial.println("Transmission failed (No ACK)");
  }

  unsigned long elapsed = millis() - start_time;
  if (elapsed < 100) {
    delay(100 - elapsed);
  }
}
