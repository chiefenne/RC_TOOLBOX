# lang.py

LANGUAGES = {
    'English': {
        'SERVO': 'SERVO',
        'WIFI': 'WiFi',
        'SETTINGS': 'SETTINGS',
        'HELP': 'HELP',
        'Servo PWM': 'Servo PWM',
        'PWM Increment': 'PWM Step',
        'PWM Minimum': 'Min PWM',
        'PWM Maximum': 'Max PWM',
        'WiFi SSID': 'SSID',
        'WiFi Password': 'Password',
        'Language': 'Language',
    },
    'Deutsch': {
        'SERVO': 'SERVO',
        'WIFI': 'WLAN',
        'SETTINGS': 'EINSTELLUNGEN',
        'HELP': 'HILFE',
        'Servo PWM': 'Servo PWM',
        'PWM Increment': 'PWM Schritt',
        'PWM Minimum': 'PWM Minimum',
        'PWM Maximum': 'PWM Maximum',
        'WiFi SSID': 'SSID',
        'WiFi Password': 'Passwort',
        'Language': 'Sprache',
    },
    'Francais': {
        'SERVO': 'SERVO',
        'WIFI': 'WiFi',
        'SETTINGS': 'PARAMETRES',
        'HELP': 'AIDE',
        'Servo PWM': 'Servo PWM',
        'PWM Increment': 'Pas PWM',
        'PWM Minimum': 'PWM Min',
        'PWM Maximum': 'PWM Max',
        'WiFi SSID': 'SSID',
        'WiFi Password': 'Mot de passe',
        'Language': 'Langue',
    },
    'Espanol': {
        'SERVO': 'SERVO',
        'WIFI': 'WiFi',
        'SETTINGS': 'AJUSTES',
        'HELP': 'AYUDA',
        'Servo PWM': 'Servo PWM',
        'PWM Increment': 'Paso PWM',
        'PWM Minimum': 'PWM Minimo',
        'PWM Maximum': 'PWM Maximo',
        'WiFi SSID': 'SSID',
        'WiFi Password': 'Contrasena',
        'Language': 'Idioma',
    },
}

AVAILABLE_LANGUAGES = list(LANGUAGES.keys())

current_language = 'EN'

def set_language(lang_code):
    global current_language
    current_language = lang_code

def translate(text):
    return LANGUAGES.get(current_language, {}).get(text, text)
