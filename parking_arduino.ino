#include <Arduino.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

// const char *ssid = "ART-NET_001048 5G";
// const char *password = "gorsimonyan";

// const char *ssid = "ALBATROS";
// const char *password = "12345679";

const char *ssid = "Gor's Galaxy A32";
const char *password = "chem asi";

const int servoPin = D1;

const int echoPin = D4;
const int trigPin = D3;

const char *check_url = "https://parking-server.simonyan-gor-397.workers.dev/?parking_id=q7uZRbKE5BMEBpEsbaOK&status=0&secret=920ef64e081d6c8d5861db5a34919506";
const char *set_status_busy_url = "https://parking-server.simonyan-gor-397.workers.dev/?parking_id=q7uZRbKE5BMEBpEsbaOK&status=2&secret=920ef64e081d6c8d5861db5a34919506";
const char *set_status_free_url = "https://parking-server.simonyan-gor-397.workers.dev/?parking_id=q7uZRbKE5BMEBpEsbaOK&status=3&secret=920ef64e081d6c8d5861db5a34919506";

Servo servo;

void servoDown() {
  servo.write(180);
}

void servoUp(int delay_sec) {
  delay(delay_sec * 1000);
  if (!checkDistance()) {
    servo.write(0);
  } else {
    servoUp(delay_sec);
  }
}

bool checkDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(20);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);

  long distance_cm = duration * 0.0343 / 2;

  Serial.print("Distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");

  return (distance_cm < 70);
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Hellooooooooooo!");

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // pinMode(LED_BUILTIN, OUTPUT);
  servo.attach(servoPin);
  Serial.println("servoUp");
  servoUp(1);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Print local IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    // Ignore SSL certificate validation
    client->setInsecure();

    //create an HTTPClient instance
    HTTPClient https;

    //Initializing an HTTPS communication using the secure client
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, check_url)) {  // HTTPS

      Serial.print("[HTTPS] GET... ");
      Serial.print(check_url);
      Serial.println();

      int httpCode = https.GET();

      if (httpCode > 0) {
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        String payload = https.getString();
        Serial.println(payload);
        if (httpCode == HTTP_CODE_OK && payload == "1") {
          servoDown();
          for (int i = 0; i <= 120; i++) {
            if (checkDistance()) {
              delay(300);
              if (!checkDistance()) {
                delay(200);
                if (!checkDistance()) {
                  continue;
                };
              }
              servoUp(3.5);

              HTTPClient set_busy_https;
              set_busy_https.begin(*client, set_status_busy_url);

              if (set_busy_https.GET() == 200) {
                Serial.println("Status is set to 'busy' (2) ");
              }
              set_busy_https.end();
              break;
            } else if (i == 120) {
              servoUp(1);
            }
            delay(500);
          }
        } else if (httpCode == HTTP_CODE_OK && payload == "-1") {
          HTTPClient set_status_free_https;
          set_status_free_https.begin(*client, set_status_free_url);

          if (set_status_free_https.GET() == 200) {
            Serial.print(set_status_free_https.getString());
          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  } else {
    Serial.println("No Wifi connection !");
  }

  Serial.println();
  int delay_time = 5;
  Serial.print("Waiting ");
  Serial.print(delay_time);
  Serial.print(" seconds before the next round...");
  delay(delay_time * 1000);
}