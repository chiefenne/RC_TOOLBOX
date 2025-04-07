# menus/menus.py

from .servo_menu import build_servo_menu
from .wifi_menu import build_wifi_menu
from .settings_menu import build_settings_menu
from .help_menu import build_help_menu

def build_menus():
    return [
        build_servo_menu(),
        build_wifi_menu(),
        build_settings_menu(),
        build_help_menu()
    ]
