# wifi_menu.py

from screens import MenuScreen, WiFiSSIDScreen, WiFiPasswordScreen
from language import language_manager as lm


def build_wifi_menu():
    menu = MenuScreen(lm.translate('WIFI'))

    ssid_screen = WiFiSSIDScreen(
        lm.translate('WiFi SSID'),
        initial_value='MySSID'
    )

    password_screen = WiFiPasswordScreen(
        lm.translate('WiFi Password'),
        initial_value='MyPassword'
    )

    menu.add_child(ssid_screen)
    menu.add_child(password_screen)

    return menu
