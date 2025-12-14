<p align="center">
  <img src="assets/RC_Toolbox_Logo.png" width="280">
</p>

> **⚠️ Work in Progress**
> This project is currently under active development. Features and documentation may change frequently.

RC TOOLBOX is a suite of software tools tailored for RC enthusiasts. Designed to run on mainly on ESP32 based boards. It provides a versatile platform for enhancing your RC experience.

## Features

### Main Modules

- **Servo Tester** – Test and calibrate up to 6 servos simultaneously
  - Sidebar with individual servo selection (toggle single or long-press for all)
  - Manual PWM control or auto-sweep mode
  - Protocol presets: Standard, Extended, Sanwa, Futaba, Digital Fast
  - Customizable PWM range (min/center/max) and frequency (50Hz / 333Hz)
  - Hardware PWM output on GPIO pins 6, 15, 16, 17, 18, 21
- **Servo Signal Analyzer** – Read and analyze incoming servo/RC signals
  - Measure PWM frequency and pulse width (µs)
  - Track min/max pulse values over time
  - Useful for receiver output testing and servo signal debugging
- **Lipo Checker** – Monitor LiPo battery cell voltages for safe charging and storage
- **Battery information** readout and store via NFC tags
- **CG Scale** – Measure center of gravity for precise aircraft balancing
- **Flap Deflection** – Measure control surface deflection angles
- **Angle of Incidence** – Measure wing and stabilizer incidence angles

### User Input

- **Touch screen** – Intuitive touch-based interaction on ILI9341 TFT display
- **EC11 rotary encoder with push button** – Precise value adjustment and menu navigation
- **Tactile buttons** – Quick shortcuts for Home, Back, and Action commands

### User Experience

- **Multi-language support** – English, German, French, Spanish, Italian, Dutch, Czech
- **Persistent settings** – Language, background color, and preferences saved to JSON
- **Customizable UI** – Multiple background color themes (Light Gray, White, Light Blue, Light Green, Cream)
- **Cross-platform development** – macOS simulator for rapid GUI development without hardware

## Roadmap

### Core Infrastructure

- ✅ LVGL 9.4 GUI framework with macOS simulator
- ✅ ESP32-S3 hardware integration (TFT, touch, encoder)
- ✅ Persistent settings (JSON on SPIFFS)
- ✅ Multi-language support (7 languages)
- ✅ Custom focus navigation with FocusOrderBuilder
- ⬜ SD card support for logging/export

### Servo Tester

- ✅ Basic PWM output on 6 channels (GPIO 6, 15, 16, 17, 18, 21)
- ✅ Manual mode with encoder control
- ✅ Auto-sweep mode
- ✅ Multi-servo selection (individual toggle, long-press for all)
- ⬜ Protocol presets (Standard, Extended, Sanwa, Futaba, Digital Fast)
- ⬜ Save/load servo profiles
- ⬜ Sweep speed adjustment

### Servo Signal Analyzer

- ⬜ Basic page structure
- ⬜ PWM input capture (pulse width measurement)
- ⬜ Frequency detection
- ⬜ Live pulse width display (µs)
- ⬜ Min/max tracking with reset
- ⬜ Signal quality indicator

### Lipo Checker

- ✅ Basic page structure
- ⬜ ADC voltage reading
- ⬜ Cell count detection
- ⬜ Per-cell voltage display
- ⬜ Low voltage warning

### Cellinator – battery tagging

- ⬜ NFC tags per battery
- ⬜ Store and update battery meta data
  - Purchase/installation date
  - Cell type/count
  - Nominal capacity
  - Charge/discharge cycles
  - Min/max cell voltages
  - Internal cell resistance
  - and more ...

### CG Scale

- ✅ Basic page structure
- ⬜ Load cell integration (HX711)
- ⬜ Calibration routine
- ⬜ CG calculation algorithm

### Flap Deflection

- ✅ Basic page structure
- ⬜ IMU integration (MPU6050/ICM20948)
- ⬜ Angle measurement
- ⬜ Reference point calibration

### Angle of Incidence

- ✅ Basic page structure
- ⬜ IMU angle readout
- ⬜ Zero-point calibration
- ⬜ Relative angle display

## GUI

The application features a graphical user interface (GUI) built with [LVGL](https://lvgl.io/) for displaying information and controlling the software. The GUI is optimized for **ILI9341 TFT touch displays** (320x240 resolution).

<p align="center">
  <img src="assets/Splash_screen.png" width="400"><br>
  <em>Splash screen running in the LVGL simulator</em>
</p>

<p align="center">
  <img src="assets/Home_screen.png" width="400"><br>
  <em>Home screen running in the LVGL simulator</em>
</p>

<p align="center">
  <img src="assets/Servo_screen.png" width="400"><br>
  <em>Servo screen running in the LVGL simulator</em>
</p>

## Hardware

### Development PCB

<p align="center">
    <span>
        <img src="assets/Custom_PCB_01.png" width="220" style="vertical-align:middle; margin-right:16px;">
        <img src="assets/Custom_PCB_03.png" width="400" style="vertical-align:middle;">
    </span>
    <br>
    <em>Custom development PCB for ESP32-S3 DevKitC-1 and ILI9341 2.4&quot; TFT touch display</em>
</p>

A custom PCB has been designed for software development to minimize cable clutter and provide a clean, reliable test platform.
The PCB ODB++ files are available from the [AISLER PCB Project Page](https://aisler.net/p/NGHJDVLN).
The schematics can be found in the assets folder.

> **Note:** The PCB is an early prototype and has not yet been fully validated. Electrical performance, power requirements, and potential design issues have not been thoroughly analyzed.

> **Known Issue:** The silkscreen labels for the LED power jumper (display backlight at 100%) are swapped—3.3V and 5V are reversed.

## Building

### Prerequisites

- **macOS**: Xcode command line tools, SDL2 (`brew install sdl2`)
- **VS Code**: PlatformIO extension for ESP32 development

### macOS Simulator
```bash
./simulator/build_sim.sh
./simulator/build_sim_debug.sh
./binaries/lvgl_simulator_macOS
```

#### Simulator Keyboard Shortcuts
| Key | Action |
|-----|--------|
| Arrow keys | Encoder rotation (adjust values) |
| Enter | Encoder press (confirm) |
| H | Home button |
| Space | Action button (start/stop) |
| Esc | Back |

### ESP32 (via VS Code + PlatformIO)
Use the PlatformIO sidebar in VS Code:
- **Build**: Click the checkmark icon or `Ctrl+Alt+B`
- **Upload**: Click the arrow icon or `Ctrl+Alt+U`
- **Monitor**: Click the plug icon for serial output

See [LVGL_Simulator](https://github.com/chiefenne/LVGL_Simulator) for detailed LVGL library setup instructions.
