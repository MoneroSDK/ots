#include "ots-internal.h"

using namespace ots::internal;

extern "C" {
    ots_result_t* ots_seed_languages(void) {
        ots_result_t* result = new ots_result_t();
        try {
            auto languages = ots::SeedLanguage::list();
            ots_handle_t* out = new ots_handle_t[languages.size()];
            for (size_t i = 0; i < languages.size(); ++i)
                out[i] = create_handle_reference(OTS_HANDLE_SEED_LANGUAGE, (void *)&(languages[i].get())); // references
            set_array(result, out, languages.size(), OTS_DATA_HANDLE, false); // handles itself are no references
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_languages_for_type(OTS_SEED_TYPE type) {
        ots_result_t* result = new ots_result_t();
        try {
            auto languages = ots::SeedLanguage::listFor(static_cast<ots::SeedType>(type));
            ots_handle_t* out = new ots_handle_t[languages.size()];
            for (size_t i = 0; i < languages.size(); ++i)
                out[i] = create_handle_reference(OTS_HANDLE_SEED_LANGUAGE, (void *)&(languages[i].get())); // references
            set_array(result, out, languages.size(), OTS_DATA_HANDLE, false); // handles itself are no references
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_for_code(const char* code) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromCode(code);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_for_name(const char* name) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromName(name);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_for_english_name(const char* name) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromEnglishName(name);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_default(OTS_SEED_TYPE type) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::defaultLanguage(static_cast<ots::SeedType>(type));
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_set_default(OTS_SEED_TYPE type, const ots_handle_t* language) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            ots::SeedLanguage::setDefaultLanguage(
                static_cast<ots::SeedType>(type),
                *static_cast<const ots::SeedLanguage*>(language->ptr)
            );
            set_handle_reference(
                result, OTS_HANDLE_SEED_LANGUAGE,
                (void *)&(ots::SeedLanguage::defaultLanguage(static_cast<ots::SeedType>(type)))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_from_code(const char* code) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromCode(code);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_from_name(const char* name) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromName(name);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_from_english_name(const char* name) {
        ots_result_t* result = new ots_result_t();
        try {
            auto language = ots::SeedLanguage::fromEnglishName(name);
            set_handle_reference(result, OTS_HANDLE_SEED_LANGUAGE, (void *)&language);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_code(const ots_handle_t* language) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(result, static_cast<const ots::SeedLanguage*>(language->ptr)->code());
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_name(const ots_handle_t* language) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(result, static_cast<const ots::SeedLanguage*>(language->ptr)->name());
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_english_name(const ots_handle_t* language) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_string(result, static_cast<const ots::SeedLanguage*>(language->ptr)->englishName());
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_supported(const ots_handle_t* language, OTS_SEED_TYPE type) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<const ots::SeedLanguage*>(language->ptr)->supported(static_cast<ots::SeedType>(type))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_is_default(const ots_handle_t* language, OTS_SEED_TYPE type) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<const ots::SeedLanguage*>(language->ptr)->isDefault(static_cast<ots::SeedType>(type))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_equals(const ots_handle_t* language1, const ots_handle_t* language2) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language1->type != OTS_HANDLE_SEED_LANGUAGE || language2->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                *static_cast<const ots::SeedLanguage*>(language1->ptr) == *static_cast<const ots::SeedLanguage*>(language2->ptr)
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_language_equals_code(const ots_handle_t* language, const char* code) {
        ots_result_t* result = new ots_result_t();
        try {
            if(language->type != OTS_HANDLE_SEED_LANGUAGE)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                *static_cast<const ots::SeedLanguage*>(language->ptr) == code
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }
}
