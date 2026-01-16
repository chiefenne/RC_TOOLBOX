<p align="center">
  <img src="assets/RC_Toolbox_Logo.png" width="280">
</p>

> **⚠️ Work in Progress**
> This project is currently under active development. Features and documentation may change frequently.

# RC TOOLBOX

RC TOOLBOX is a suite of tools tailored for RC enthusiasts.
It is designed to run mainly on ESP32-based boards and provides a versatile platform for RC setup, testing, and measurement tasks.

## 📘 Documentation

The **full user documentation** (features, modules, roadmap, hardware, usage, simulator, ESP32 build instructions) is available here:

👉 **<https://chiefenne.github.io/RC_TOOLBOX/>**

## Highlights

- Servo Tester (up to 6 channels)
- Servo Signal Analyzer
- LiPo Checker
- NFC reader/writer
  - Tag for batteries (buying date, capacity, charge current, etc.)
  - Tag planes (CG location, weight, span, etc.)
  - UAS Operator ID (EU regulation)
- CG Scale
- Flap Deflection & Angle of Incidence tools
- Touch-driven UI (ILI9341)
- Rotary encoder + tactile buttons
- Multi-language UI
- macOS GUI simulator for rapid development

## Hardware

### PCB


<p align="center">
    <span>
        <img src="docs/assets/RC_TOOLBOX_PCB_ISO_front.png" width="400" style="vertical-align:middle; margin-right:16px;">
        <img src="docs/assets/RC_TOOLBOX_PCB_ISO_back.png" width="400" style="vertical-align:middle;">
    </span>
    <br>
    <em>PCB for ESP32-S3 DevKitC-1 and ILI9341 2.4&quot; TFT touch display</em>
</p>

The PCB design sources and manufacturing outputs are stored in the repository under the `hardware/` folder:

- Board layout: `hardware/RC TOOLBOX.fbrd`
- Schematic: `hardware/RC TOOLBOX.fsch`
- Schematic PDF: `hardware/RC TOOLBOX_Schematic_v194.pdf`
- Gerbers: `hardware/RC TOOLBOX_Gerber_v432.zip`

Direct link: <https://github.com/chiefenne/RC_TOOLBOX/tree/main/hardware>


## Credits

This project was inspired by following resources:

- <https://github.com/Ziege-One/Servotester_Deluxe>
- <https://github.com/TheDIYGuy999/Servotester_Deluxe>
- <https://github.com/shockyfan/Servotester_Deluxe>

## GUI Template for LVGL

- <https://github.com/chiefenne/LVGL_Simulator>
