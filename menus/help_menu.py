# help_menu.py

from screens import MenuScreen, ValueScreen
from language import language_manager as lm


def build_help_menu():
    menu = MenuScreen(lm.translate('HELP'))

    help1 = ValueScreen(lm.translate('Help Option 1'))
    help2 = ValueScreen(lm.translate('Help Option 2'))

    menu.add_child(help1)
    menu.add_child(help2)

    return menu
