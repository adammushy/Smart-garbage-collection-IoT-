#include "Arduino.h"
#include "heltec.h"
#include "WiFi.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>



#include <HardwareSerial.h>



#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#define WIFI_SSID "Mushi Home"
#define WIFI_PASSWORD "20002000"

#include <addons/TokenHelper.h>

/* 2. Define the API Key */
#define API_KEY "AIzaSyB7s-Lzzk6IEV6XC3Y6Ojte4hTMGV7JpcM"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "smart-garbage-collection-8fb03"

/* 4. Define the user Email and password that alreadey registerd or added in your project */

#define USER_EMAIL "esp8266@gmail.com"
#define USER_PASSWORD "123456"



// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Define the GPS
double Longitude = 39.209917, Latitude =-6.722554 ;

// -6.722554, 39.209917

unsigned long dataMillis = 0;
const int trigPin = 12;
const int echoPin = 14;
// define speed 
#define SOUND_SPEED 0.034



long duration;
float distanceCm;
String state;
int value;
String name = "Bin test";

bool taskcomplete = false;
//define sound speed in cm/uS
#define SOUND_SPEED 0.034
int pulsation=400;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Starts the serial communication
    // ss.begin(GPSBaud);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Serial.println("user is authenticated ");

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

#if defined(ESP8266)
    // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
#endif



    fbdo.setResponseSize(2048);

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);


  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  // Prints the distance in the Serial Monitor

  if (distanceCm <=10){
    value=100;
    state="full";  
  }else if (distanceCm > 10 && distanceCm <=20){
  state="almost full"; 
  value=75;  
  }else if(distanceCm > 20 && distanceCm <=30){
    value=50;
    state="Medium"; 
  } else if(distanceCm > 30 && distanceCm <=40){
    value=25;
    state="Low"; 
  }else if(distanceCm > 40){
    value=0;
    state="empty";
  }
  Serial.print("Distance (cm): ");
  Serial.println(int(distanceCm));
  Serial.println("state of trash ");
  
  Serial.println(state);

  delay(1000);


    if (Firebase.ready() && (millis() - dataMillis > 15000 || dataMillis == 0))
        {
            dataMillis = millis();

            // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
            FirebaseJson content;

            // aa is the collection id, bb is the document id.
            // String documentPath = "data/0k3LWgHIV4Zs4QgQ0Orc/";
            String documentPath = "data/0k3LWgHIV4Zs4QgQ0rrr/";

            // String documentPath = "data/"+"0k3LWgHIV4Zs4QgQ0Orc"+"";


            // If the document path contains space e.g. "a b c/d e f"
            // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

            if (!taskcomplete)
            {
                taskcomplete = true;

  
                content.clear();

                content.set("fields/Latitude/doubleValue", Latitude);
                content.set("fields/Longitude/doubleValue", Longitude);
                content.set("fields/state/stringValue", state);
                content.set("fields/percentage/integerValue", value);
                content.set("fields/name/stringValue", name);

                Serial.print("Create a document... ");

                Serial.print("Create a document... ");

                if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()  ))
                    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
                else
                    Serial.println(fbdo.errorReason());
            }

                content.clear();
                content.set("fields/Latitude/doubleValue", Latitude);
                content.set("fields/Longitude/doubleValue", Longitude);
                content.set("fields/state/stringValue", state);
                content.set("fields/percentage/integerValue", value);
                content.set("fields/name/stringValue", "bin test");

                Serial.print("Update a Smart... ");

            /** if updateMask contains the field name that exists in the remote document and
            * this field name does not exist in the document (content), that field will be deleted from remote document
            */

            if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw(), "name,state,Latitude,Longitude,percentage" /* updateMask */))
                Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
            else
                Serial.println(fbdo.errorReason());
        }


}
