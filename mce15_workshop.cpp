#include <application.h>
#include <string.h>

int LED_PIN  = D7;
int SEND_PIN = D1;
int RECV_PIN = D4;
int TOGGLE_EMBERLIGHT_PIN = D6;
int CAP_TIMEOUT = 10000;
int CAP_THRESHOLD = 10;
int OFF_STATE = 0;
int ON_STATE = 1;
int STATE_CHANGE_THRESHOLD = 2;
int LOOP_DELAY_MILLIS = 50;
String EMBERLIGHT_DEVICE_ID = "962a976a-1b58-49e9-9f97-85563d883ff3";

int checkTouchSensor();
void toggleLed();
void turnLedOn();
void turnLedOff();
void toggleEmberlight();
void turnEmberlightOn();
void turnEmberlightOff();
void changeEmberlightDeviceState(char state[]);

int touchState = 0;
int lastTouchState = 0;
int ledState = 0;
int deviceState = 0;
int stateChangeCounter = 0;
TCPClient client;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(TOGGLE_EMBERLIGHT_PIN, INPUT_PULLDOWN);
  Serial.begin(9600);
}


void loop()
{
  touchState = checkTouchSensor();
  if(touchState != lastTouchState && touchState == ON_STATE) {
    int useEmberlight = digitalRead(TOGGLE_EMBERLIGHT_PIN);
    Serial.print("Use Emberlight: ");
    Serial.print(useEmberlight);
    toggleLed();
    if(useEmberlight == HIGH) {
      toggleEmberlight();
    }
  }

  lastTouchState = touchState;
}
























int checkTouchSensor()
{
  int result = -1;
  int counter = 0;
  int recvValue = LOW;

  pinMode(SEND_PIN, OUTPUT);
  pinMode(RECV_PIN, INPUT);

  digitalWrite(SEND_PIN, HIGH);

  while(recvValue == LOW && counter < CAP_TIMEOUT) {
    counter++;
    recvValue = digitalRead(RECV_PIN);
  }

  digitalWrite(SEND_PIN, LOW);
  delay(LOOP_DELAY_MILLIS);

  Serial.println(counter);

  if(counter > CAP_THRESHOLD) {
    stateChangeCounter = 0;
    result = ON_STATE;
    /*
    if(touchState == ON_STATE) {
      stateChangeCounter = 0;
      result = ON_STATE;
    }
    else {
      stateChangeCounter++;
    }
    */
  }
  else {
    if(touchState == OFF_STATE) {
      stateChangeCounter = 0;
      result = OFF_STATE;
    }
    else {
      stateChangeCounter++;
    }
  }

  if(result == -1) {
    if(stateChangeCounter > STATE_CHANGE_THRESHOLD) {
      stateChangeCounter = 0;
      if(touchState == ON_STATE) {
        result = OFF_STATE;
      }
      else {
        result = ON_STATE;
      }
    }
    else {
      result = touchState;
    }
  }

  return result;
}

void toggleLed() {
  if(ledState == ON_STATE) {
    turnLedOff();
  }
  else {
    turnLedOn();
  }
}

void turnLedOn() {
  Serial.println("Turning LED On");
  digitalWrite(LED_PIN, HIGH);
  ledState = ON_STATE;
}

void turnLedOff() {
  Serial.println("Turning LED Off");
  digitalWrite(LED_PIN, LOW);
  ledState = OFF_STATE;
}

void toggleEmberlight() {
  if(deviceState == ON_STATE) {
    turnEmberlightOff();
  }
  else {
    turnEmberlightOn();
  }
}

void turnEmberlightOn()
{
  Serial.println("Turning Emberlight On");
  changeEmberlightDeviceState("on");
  deviceState = ON_STATE;
  Serial.println("Emberlight On");
}

void turnEmberlightOff()
{
  Serial.println("Turning Emberlight Off");
  changeEmberlightDeviceState("off");
  deviceState = OFF_STATE;
  Serial.println("Emberlight Off");
}

void changeEmberlightDeviceState(char action[])
{
  char *server = "passthrough.herokuapp.com";
  bool connected = client.connected();

  if(!connected) {
    Serial.println("Connecting...");
    connected = client.connect(server, 80);
  }

  if(connected) {
    Serial.println("Sending...");
    client.print("GET /emberlight/");
    client.print(action);
    client.print("?device_id=");
    client.print(EMBERLIGHT_DEVICE_ID);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: keep-alive");
    client.println();
    client.flush();

    Serial.println("Reading...");
    while(client.available()) {
      char data = client.read();
      Serial.print(data);
    }

  }
  else {
    Serial.println("Connect Failed");
  }
}
