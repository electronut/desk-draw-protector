// Include the ESP8266 WiFi library. (Works a lot like the
// Arduino WiFi library.)
#include <ESP8266WiFi.h>

/*
  mysettings.h

  You need to create this file. Contents looks like:

  const char WiFiSSID[] = "xyz";
  const char WiFiPSK[] = "abc";
  char MakerIFTTT_Key[] = "kkkkkkkk";

*/
#include "mysettings.h"

/////////////////////
// Pin Definitions //
/////////////////////
const int LED_PIN = 5; // Thing's onboard, green LED
const int ANALOG_PIN = A0; // The only analog pin on the Thing
const int DIGITAL_PIN = 12; // Digital pin to be read

/////////////////
// Post Timing //
/////////////////
const unsigned long postRate = 5000;
unsigned long lastPost = 0;

void setup()
{
  initHardware();
  connectWiFi();
  digitalWrite(LED_PIN, HIGH);
}

void loop()
{
  if (lastPost + postRate <= millis())
  {
    if (postToIFTTT())
      lastPost = millis();
    else
      delay(100);
  }
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

    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
}

void initHardware()
{
  Serial.begin(9600);
  pinMode(DIGITAL_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  // Don't need to set ANALOG_PIN as input,
  // that's all it can be.
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
  Serial.println("Posting...");

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

#if 0
  //Serial.println("connected!");
  client.println("GET /trigger/desk_drawer_open/with?key=6zmfaOBei1DgdmlOgOi6C HTTP/1.0");
  //client.println("GET /search?q=arduino HTTP/1.0");
  client.println();
#endif


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

#if 0

    // construct the JSON; remember where we started so we will know len
    char *json_start = p;

    // As described - this example reports a pin, uptime, and "hello world"
    p = append_str(p, "{\"value1\":\"");
    p = append_ul(p, analogRead(READ_THIS_PIN));
    p = append_str(p, "\",\"value2\":\"");
    p = append_ul(p, millis());
    p = append_str(p, "\",\"value3\":\"");
    p = append_str(p, "hello, world!");
    p = append_str(p, "\"}");

    // go back and fill in the JSON length
    // we just know this is at most 2 digits (and need to fill in both)
    int i = strlen(json_start);
    content_length_here[0] = '0' + (i/10);
    content_length_here[1] = '0' + (i%10);
#endif

    Serial.print(post_rqst);
    // finally we are ready to send the POST to the server!
    client.print(post_rqst);
    client.stop();

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line); // Trying to avoid using serial
  }

  // Before we exit, turn the LED off.
  digitalWrite(LED_PIN, LOW);

  return 1; // Return success
}
