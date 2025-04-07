# language_screen.py

from screens import ValueScreen
from language import language_manager as lm
from settings import settings # instantiates a global settings object automatically


class LanguageScreen(ValueScreen):
    def __init__(self, name='Language'):
        self.lang_index = (lm.languages.index(lm.get_language())
                           if lm.get_language() in lm.languages else 0)
        super().__init__(name=name, initial_value=lm.languages[self.lang_index])
        self.edit_mode = False

    def get_increment(self, input_type):
        if input_type.startswith('right'):
            return 1
        elif input_type.startswith('left'):
            return -1
        return 0

    def handle_input(self, input_data):
        if self.edit_mode:
            delta = self.get_increment(input_data)
            if delta != 0:
                self.lang_index = (self.lang_index + delta) % len(lm.languages)
                self.value = lm.languages[self.lang_index]
                self.show()
                return True
            elif input_data in ('double_click', 'long_press'):
                lm.set_language(self.value)
                settings.set("language", self.value) # save for next boot
                self.edit_mode = False
                self.show()
                return True
        else:
            if input_data == 'single_click':
                self.edit_mode = True
                self.show()
                return True
            elif input_data in ('double_click', 'long_press'):
                return self.parent
            elif input_data in ('left', 'right'):
            # Let the parent ValueScreen handle navigation to siblings
                return super().handle_input(input_data)

        return False
