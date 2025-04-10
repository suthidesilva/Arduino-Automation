// Filename: TelescopeControlSystem.ino
// Author: Suthi de Silva, Dr. Dave Rosoff

// Just importing the primary modules
#include <Arduino.h>
#include <ESP8266WiFi.h> // For ESP8266
#include <ArduinoWebsockets.h> // WebSockets library
#include <Servo.h> // For controlling the telescope motors

using namespace websockets;

// WiFi credentials, maybe for .env
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

// WebSocket server (could be done with socket.io)
const char* websocketServer = "wss://yourserver.com";
WebsocketsClient wsClient;

// Pin definitions
const int servoPin1 = 9; 
const int servoPin2 = 10;
const int calibrationSwitchPin = 7; 

// Servo objects
Servo servo1;
Servo servo2;

// State variables
bool isCalibrating = false;
bool systemStable = true;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected!");

  wsClient.onMessage(onWebSocketMessage);
  wsClient.onEvent(onWebSocketEvent);
  wsClient.connect(websocketServer);

  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  pinMode(calibrationSwitchPin, OUTPUT);

  runCalibration();
}

void loop() {
  wsClient.poll();

  // main call to simulate telescope control based on received commands
  if (isCalibrating) {
    automateCalibration();
  }

  // Failure management (main fuse system for us)
  if (!systemStable) {
    handleFailure();
  }
}

// This is function with conditionals with to handle incoming WebSocket messages
// tested partially though, but works
void onWebSocketMessage(WebsocketsMessage message) {
  Serial.println("Message received: " + message.data());

  // Parse and execute commands 
  if (message.data() == "MOVE_UP") {
    moveTelescope(0, -10); 
  } else if (message.data() == "MOVE_DOWN") {
    moveTelescope(0, 10);
  } else if (message.data() == "START_CALIBRATION") {
    isCalibrating = true;
  } else if (message.data() == "STOP_CALIBRATION") {
    isCalibrating = false;
  }
}

// This is the function to handle WebSocket events
void onWebSocketEvent(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("WebSocket connection opened!");
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("WebSocket connection closed!");
    systemStable = false;
  } else if (event == WebsocketsEvent::GotPing) {
    Serial.println("WebSocket ping received!");
  }
}

// function to move the telescope
void moveTelescope(int x, int y) {
  int newServo1Pos = constrain(servo1.read() + x, 0, 180);
  int newServo2Pos = constrain(servo2.read() + y, 0, 180);

  servo1.write(newServo1Pos);
  servo2.write(newServo2Pos);

  Serial.println("Telescope moved to position: X=" + String(newServo1Pos) + ", Y=" + String(newServo2Pos));
}

// function to automate calibration
void automateCalibration() {
  Serial.println("Running calibration...");
  digitalWrite(calibrationSwitchPin, HIGH);
  delay(1000); // Simulate calibration process
  digitalWrite(calibrationSwitchPin, LOW);
  isCalibrating = false;
  Serial.println("Calibration completed!");
}

// function to handle failures
void handleFailure() {
  Serial.println("System failure detected. Reconnecting...");
  wsClient.close();
  delay(1000);
  wsClient.connect(websocketServer);
  systemStable = true;
}

// Still in testing phase!! ,initial calibration setup
void runCalibration() {
  Serial.println("Performing initial calibration...");
  automateCalibration();
}
