# GT86 Temperature Monitor

**English** | [Русский](README.ru.md)

Oil and coolant temperature monitor for **Toyota GT86 (2012–2014, FA20)**.

Target hardware:

- **MCU:** ESP32-WROOM-32 (DevKit V1), Bluetooth Classic
- **OBD-II:** ROKODIL ScanX
- **Display:** TFT IPS 1.54″ 240×240 **ST7789V** (SPI) — pinout agreed (see below)
- **ESP32 power:** car USB (backlight follows USB/ACC power)

Simulation is via the **Wokwi extension in Cursor / VS Code**.

Current code status:

- UI (°C only), labels and values centered
- Thresholds in `config.h`: green from 80 °C, red from 115 °C
- SIM: temperature from potentiometer (0…120 °C)
- OBD / oil PID — not implemented yet

---

## Layout

```
gt86-temp-monitor/
├── platformio.ini
├── wokwi.toml              # Wokwi extension → PlatformIO firmware
├── diagram.json            # Wokwi simulator schematic
├── include/config.h
├── src/
├── README.md
└── README.ru.md
```

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

The simulator schematic uses **ILI9341** (built into Wokwi). The real module is **ST7789V**. TFT GPIO pinout matches the car table below; Wokwi also has a potentiometer on GPIO34.

---

## PlatformIO (macOS)

Installed under `~/.platformio/penv/bin` (`pio` on PATH via `~/.zshrc`).

```bash
pio --version
pio run -e esp32dev              # real board + ST7789V
pio run -e esp32dev_wokwi_tft    # Wokwi simulator
```

---

## Flash ESP32

```bash
pio run -e esp32dev -t upload
pio device monitor --baud 115200
```

---

## GPIO / real display wiring (agreed)

All pins in [`include/config.h`](include/config.h). ESP32 from car USB; **BLK → 3V3**.

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

Profile **1** (`esp32dev_wokwi_tft`) uses the same TFT pins; only the chip driver and pot differ.

---

## Roadmap

1. Wire/flash real hardware per the table above (with consent).
2. Bluetooth → ScanX; coolant `01 05`, oil `22 10 E7` per the project spec.

---

## Status

Pet project — work in progress.
