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

//#define ENABLE_SERIAL_DEBUG

/////////////////////
// Pin Definitions //
/////////////////////
const int LED_PIN = 5; // Thing's onboard, green LED - LOW turns on the LED
const int LDR_PIN = 12;
const int BUZZER_PIN = 14;

/////////////////
// Post Timing //
/////////////////
const unsigned long postRate = 5000;
unsigned long lastPost = 0;

void setup()
{
  initHardware();
  //connectWiFi();
}

void loop()
{
  // as soon as you enter the loop, go to sleep
  // set up so that you wake up on a pin change interrupt
#ifdef ENABLE_SERIAL_DEBUG
  Serial.println("going to light sleep...");
#endif

#ifdef ENABLE_SERIAL_DEBUG
  Serial.println("woke up...");
#endif

  // LED off
  digitalWrite(LED_PIN, HIGH);

  // disconnect
  wifi_station_disconnect();
  wifi_set_opmode(NULL_MODE);

  // go to light sleep
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  wifi_fpm_open();
  gpio_pin_wakeup_enable(GPIO_ID_PIN(LDR_PIN), GPIO_PIN_INTR_LOLEVEL);
  wifi_fpm_do_sleep(0xFFFFFFF);

  // woken up by unterrupt...
  delay(200);
  // disable interrupt
  gpio_pin_wakeup_disable();

  // connect to WiFi
  delay(200);
  connectWiFi();
  delay(200);

  // this yield is critical
  yield();

  // post to IFTTT
  postToIFTTT();

  // more annoyng noises
  makeNoise();
}

// create some havoc with the buzzer
void makeNoise()
{
  analogWrite(BUZZER_PIN, 128);

  for (int i = 0; i < 10; i++) {
    analogWriteFreq(1318);
    delay(300);
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
  while (WiFi.status() != WL_CONNECTED)
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

char MakerIFTTT_Event[] = "desk_drawer_open";

// helper functions for constructing the POST data
// append a string or int to a buffer, return the resulting end of string

char *append_str(char *here, char *s) {
    while (*here++ = *s++)
	;
    return here-1;
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

// construct the POST request
    char post_rqst[256];    // hand-calculated to be big enough

    char *p = post_rqst;
    p = append_str(p, "POST /trigger/");
    p = append_str(p, MakerIFTTT_Event);
    p = append_str(p, "/with/key/");
    p = append_str(p, MakerIFTTT_Key);
    p = append_str(p, " HTTP/1.1\r\n");
    p = append_str(p, "Host: maker.ifttt.com\r\n");
    p = append_str(p, "Content-Type: application/json\r\n");
    p = append_str(p, "Content-Length: 0");

    // we need to remember where the content length will go, which is:
    char *content_length_here = p;

    // it's always two digits, so reserve space for them (the NN)
    p = append_str(p, "\r\n");

    // end of headers
    p = append_str(p, "\r\n");


#ifdef ENABLE_SERIAL_DEBUG
    Serial.print(post_rqst);
#endif

    // finally we are ready to send the POST to the server!
    client.print(post_rqst);
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
