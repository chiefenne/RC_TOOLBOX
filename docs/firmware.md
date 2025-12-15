# Firmware

## Building from source

### Prerequisites

- [VS Code](https://code.visualstudio.com/)
- [PlatformIO extension](https://platformio.org/install/ide?install=vscode)

### Build steps

1. Open the `RC_TOOLBOX` folder in VS Code
2. PlatformIO will automatically download dependencies (ESP32 toolchain, LVGL, TFT_eSPI, etc.)
3. Click **Build** (✓ checkmark) in the PlatformIO toolbar

### Upload to device

1. Connect your ESP32-S3 via USB
2. Click **Upload** (→ arrow) in the PlatformIO toolbar
3. Wait for the upload to complete
4. The device will reboot automatically

Alternatively, use the terminal:

```bash
pio run --target upload
```

### Monitor serial output

To see debug messages:

```bash
pio device monitor
```

Or click the **Serial Monitor** icon in PlatformIO.

---

## Pre-built binaries

Pre-built firmware is available in the `binaries/` folder:

| File | Description |
|------|-------------|
| `firmware.bin` | ESP32-S3 firmware |

### Flashing with esptool

If you don't want to build from source, flash the pre-built binary:

```bash
pip install esptool
esptool.py --chip esp32s3 --port /dev/ttyUSB0 write_flash 0x10000 binaries/firmware.bin
```

Replace `/dev/ttyUSB0` with your serial port:
- **macOS**: `/dev/cu.usbmodem*` or `/dev/cu.SLAB_USBtoUART`
- **Windows**: `COM3` (check Device Manager)
- **Linux**: `/dev/ttyUSB0` or `/dev/ttyACM0`

---

## macOS Simulator

For GUI development without hardware, a macOS simulator is included:

```bash
./simulator/build_sim.sh      # Release build
./simulator/build_sim_debug.sh  # Debug build
```

Pre-built simulator binaries are in `binaries/`:
- `lvgl_simulator_macOS` – Release
- `lvgl_simulator_macOS_debug` – Debug

Run the simulator:
```bash
./binaries/lvgl_simulator_macOS
```

---

## Touch calibration

If touch input is offset, run the calibration environment:

1. Edit `platformio.ini` by switching from the default environment to the touch environment:

```bash
[platformio]
; default_envs = esp32-s3-devkitc-1
; src_dir = src
default_envs = touch-calibration
src_dir = touch_calibration
```

2. Build and upload
3. Follow on-screen instructions to tap calibration points
4. Note the calibration values shown on screen
5. Update `src/main.cpp` with your calibration data:
   ```cpp
   uint16_t calData[5] = {403, 3387, 375, 3250, 7};  // Your values
   tft.setTouch(calData);
   ```

6. Switch back to main environment and re-upload
