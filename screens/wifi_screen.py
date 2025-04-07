
from screens import ValueScreen
from language import language_manager as lm
from settings import settings # instantiates a global settings object automatically


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
