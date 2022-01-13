#include <Arduino.h>
#include <M5StickCPlus.h>
#include <BleKeyboard.h>

#define BATT_MIN 3.0
#define BATT_MAX 4.2
#define UPDATE_INTERVAL 60000

BleKeyboard bleKeyboard("Zoom Keeb");

void muteUnmute()
{
  static bool state_change = false;

  if (M5.BtnB.wasPressed())
  {
    M5.Beep.tone(4000);
    delay(100);
    M5.Beep.mute();
    delay(100);
    state_change = true;
  }

  if (state_change)
  {
    M5.Beep.tone(4000);
    delay(100);
    M5.Beep.mute();

    bleKeyboard.press(KEY_LEFT_GUI);
    delay(10);
    bleKeyboard.press(KEY_LEFT_SHIFT);
    delay(10);
    bleKeyboard.press(0x41);
    delay(30);
    bleKeyboard.releaseAll();
    delay(30);
    state_change = false;
  }
}

void sendSpace()
{
  static bool send_space = false;

  if (M5.BtnA.wasPressed())
  {
    M5.Beep.tone(4000);
    delay(100);
    M5.Beep.mute();
    delay(100);
    send_space = true;
  }

  if (M5.BtnA.isReleased())
  {
    send_space = false;
  }

  if (send_space)
  {
    bleKeyboard.press(0x20);
    delay(10);
  }
  else
  {
    bleKeyboard.release(0x20);
    delay(10);
  }
}

void updateBatteryLevel()
{
  static int last_updated = 0;

  int now = millis();
  if (UPDATE_INTERVAL < (now - last_updated))
  {
    float voltage = M5.Axp.GetBatVoltage();

    if (voltage > BATT_MAX)
    {
      voltage = BATT_MAX;
    }

    uint8_t capacity = (uint8_t)(((voltage - BATT_MIN) / (BATT_MAX - BATT_MIN)) * 100);
    bleKeyboard.setBatteryLevel(capacity);

    last_updated = now;
  }
}

void setup()
{
  M5.begin();
  M5.Lcd.setTextSize(5);
  M5.Lcd.setRotation(0);
  M5.Lcd.setCursor(10, 10, 1);
  M5.Lcd.print("ZOOM");
  M5.Lcd.setCursor(10, 50, 1);
  M5.Lcd.print("KEEB");
  bleKeyboard.begin();
}

void loop()
{
  if (bleKeyboard.isConnected())
  {
    M5.update();
    updateBatteryLevel();
    muteUnmute();
    sendSpace();
  }
  delay(100);
}
