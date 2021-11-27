/*
 * ATOM LiteをBLEキーボードにする。
 *
 * 送信するキーを変更するにはsendKeyの値を変更する。
 * Fast LED 3.4はなぜか光らなかった
 */
#include <BleKeyboard.h> // https://github.com/T-vK/ESP32-BLE-Keyboard
#include <M5Atom.h>
#include "EEPROM.h"

#define MODE_SWITCH_TIME 3000
#define EEPROM_SIZE 1

enum Mode {
  KeyboardMode,
  SettingMode
};

const char KEYS[] = {
  KEY_RETURN,
  KEY_LEFT_ARROW,
  KEY_RIGHT_ARROW,
  KEY_UP_ARROW,
  KEY_DOWN_ARROW
};
const String KEY_DESCS[] = {
  "Enter",
  "Arrow Left",
  "Arrow Right",
  "Arrow Up",
  "Arrow Down"
};
uint8_t sendKey = KEY_RETURN;

const std::string DEVICE_NAME("ATOM Lite BLE Keyboard");
const std::string DEVICE_MANUFACTURER("M5Stack");

const CRGB CRGB_BLE_CONNECTED(0x00, 0xf0, 0x00);
// 数値は緑だが Lite では赤く光る https://github.com/m5stack/M5Atom/issues/5
const CRGB CRGB_BLE_DISCONNECTED(0xf0, 0x00, 0x00);

BleKeyboard bleKeyboard(DEVICE_NAME, DEVICE_MANUFACTURER);
bool isBleConnected = false;
bool isKeyPressed = false;
bool ignoreNextKeyRelease = false;
Mode mode = KeyboardMode;
unsigned long pressBeginTime;

int findIndex(char key) {
  int maxSize = sizeof(KEYS);
  for (int i = 0; i < maxSize; i++) {
    if (KEYS[i] == key) return i;
  }
  return -1;
}

void keyboardLoop(bool isJustKeyPressed, bool isJustKeyReleased) {
  if (!isJustKeyReleased) return;
  bleKeyboard.press(sendKey);
  bleKeyboard.release(sendKey);
}

void settingLoop(bool isJustKeyPressed, bool isJustKeyReleased) {
  if (!isJustKeyReleased) return;

  int maxSize = sizeof(KEYS);
  int i = findIndex(sendKey);
  int nextIndex = (i + 1) % maxSize;
  bleKeyboard.printf("%s\n", KEY_DESCS[nextIndex]);
  sendKey = KEYS[nextIndex];
  return;
}

void restoreSendKey() {
  char ch = char(EEPROM.read(0));
  int index = findIndex(ch);
  if (index != -1) sendKey = ch;
}

void setup() {
  //Serial.begin(115200);
  M5.begin(true, false, true); // Serial: Enable, I2C: Disable, Display: Enable
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to initialise EEPROM");
    delay(1000000);
  }

  restoreSendKey();

  bleKeyboard.begin();
  M5.dis.drawpix(0, CRGB_BLE_DISCONNECTED);
  Serial.println(DEVICE_NAME.c_str());
}

void loop() {
  M5.update();
  if (bleKeyboard.isConnected()) {
    bool isJustKeyPressed = false;
    bool isJustKeyReleased = false;
    
    if (!isBleConnected) {
      M5.dis.drawpix(0, CRGB_BLE_CONNECTED);
      isBleConnected = true;
      Serial.println("Connected");
      return;
    }

    if (isKeyPressed) {
      if (M5.Btn.isReleased()) {
        isJustKeyReleased = true;
      }
    } else {
      if (M5.Btn.isPressed()) {
        isJustKeyPressed = true;
      }
    }

    if (isJustKeyReleased) {
      isKeyPressed = false;
      Serial.println("Released");
      if (ignoreNextKeyRelease) {
        ignoreNextKeyRelease = false;
        return;
      }
    }

    if (isJustKeyPressed) {
      pressBeginTime = millis();
      isKeyPressed = true;
      Serial.println("Pressed");
    }

    if (isKeyPressed && pressBeginTime != 0 && millis() - pressBeginTime > MODE_SWITCH_TIME) {
      switch (mode) {
      case KeyboardMode:
        bleKeyboard.printf("Start Setting\n");
        mode = SettingMode;
        break;
      case SettingMode:
        EEPROM.write(0, sendKey);
        EEPROM.commit();
        bleKeyboard.printf("Finish Setting\n");
        mode = KeyboardMode;
        break;
      }
      pressBeginTime = 0;
      ignoreNextKeyRelease = true;
      return;
    }

    switch (mode) {
    case KeyboardMode:
      keyboardLoop(isJustKeyPressed, isJustKeyReleased);
      break;
    case SettingMode:
      settingLoop(isJustKeyPressed, isJustKeyReleased);
      break;
    }
  } else {
    if (isBleConnected) {
      M5.dis.drawpix(0, CRGB_BLE_DISCONNECTED);
      isBleConnected = false;
      isKeyPressed = false;
      Serial.println("Disconnected");
    }
  }
  delay(100);
}
