#include "gui/lang.h"

// Language string files
#include "gui/lang/strings_en.h"
#include "gui/lang/strings_de.h"
#include "gui/lang/strings_fr.h"
#include "gui/lang/strings_es.h"
#include "gui/lang/strings_it.h"
#include "gui/lang/strings_nl.h"
#include "gui/lang/strings_cs.h"

static Language current_lang = LANG_EN;

static const char** all_strings[LANG_COUNT] = {
    strings_en,
    strings_de,
    strings_fr,
    strings_es,
    strings_it,
    strings_nl,
    strings_cs
};

Language lang_get() {
    return current_lang;
}

void lang_set(Language lang) {
    if (lang < LANG_COUNT) {
        current_lang = lang;
    }
}

const char* tr(StringId id) {
    if (id < STR_COUNT) {
        return all_strings[current_lang][id];
    }
    return "???";
}
