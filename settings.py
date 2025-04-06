import ujson
import os

SETTINGS_FILE = "settings.json"

DEFAULTS = {
    "language": "en",
    "servo_pwm": 1500,
    "pwm_increment": 10,
    "wifi_ssid": "MySSID",
    "wifi_password": "MyPassword"
}

class Settings:
    def __init__(self, defaults=None):
        self.defaults = defaults or DEFAULTS
        self.data = {}
        self.load()

    def load(self):
        try:
            with open(SETTINGS_FILE, "r") as f:
                self.data = ujson.load(f)
        except (OSError, ValueError):
            self.data = self.defaults.copy()
            self.save()

    def save(self):
        try:
            with open(SETTINGS_FILE, "w") as f:
                ujson.dump(self.data, f)
        except Exception as e:
            print("⚠️ Failed to save settings:", e)

    def get(self, key):
        return self.data.get(key, self.defaults.get(key))

    def set(self, key, value):
        self.data[key] = value
        self.save()

    def reset(self):
        self.data = self.defaults.copy()
        self.save()

# Create a global instance
settings = Settings()
