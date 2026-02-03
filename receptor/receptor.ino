#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// pines CE y CSN
RF24 radio(7, 8); 

const byte address[6] = "node1";

struct Payload {
  float timestamp;
  float ax, ay, az;
  float gx, gy, gz;
};

Payload data;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!radio.begin()) {
    Serial.println("Radio hardware not responding!");
    while (1);
  }

  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
  
  Serial.println("Time,Ax,Ay,Az,Gx,Gy,Gz");
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(data));

    Serial.print(data.timestamp); Serial.print(",");
    Serial.print(data.ax);        Serial.print(",");
    Serial.print(data.ay);        Serial.print(",");
    Serial.print(data.az);        Serial.print(",");
    Serial.print(data.gx);        Serial.print(",");
    Serial.print(data.gy);        Serial.print(",");
    Serial.println(data.gz);
  }
}
