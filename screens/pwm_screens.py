
from screens import ValueScreen
from language import language_manager as lm
from settings import settings  # instantiates a global settings object automatically


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
                if sibling.name == lm.translate('Servo PWM'):
                    sibling.increment = self.value
        return result


# PWM Minimum Screen: Updates the min_value of 'Servo PWM'
class PWMMinScreen(ValueScreen):
    def handle_input(self, input_data):
        result = super().handle_input(input_data)

        if self.edit_mode:
            for sibling in self.parent.children:
                if sibling.name == lm.translate('Servo PWM'):
                    sibling.min_value = self.value
                    sibling.value = max(sibling.value, sibling.min_value)
        return result


# PWM Maximum Screen: Updates the max_value of 'Servo PWM'
class PWMMaxScreen(ValueScreen):
    def handle_input(self, input_data):
        result = super().handle_input(input_data)

        if self.edit_mode:
            for sibling in self.parent.children:
                if sibling.name == lm.translate('Servo PWM'):
                    sibling.max_value = self.value
                    sibling.value = min(sibling.value, sibling.max_value)
        return result
