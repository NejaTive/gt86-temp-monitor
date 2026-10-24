# GT86 Temperature Monitor

[English](README.md) | **Русский**

Монитор температуры масла и охлаждающей жидкости для **Toyota GT86 (2012–2014, FA20)**.

Целевое железо:

- **Контроллер:** ESP32-WROOM-32 (DevKit V1), Bluetooth Classic
- **OBD-II:** ROKODIL ScanX
- **Дисплей:** TFT IPS 1,54″ 240×240 **ST7789V** (SPI) — распиновка согласована (см. ниже)
- **Питание ESP32:** штатный USB автомобиля (подсветка вместе с питанием)

Симуляция — только через **расширение Wokwi в Cursor \ VS Code**.

Сейчас в коде:

- UI (только °C), названия и значения по центру
- пороги в `config.h`: зелёный с 80 °C, красный с 115 °C
- SIM: температура с потенциометра (0…120 °C)
- OBD / PID масла — ещё нет

---

## Структура

```
gt86-temp-monitor/
├── platformio.ini
├── wokwi.toml              # расширение Wokwi → прошивка PlatformIO
├── diagram.json            # схема для симулятора Wokwi
├── include/config.h
├── src/
├── README.md
└── README.ru.md
```

---

## Симулятор (расширение Wokwi)

### Один раз
1. Расширение **Wokwi** установлено.
2. `F1` → **Wokwi: Request a new License** (активировать аккаунт).
3. Открыт workspace = корень этого репозитория.

### Каждый раз
```bash
pio run -e esp32dev_wokwi_tft    # прошивка с TFT (для схемы с экраном)
```

Затем: `F1` → **Wokwi: Start Simulator**.

Нужны файлы в корне: `wokwi.toml` + `diagram.json` (уже есть).

В схеме симулятора дисплей — **ILI9341** (встроенный в Wokwi). Реальный модуль — **ST7789V**. GPIO TFT совпадает с таблицей ниже; в Wokwi дополнительно потенциометр на GPIO34.

---

## PlatformIO на Mac

Уже установлено в `~/.platformio/penv/bin` (`pio` в PATH через `~/.zshrc`).

```bash
pio --version
pio run -e esp32dev              # реальная плата + ST7789V
pio run -e esp32dev_wokwi_tft    # симулятор Wokwi
```

---

## Загрузка на ESP32

```bash
pio run -e esp32dev -t upload
pio device monitor --baud 115200
```

---

## GPIO / распиновка реального дисплея (согласовано)

Всё в [`include/config.h`](include/config.h). Питание ESP32 — USB авто; **BLK → 3V3**.

| Дисплей ST7789V | ESP32 DevKit |
|-----------------|--------------|
| GND | GND |
| VCC | 3V3 |
| SCL | D18 |
| SDA | D23 |
| RES | D4 |
| DC | D2 |
| CS | D5 |
| BLK | 3V3 |

Профиль **1** (`esp32dev_wokwi_tft`) — те же пины TFT; отличаются только драйвер чипа и потенциометр.

---

## Что дальше

1. Собрать/прошить реальное железо по таблице выше (с согласия).
2. Bluetooth → ScanX; coolant `01 05`, oil `22 10 E7` по ТЗ.

---

## Статус

Pet-проект в разработке.
