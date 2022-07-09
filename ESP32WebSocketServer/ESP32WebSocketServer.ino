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
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

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
bool MoveDone = 1;

bool Connected = 0;

// Create Instance of Stepper library
Stepper myStepper(stepsPerRevolution, 2, 0, 4, 16);

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

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
      MoveDone = 0;
      //myStepper.step(stepCount);  
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

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println(F("Connecting to WiFi.."));
    display.display();
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Connected!"));
  display.println(F("IP Address:"));
  display.println(WiFi.localIP());
  display.display();
  
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
  //Serial.println(digitalRead(HallSensor));
  while(Connected == 1 && HomingDone == 0 && digitalRead(HallSensor) == 1)
  {
        myStepper.step(-10);
        if(digitalRead(HallSensor) == 0){
          HomingDone = 1;
        }     
  }
   
  while(Connected == 1 && HomingDone == 1 && MoveDone == 0)
  {
        myStepper.step(stepCount);  
        MoveDone = 1;
  }
    

}
