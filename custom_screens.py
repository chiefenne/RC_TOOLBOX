
from screens import ValueScreen
from lang import AVAILABLE_LANGUAGES, set_language, current_language, translate


# PWM Increment Screen: Updates the increment value of the 'Servo PWM'
class PWMIncrementScreen(ValueScreen):
    def get_increment(self, input_type):
        # Exponential speed: faster input = bigger steps
        base = 1 if self.value < 100 else 5
        if input_type == 'right':
            return base
        elif input_type == 'right_fast':
            return base * 5
        elif input_type == 'right_super_fast':
            return base * 10
        elif input_type == 'left':
            return -base
        elif input_type == 'left_fast':
            return -base * 5
        elif input_type == 'left_super_fast':
            return -base * 10
        return 0

    def handle_input(self, input_data):
        result = super().handle_input(input_data)

        if self.edit_mode:
            # Update Servo PWM increment if in same menu
            for sibling in self.parent.children:
                if sibling.name == translate('Servo PWM'):
                    sibling.increment = self.value
        return result


# PWM Minimum Screen: Updates the min_value of 'Servo PWM'
class PWMMinScreen(ValueScreen):
    def handle_input(self, input_data):
        result = super().handle_input(input_data)

        if self.edit_mode:
            for sibling in self.parent.children:
                if sibling.name == translate('Servo PWM'):
                    sibling.min_value = self.value
                    sibling.value = max(sibling.value, sibling.min_value)
        return result


# PWM Maximum Screen: Updates the max_value of 'Servo PWM'
class PWMMaxScreen(ValueScreen):
    def handle_input(self, input_data):
        result = super().handle_input(input_data)

        if self.edit_mode:
            for sibling in self.parent.children:
                if sibling.name == translate('Servo PWM'):
                    sibling.max_value = self.value
                    sibling.value = min(sibling.value, sibling.max_value)
        return result


# WiFi SSID Screen (Placeholder for future validation)
class WiFiSSIDScreen(ValueScreen):
    def handle_input(self, input_data):
        # In the future, you can add validation or keyboard screen entry here
        return super().handle_input(input_data)


# WiFi Password Screen (Placeholder for masking input, etc.)
class WiFiPasswordScreen(ValueScreen):
    def handle_input(self, input_data):
        # Later, you might mask or hash the password here
        return super().handle_input(input_data)


class LanguageScreen(ValueScreen):
    def __init__(self, name='Language'):
        self.languages = AVAILABLE_LANGUAGES
        self.lang_index = self.languages.index(current_language) if current_language in self.languages else 0
        super().__init__(name=name, initial_value=self.languages[self.lang_index])
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
                self.lang_index = (self.lang_index + delta) % len(self.languages)
                self.value = self.languages[self.lang_index]
                self.show()
                return True
            elif input_data in ('double_click', 'long_press'):
                set_language(self.languages[self.lang_index])
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
