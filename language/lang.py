# lang.py

from .languages_data import LANGUAGES


class LanguageManager:
    def __init__(self, default_language='en'):
        self.current_language = default_language
        self.languages = list(LANGUAGES.keys())

    def set_language(self, lang_code):
        self.current_language = lang_code

    def get_language(self):
        return self.current_language

    def get_available_languages(self):
        return self.languages

    def translate(self, text):
        return LANGUAGES.get(self.current_language, {}).get(text, text)

# Create a shared instance of LanguageManager
language_manager = LanguageManager()
