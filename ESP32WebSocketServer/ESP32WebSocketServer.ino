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
#include <Stepper.h>


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

const int ledPinSet1 = 25;
const int ledPinSet2 = 26;
const int ledPinSet3 = 27;

// Number of steps per output rotation
const int stepsPerRevolution = 50;
int stepCount = 0;

const int HallSensor = 5;
const int LED = 18;
bool HomingDone = 0;

bool Connected = 0;

// Create Instance of Stepper library
Stepper myStepper(stepsPerRevolution, 2, 0, 4, 16);

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
    String text_received = (char*)data;
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
    }else if (text_received.indexOf("Stepper") == 0){
      Serial.print("Stepper motor: ");
      String StepperValue = text_received.substring(7);
      Serial.println(StepperValue.toInt());
      stepCount = 50 * StepperValue.toInt();
      myStepper.step(stepCount);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      HomingDone = 0;
      Connected = 1;
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      Connected = 0;
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
  
  // set the speed at 60 rpm:
  myStepper.setSpeed(60);

  // set for Hall Sensor
  pinMode(LED, OUTPUT);
  pinMode(HallSensor, INPUT);
  digitalWrite(LED,LOW);
  
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
  digitalWrite(LED, !digitalRead(HallSensor));
  while(Connected == 1 && HomingDone == 0 && digitalRead(HallSensor) == 1){
        myStepper.step(-100);
        if(digitalRead(HallSensor) == 0){
          HomingDone = 1;
        }     
   }   

}
