

#include <Homie.h>

#define FW_NAME "rollers-firmware"
#define FW_VERSION "1.0.0"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */

const int PIN_RELAY_1 = D1;
const int PIN_RELAY_2 = D5;
const int PIN_LDR = A0;

const int CONTACT_TIMEOUT = 3000;
const int CONTACT_TIMEOUT2 = 1000;
const int LUMINOSITY_INTERVAL = 1;

unsigned long lastLuminositySent = 0;

HomieNode cmvNode("rollers", "switch");
HomieNode luminosityNode("luminosity", "luminosity");

void idle() {
  digitalWrite(PIN_RELAY_1, LOW);
  digitalWrite(PIN_RELAY_2, LOW);
  Serial.println("Roller shutters are idle");
}

void cmdgrp() {
  digitalWrite(PIN_RELAY_1, HIGH);
  digitalWrite(PIN_RELAY_2, HIGH);
}

void open() {
  digitalWrite(PIN_RELAY_1, HIGH);
  digitalWrite(PIN_RELAY_2, LOW);
  Homie.setNodeProperty(cmvNode, "state", "open");
  Serial.println("Roller shutters are opened");
}

void close() {
  digitalWrite(PIN_RELAY_1, LOW);
  digitalWrite(PIN_RELAY_2, HIGH);
  Homie.setNodeProperty(cmvNode, "state", "close");
  Serial.println("Roller shutters are closed");
}

bool cmvStateHandler(String value) {
  if (value == "open") {
    cmdgrp();
    delay(CONTACT_TIMEOUT);
    idle();
    delay(CONTACT_TIMEOUT2);
    open();
    delay(CONTACT_TIMEOUT2);
    idle();
  } else if (value == "close") {
    cmdgrp();
    delay(CONTACT_TIMEOUT);
    idle();
    delay(CONTACT_TIMEOUT2);
    close();
    delay(CONTACT_TIMEOUT2);
    idle();
  } else {
    return false;
  }

  return true;
}
void loopHandler() {
  if (millis() - lastLuminositySent >= LUMINOSITY_INTERVAL * 1000UL || lastLuminositySent == 0) {
    int luminosity = analogRead(PIN_LDR);

    if (!Homie.setNodeProperty(luminosityNode, "state", String(luminosity), true)) {
      Serial.println("Luminosity sending failed");
    } else {
      lastLuminositySent = millis();
    }
  }
}

void setup() {
  // Set to normally closed by default
  pinMode(PIN_RELAY_1, OUTPUT);
  digitalWrite(PIN_RELAY_1, LOW);
  pinMode(PIN_RELAY_2, OUTPUT);
  digitalWrite(PIN_RELAY_2, LOW);

  Homie.setFirmware(FW_NAME, FW_VERSION);
  cmvNode.subscribe("state", cmvStateHandler);
  Homie.registerNode(luminosityNode);
  Homie.registerNode(cmvNode);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
}

void loop() {
  Homie.loop();
}

