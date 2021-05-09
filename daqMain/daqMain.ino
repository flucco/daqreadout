/* DAQ MAIN CODE
 *  subroutines: analogSensors, digitalSensors, saveAndCompile
 */
/* TODO
 *  pull data from I2C bus
 *  pull data from MOTEC/CAN
 *  write functions for manipulating data:
 *    Non testing:
 *      lambdas, wheel speed (DIG) , brake temp, shock potent, brake pressure, throttle pos
 *      intake manifold ap, intake manifold at, oil pressure, cam angle (DIG)
 *      coolant temp, oil temp, crank angle
 *    Testing:
 *      steering angle, pitot tubes, strain gauges, accelerometer, gyroscope, thermocouple
 */

#include "SD.h"
#include"SPI.h"
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <SoftwareSerial.h>
#include <can.h>
#include <global.h>
#include <Canbus.h>
#include <mcp2515_defs.h>
#include <mcp2515.h>
#include <defaults.h>

//#define TEST_MODE
#ifdef TEST_MODE
    #define Sprintln(a) (Serial.println(a))
#else
   #define Sprintln(a)
#endif


SoftwareSerial xbee(0,1);

Adafruit_ADS1115 ads1115a(0x48);
Adafruit_ADS1115 ads1115b(0x49);
Adafruit_ADS1115 ads1115c(0x4A);
//GLOBALS
const int CSpin = 49;
String dataString =""; // holds the data to be written to the SD card
String fileName;
File sensorData;
int readFrequency = 1000; // CHANGE THIS TO READ FASTER/SLOWER (units in ms)

// leds for detecting sd card
int led_r = 39;
int led_g = 40;
int led_y = 41;

unsigned long previousTimeDigital = millis();
unsigned long previousTimeAnalog = millis();
unsigned long currentTime;
unsigned long FL_VSS_LastRead,FR_VSS_LastRead, BL_VSS_LastRead, BR_VSS_LastRead, diff = millis();

// SENSOR ARRAYS
float allSensors[43];

// SENSOR GLOBALS
int sensorVoltage = 0;
int systemVoltage = 5;
int resolution = 1024;

// Wheel Speed
int wheelCirc = 3.24*2*8;
int wheelSpeed = 0;
int FL_VSS_PIN = 2;
int FR_VSS_PIN = 3;
float FL_VSS, FR_VSS, BL_VSS, BR_VSS;

// Brake Temperature
int FL_BRK_TMP_PIN = A0;
int FR_BRK_TMP_PIN = A1;
float FL_BRK_TMP, FR_BRK_TMP,BL_BRK_TMP,BR_BRK_TMP;

// Suspension Potentiometer
int FL_SUS_POT_PIN = A2;
int FR_SUS_POT_PIN = A3;
float FL_SUS_POT, FR_SUS_POT, BL_SUS_POT, BR_SUS_POT;

// Brake Pressure
int F_BRK_PRES_PIN = A4;
float F_BRK_PRES = 0;
float B_BRK_PRES = 0;

// Steer Angle
int STEER_ANG_PIN = A5;
float STEER_ANG = 0;

// Rest of Motec Reads
float TPS, OIL_PRES,OIL_TEMP,COOL_TEMP, MAP, MAT, NEUT, LAMBDA1, LAMBDA2;

//  Rest of ADC reads
float ACCEL = 0;
float GYRO = 0;
float GPS = 0;
float STRAIN1, STRAIN2, STRAIN3, STRAIN4;

// PTUBES
int PTUBE1_PIN = A6;
int PTUBE2_PIN = A7;
int PTUBE3_PIN = A8;
int PTUBE4_PIN = A9;
int PTUBE5_PIN = A10;
int PTUBE6_PIN = A11;
int PTUBE7_PIN = A12;
int PTUBE8_PIN = A13;
int PTUBE9_PIN = A14;
int PTUBE10_PIN = A15;
float PTUBE1, PTUBE2, PTUBE3, PTUBE4, PTUBE5, PTUBE6, PTUBE7, PTUBE8, PTUBE9, PTUBE10, PTUBE11, PTUBE12;


// OFFSETS
float PTUBE_CLB, STRAIN1_CLB, STRAIN2_CLB, STRAIN3_CLB, STRAIN4_CLB, STEER_ANG_CLB, TPS_CLB, F_BRK_PRES_CLB, B_BRK_PRES_CLB, FL_SUS_POT_CLB, FR_SUS_POT_CLB, BL_SUS_POT_CLB, BR_SUS_POT_CLB;


float convertSensor(int sensorValue, int calibration=0);
// sensor value from 0 to 2^16 and returns a voltage between 0 and 5 V
float convertSensor(int sensorValue, int calibration=0){
  float sensorVoltage = (sensorValue * ((float)systemVoltage/resolution)) - calibration;
  return sensorVoltage;
}
float digitalReading = 0;

float fakeAllSensors[3]={digitalReading, 2, 100};


void fakeDigitalSensors(){
  digitalReading = digitalReading + 1.0;
  fakeAllSensors[0] = digitalReading;
}

void fakeAnalogSensors(){
  for (int i = 1; i < sizeof(fakeAllSensors)/sizeof(float); i = i + 1) {
    fakeAllSensors[i] = fakeAllSensors[i]+2;
    }
  };

void setup() {
  // Open serial communications
  //Serial.begin(9600);
  Sprintln("Initializing SD card...");
  xbee.begin(9600);
  can_setup();
  pinMode(CSpin, OUTPUT);
  //

  // declare led pins as outputs
  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(led_y, OUTPUT);

  // see if the card is present and can be initialized
  if (!SD.begin(CSpin)) {
  Sprintln("Card failed/not found");
  digitalWrite(led_r, LOW);
  digitalWrite(led_y, HIGH);
  digitalWrite(led_g, HIGH);
  // stop
  return;
  }
  Sprintln("card initialized.");

  pinMode(FL_VSS_PIN, INPUT);
  pinMode(FR_VSS_PIN, INPUT);

  //   get current version for file name and then update version
  int dataVer;
  File sensorDataVer = SD.open("VerTrack.txt");
  if (sensorDataVer) {
        dataVer = sensorDataVer.read();
  } else {
     Sprintln("File unavailable");
    digitalWrite(led_r, LOW);
    digitalWrite(led_y, LOW);
    digitalWrite(led_g, HIGH);
    delay(2000);
  }
  sensorDataVer.close();
  sensorDataVer = SD.open("VerTrack.txt", O_RDWR);
  sensorDataVer.write(dataVer+1);
  sensorDataVer.close();
  fileName = "data" + String(dataVer) + ".csv";
 
  // write headers
  sensorData = SD.open(fileName, FILE_WRITE);
  if (sensorData){
      sensorData.println("FL_VSS,FR_VSS,BL_VSS,BR_VSS,FL_BRK_TMP,FR_BRK_TMP,BL_BRK_TMP,BR_BRK_TMP,FL_SUS_POT,FR_SUS_POT,BL_SUS_POT,BR_SUS_POT,F_BRK_PRES,B_BRK_PRES,STEER_ANG,TPS,OIL_PRES,OIL_TEMP,COOL_TEMP,MAP,MAT,NEUT,LAMBDA1,LAMBDA2,ACCEL,GYRO,GPS,STRAIN1,STRAIN2,STRAIN3,STRAIN4,PTUBE1,PTUBE2,PTUBE3,PTUBE4,PTUBE5,PTUBE6,PTUBE7,PTUBE8,PTUBE9,PTUBE10,PTUBE11,PTUBE12");
      sensorData.close(); // close the file
    } else {
    Sprintln("Error writing to file !");
    digitalWrite(led_r, LOW);
    digitalWrite(led_y, LOW);
    digitalWrite(led_g, HIGH);
    delay(2000);
  }

  // set calibration variables ** these are in voltage
  FL_SUS_POT_CLB = convertSensor(analogRead(FL_SUS_POT_PIN));
  FR_SUS_POT_CLB = convertSensor(analogRead(FR_SUS_POT_PIN));
  //  BL_SUS_POT_CLB = back l suspot
  //  BR_SUS_POT_CLB = back r suspot
  F_BRK_PRES_CLB = convertSensor(analogRead(F_BRK_PRES_PIN));
  //  B_BRK_PRES_CLB = back brk pres
  STEER_ANG_CLB = convertSensor(analogRead(STEER_ANG_PIN));
  //  TPS_CLB = TPS
  STRAIN1_CLB = convertSensor(ads1115b.readADC_Differential_0_1());
  STRAIN2_CLB = convertSensor(ads1115b.readADC_Differential_2_3());
  STRAIN3_CLB = convertSensor(ads1115c.readADC_Differential_0_1());
  STRAIN4_CLB = convertSensor(ads1115c.readADC_Differential_2_3());
  PTUBE_CLB = convertSensor(analogRead(PTUBE1_PIN));
  digitalWrite(led_r, HIGH);
  digitalWrite(led_y, HIGH);
  digitalWrite(led_g, LOW);
}


void loop() {
  currentTime = millis();

//  run checks for digital sensors every single loop, check for reading of 0
  fakeDigitalSensors();
//  check for analog reading every second
//  frequency of change of data
  if (currentTime - previousTimeAnalog > readFrequency){
    previousTimeAnalog = currentTime;
    compileCurData();
    saveData();
    // for now write to xbee every second, may shorten interval
    writeXbee();
  }
}
