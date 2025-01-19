#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include "ots.hpp"

struct SeedLanguageTestCase {
    std::string name = "";
    std::string englishName = "";
    std::string code = "";
    struct index {
        int monero = -1;
        int polyseed = -1;
    } index;
    struct supported {
        bool monero = true;
        bool polyseed = true;
    } supported;
    bool valid = true;
};

/*
Deutsch, German, de, {-1, -1}, {true, false}
English, English, en, {-1, 0}, {true, true}
Español, Spanish, es, {-1, 3}, {true, true}
Français, French, fr, {-1, 4}, {true, true}
Italiano, Italian, it, {-1, 5}, {true, true}
Nederlands, Dutch, nl, {-1, -1}, {true, false}
Português, Portuguese, pt, {-1, 7}, {true, true}
русский язык, Russian, ru, {-1, -1}, {true, false}
日本語, Japanese, jp, {-1, 1}, {true, true}
简体中文 (中国), Chinese (simplified), zh-Hans, {-1, 8}, {true, true}
Esperanto, Esperanto, eo, {-1, -1}, {true, false}
Lojban, Lojban, lojban, {-1, -1}, {true, false}
한국어, Korean, ko, {-1, 2}, {false, true}
čeština, Czech, cs, {-1, 6}, {false, true}
中文(繁體), Chinese (Traditional), zh-Hant, {-1, 9}, {false, true}
*/
inline const std::vector<SeedLanguageTestCase> seed_language_test_cases = {
    {"XXX", "XXX", "xx", {-1, -1}, {false, false}, false},
    {"Deutsch", "German", "de", {-1, -1}, {true, false}},
    {"English", "English", "en", {-1, 0}},
    {"Español", "Spanish", "es", {-1, 3}},
    {"Français", "French", "fr", {-1, 4}},
    {"Italiano", "Italian", "it", {-1, 5}},
    {"Nederlands", "Dutch", "nl", {-1, -1}, {true, false}},
    {"Português", "Portuguese", "pt", {-1, 7}},
    {"русский язык", "Russian", "ru", {-1, -1}, {true, false}},
    {"日本語", "Japanese", "jp", {-1, 1}},
    {"简体中文 (中国)", "Chinese (simplified)", "zh-Hans", {-1, 8}},
    {"Esperanto", "Esperanto", "eo", {-1, -1}, {true, false}},
    {"Lojban", "Lojban", "lojban", {-1, -1}, {true, false}},
    {"한국어", "Korean", "ko", {-1, 2}, {false, true}},
    {"čeština", "Czech", "cs", {-1, 6}, {false, true}},
    {"中文(繁體)", "Chinese (Traditional)", "zh-Hant", {-1, 9}, {false, true}}
};

inline int total_languages_count() {
    return std::count_if(
        seed_language_test_cases.begin(),
        seed_language_test_cases.end(), 
        [](const SeedLanguageTestCase& tc) {
            return tc.supported.monero | tc.supported.polyseed;
        });
}

inline int monero_languages_count() {
    return std::count_if(
        seed_language_test_cases.begin(),
        seed_language_test_cases.end(), 
        [](const SeedLanguageTestCase& tc) {
        return tc.supported.monero;
    });
}

inline int polyseed_languages_count() {
    return std::count_if(
        seed_language_test_cases.begin(),
        seed_language_test_cases.end(), 
        [](const SeedLanguageTestCase& tc) {
            return tc.supported.polyseed;
    });
}
