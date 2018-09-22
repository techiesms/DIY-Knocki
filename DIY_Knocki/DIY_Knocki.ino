/*
   Code for DIY Knocki
   a project made by Sachin Soni for his YouTube Channel named "techiesms"

   Originally from Steve Hoefer http://grathio.com Version 0.1.10.20.10
   Updates by Lee Sonko

   Licensed under Creative Commons Attribution-Noncommercial-Share Alike 3.0
   http://creativecommons.org/licenses/by-nc-sa/3.0/us/
   (In short: Do what you want, just be sure to include this line and the four above it, and don't sell it or use it in anything you sell without contacting me.)
*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;

// Pin definitions
#define Green D4 // for green LED
#define Red D5   // for red LED
#define knockSensor A0  // for accelerometer

String Webhooks_key = "Your Webhooks' Key";

void Knock1_Action(String Event_name);
void Knock2_Action(String Event_name);
void Knock3_Action(String Event_name);
void Knock4_Action(String Event_name);
void Knock5_Action(String Event_name);
void Knock6_Action(String Event_name);


int threshold = 0;           // Minimum signal from the accelerometer to register as a knock

// Tuning constants.
const int rejectValue = 25;        // If an individual knock is off by this percentage of a knock we don't recognize.
const int averageRejectValue = 15; // If the average timing of the knocks is off by this percent we don't recognize.
const int knockFadeTime = 150;     // milliseconds we allow a knock to fade before we listen for another one. (Debounce timer.)

const int maximumKnocks = 20;       // Maximum number of knocks to listen for.
const int knockComplete = 1200;     // Longest time to wait for a knock before we assume that it's finished.

// Stored Knock Patterns
int secretCode1[maximumKnocks] = {100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // ..
int secretCode2[maximumKnocks] = {100, 90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // ...
int secretCode3[maximumKnocks] = {75, 20, 100, 72, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // . .. . ..
int secretCode4[maximumKnocks] = {25, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // ..   .
int secretCode5[maximumKnocks] = {29, 100, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // .. ..


int knockReadings[maximumKnocks];   // When someone knocks this array fills with delays between knocks.
int knockSensorValue = 0;           // Last reading of the knock sensor.
int programButtonPressed = false;   // Flag so we remember the programming button setting at the end of the cycle.


void setup()
{

  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  digitalWrite(Red, LOW);
  digitalWrite(Green, LOW);

//---------------------------------- Logic for calibration of surface
  digitalWrite(Red, HIGH);

  Serial.begin(115200);

  // Connect to WiFi network
  for (uint8_t t = 4; t > 0; t--)
  {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  for (int c = 0; c <= 20 ; c++)
  {
    int thresho = analogRead(knockSensor);
    threshold = thresho + threshold;
    delay(10);
  }

  threshold = (threshold / 20) - 2; 

  Serial.print("Thresold Value is : "); Serial.println(threshold);

  digitalWrite(Red, LOW);
  digitalWrite(Green, HIGH);
  delay(500);
  digitalWrite(Green, LOW);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("SSID", "PASS");
  WiFiMulti.addAP("SSID", "PASS");
  Serial.println("Program start.");

}

void loop()
{
  while (WiFiMulti.run() != WL_CONNECTED)
  {
    Serial.println("WiFi not Connected.");
    delay(100);
  }

  // Listen for any knock at all.
  knockSensorValue = analogRead(knockSensor);

  if (knockSensorValue >= threshold)
  {
    listenToSecretKnock();
  }
}

// Records the timing of knocks.
void listenToSecretKnock()
{
  Serial.println("knock starting");
  int i = 0;
  // First lets reset the listening array.
  for (i = 0; i < maximumKnocks; i++)
  {
    knockReadings[i] = 0;
  }

  int currentKnockNumber = 0;              // Incrementer for the array.
  int startTime = millis();                // Reference for when this knock started.
  int now = millis();

  delay(knockFadeTime);                                // wait for this peak to fade before we listen to the next one.

  do
  {
    //listen for the next knock or wait for it to timeout.
    knockSensorValue = analogRead(knockSensor);

    if (knockSensorValue >= threshold)                  //got another knock...
    {

      Serial.println("knock.");
      //record the delay time.
      now = millis();
      knockReadings[currentKnockNumber] = now - startTime;
      currentKnockNumber ++;                             //increment the counter
      startTime = now;
      // and reset our timer for the next knock

      delay(knockFadeTime);                              // again, a little delay to let the knock decay.

    }
    now = millis();

    //did we timeout or run out of knocks?
  } while ((now - startTime < knockComplete) && (currentKnockNumber < maximumKnocks));

  //we've got our knock recorded, lets see if it's valid
  if (programButtonPressed == false)           // only if we're not in progrmaing mode.
  {
    if (validateKnock1() == true)
    {
      Knock1_Action("EVENT_NAME");
    }
    else if (validateKnock2() == true)
    {
      Knock2_Action("EVENT_NAME");
    }
    else if (validateKnock3() == true)
    {
      Knock3_Action("EVENT_NAME");
    }
    else if (validateKnock4() == true)
    {
      Knock4_Action("EVENT_NAME");
    }
    else if (validateKnock5() == true)
    {
      Knock5_Action("EVENT_NAME");
    }
    else
    {
      Serial.println("Secret knock failed.");
      for (int i = 0; i < 3; i++)
      {
        digitalWrite(Red, HIGH);
        delay(200);
        digitalWrite(Red, LOW);
        delay(100);
      }
    }
  }
  else
  {

  }

}

boolean validateKnock1()
{
  int i = 0;

  // simplest check first: Did we get the right number of knocks?
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  int maxKnockInterval = 0;                // We use this later to normalize the times.

  for (i = 0; i < maximumKnocks; i++) {
    if (knockReadings[i] > 0) {
      currentKnockCount++;
    }
    if (secretCode1[i] > 0) {            //todo: precalculate this.
      secretKnockCount++;
    }

    if (knockReadings[i] > maxKnockInterval) {  // collect normalization data while we're looping.
      maxKnockInterval = knockReadings[i];
    }
  }

  // If we're recording a new knock, save the info and get out of here.
  if (programButtonPressed == true) {
    for (i = 0; i < maximumKnocks; i++) { // normalize the times
      secretCode1[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    }

    return false;   // We don't unlock the door when we are recording a new knock.
  }

  if (currentKnockCount != secretKnockCount) {
    return false;
  }

  /*  Now we compare the relative intervals of our knocks, not the absolute time between them.
      (ie: if you do the same pattern slow or fast it should still open the door.)
      This makes it less picky, which while making it less secure can also make it
      less of a pain to use if your tempo is a little slow or fast.
  */
  int totaltimeDifferences = 0;
  int timeDiff = 0;
  for (i = 0; i < maximumKnocks; i++) { // Normalize the times
    knockReadings[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    timeDiff = abs(knockReadings[i] - secretCode1[i]);
    if (timeDiff > rejectValue) { // Individual value too far out of whack
      return false;
    }
    totaltimeDifferences += timeDiff;
  }
  // It can also fail if the whole thing is too inaccurate.
  if (totaltimeDifferences / secretKnockCount > averageRejectValue) {
    return false;
  }
  Serial.println("Pattern Matched!!");
  return true;

}

boolean validateKnock2() {
  int i = 0;

  // simplest check first: Did we get the right number of knocks?
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  int maxKnockInterval = 0;                // We use this later to normalize the times.

  for (i = 0; i < maximumKnocks; i++) {
    if (knockReadings[i] > 0) {
      currentKnockCount++;
    }
    if (secretCode2[i] > 0) {            //todo: precalculate this.
      secretKnockCount++;
    }

    if (knockReadings[i] > maxKnockInterval) {  // collect normalization data while we're looping.
      maxKnockInterval = knockReadings[i];
    }
  }

  // If we're recording a new knock, save the info and get out of here.
  if (programButtonPressed == true) {
    for (i = 0; i < maximumKnocks; i++) { // normalize the times
      secretCode2[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    }
    return false;   // We don't unlock the door when we are recording a new knock.
  }

  if (currentKnockCount != secretKnockCount) {
    return false;
  }

  /*  Now we compare the relative intervals of our knocks, not the absolute time between them.
      (ie: if you do the same pattern slow or fast it should still open the door.)
      This makes it less picky, which while making it less secure can also make it
      less of a pain to use if your tempo is a little slow or fast.
  */
  int totaltimeDifferences = 0;
  int timeDiff = 0;
  for (i = 0; i < maximumKnocks; i++) { // Normalize the times
    knockReadings[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    timeDiff = abs(knockReadings[i] - secretCode2[i]);
    if (timeDiff > rejectValue) { // Individual value too far out of whack
      return false;
    }
    totaltimeDifferences += timeDiff;
  }
  // It can also fail if the whole thing is too inaccurate.
  if (totaltimeDifferences / secretKnockCount > averageRejectValue) {
    return false;
  }
  Serial.println("Pattern Matched!!");
  return true;

}

boolean validateKnock3() {
  int i = 0;

  // simplest check first: Did we get the right number of knocks?
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  int maxKnockInterval = 0;                // We use this later to normalize the times.

  for (i = 0; i < maximumKnocks; i++) {
    if (knockReadings[i] > 0) {
      currentKnockCount++;
    }
    if (secretCode3[i] > 0) {            //todo: precalculate this.
      secretKnockCount++;
    }

    if (knockReadings[i] > maxKnockInterval) {  // collect normalization data while we're looping.
      maxKnockInterval = knockReadings[i];
    }
  }

  // If we're recording a new knock, save the info and get out of here.
  if (programButtonPressed == true) {
    for (i = 0; i < maximumKnocks; i++) { // normalize the times
      secretCode3[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    }
    return false;   // We don't unlock the door when we are recording a new knock.
  }

  if (currentKnockCount != secretKnockCount) {
    return false;
  }

  /*  Now we compare the relative intervals of our knocks, not the absolute time between them.
      (ie: if you do the same pattern slow or fast it should still open the door.)
      This makes it less picky, which while making it less secure can also make it
      less of a pain to use if your tempo is a little slow or fast.
  */
  int totaltimeDifferences = 0;
  int timeDiff = 0;
  for (i = 0; i < maximumKnocks; i++) { // Normalize the times
    knockReadings[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    timeDiff = abs(knockReadings[i] - secretCode3[i]);
    if (timeDiff > rejectValue) { // Individual value too far out of whack
      return false;
    }
    totaltimeDifferences += timeDiff;
  }
  // It can also fail if the whole thing is too inaccurate.
  if (totaltimeDifferences / secretKnockCount > averageRejectValue) {
    return false;
  }
  Serial.println("Pattern Matched!!");
  return true;

}

boolean validateKnock4() {
  int i = 0;

  // simplest check first: Did we get the right number of knocks?
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  int maxKnockInterval = 0;                // We use this later to normalize the times.

  for (i = 0; i < maximumKnocks; i++) {
    if (knockReadings[i] > 0) {
      currentKnockCount++;
    }
    if (secretCode4[i] > 0) {            //todo: precalculate this.
      secretKnockCount++;
    }

    if (knockReadings[i] > maxKnockInterval) {  // collect normalization data while we're looping.
      maxKnockInterval = knockReadings[i];
    }
  }

  // If we're recording a new knock, save the info and get out of here.
  if (programButtonPressed == true) {
    for (i = 0; i < maximumKnocks; i++) { // normalize the times
      secretCode4[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    }
    return false;   // We don't unlock the door when we are recording a new knock.
  }

  if (currentKnockCount != secretKnockCount) {
    return false;
  }

  /*  Now we compare the relative intervals of our knocks, not the absolute time between them.
      (ie: if you do the same pattern slow or fast it should still open the door.)
      This makes it less picky, which while making it less secure can also make it
      less of a pain to use if your tempo is a little slow or fast.
  */
  int totaltimeDifferences = 0;
  int timeDiff = 0;
  for (i = 0; i < maximumKnocks; i++) { // Normalize the times
    knockReadings[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    timeDiff = abs(knockReadings[i] - secretCode4[i]);
    if (timeDiff > rejectValue) { // Individual value too far out of whack
      return false;
    }
    totaltimeDifferences += timeDiff;
  }
  // It can also fail if the whole thing is too inaccurate.
  if (totaltimeDifferences / secretKnockCount > averageRejectValue) {
    return false;
  }
  Serial.println("Pattern Matched!!");
  return true;

}


boolean validateKnock5() {
  int i = 0;

  // simplest check first: Did we get the right number of knocks?
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  int maxKnockInterval = 0;                // We use this later to normalize the times.

  for (i = 0; i < maximumKnocks; i++) {
    if (knockReadings[i] > 0) {
      currentKnockCount++;
    }
    if (secretCode5[i] > 0) {            //todo: precalculate this.
      secretKnockCount++;
    }

    if (knockReadings[i] > maxKnockInterval) {  // collect normalization data while we're looping.
      maxKnockInterval = knockReadings[i];
    }
  }

  // If we're recording a new knock, save the info and get out of here.
  if (programButtonPressed == true) {
    for (i = 0; i < maximumKnocks; i++) { // normalize the times
      secretCode5[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    }
    return false;   // We don't unlock the door when we are recording a new knock.
  }

  if (currentKnockCount != secretKnockCount) {
    return false;
  }

  /*  Now we compare the relative intervals of our knocks, not the absolute time between them.
      (ie: if you do the same pattern slow or fast it should still open the door.)
      This makes it less picky, which while making it less secure can also make it
      less of a pain to use if your tempo is a little slow or fast.
  */
  int totaltimeDifferences = 0;
  int timeDiff = 0;
  for (i = 0; i < maximumKnocks; i++) { // Normalize the times
    knockReadings[i] = map(knockReadings[i], 0, maxKnockInterval, 0, 100);
    timeDiff = abs(knockReadings[i] - secretCode5[i]);
    if (timeDiff > rejectValue) { // Individual value too far out of whack
      return false;
    }
    totaltimeDifferences += timeDiff;
  }
  // It can also fail if the whole thing is too inaccurate.
  if (totaltimeDifferences / secretKnockCount > averageRejectValue) {
    return false;
  }
  Serial.println("Pattern Matched!!");
  return true;

}


void Knock1_Action(String Event_name)
{


  digitalWrite(Green, HIGH);
  if ((WiFiMulti.run() == WL_CONNECTED))
  {

    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
    http.begin("http://maker.ifttt.com/trigger/" + Event_name + "/with/key/" + Webhooks_key); //HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      digitalWrite(Green,LOW);
      digitalWrite(Red,HIGH);
      delay(2000);
      digitalWrite(Red,LOW);       
    }

    http.end();
  }
  digitalWrite(Green, LOW);
}

void Knock2_Action(String Event_name)
{
  digitalWrite(Green, HIGH);
  if ((WiFiMulti.run() == WL_CONNECTED))
  {

    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
    http.begin("http://maker.ifttt.com/trigger/" + Event_name + "/with/key/" + Webhooks_key); //HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      digitalWrite(Green,LOW);
      digitalWrite(Red,HIGH);
      delay(2000);
      digitalWrite(Red,LOW);       
    }

    http.end();
  }
  digitalWrite(Green, LOW);
}

void Knock3_Action(String Event_name)
{
  digitalWrite(Green, HIGH);
  if ((WiFiMulti.run() == WL_CONNECTED))
  {

    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
    http.begin("http://maker.ifttt.com/trigger/" + Event_name + "/with/key/" + Webhooks_key); //HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      digitalWrite(Green,LOW);
      digitalWrite(Red,HIGH);
      delay(2000);
      digitalWrite(Red,LOW);       
    }

    http.end();
  }
  digitalWrite(Green, LOW);
}

void Knock4_Action(String Event_name)
{
  digitalWrite(Green, HIGH);
  if ((WiFiMulti.run() == WL_CONNECTED))
  {

    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
    http.begin("http://maker.ifttt.com/trigger/" + Event_name + "/with/key/" + Webhooks_key); //HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      digitalWrite(Green,LOW);
      digitalWrite(Red,HIGH);       
      delay(2000);       
      digitalWrite(Red,LOW);       
          
    }

    http.end();
  }
  digitalWrite(Green, LOW);
}

void Knock5_Action(String Event_name)
{

  digitalWrite(Green, HIGH);
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
    http.begin("http://maker.ifttt.com/trigger/" + Event_name + "/with/key/" + Webhooks_key); //HTTP
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());   
      digitalWrite(Green,LOW);
      digitalWrite(Red,HIGH);
      delay(2000);
      digitalWrite(Red,LOW);       
    }

    http.end();
  }

  digitalWrite(Green, LOW);
}


