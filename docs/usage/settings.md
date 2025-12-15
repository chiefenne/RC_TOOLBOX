# Settings

The Settings page lets you configure language, display appearance, servo parameters, and view system information. All changes are saved automatically when you leave the page.

![Settings Page](../assets/images/page_settings.png)

---

## Language

Select your preferred interface language:

- English
- Deutsch (German)
- Français (French)
- Español (Spanish)
- Italiano (Italian)
- Nederlands (Dutch)
- Čeština (Czech)

The page refreshes immediately when you change the language.

---

## Display

### Brightness

Adjust the display backlight brightness (10–100%).

### Background Color

Choose a background color preset:

- Light Gray
- White
- Light Blue
- Light Green
- Cream

---

## Servo

Configure default PWM parameters for all servos. The servo driver uses these values as boundaries.

### Protocol Presets

Quickly apply common servo timing standards:

| Protocol | PWM Min | Center | PWM Max | Frequency |
|----------|---------|--------|---------|-----------|
| Standard | 1000 µs | 1500 µs | 2000 µs | 50 Hz |
| Extended | 500 µs | 1500 µs | 2500 µs | 50 Hz |
| Sanwa | 760 µs | 1520 µs | 2280 µs | 50 Hz |
| Futaba | 920 µs | 1520 µs | 2120 µs | 50 Hz |
| Digital Fast | 500 µs | 1500 µs | 2500 µs | 333 Hz |
| Custom | (user-defined) | | | |

Selecting a preset automatically fills in the PWM values and frequency. If you manually change any slider, the protocol switches to **Custom**.

### Frequency

Choose the PWM update rate:

- **50 Hz** – Standard analog servos
- **333 Hz** – High-speed digital servos

!!! warning
    Using 333 Hz with analog servos may cause erratic behavior or damage.

### PWM Values

Fine-tune the pulse widths with sliders:

| Setting | Range | Description |
|---------|-------|-------------|
| PWM Min | 500–1500 µs | Minimum pulse width (full left) |
| PWM Center | 1000–2000 µs | Neutral position |
| PWM Max | 1500–2500 µs | Maximum pulse width (full right) |

### Per-Servo Step Size

Each of the 6 servo channels has its own **Step** slider (1–100 µs). This controls how much the PWM changes per encoder click in Manual mode on the Servo Tester page.

- **Servo Step 1** through **Servo Step 6**

Use smaller steps for precise positioning or larger steps for quick sweeps.

### Reset Button

The **Reset Servo Steps** button restores all per-servo step values to the default (10 µs).

---

## System

Displays read-only information:

| Field | Description |
|-------|-------------|
| Firmware | Application version (e.g., `1.0.0`) |
| LVGL | GUI library version (e.g., `9.4.0`) |

---

## Navigation

Use the encoder to scroll through settings and adjust values:

| Action | Effect |
|--------|--------|
| Rotate | Scroll / adjust slider value |
| Press | Open dropdown / confirm selection |

Tap **Home** in the footer bar to return to the main menu.
