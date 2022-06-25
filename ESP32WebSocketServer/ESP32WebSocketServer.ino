/***********************************************************************************************************************
 * Created by :
 * Rui Santos
 * 
 * Edited by :
 * Ong Kai Li
 * 25 June 2022
 * Project: Driver Monitoring System Test System
 * 
 * 
 **********************************************************************************************************************/


// Import required libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//// Replace with your network credentials
const char* ssid = "Vodafone-E3EA";
const char* password = "MEFm6ndc4PAbRA6Q";

//// Replace with your network credentials
//const char* ssid = "Xiaomi 11 Lite 5G NE";
//const char* password = "zqrc3prgaanykaa";

bool ledSet1State = 0;
bool ledSet2State = 0;
bool ledSet3State = 0;
bool ledSetOffState = 0;

bool threeLED = 0;
bool sixLED = 0;
bool twelveLED = 0;
bool offLED = 0;

const int ledPinSet1 = 2;
const int ledPinSet2 = 4;
const int ledPinSet3 = 5;


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


void notifyClients() {
  ws.textAll(String(ledSet1State));
  ws.textAll(String(ledSet2State));
  ws.textAll(String(ledSet3State));
  ws.textAll(String(ledSetOffState));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle1") == 0) {
      threeLED = 1;
      ledSet1State = 1;
      ledSet2State = 0;
      ledSet3State = 0;
    }else if (strcmp((char*)data, "toggle2") == 0){
      sixLED = 1;
      ledSet1State = 1;
      ledSet2State = 1;
      ledSet3State = 0;
    }else if (strcmp((char*)data, "toggle3") == 0){
      twelveLED = 1;
      ledSet1State = 1;
      ledSet2State = 1;
      ledSet3State = 1;
    }else if (strcmp((char*)data, "toggleOFF") == 0){
      offLED = 1;
      ledSet1State = 0;
      ledSet2State = 0;
      ledSet3State = 0;
    }

//    if (threeLED == 1){
//  
//  Serial.println("Enter 3LED");
//  }else if (sixLED == 1){
////    ledSet1State = !ledSet1State;
////    ledSet2State = !ledSet2State;
////    ledSet3State = 0;
//  digitalWrite(ledPinSet1, 1);
//  digitalWrite(ledPinSet2, 1);
//  digitalWrite(ledPinSet3, 0);
//  Serial.println("Enter 6LED");
//  }else if (twelveLED == 1){
////    ledSet1State = !ledSet1State;
////    ledSet2State = !ledSet2State;
////    ledSet3State = !ledSet3State;
//  digitalWrite(ledPinSet1, 1);
//  digitalWrite(ledPinSet2, 1);
//  digitalWrite(ledPinSet3, 1);
//  Serial.println("Enter 12LED");
//  }else if (offLED == 1){
//  digitalWrite(ledPinSet1, LOW);
//  digitalWrite(ledPinSet2, LOW);
//  digitalWrite(ledPinSet3, LOW);
//  Serial.println("Enter offLED");
//  }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledSet1State){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(ledPinSet1, OUTPUT);
  digitalWrite(ledPinSet1, LOW);
  pinMode(ledPinSet2, OUTPUT);
  digitalWrite(ledPinSet2, LOW);
  pinMode(ledPinSet3, OUTPUT);
  digitalWrite(ledPinSet3, LOW);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

//  // Route for root / web page
//  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//    request->send_P(200, "text/html", index_html, processor);
//  });

  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();
  digitalWrite(ledPinSet1, ledSet1State);
  digitalWrite(ledPinSet2, ledSet2State);
  digitalWrite(ledPinSet3, ledSet3State);

}
