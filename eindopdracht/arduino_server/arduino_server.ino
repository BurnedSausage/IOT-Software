#include "Arduino.h"
#include "DFRobot_VL53L0X.h"
#include "Wire.h"
#include <Ethernet.h>

extern "C" {
#include "cserver.h"
}

// unique MAC address, correct IP address!
byte mac[] = {0xDE, 0xAD, 0xDE, 0xEF, 0xFE, 0xED};
IPAddress ip(10, 1, 1, 12);
EthernetServer server(80);
EthernetClient httpClient;

// Pin deffenitions
const short interruptPin = 2;
const short ldr = A0;
const short green[] = {9, 8};
const short red[] = {7, 6};
const short yellow[] = {5, 3};

// Distance sensor
DFRobot_VL53L0X sensor;

// Other vars
bool firstBoot = 0;
unsigned long long readSensorsT = 0;
unsigned long long receivedMessage = 0;
unsigned long long interruptedT = 0;
bool passive = 1;
bool interrupted = 0;

// make httpClient methods available as ordinary functions
int clientAvailable() {
  return httpClient.connected() && httpClient.available();
}
char clientRead() { return httpClient.read(); }
char clientPeek() { return httpClient.peek(); }

void setup() {
  Serial.begin(9600);

  // Server setup
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  Serial.println(Ethernet.hardwareStatus());

  // Pin setup
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), isr,
                  RISING);
  pinMode(ldr, INPUT);
  for (int i = 0; i < 2; i++) {
    pinMode(green[i], OUTPUT);
    pinMode(red[i], OUTPUT);
    pinMode(yellow[i], OUTPUT);
  }

  // tof setup
  Wire.begin();
  sensor.begin(0x50);
  sensor.setMode(sensor.eContinuous, sensor.eHigh);
  sensor.start();
}

void loop() {
  // Call reset when intterupted
  if (interrupted) {
    resetCB();
    interrupted = 0;
  }

  // Check if buffers are init
  if (!checkBufferInit()) {
    for (int i = 0; i < 2; i++) {
      digitalWrite(red[i], HIGH);
      digitalWrite(yellow[i], HIGH);
      digitalWrite(green[i], HIGH);
    }
  } else if (!firstBoot) {
    for (int i = 0; i < 2; i++) {
      digitalWrite(red[i], LOW);
      digitalWrite(yellow[i], LOW);
      digitalWrite(green[i], LOW);
    }
    firstBoot = 1;
  }

  // Turn buffer lights red when full
  for (int i = 0; i < 2; i++) {
    digitalWrite(red[i], bufferFull(i));
  }

  httpClient = server.available();
  if (httpClient) {
    Serial.println("new client");
    receivedMessage = millis();
    digitalWrite(yellow[0], HIGH);

    struct stream stream = {clientAvailable, clientPeek,
                            clientRead};

    struct response res = handleRequest(stream);

    switch (res.code) {
    case BAD_REQUEST_400:
      Serial.println("HTTP/1.0 400 BAD REQUEST");
      httpClient.println("HTTP/1.0 400 BAD REQUEST");
      break;
    case NOT_FOUND_404:
      Serial.println("HTTP/1.0 404 NOT FOUND");
      httpClient.println("HTTP/1.0 404 NOT FOUND");
      break;
    case CREATED_201_PUT_MODE_ACTIVE:
      Serial.println("HTTP/1.0 201 MODE OK");
      httpClient.println("HTTP/1.0 201 MODE OK");
      passive = 0;
      break;
    case CREATED_201_PUT_MODE_PASSIVE:
      Serial.println("HTTP/1.0 201 MODE OK");
      httpClient.println("HTTP/1.0 201 MODE OK");
      passive = 1;
      break;
    case CREATED_201_DELETE_MEASUREMENTS:
      Serial.println("HTTP/1.0 201 DELETE OK");
      httpClient.println("HTTP/1.0 201 DELETE OK");
      break;
    case CREATED_201_PUT_CBUFFSIZE:
      Serial.println("HTTP/1.0 201 BUFFERSIZE OK");
      httpClient.println("HTTP/1.0 201 BUFFERSIZE OK");
      break;
    case OK_200_GET_AVG:
      Serial.println("HTTP/1.0 200 GET AVG: ");
      Serial.println("");
      Serial.println(res.get_avg);
      httpClient.println("HTTP/1.0 200 GET AVG: ");
      httpClient.println("");
      httpClient.println(String(res.get_avg, 2));
      break;
    case OK_200_GET_STDEV:
      Serial.println("HTTP/1.0 200 GET STDEV: ");
      Serial.println("");
      Serial.println(res.get_stdev);
      httpClient.println("HTTP/1.0 200 GET STDEV: ");
      httpClient.println("");
      httpClient.println(String(res.get_stdev, 2));
      break;
    case OK_200_GET_ACTUAL:
      Serial.println("HTTP/1.0 200 GET ACTUAL: ");
      Serial.println("");
      Serial.println(res.get_actual);
      httpClient.println("HTTP/1.0 200 GET ACTUAL: ");
      httpClient.println("");
      httpClient.println(String(res.get_actual, 2));
      break;
    case CREATED_201_POST_MEASUREMENT:
      Serial.println("HTTP/1.0 201 POST OK");
      httpClient.println("HTTP/1.0 201 POST OK");
      break;
    default:
      Serial.println("Something went wrong parsing");
      httpClient.println(
          "HTTP/1.0 200 INTERNAL SERVER ERROR");
      break;
    }
    httpClient.stop(); // close connection
    Serial.println("client disconnected");
  }
  // All timed stuff
  // Read sensors and putt in buffer
  if (readSensorsT + 100 <= millis() && !passive) {
    Serial.println("added values to buffer");
    readSensorsT = millis();
    addValues(analogRead(ldr), sensor.getDistance());
  }
  // Turn yellow off after half a second of receivingMessage
  if (receivedMessage + 500 <= millis() &&
      receivedMessage != 0) {
    digitalWrite(yellow[0], LOW);
    receivedMessage = 0;
  }
  // Turn green off after half a second of intterupt
  if (interruptedT + 500 <= millis() && interruptedT != 0) {
    digitalWrite(green[0], LOW);
    interruptedT = 0;
  }
}

void isr() {
  interruptedT = millis();
  digitalWrite(green[0], HIGH);
  interrupted = 1;
}
