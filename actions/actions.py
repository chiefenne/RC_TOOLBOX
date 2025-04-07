
from machine import Pin, PWM


class Action:
    """Base Action class defining interface for hardware actions."""
    def __init__(self, immediate=True):
        self.immediate = immediate  # trigger on every change or on confirm only

    def on_value_changed(self, value):
        """Handle value change (for immediate actions). Override in subclass if needed."""
        pass  # default: do nothing for value changes

    def on_value_confirmed(self, value):
        """Handle value confirmation (on-demand actions or final commit)."""
        pass

class ServoPWMAction(Action):
    """
    Action to control a servo using PWM on a given pin.
    Expects 'value' to be a pulse width in microseconds (e.g. 1000-2000us for 0-180°).
    """
    def __init__(self, pin_num, freq=50, min_us=1000, max_us=2000, immediate=True):
        super().__init__(immediate)
        self.pin = Pin(pin_num, Pin.OUT)
        self.pwm = PWM(self.pin)
        self.pwm.freq(freq)
        self.min_us = min_us
        self.max_us = max_us

    def _set_pulse_width(self, pulse_us):
        pulse_us = max(self.min_us, min(pulse_us, self.max_us))
        pulse_ns = pulse_us * 1000
        self.pwm.duty_ns(pulse_ns)  # output PWM pulse

    def on_value_changed(self, value):
        if self.immediate:
            # For immediate mode, move servo on every change
            self._set_pulse_width(value)

    def on_value_confirmed(self, value):
        if not self.immediate:
            # For on-demand mode, move servo only when confirmed
            self._set_pulse_width(value)
        # Still, we could also call _set_pulse_width here to ensure final position
        # (or perform any finalization like stopping movement if needed).
