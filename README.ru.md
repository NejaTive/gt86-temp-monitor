# GT86 Temperature Monitor

[English](README.md) | **Русский**

Монитор температуры масла и охлаждающей жидкости для **Toyota GT86 (2012–2014, FA20)**.

Целевое железо:

- **Контроллер:** ESP32-WROOM-32 (DevKit V1), Bluetooth Classic
- **OBD-II:** ROKODIL ScanX
- **Дисплей:** TFT IPS 1,54″ 240×240 (предположительно ST7789, SPI) — распиновка пока не зафиксирована
- **Питание ESP32:** штатный USB автомобиля

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

В схеме дисплей — **ILI9341** (встроенный в Wokwi). Реальный модуль планируется как ST7789 — пины для машины по-прежнему TBD в `config.h`.

Без экрана (только Serial/LED): `pio run -e esp32dev` и в `wokwi.toml` пути на `.pio/build/esp32dev/`.

---

## PlatformIO на Mac

Уже установлено в `~/.platformio/penv/bin` (`pio` в PATH через `~/.zshrc`).

```bash
pio --version
pio run -e esp32dev              # реальная плата, TFT выключен
pio run -e esp32dev_wokwi_tft    # симулятор / профиль с дисплеем
```

---

## Загрузка на ESP32

```bash
pio run -e esp32dev -t upload
pio device monitor --baud 115200
```

---

## GPIO

Всё в [`include/config.h`](include/config.h).

- Профиль **0** (`esp32dev`): LED GPIO 2; TFT = TBD.
- Профиль **1** (`esp32dev_wokwi_tft`): пины под схему Wokwi; LED на 13.

Не копируйте wokwi-пины на физический дисплей без проверки модуля.

---

## Что дальше

1. UI в расширении Wokwi (потенциометр, цвета, OVERHEAT).
2. Реальный ESP32 + распиновка ST7789.
3. Bluetooth → ScanX; coolant, затем oil после проверки на FA20.

---

## Статус

Pet-проект в разработке.
