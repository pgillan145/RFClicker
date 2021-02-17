#include <RFduinoBLE.h>

#define CEREAL

const int RIGHT = 2;
const int DOWN = 3;
const int LEFT = 4;
const int UP = 5;
const int LED = 6;

void setup() {
#ifdef CEREAL
  Serial.begin(9600);
#endif
  delay(1000);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);  
  pinMode(UP, INPUT_PULLUP);
  pinMode(LED, OUTPUT);

  // in ms (converted to 0.625ms units) - range 20ms to 10.24s - default 80ms
  RFduinoBLE.advertisementInterval = 250;
  // -20 dBm to +4 dBm - default +4 dBm
  RFduinoBLE.txPowerLevel = 0;
  RFduinoBLE.deviceName = "RFClicker";
  RFduinoBLE.advertisementData = "clicker";
  RFduinoBLE.begin(); 
}

int8_t last_button_status = -1;
int32_t right_millis = 0;
int32_t down_millis = 0;
int32_t left_millis = 0;
int32_t up_millis = 0;
bool connected = false;
uint32_t now = 0;
void loop() {
  now = millis();
  if (connected) {
    uint8_t right_click = 0;
    uint8_t down_click = 0;
    
    uint8_t right = digitalRead(RIGHT);
    uint8_t down = digitalRead(DOWN);
    uint8_t left = digitalRead(LEFT);
    uint8_t up = digitalRead(UP);

    uint8_t button_status = 0;

    if (right == HIGH) {
      if (right_millis == 0) {
        right_millis = now;
      }
    }
    else { // right == LOW
      uint16_t held = now - right_millis;
      if (held >= 100) {
#ifdef CEREAL
        Serial.println("right button");
#endif
        button_status = button_status | ( 1 << 0);
      }
      right_millis = 0;
    }
    
    if (down == HIGH) {
      if (down_millis == 0) {
        down_millis = now;
      }
    }  
    else { // down == LOW
      uint16_t held = now - down_millis;
      if (held >= 100) {
#ifdef CEREAL
        Serial.println("down button");
#endif
        button_status = button_status | ( 1 << 1);
      }
      down_millis = 0;
    }

    if (left == HIGH) {
      if (left_millis == 0) {
        left_millis = now;
      }
    }
    else { // left == LOW
      uint16_t held = now - left_millis;
      if (held >= 100) {
#ifdef CEREAL
        Serial.println("left button");
#endif
        button_status = button_status | ( 1 << 2);
      }
      left_millis = 0;
    }
    
    if (up == HIGH) {
      if (up_millis == 0) {
        up_millis = now;
      }
    }
    else { // up == LOW
      uint16_t held = now - up_millis;
      if (held >= 100) {
#ifdef CEREAL
        Serial.println("up button");
#endif
        button_status = button_status | ( 1 << 3);
      }
      up_millis = 0;
    }


    if (button_status != last_button_status) {
#ifdef CEREAL
      Serial.print("sending:");
      Serial.println(button_status);
#endif
      RFduinoBLE.send(button_status);
      last_button_status = button_status;
    }
  } // connected
} // loop()

void RFduinoBLE_onConnect() {
#ifdef CEREAL
  Serial.println("connected()");
#endif
  digitalWrite(LED, HIGH);
  connected = true;
}
void RFduinoBLE_onDisconnect() {
#ifdef CEREAL
  Serial.println("disconnected()");
#endif
  digitalWrite(LED, LOW);
  connected = false;
}
