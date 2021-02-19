#include <RFduinoBLE.h>

#undef CEREAL

// pins
const int RIGHT = 2;
const int DOWN = 3;
const int LEFT = 4;
const int UP = 5;
const int LED = 6;

// other settings
const int HOLD_TIMEOUT = 300;   // how long a button needs to be held down before it triggers an input and resets
const int BOUNCE_TIMEOUT = 35;  // the minimum time a button needs to be held down to count as a click

void setup() {
#ifdef CEREAL
  Serial.begin(9600);
#endif
  delay(1000);
#ifdef CEREAL
  Serial.println("setup()");
#endif

  pinMode(RIGHT, INPUT_PULLDOWN);
  pinMode(DOWN, INPUT_PULLDOWN);
  pinMode(LEFT, INPUT_PULLDOWN);  
  pinMode(UP, INPUT_PULLDOWN);
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
int32_t right_holdtime = 0;
int32_t right_last_holdtime = 0;
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
      //Serial.println("HIGH");
      if (right_millis == 0) {
        right_millis = now;
      }
      else if ((now - right_millis) > HOLD_TIMEOUT)  {
#ifdef CEREAL
        Serial.println("right button held");
#endif
        button_status += 1;
        right_millis = now;
      }
    }
    else if (right == LOW && right_millis > 0 && (now - right_millis) >= BOUNCE_TIMEOUT) {
#ifdef CEREAL
      Serial.println("right button clicked");
#endif
      button_status += 1;
      right_millis = 0;
    }
    
    if (down == HIGH) {
      if (down_millis == 0) {
        down_millis = now;
      }
      else if ((now - down_millis) > HOLD_TIMEOUT) {
#ifdef CEREAL
        Serial.println("down button held");
#endif
        button_status += 2;
        down_millis = now;
      }
    }  
    else if (down == LOW && down_millis > 0 && (now - down_millis) >= BOUNCE_TIMEOUT ) {
#ifdef CEREAL
      Serial.println("down button clicked");
#endif
      button_status += 2;
      down_millis = 0;
    }

    if (left == HIGH) {
      if (left_millis == 0) {
        left_millis = now;
      }
      else if ((now - left_millis) > HOLD_TIMEOUT) {
#ifdef CEREAL
        Serial.println("left button held");
#endif
        button_status += 4;
        left_millis = now;
      }
    }
    else if (left == LOW && left_millis > 0 && (now - left_millis) >= BOUNCE_TIMEOUT ) {
#ifdef CEREAL
      Serial.println("left button clicked");
#endif
      button_status += 4;
      left_millis = 0;
    }
    
    if (up == HIGH) {
      if (up_millis == 0) {
        up_millis = now;
      }
      else if ((now - up_millis) > HOLD_TIMEOUT) {
#ifdef CEREAL
        Serial.println("up button held");
#endif
        button_status += 8;
        up_millis = now;
      }
    }
    else if (up == LOW && up_millis > 0 && (now - up_millis) >= BOUNCE_TIMEOUT) {
#ifdef CEREAL
      Serial.println("up button clicked");
#endif
      button_status += 8;
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
