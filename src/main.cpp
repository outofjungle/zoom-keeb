#include <Arduino.h>
#include <M5StickCPlus.h>
#include <BleKeyboard.h>

#define BATT_MIN 3.0
#define BATT_MAX 4.2
#define UI_UPDATE_INTERVAL 1000
#define UPDATE_INTERVAL 60000

BleKeyboard bleKeyboard("Zoom Keeb");
unsigned long now;

void muteUnmute()
{
  static bool state_change = false;

  if (M5.BtnA.wasPressed())
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

  if (M5.BtnB.wasPressed())
  {
    M5.Beep.tone(4000);
    delay(100);
    M5.Beep.mute();
    delay(100);
    send_space = true;
  }

  if (M5.BtnB.isReleased())
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

uint8_t batteryLevel()
{
  static int last_updated = 0;
  static uint8_t lvl = 0;

  if (UI_UPDATE_INTERVAL < (now - last_updated))
  {
    float voltage = M5.Axp.GetBatVoltage();

    if (voltage > BATT_MAX)
    {
      voltage = BATT_MAX;
    }

    lvl = (uint8_t)(((voltage - BATT_MIN) / (BATT_MAX - BATT_MIN)) * 100);

    last_updated = now;
  }

  return lvl;
}

void btUpdateBatteryLevel(uint8_t lvl = 0)
{
  static int last_updated = 0;

  if (UPDATE_INTERVAL < (now - last_updated))
  {
    bleKeyboard.setBatteryLevel(lvl);
    last_updated = now;
  }
}

bool isNotMoved()
{
  static unsigned long last_moved = 0;
  static float lastX, lastY, lastZ = 0;
  float x, y, z, dx, dy, dz;

  M5.IMU.getGyroData(&x, &y, &z);

  dx = lastX - abs(x);
  dy = lastY - abs(y);
  dz = lastZ - abs(z);

  lastX = x;
  lastY = y;
  lastZ = z;
  float avgG = (dx + dy + dz) / 3;

  if (avgG > 10)
  {
    last_moved = millis();
  }

  if (UPDATE_INTERVAL < (now - last_moved))
  {
    return true;
  }
  return false;
}

void renderScreen(uint8_t brightness = 10, uint8_t battLvl = 0)
{
  static bool staticViewRendered = false;
  static unsigned long last_updated = 0;
  static uint8_t last_brightness = 10;

  if (!staticViewRendered)
  {
    M5.Lcd.setTextSize(5);
    M5.Lcd.setRotation(0);
    M5.Lcd.fillRect(0, 0, 135, 30, DARKGREY);
    M5.Lcd.fillRect(75, 5, 50, 20, BLACK);
    M5.Lcd.fillRect(125, 8, 5, 14, BLACK);

    M5.Lcd.setTextColor(BLUE, BLACK);
    M5.Lcd.setCursor(10, 40, 1);
    M5.Lcd.print("ZOOM");
    M5.Lcd.setCursor(10, 80, 1);
    M5.Lcd.print("KEEB");
    staticViewRendered = true;
  }

  bool updateNow = false;
  if (UI_UPDATE_INTERVAL < (now - last_updated))
  {
    updateNow = true;
    last_updated = now;
  }

  if (brightness != last_brightness)
  {
    updateNow = true;
    last_brightness = brightness;
  }

  if (updateNow)
  {
    M5.Axp.ScreenBreath(brightness);

    uint8_t lvlWidth = (uint8_t)((battLvl * 46) / 100);
    M5.Lcd.fillRect(75, 5, 50, 20, BLACK);
    M5.Lcd.fillRect(77, 7, lvlWidth, 16, GREEN);
  }
}

void setup()
{
  M5.begin();
  bleKeyboard.begin();
  M5.IMU.Init();
}

void loop()
{
  now = millis();

  M5.update();

  uint8_t brightness = (isNotMoved()) ? 7 : 10;
  uint8_t battLvl = batteryLevel();

  renderScreen(brightness, battLvl);

  if (bleKeyboard.isConnected())
  {
    btUpdateBatteryLevel(battLvl);
    muteUnmute();
    sendSpace();
  }
  delay(100);
}
