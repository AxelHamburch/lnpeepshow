#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <ESP32Servo.h>

fs::SPIFFSFS &FlashFS = SPIFFS;
#define FORMAT_ON_FAIL true
#define PARAM_FILE "/elements.json"

///////////special////////////////

Servo motorvorhang;
int pos = 70; // start position close = 70° - open 180°
Servo motorteller;

int taster = 4; // ex portalPin / Pull up -> touch to switch
int relais16 = 16;
int relais17 = 17;
int relais18 = 18;

/////////////////////////////////

// Access point variables
String payloadStr;
String password;
String serverFull;
String lnbitsServer;
String ssid;
String wifiPassword;
String deviceId;
String dataId;
String lnurl;

bool paid;
bool down = false;
bool triggerConfig = false;

WebSocketsClient webSocket;

struct KeyValue
{
  String key;
  String value;
};

void setup()
{
  Serial.begin(115200);
  int timer = 0;
  pinMode(23, OUTPUT);

  // special ->

  Serial.println("setup..");

  pinMode(taster, INPUT_PULLUP);
  pinMode(relais16, OUTPUT);
  pinMode(relais17, OUTPUT);
  pinMode(relais18, OUTPUT);

  motorvorhang.attach(26);
  motorteller.attach(27);

  // <- special

  while (timer < 2000)
  {
    digitalWrite(23, LOW);
    // special ->
    Serial.println(digitalRead(taster));
    if (digitalRead(taster) == false)
    // <- special
    {
      Serial.println("Launch portal");
      triggerConfig = true;
      timer = 2000;
    }
    delay(150);
    digitalWrite(23, HIGH);
    timer = timer + 300;
    delay(150);
  }

  timer = 0;

  FlashFS.begin(FORMAT_ON_FAIL);

  // get the saved details and store in global variables
  readFiles();

  if (triggerConfig == false)
  {
    WiFi.begin(ssid.c_str(), wifiPassword.c_str());
    while (WiFi.status() != WL_CONNECTED && timer < 20000)
    {
      delay(500);
      digitalWrite(23, HIGH);
      Serial.print(".");
      timer = timer + 1000;
      if (timer > 19000)
      {
        triggerConfig = true;
      }
      delay(500);
      digitalWrite(23, LOW);
    }
  }

  if (triggerConfig == true)
  {
    digitalWrite(23, HIGH);
    Serial.println("USB Config triggered");
    configOverSerialPort();
  }

  Serial.println(lnbitsServer + "/api/v1/ws/" + deviceId);
  webSocket.beginSSL(lnbitsServer, 443, "/api/v1/ws/" + deviceId);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(1000);
}

void loop()
{
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Failed to connect");
    delay(500);
  }
  digitalWrite(23, LOW);
  payloadStr = "";
  delay(2000);
  while ((paid == false) && (digitalRead(taster) == true))
  {
    webSocket.loop();
    if (paid || (digitalRead(taster) == false))
    {

      // special ->

      if ((getValue(payloadStr, '-', 0).toInt()) == 12)
      {
        Serial.println("Restart Webcam");
        pinMode(getValue(payloadStr, '-', 0).toInt(), OUTPUT);
        digitalWrite(getValue(payloadStr, '-', 0).toInt(), HIGH);
        delay(getValue(payloadStr, '-', 1).toInt());
        digitalWrite(getValue(payloadStr, '-', 0).toInt(), LOW);
      }
      else
      {
        Serial.println("Start party");
        // Ambient lighting ON
        digitalWrite(relais17, HIGH);
        delay(6000);
        // Turn the motor slowly
        motorteller.write(87);
        // Open curtain
        if (pos <= 180)
        {
          for (pos = 70; pos <= 180; pos += 1)
          { // goes from 0 degrees to 180 degrees
            // in steps of 1 degree
            motorvorhang.write(pos); // tell servo to go to position in variable 'pos'
            delay(100);              // waits 15ms for the servo to reach the position
          }
          // Spot ON
          digitalWrite(relais18, HIGH);
          delay(5000);
          // Lighting bright ON
          digitalWrite(relais16, HIGH);
        }

        Serial.println("30 seconds peepshow");
        delay(30000);

        // Close the curtain
        if (pos >= 70)
        {
          for (pos = 180; pos >= 70; pos -= 1)
          { // goes from 0 degrees to 180 degrees
            // in steps of 1 degree
            motorvorhang.write(pos); // tell servo to go to position in variable 'pos'
            delay(100);              // waits 15ms for the servo to reach the position
          }
          Serial.println("Curtain is closed");
          // Plate motor OFF
          motorteller.write(90);
          // Spot OFF
          digitalWrite(relais18, LOW);
          delay(3000);
          // Lighting bright OFF
          digitalWrite(relais16, LOW);
          delay(6000);
          // Ambient lighting OFF
          digitalWrite(relais17, LOW);
        }
      }
      // <- special
    }
  }
  Serial.println("Paid");
  paid = false;
}

//////////////////HELPERS///////////////////

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void readFiles()
{
  File paramFile = FlashFS.open(PARAM_FILE, "r");
  if (paramFile)
  {
    StaticJsonDocument<1500> doc;
    DeserializationError error = deserializeJson(doc, paramFile.readString());

    const JsonObject maRoot0 = doc[0];
    const char *maRoot0Char = maRoot0["value"];
    password = maRoot0Char;
    Serial.println(password);

    const JsonObject maRoot1 = doc[1];
    const char *maRoot1Char = maRoot1["value"];
    ssid = maRoot1Char;
    Serial.println(ssid);

    const JsonObject maRoot2 = doc[2];
    const char *maRoot2Char = maRoot2["value"];
    wifiPassword = maRoot2Char;
    Serial.println(wifiPassword);

    const JsonObject maRoot3 = doc[3];
    const char *maRoot3Char = maRoot3["value"];
    serverFull = maRoot3Char;
    lnbitsServer = serverFull.substring(5, serverFull.length() - 33);
    deviceId = serverFull.substring(serverFull.length() - 22);

    const JsonObject maRoot4 = doc[4];
    const char *maRoot4Char = maRoot4["value"];
    lnurl = maRoot4Char;
    Serial.println(lnurl);
  }
  paramFile.close();
}

//////////////////NODE CALLS///////////////////

void checkConnection()
{
  WiFiClientSecure client;
  client.setInsecure();
  const char *lnbitsserver = lnbitsServer.c_str();
  if (!client.connect(lnbitsserver, 443))
  {
    down = true;
    return;
  }
}

//////////////////WEBSOCKET///////////////////

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED:
  {
    Serial.printf("[WSc] Connected to url: %s\n", payload);

    // send message to server when Connected
    webSocket.sendTXT("Connected");
  }
  break;
  case WStype_TEXT:
    payloadStr = (char *)payload;
    paid = true;
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    break;
  }
}