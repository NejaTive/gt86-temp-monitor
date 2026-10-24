# GT86 Temperature Monitor

**English** | [Русский](README.ru.md)

Oil and coolant temperature monitor for **Toyota GT86 (2012–2014, FA20)**.

Target hardware:

- **MCU:** ESP32-WROOM-32 (DevKit V1), Bluetooth Classic SPP
- **OBD-II:** ROKODIL ScanX (ELM327-compatible)
- **Display:** TFT IPS 1.54″ 240×240 **ST7789V** (SPI) — pinout agreed (see below)
- **ESP32 power:** car USB (backlight follows USB/ACC power)

Simulation is via the **Wokwi extension in Cursor / VS Code**.

Current code status:

- UI (°C only), labels and values centered; short BT/OBD status top-left
- Fresh value `95 C`, stale `95* C`, no data `-- C`
- Thresholds in `config.h`: green from 80 °C, red from 115 °C
- **SIM** (`esp32dev_wokwi_tft`): 2 pots (coolant / oil); green **BT** button = connect/disconnect (no real Bluetooth)
- **PROD** (`esp32dev`): OBD over Bluetooth Classic SPP — coolant `01 05`, oil `22 10 E7` (not `01 5C`)

---

## Layout

One repo, folders by role (`common` / `prod` / `sim`):

```
gt86-temp-monitor/
├── platformio.ini
├── wokwi.toml                 # Wokwi extension → PlatformIO firmware
├── diagram.json               # Wokwi simulator schematic
├── include/
│   ├── common/                # shared: config.h, UI, obd_parse
│   ├── prod/                  # config_prod.h, obd_bt, obd_client
│   └── sim/                   # config_sim.h, bt_sim, temp_sim, sim_controls
├── src/
│   ├── common/                # display_ui, obd_parse
│   ├── prod/                  # obd_*, main.cpp (car)
│   └── sim/                   # bt_sim, temp_sim, sim_controls, main.cpp (Wokwi)
├── test/test_obd_parse/       # parser unit tests (TZ examples)
├── README.md
└── README.ru.md
```

`esp32dev` builds only `common` + `prod`. `esp32dev_wokwi_tft` builds only `common` + `sim`.

---

## SIM vs PROD

| | Wokwi SIM | Car PROD |
|---|-----------|----------|
| PlatformIO env | `esp32dev_wokwi_tft` | `esp32dev` |
| `HW_PROFILE` | 1 | 0 |
| Source folders | `common` + `sim` | `common` + `prod` |
| Data source | 2 pots / Serial (when BT-SIM connected) | ScanX OBD |
| TFT | ILI9341 stand-in | ST7789V |
| Bluetooth | simulated by green button (GPIO27) | Classic SPP |

Edit adapter name and PIN in [`include/prod/config_prod.h`](include/prod/config_prod.h):

```c
#define OBD_BT_NAME "ScanX"   // exact name from phone Bluetooth list
#define OBD_BT_PIN  "1234"
```

SIM pots / range: [`include/sim/config_sim.h`](include/sim/config_sim.h). Shared thresholds and TFT pins: [`include/common/config.h`](include/common/config.h).

On the car: only this project should use the adapter (disconnect Car Scanner / phone BT).

---

## Simulator (Wokwi extension)

### Once
1. Install the **Wokwi** extension.
2. `F1` → **Wokwi: Request a new License** (activate account).
3. Open this repository root as the workspace.

### Every time
```bash
pio run -e esp32dev_wokwi_tft    # firmware with TFT (schematic with display)
```

Then: `F1` → **Wokwi: Start Simulator**.

Root files required: `wokwi.toml` + `diagram.json` (already present).

The simulator schematic uses **ILI9341** (built into Wokwi). The real module is **ST7789V**. TFT GPIO pinout matches the car table below.

Wokwi extras:

- Pot **GPIO34** → coolant 0…120 °C (only while “BT connected”)
- Pot **GPIO35** → oil 0…120 °C (only while “BT connected”)
- Green **BT** button **GPIO27** → connect with delay (`Offline` → `Connecting` → `Starting` → `Online` + temps); press again → `Offline`

---

## PlatformIO (macOS)

Installed under `~/.platformio/penv/bin` (`pio` on PATH via `~/.zshrc`).

```bash
pio --version
pio run -e esp32dev              # real board + ST7789V + OBD
pio run -e esp32dev_wokwi_tft    # Wokwi simulator
```

Parser unit tests (examples from the project spec, runs on the Mac — no board):

```bash
pio test -e native
```

---

## Flash ESP32

Only with your explicit OK (needs a connected board):

```bash
pio run -e esp32dev -t upload
```

Debug Serial output is disabled. Check behaviour on the **TFT** (status + °C). In SIM you can still type `c 95` / `o 110` in the Serial panel to set temps (optional; pots work without it).

### How to verify OBD on the car

1. Set `OBD_BT_NAME` / `OBD_BT_PIN` in `include/prod/config_prod.h`, upload.
2. On the TFT: status should go `Connecting` → `Starting` → `Online`; values `NN C` / stale `NN* C` / missing `-- C`.
3. Firmware asks coolant `0105` and oil `2210E7` (after `ATSH7E0`). Do **not** use oil PID `01 5C` (NO DATA on this car).
4. Optional cross-check with Car Scanner (phone disconnected from the adapter).

First BT connect by name can take up to ~30 s (device inquiry).

---

## GPIO / real display wiring (agreed)

TFT pins in [`include/common/config.h`](include/common/config.h) (same for SIM and PROD). ESP32 from car USB; **BLK → 3V3**.

| ST7789V | ESP32 DevKit |
|---------|--------------|
| GND | GND |
| VCC | 3V3 |
| SCL | D18 |
| SDA | D23 |
| RES | D4 |
| DC | D2 |
| CS | D5 |
| BLK | 3V3 |

Profile **1** (`esp32dev_wokwi_tft`) uses the same TFT pins; only the chip driver and SIM pots/button differ.

---

## Roadmap

1. Confirm `OBD_BT_NAME` / PIN in `config_prod.h`; flash with consent.
2. Road test: coolant + oil vs Car Scanner (phone disconnected from adapter).
3. Optional: connect by BT MAC if name discovery is slow.

---

## Status

Pet project — OBD client + parser in progress; validate on hardware before relying on it while driving.
