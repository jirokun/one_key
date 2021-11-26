/*
 * ATOM LiteをBLEキーボードにする。
 *
 * 送信するキーを変更するにはSEND_KEYの値を変更する。
 */

#include <BleKeyboard.h> // https://github.com/T-vK/ESP32-BLE-Keyboard
#include <M5Atom.h>

const uint8_t SEND_KEY = 0x0a;

const std::string DEVICE_NAME("ATOM Lite BLE Keyboard");
const std::string DEVICE_MANUFACTURER("M5Stack");

const CRGB CRGB_BLE_CONNECTED(0x00, 0x00, 0xf0);
// 数値は緑だが Lite では赤く光る https://github.com/m5stack/M5Atom/issues/5
const CRGB CRGB_BLE_DISCONNECTED(0x00, 0xf0, 0x00);

BleKeyboard bleKeyboard(DEVICE_NAME, DEVICE_MANUFACTURER);
bool isBleConnected = false;
bool isKeyPressed = false;
unsigned long pressBeginTime = 0;
unsigned long pressEndTime = 0;

CRGB dispColor(uint8_t g, uint8_t r, uint8_t b) {
  return (CRGB)((g << 16) | (r << 8) | b);
}

void setup() {
  M5.begin(true, false, true); // Serial: Enable, I2C: Disable, Display: Enable
  bleKeyboard.begin();
  M5.dis.drawpix(0, 0xf00000);
  Serial.println(DEVICE_NAME.c_str());
}

void loop() {
  M5.update();
  if (bleKeyboard.isConnected()) {
    if (!isBleConnected) {
      M5.dis.drawpix(0, 0xf00000);
      isBleConnected = true;
      Serial.println("Connected");
    }
    if (isKeyPressed) {
      if (M5.Btn.isReleased()) {
        isKeyPressed = false;
        Serial.println("Released");
        pressEndTime = millis();
        unsigned long pressTime = pressEndTime - pressBeginTime;
        if (pressTime >= 3000 && pressTime < 10000) {
          modeChange();
        }
      }
    } else {
      if (M5.Btn.isPressed()) {
        bleKeyboard.press(SEND_KEY);
        bleKeyboard.release(SEND_KEY);
        isKeyPressed = true;
        Serial.println("Pressed");
        pressBeginTime = millis();
      }
    }
  } else {
    if (isBleConnected) {
      M5.dis.drawpix(0, dispColor(0, 1, 0));
      isBleConnected = false;
      isKeyPressed = false;
      Serial.println("Disconnected");
    }
  }
  delay(100);
}