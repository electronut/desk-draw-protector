// Include the ESP8266 WiFi library. (Works a lot like the
// Arduino WiFi library.)
#include <ESP8266WiFi.h>

extern "C" {
#include "gpio.h"
#include "user_interface.h"
}

/*
  mysettings.h

  You need to create this file. Contents looks like:

  const char WiFiSSID[] = "xyz";
  const char WiFiPSK[] = "abc";
  char MakerIFTTT_Key[] = "kkkkkkkk";

*/
#include "mysettings.h"

// enable this for debugging
// #define ENABLE_SERIAL_DEBUG

// pins
const int LED_PIN = 5; // Thing's onboard, green LED - LOW turns on the LED
const int LDR_PIN = 12;
const int BUZZER_PIN = 14;

// IFTTT event name
char MakerIFTTT_Event[] = "desk_drawer_open";

// POST string
String postReqStr;

void initHardware()
{

#ifdef ENABLE_SERIAL_DEBUG
  Serial.begin(9600);
#endif

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(LDR_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
}

void setup()
{
  initHardware();
  // prepare POST request string
  postReqStr += "POST /trigger/";
  postReqStr += MakerIFTTT_Event;
  postReqStr += "/with/key/";
  postReqStr += MakerIFTTT_Key;
  postReqStr += " HTTP/1.1\r\n";
  postReqStr += "Host: maker.ifttt.com\r\n";
  postReqStr += "Content-Type: application/json\r\n";
  postReqStr += "Content-Length:  0";
  postReqStr += "\r\n";
  postReqStr += "\r\n";
}

void loop()
{
  // leave LED on
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  // leave LED on
  digitalWrite(LED_PIN, HIGH);

  // awake here...

  // more annoying noises
  makeNoise();

  // connect to WiFi
  delay(200);
  connectWiFi();
  delay(200);

  // this yield is critical
  yield();

  // post to IFTTT
  postToIFTTT();

  // go to deep sleep
#ifdef ENABLE_SERIAL_DEBUG
  Serial.println("going to sleep...");
#endif

  ESP.deepSleep(0);
  yield();
}

// create some havoc with the buzzer
void makeNoise()
{
  analogWrite(BUZZER_PIN, 128);

  // play Jaws music
  for (int i = 0; i < 10; i++) {
    // E6
    analogWriteFreq(1318);
    delay(300);
    // F6
    analogWriteFreq(1397);
    delay(300);
  }
  analogWrite(BUZZER_PIN, 0);
}

void connectWiFi()
{
  byte ledStatus = LOW;

  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);

  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  // make N attempts
  int NA = 10;
  int attempts = 0;
  while ((WiFi.status() != WL_CONNECTED) && attempts++ < NA)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;

    yield();
    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }

  // leave LED on
  digitalWrite(LED_PIN, LOW);
}

int postToIFTTT()
{

#ifdef ENABLE_SERIAL_DEBUG
  Serial.println("Posting...");
#endif

  // LED turns on when we enter, it'll go off when we
  // successfully post.
  digitalWrite(LED_PIN, HIGH);


  // Now connect to IFTTT
  WiFiClient client;
  if (!client.connect("maker.ifttt.com", 80))
  {
    // If we fail to connect, return 0.
    return 0;
  }

#ifdef ENABLE_SERIAL_DEBUG
    Serial.print(postReqStr);
#endif

    // finally we are ready to send the POST to the server!
    client.print(postReqStr);
    client.stop();

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');

#ifdef ENABLE_SERIAL_DEBUG
    Serial.print(line); // Trying to avoid using serial
#endif

  }

  // Before we exit, turn the LED off.
  digitalWrite(LED_PIN, LOW);

  return 1; // Return success
}
