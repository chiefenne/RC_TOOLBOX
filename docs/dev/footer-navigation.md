# Footer Navigation System

## Hub-and-Spoke Navigation Model

The RC TOOLBOX uses a **hub-and-spoke** navigation pattern:

- **Home pages** (hub): Navigate between home pages using footer Prev/Next buttons
- **Tool pages** (spokes): Accessed only via home page buttons, no sequential navigation
- **Home button**: Always returns to first home page (PAGE_HOME)

---

## Navigation Behavior

### On Home Pages (PAGE_HOME, PAGE_HOME_2)
- **Prev button**: Navigate to previous home page (wraps around)
- **Next button**: Navigate to next home page (wraps around)
- **Home button**: Goes to PAGE_HOME (same as clicking any home button)
- **Settings button**: Opens settings page

> **Note:** The Home hub can consist of more than two pages as the number of tools grows.

### On Tool Pages (Servo, Lipo, CG Scale, etc.)
- **Prev/Next buttons**: By default, do nothing (can be customized per page)
- **Home button**: Always returns to PAGE_HOME
- **Settings button**: Opens settings page

### On Settings/About Pages
- **Prev/Next buttons**: Do nothing by default
- **Home button**: Returns to PAGE_HOME
- **Settings button**: Disabled (already on settings)

---

## Adding Custom Prev/Next Behavior to Tool Pages

Tool pages can implement custom footer button behavior by providing callback functions in the page registry.

### Step 1: Define Handler Functions

In your page implementation (e.g., `page_servo.cpp`):

```cpp
// Custom handler for Prev button - decrease frequency
static void servo_on_prev() {
    // Decrease servo frequency or change protocol
    // Update UI accordingly
}

// Custom handler for Next button - increase frequency
static void servo_on_next() {
    // Increase servo frequency or change protocol
    // Update UI accordingly
}
```

### Step 2: Export Functions in Header

In your page header (e.g., `page_servo.h`):

```cpp
void servo_on_prev();  // Custom prev handler
void servo_on_next();  // Custom next handler
```

### Step 3: Register in Page Registry

In `gui/gui.cpp`, update the page entry:

```cpp
// PAGE_SERVO - custom prev/next for adjusting parameters
{ STR_PAGE_SERVO, page_servo_create, page_servo_destroy,
  page_servo_is_running, page_servo_stop, get_servo_protocol_name,
  servo_on_prev,  // Custom prev handler
  servo_on_next   // Custom next handler
},
```

---

## Example Use Cases for Page-Specific Footer Buttons

| Page | Prev Button | Next Button | Notes |
|------|-------------|-------------|-------|
| **Servo** | Decrease frequency/protocol | Increase frequency/protocol | Adjust primary parameter |
| **Lipo** | Decrease cell count | Increase cell count | Quick cell adjustment |
| **CG Scale** | Switch unit (g→oz) | Switch unit (oz→g) | Toggle measurement units |
| **Deflection** | Previous surface | Next surface | Multi-surface navigation |
| **Angle** | Decrease sensitivity | Increase sensitivity | Adjust measurement params |
| **Serial** | Scroll up | Scroll down | Navigate log output |
| **Settings** | Previous setting | Next setting | Navigate setting items |
| **About** | nullptr | nullptr | No functionality needed |

---

## Visual Feedback Considerations

To improve UX, consider:

1. **Dynamic button labels**: Change footer button text per page
   - Home pages: "◀" and "▶" (navigation arrows)
   - Servo page: "⊖ FREQ" and "FREQ ⊕" (parameter adjustment)

2. **Disable unused buttons**: Visually gray out Prev/Next if not functional

3. **Button color coding**: Use different colors for navigation vs. adjustment
   - Navigation (home pages): Blue/gray
   - Adjustment (tool pages): Orange/green

These improvements can be added later without changing the navigation architecture.

---

## Architecture Benefits

✅ **Scalable**: Easy to add more home pages without affecting tool navigation
✅ **Flexible**: Each tool page can define custom footer behavior or leave blank
✅ **Consistent**: Home button always returns to main hub
✅ **Encoder-friendly**: Fast navigation between home pages (1-2 clicks max)
✅ **Discoverable**: All tools visible from home pages, no hidden sequential navigation
