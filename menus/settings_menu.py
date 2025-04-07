# settings_menu.py

from screens import MenuScreen, ValueScreen
from screens import LanguageScreen
from language import language_manager as lm


def build_settings_menu():
    menu = MenuScreen(lm.translate('SETTINGS'))

    language_screen = LanguageScreen('Language')  # Do not translate here
    option2 = ValueScreen('Option 2')

    menu.add_child(language_screen)
    menu.add_child(option2)

    return menu
