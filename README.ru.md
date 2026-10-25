# GT86 Temperature Monitor

[English](README.md) | **Русский**

Монитор температуры масла и охлаждающей жидкости для **Toyota GT86 (2012–2014, FA20)**.

Целевое железо:

- **Контроллер:** ESP32-WROOM-32 (DevKit V1), Bluetooth Classic SPP
- **OBD-II:** ROKODIL ScanX (совместим с ELM327)
- **Дисплей:** TFT IPS 1,54″ 240×240 **ST7789V** (SPI) — распиновка согласована (см. ниже)
- **Питание ESP32:** штатный USB автомобиля (подсветка вместе с питанием)

Симуляция — только через **расширение Wokwi в Cursor \ VS Code**.

Сейчас в коде:

- UI (только °C), названия и значения по центру; краткий статус BT/OBD слева сверху
- Свежее `95 C`, устаревшее `95* C`, нет данных `-- C`
- пороги в `config.h`: зелёный с 80 °C, красный с 115 °C
- **SIM** (`esp32dev_wokwi_tft`): 2 pot (ОЖ / масло); зелёная кнопка **BT** = connect/disconnect (без реального Bluetooth)
- **PROD** (`esp32dev`): OBD по Bluetooth Classic SPP — coolant `01 05`, oil `22 10 E7` (не `01 5C`)

---

## Структура

Один репозиторий, папки по ролям (`common` / `prod` / `sim`):

```
gt86-temp-monitor/
├── platformio.ini
├── wokwi.toml                 # расширение Wokwi → прошивка PlatformIO
├── diagram.json               # схема для симулятора Wokwi
├── include/
│   ├── common/                # общее: config.h, UI, obd_parse
│   ├── prod/                  # config_prod.h, obd_bt, obd_client
│   └── sim/                   # config_sim.h, bt_sim, temp_sim, sim_controls
├── src/
│   ├── common/                # ui_display, obd_parse
│   ├── prod/                  # obd_*, main.cpp (машина)
│   └── sim/                   # bt_sim, temp_sim, sim_controls, main.cpp (Wokwi)
├── test/test_obd_parse/       # unit-тесты парсера (примеры из ТЗ)
├── README.md
└── README.ru.md
```

`esp32dev` собирает только `common` + `prod`. `esp32dev_wokwi_tft` — только `common` + `sim`.

---

## SIM vs PROD

| | Wokwi SIM | Машина PROD |
|---|-----------|-------------|
| env PlatformIO | `esp32dev_wokwi_tft` | `esp32dev` |
| `HW_PROFILE` | 1 | 0 |
| Папки исходников | `common` + `sim` | `common` + `prod` |
| Источник данных | 2 pot (если BT-SIM «подключён») | ScanX OBD |
| TFT | ILI9341 stand-in | ST7789V |
| Bluetooth | симуляция зелёной кнопкой (GPIO27) | Classic SPP |

Имя адаптера и PIN — в [`include/prod/config_prod.h`](include/prod/config_prod.h):

```c
#define OBD_BT_NAME "ScanX"   // как в списке Bluetooth телефона
#define OBD_BT_PIN  "1234"
```

Два pot / диапазон SIM: [`include/sim/config_sim.h`](include/sim/config_sim.h). Общие пороги и пины TFT: [`include/common/config.h`](include/common/config.h).

В машине к адаптеру должен быть подключён только этот проект (отключи Car Scanner / BT телефона).

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

В схеме симулятора дисплей — **ILI9341** (встроенный в Wokwi). Реальный модуль — **ST7789V**. GPIO TFT совпадает с таблицей ниже.

В Wokwi дополнительно:

- Pot **GPIO34** → ОЖ 0…120 °C (только пока «BT подключён»)
- Pot **GPIO35** → масло 0…120 °C (только пока «BT подключён»)
- Зелёная кнопка **BT** **GPIO27** → connect с задержкой (`Offline` → `Connecting` → `Starting` → `Online` + °C); ещё раз → `Offline`

---

## PlatformIO на Mac

Уже установлено в `~/.platformio/penv/bin` (`pio` в PATH через `~/.zshrc`).

```bash
pio --version
pio run -e esp32dev              # реальная плата + ST7789V + OBD
pio run -e esp32dev_wokwi_tft    # симулятор Wokwi
```

Unit-тесты парсера (примеры из ТЗ, на Mac — плата не нужна):

```bash
pio test -e native
```

---

## Загрузка на ESP32

Только с твоего явного согласия (нужна подключённая плата):

```bash
pio run -e esp32dev -t upload
```

USB Serial в прошивке не используется (нет логов и ввода °C с клавиатуры). Смотри поведение на **TFT**.

### Как проверить OBD в машине

1. Пропиши `OBD_BT_NAME` / `OBD_BT_PIN` в `include/prod/config_prod.h`, прошей.
2. На TFT: статус `Connecting` → `Starting` → `Online`; значения `NN C` / устаревшее `NN* C` / нет данных `-- C`.
3. Прошивка запрашивает coolant `0105` и oil `2210E7` (после `ATSH7E0`). **Не** используй PID масла `01 5C` (на авто — NO DATA).
4. По желанию сверь с Car Scanner (телефон от адаптера отключён).

Первое подключение BT по имени может занять до ~30 с (поиск устройства).

---

## GPIO / распиновка реального дисплея (согласовано)

Пины TFT в [`include/common/config.h`](include/common/config.h) (одинаковы для SIM и PROD). Питание ESP32 — USB авто; **BLK → 3V3**.

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

Профиль **1** (`esp32dev_wokwi_tft`) — те же пины TFT; отличаются только драйвер чипа и pot/кнопка SIM.

---

## Что дальше

1. Уточнить `OBD_BT_NAME` / PIN в `config_prod.h`; прошивка — с согласия.
2. Проверка в авто: coolant + oil рядом с Car Scanner (телефон от адаптера отключён).
3. По желанию: подключение по MAC, если поиск по имени долгий.

---

## Статус

Pet-проект — OBD-клиент и парсер добавлены; на железе нужно проверить до доверия показаниям в поездке.
