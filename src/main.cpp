/*
 * ATOM LiteをBLEキーボードにする。
 *
 * 送信するキーを変更するにはSEND_KEYの値を変更する。
 */

#include <BleKeyboard.h> // https://github.com/T-vK/ESP32-BLE-Keyboard
#include <M5Atom.h>

const uint8_t SEND_KEY = 0x0a; // 0x20: スペース

const std::string DEVICE_NAME("ATOM Lite BLE Keyboard");
const std::string DEVICE_MANUFACTURER("M5Stack");

const CRGB CRGB_BLE_CONNECTED(0x00, 0x00, 0xf0);
// 数値は緑だが Lite では赤く光る https://github.com/m5stack/M5Atom/issues/5
const CRGB CRGB_BLE_DISCONNECTED(0x00, 0xf0, 0x00);

BleKeyboard bleKeyboard(DEVICE_NAME, DEVICE_MANUFACTURER);
bool isBleConnected = false;
bool isKeyPressed = false;

void setup() {
  //Serial.begin(115200);
  M5.begin(true, false, true); // Serial: Enable, I2C: Disable, Display: Enable
  bleKeyboard.begin();
  M5.dis.drawpix(0, CRGB_BLE_DISCONNECTED);
  Serial.println(DEVICE_NAME.c_str());
}

void loop() {
  M5.update();
  if (bleKeyboard.isConnected()) {
    if (!isBleConnected) {
      M5.dis.drawpix(0, CRGB_BLE_CONNECTED);
      isBleConnected = true;
      Serial.println("Connected");
    }
    if (isKeyPressed) {
      if (M5.Btn.isReleased()) {
        isKeyPressed = false;
        Serial.println("Released");
      }
    } else {
      if (M5.Btn.isPressed()) {
        bleKeyboard.press(SEND_KEY);
        bleKeyboard.release(SEND_KEY);
        isKeyPressed = true;
        Serial.println("Pressed");
      }
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