#include "ots.hpp"
#include "polyseed.h"
#include "../polyseed/src/lang.h"
#include "mnemonics/electrum-words.h"
#include "mnemonics/language_base.h"

namespace ots {

	const std::string& SeedLanguage::name() const {
		return m_name;
	}

	const std::string& SeedLanguage::englishName() const {
		return m_englishName;
	}

	const std::string& SeedLanguage::code() const {
		return m_code;
	}

	bool SeedLanguage::supported(SeedType type) const {
        auto it = m_supported.find(type);
        if(it != m_supported.end())
            return it->second;
		return false;
	}

	bool SeedLanguage::isDefault(SeedType type) const {
		auto it = s_default.find(type);
		return it != s_default.end() && it->second.get().m_code == this->m_code;
	}

    int SeedLanguage::index(SeedType type) const noexcept {
        auto it = m_index.find(type);
        if(it != m_index.end())
            return it->second;
        return -1;
    }

	bool SeedLanguage::operator==(const SeedLanguage& other) const {
		return m_code == other.m_code;
	}

	bool SeedLanguage::operator==(SeedLanguage& other) const {
		return m_code == other.m_code;
	}

	bool SeedLanguage::operator==(const std::string& code) const {
		return m_code == code;
	}

    SeedLanguage::operator const std::string&() const {
        return m_englishName;
    }

	const SeedLanguage& SeedLanguage::fromName(const std::string& name) {
		for(const SeedLanguage& item : list())
			if(item.name() == name)
				return item;
		throw ots::exception::seed::LanguageNotFound();
	}

	const SeedLanguage& SeedLanguage::fromEnglishName(const std::string& name) {
		for(const SeedLanguage& item : list())
			if(item.englishName() == name)
				return item;
		throw ots::exception::seed::LanguageNotFound();
	}

	const SeedLanguage& SeedLanguage::fromCode(const std::string& code) {
		for(const SeedLanguage& item : list())
			if(item == code)
				return item;
		throw ots::exception::seed::LanguageNotFound();
	}

	const std::vector<std::reference_wrapper<const SeedLanguage>> SeedLanguage::list() {
		if(s_list.size() == 0) {
            /**
             * @brief Map of language names to language codes
             * serves also as a list of supported languages, and harmonize spelling differences
             *
             * @warning if monero or polyseed adds a language, it must be added to the map
             * @note if there are more conflicting languages, there should be maybe another way to unify the name.
             * @internal
             * */
            std::map<std::string, std::string> lang_map = {
                {"English", "en"}, // Monero and Polyseed
                {"Spanish", "es"}, // Monero and Polyseed
                {"French", "fr"}, // Monero and Polyseed
                {"Italian", "it"}, // Monero and Polyseed
                {"Portuguese", "pt"}, // Monero and Polyseed
                {"German", "de"}, // Monero
                {"Dutch", "nl"}, // Monero
                {"Russian", "ru"}, // Monero
                {"Japanese", "jp"}, // Monero and Polyseed
                {"Chinese (Simplified)", "zh-Hans"}, // Monero (how this is generated first, Simplified will be with the first letter capitalized)
                {"Chinese (simplified)", "zh-Hans"}, // Polyseed (and this will bill be added to the Chine above)
                {"Chinese (Traditional)", "zh-Hant"}, // Polyseed
                {"Esperanto", "eo"}, // Monero
                {"Korean", "ko"}, // Polyseed
                {"Czech", "cs"}, // Polyseed
                {"Lojban", "lojban"} // Monero
            };
            // iterate over all languages for monero seed phrases and create SeedLanguage objects
            std::vector<std::string> lang_list;
            crypto::ElectrumWords::get_language_list(lang_list);
            for(auto lang : lang_list) {
                std::string lang_en = crypto::ElectrumWords::get_english_name_for(lang);
                if(lang_map.find(lang_en) == lang_map.end())
                    continue; // skip languages not in the map
                std::string lang_code = lang_map[lang_en];
                SeedLanguage sl = SeedLanguage();
                sl.m_name = lang;
                sl.m_englishName = lang_en;
                sl.m_code = lang_code;
                sl.m_supported[SeedType::Monero] = true;
                s_list.push_back(sl);
            }
            // iterate over all languages for polyseed seed phrases and create SeedLanguage objects where needed, or update existing ones.
            int ps_langs = polyseed_get_num_langs();
            for(int i = 0; i < ps_langs; i++) {
                const polyseed_lang* ps_lang = polyseed_get_lang(i);
                std::string lang_name_en(ps_lang->name_en);
                if(lang_map.find(lang_name_en) == lang_map.end())
                    continue; // skip languages not in the map
                std::string lang_code = lang_map[lang_name_en];
                bool found = false;
                for(SeedLanguage& item: s_list) {
                    if(item.code() == lang_code) {
                        item.m_index[SeedType::Polyseed] = i;
                        item.m_supported[SeedType::Polyseed] = true;
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    SeedLanguage sl;
                    sl = SeedLanguage();
                    sl.m_name = ps_lang->name;
                    sl.m_englishName = lang_name_en;
                    sl.m_code = lang_code;
                    sl.m_index[SeedType::Polyseed] = i;
                    sl.m_supported[SeedType::Polyseed] = true;
                    s_list.push_back(sl);
                }
            }
		}
        // we return only references
		std::vector<std::reference_wrapper<const SeedLanguage>> out{};
		for(const SeedLanguage& item : s_list)
            out.push_back(std::cref(item));
		return std::move(out);
	}

	const std::vector<std::reference_wrapper<const SeedLanguage>> SeedLanguage::listFor(SeedType type) {
		std::vector<std::reference_wrapper<const SeedLanguage>> out{};
		for(const SeedLanguage& item : list()) {
			if(item.supported(type))
				out.push_back(std::cref(item));
		}
		return std::move(out);
	}

	const SeedLanguage& SeedLanguage::defaultLanguage(SeedType type) {
		auto it = s_default.find(type);
		if(it != s_default.end())
			return it->second;
		throw ots::exception::seed::NoDefaultLanguageSet();
	}

    void SeedLanguage::setDefaultLanguage(SeedType type, const SeedLanguage& language) {
        if(!language.supported(type))
            throw ots::exception::seed::LanguageNotSupportedBySeedType();
        try {
        if(defaultLanguage(type) == language) // already set, nothing to do
            return;
        } catch(const ots::exception::seed::NoDefaultLanguageSet& unused) {} // no default set, continue
        for(SeedLanguage& lang: s_list) {
            if(lang == language) {
                s_default.insert_or_assign(type, std::cref(lang));
                return;
            }
        }
    }

    std::vector<SeedLanguage> SeedLanguage::s_list;
    std::map<SeedType, std::reference_wrapper<const SeedLanguage>> SeedLanguage::s_default;
}
