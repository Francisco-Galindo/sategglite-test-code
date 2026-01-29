// Prueba de sensores para el sategglite

#include <SoftwareSerial.h>
#include <TinyGPS.h>

/* #include "FS.h" */
/* #include "SD.h" */
/* #include "SPI.h" */

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_HMC5883_U.h>


/*
 * ===================================================================
 * LEER
 * Los sensores (exceptuando el GPS) se conectan en paralelo:
 * - TODOS los pines SDA se conectan al pin SDA del ESP32
 * - TODOS los pines SCL se conectan al pin SCL del ESP32
 * - TODAS las tierras se conectan juntas (al carril azul)
 * - TODOS los pines VCC van a 3.3V (el carril rojo de)
 * ===================================================================
 */


// Estos pines se puede modificar si no hacen sentido con la placa específica
#define UART_TX 43 // Acá va el RX del GPS (el RX del GPS es el TX del ESP)
#define UART_RX 44 // Acá va el TX del GPS
#define SDA_PIN 5
#define SCL_PIN 6


TinyGPS gps;
SoftwareSerial serialgps(UART_RX, UART_TX);

Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;
unsigned long lastTemRead = 0;

int year;
byte month, day, hour, minute, second, hundredths;
unsigned long chars;
unsigned short sentences, failed_checksum;

unsigned long lastCaptureTime = 0;

Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_temp, *mpu_accel, *mpu_gyro;
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
/* Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085); */
unsigned long lastMPURead = 0;

bool mpu_valid = true;
bool mag_valid = true;
bool aht_valid = true;
bool bmp_valid = true;


void setup() {
  Serial.begin(115200);
  serialgps.begin(9600);


  Wire.begin(SDA_PIN, SCL_PIN);
  
  if (!bmp.begin(0x77)) {
    Serial.println(F("No se encuentra un sensor BMP280 compatible, revisa la conexión"));
    bmp_valid = false;
  } else {
    Serial.println(F("BMP280 OK"));
    /* Configuración default según el datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Modo de Operación. */
		    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
		    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
		    Adafruit_BMP280::FILTER_X16,      /* Filtrado. */
		    Adafruit_BMP280::STANDBY_MS_500); /* Tiempo de Standby. */

  }


  if (!aht.begin()) {
    Serial.println("No se ha podido encontrar el módulo AHT20");
    aht_valid = false;
  } else {
    Serial.println(F("AHT20 OK"));
  }
  

  if (!mpu.begin()) { Serial.println("MPU6050 falla! Checa conexión"); mpu_valid = false; }
  if (!mag.begin()) { Serial.println("HMC5883 falla! Checa conexión"); mag_valid = false; }


  if (mpu_valid) {
    mpu_accel = mpu.getAccelerometerSensor();
    mpu_accel->printSensorDetails();

    mpu_gyro = mpu.getGyroSensor();
    mpu_gyro->printSensorDetails();
  }


}

void loop() {
  unsigned long now = millis();
  bool newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;) {
    while (serialgps.available()) {
      char c = serialgps.read();
      Serial.write(c);
      if (gps.encode(c))
	newData = true;
    }
  }
  Serial.println();
  if (newData) {
    float latitude, longitude;
    gps.f_get_position(&latitude, &longitude);
    Serial.print("Latitud/Longitud: "); 
    Serial.print(latitude,5); 
    Serial.print(", "); 
    Serial.println(longitude,5);


    gps.crack_datetime(&year,&month,&day,&hour,&minute,&second,&hundredths);
    Serial.print("Fecha: "); Serial.print(day, DEC); Serial.print("/"); 
    Serial.print(month, DEC); Serial.print("/"); Serial.print(year);
    Serial.print(" Hora: "); Serial.print(hour, DEC); Serial.print(":"); 
    Serial.print(minute, DEC); Serial.print(":"); Serial.print(second, DEC); 
    Serial.print("."); Serial.println(hundredths, DEC);
    Serial.print("Altitud (metros): ");
    Serial.println(gps.f_altitude()); 
    Serial.print("Rumbo (grados): "); Serial.println(gps.f_course()); 
    Serial.print("Velocidad(kmph): ");
    Serial.println(gps.f_speed_kmph());
    Serial.print("Satelites: "); Serial.println(gps.satellites());
    Serial.println();
    gps.stats(&chars, &sentences, &failed_checksum);  
  }


  if ((bmp_valid || aht_valid) && now - lastTemRead >= 3000) {

    float pressure = bmp.readPressure();
    sensors_event_t humidity, temp;

    aht.getEvent(&humidity, &temp);
    Serial.println("===== Lecturas de temperatura, humedad y presion====");
    if (aht_valid) {
	Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
	Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
    }
    if (bmp_valid) {
      Serial.print("Presion: "); Serial.print(pressure); Serial.println(" Pa");
    }
    Serial.println();

    lastTemRead = now;
  }

  if (mpu_valid && mag_valid && now - lastMPURead >= 1000) {
    sensors_event_t accel;
    sensors_event_t gyro;
    mpu_accel->getEvent(&accel);
    mpu_gyro->getEvent(&gyro);

    Serial.println("===== Lecturas de MPU ====");
    Serial.print("Accel X: ");
    Serial.print(accel.acceleration.x);
    Serial.print(" \tY: ");
    Serial.print(accel.acceleration.y);
    Serial.print(" \tZ: ");
    Serial.print(accel.acceleration.z);
    Serial.println(" m/s^2 ");

    Serial.print("Gyro X: ");
    Serial.print(gyro.gyro.x);
    Serial.print(" \tY: ");
    Serial.print(gyro.gyro.y);
    Serial.print(" \tZ: ");
    Serial.print(gyro.gyro.z);
    Serial.println(" radians/s ");
  
    sensors_event_t event;
    mag.getEvent(&event);
    Serial.print("Mag X: "); Serial.print(event.magnetic.x);
    Serial.print(" \tY: "); Serial.print(event.magnetic.y);
    Serial.print(" \tZ: "); Serial.println(event.magnetic.z);
    Serial.println();

    /* // BMP180 data */
    /* sensors_event_t bmp_event; */
    /* bmp.getEvent(&bmp_event); */
    /* Serial.print("Pressure: "); Serial.print(bmp_event.pressure); */
    /* Serial.println(" hPa"); */

    lastMPURead = now;
  }
}
