#include <RFduinoBLE.h>

#undef CEREAL

// pins
const int POWER = 1;
const int RIGHT = 6;
const int DOWN = 5;
const int LEFT = 4;
const int UP = 3;
const int LED = 2;

// other settings
const int HISTORY_LENGTH = 10;
#define HOLD_TIMEOUT   300   // how long a button needs to be held down before it triggers an input and resets
#define BOUNCE_TIMEOUT  35   // the minimum time a button needs to be held down to count as a click
#define SEND_BLINK_TIME 100  // how long the connection light brightness should jump when data is "sent".

void setup() {
#ifdef CEREAL
  Serial.begin(9600);
#endif
  delay(1000);
#ifdef CEREAL
  Serial.println("setup()");
#endif
  pinMode(POWER, OUTPUT);
  pinMode(RIGHT, INPUT_PULLDOWN);
  pinMode(DOWN, INPUT_PULLDOWN);
  pinMode(LEFT, INPUT_PULLDOWN);  
  pinMode(UP, INPUT_PULLDOWN);
  pinMode(LED, OUTPUT);

  // in ms (converted to 0.625ms units) - range 20ms to 10.24s - default 80ms
  RFduinoBLE.advertisementInterval = 20;
  // -20 dBm to +4 dBm - default +4 dBm
  //RFduinoBLE.txPowerLevel = 0;
  RFduinoBLE.deviceName = "RFClicker";
  RFduinoBLE.advertisementData = "clicker";
  RFduinoBLE.begin(); 
  digitalWrite(LED, LOW);
  digitalWrite(POWER, HIGH);
  
#ifdef CEREAL
  Serial.println("done with setup()");
#endif
  
}

char button_status = 0;
uint8_t count = 0;
int32_t right_millis = 0;
bool right_held = false;
int32_t down_millis = 0;
bool down_held = false;
int32_t left_millis = 0;
bool left_held = false;
int32_t up_millis = 0;
bool up_held = false;
bool connected = false;
uint32_t now = 0;
uint32_t last_send = 0;
char button_history[HISTORY_LENGTH*2];
void loop() {
  now = millis();
  if (connected) {
    if (digitalRead(RIGHT) == HIGH) {
      if (right_millis == 0) {
        // button was *just* pressed
        right_millis = now;
      }
      else if ((now - right_millis) > HOLD_TIMEOUT)  {
        // if the button is held for a long time, consider that a click and restart
        //   the clock
#ifdef CEREAL
        Serial.println("right button held");
#endif
        button_status |= (1 << 0);
        right_millis = now;
        // note that we've been in this long enough to consider it a hold so there's no
        //   we can avoid triggering a "normal" click after the release if we've gone
        //   BOUNCE_TIMEOUT over into the next hold period.
        right_held = true;
      }
      else {
        // the button hasn't been held down long enough to count as a click
        button_status &= ~(1 << 0);
      }
    }
    else { // RIGHT == LOW
      if (right_millis > 0 && (now - right_millis) >= BOUNCE_TIMEOUT && !right_held) {
        // send a 'clicked' event
#ifdef CEREAL
        Serial.println("right button clicked");
#endif
        button_status |= (1 << 0);
      }
      else {
        // clear the click
        button_status &= ~(1 << 0);
      }
      right_millis = 0;
      right_held = false;
    }

    if (digitalRead(DOWN) == HIGH) {
      if (down_millis == 0) {
        down_millis = now;
      }
      else if ((now - down_millis) > HOLD_TIMEOUT) {
#ifdef CEREAL
        Serial.println("down button held");
#endif
        button_status |= (1 << 1);
        down_millis = now;
        down_held = true;
      }
      else {
        button_status &= ~(1 << 1);
      }
    }  
    else {
      if (down_millis > 0 && (now - down_millis) >= BOUNCE_TIMEOUT && !down_held) {
#ifdef CEREAL
        Serial.println("down button clicked");
#endif
        button_status |= (1 << 1);
      }
      else {
        button_status &= ~(1 << 1);
      }
      down_millis = 0;
      down_held = false;
    }

    if (digitalRead(LEFT) == HIGH) {
      if (left_millis == 0) {
        left_millis = now;
      }
      else if ((now - left_millis) > HOLD_TIMEOUT) {
#ifdef CEREAL
        Serial.println("left button held");
#endif
        button_status |= (1 << 2);
        left_millis = now;
        left_held = true;
      }
      else {
        button_status &= ~(1 << 2);
      }
    }
    else { // LEFT == LOW
      if (left_millis > 0 && (now - left_millis) >= BOUNCE_TIMEOUT && !left_held) {
#ifdef CEREAL
        Serial.println("left button clicked");
#endif
        button_status |= (1 << 2);
      }
      else {
        button_status &= ~(1 << 2);
      }
      left_millis = 0;
      left_held = false;
    }
    
    if (digitalRead(UP) == HIGH) {
      if (up_millis == 0) {
        up_millis = now;
      }
      else if ((now - up_millis) > HOLD_TIMEOUT) {
#ifdef CEREAL
        Serial.println("up button held");
#endif
        button_status |= (1 << 3);
        up_millis = now;
        up_held = true;
      }
      else {
        button_status &= ~(1 << 3);
      }
    }
    else { // UP == LOW
      if (up_millis > 0 && (now - up_millis) >= BOUNCE_TIMEOUT && !up_held) {
#ifdef CEREAL
        Serial.println("up button clicked");
#endif
        button_status = button_status | (1 << 3);
      }
      else {
        button_status = button_status & ~(1 << 3);
      }
      up_millis = 0;
      up_held = false;
    }
    
    if (button_status != button_history[(HISTORY_LENGTH*2)-1]) {
      count++;
      if (count == 0) {
        count = 1;
      }
      push(count, button_history, HISTORY_LENGTH*2);
      push(button_status, button_history, HISTORY_LENGTH*2);
      digitalWrite(LED, HIGH);
      last_send = now;
      RFduinoBLE.send(button_history, HISTORY_LENGTH*2);
    }
    if (now - last_send > SEND_BLINK_TIME) {
      analogWrite(LED, 128);
    }
  } // connected
} // loop()

void RFduinoBLE_onConnect() {
#ifdef CEREAL
  Serial.println("connected()");
#endif
  fillArray(0, button_history, HISTORY_LENGTH*2);
  RFduinoBLE.send(button_history, HISTORY_LENGTH*2);
  count = 0;
  analogWrite(LED, 128);
  connected = true;
}

void RFduinoBLE_onDisconnect() {
#ifdef CEREAL
  Serial.println("disconnected()");
#endif
  digitalWrite(LED, LOW);
  connected = false;
}

void push(char value, char *foo, uint16_t size) {
  for (uint16_t i = 1; i<size; i++) {
    foo[i-1] = foo[i];
  }
  foo[size-1] = value;
}

void fillArray(char value, char *foo, uint16_t size) {
  for (uint16_t i = 0; i<size; i++) {
    foo[i] = value;
  }
}
