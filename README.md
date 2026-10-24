# GT86 Temperature Monitor

**English** | [Русский](README.ru.md)

Oil and coolant temperature monitor for **Toyota GT86 (2012–2014, FA20)**.

Target hardware:

- **MCU:** ESP32-WROOM-32 (DevKit V1), Bluetooth Classic
- **OBD-II:** ROKODIL ScanX
- **Display:** TFT IPS 1.54″ 240×240 (likely ST7789, SPI) — pinout not finalized yet
- **ESP32 power:** car USB

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

The schematic uses **ILI9341** (built into Wokwi). The real module is planned as ST7789 — car pins remain TBD in `config.h`.

Without display (Serial/LED only): `pio run -e esp32dev` and point `wokwi.toml` at `.pio/build/esp32dev/`.

---

## PlatformIO (macOS)

Installed under `~/.platformio/penv/bin` (`pio` on PATH via `~/.zshrc`).

```bash
pio --version
pio run -e esp32dev              # real board, TFT off
pio run -e esp32dev_wokwi_tft    # simulator / display profile
```

---

## Flash ESP32

```bash
pio run -e esp32dev -t upload
pio device monitor --baud 115200
```

---

## GPIO

All pins live in [`include/config.h`](include/config.h).

- Profile **0** (`esp32dev`): LED GPIO 2; TFT = TBD.
- Profile **1** (`esp32dev_wokwi_tft`): pins match the Wokwi schematic; LED on 13.

Do not copy Wokwi pins onto a physical display without checking the module.

---

## Roadmap

1. UI in the Wokwi extension (potentiometer, colors, OVERHEAT).
2. Real ESP32 + ST7789 pinout.
3. Bluetooth → ScanX; coolant first, then oil after FA20 validation.

---

## Status

Pet project — work in progress.
