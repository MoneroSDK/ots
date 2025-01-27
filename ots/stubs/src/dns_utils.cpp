#include "common/dns_utils.h"
#include <stdexcept>

namespace tools {
    namespace dns_utils {
        bool load_txt_records_from_dns(std::vector<std::string>& records,
                const std::vector<std::string>& dns_urls) {
            return false; // offline mode, ergo no DNS
        }

        std::string get_account_address_as_str_from_url(const std::string& url,
                bool& dnssec_valid,
                std::function<std::string(const std::string&, const std::vector<std::string>&, bool)> dnssec_checker) {
            dnssec_valid = false;
            throw std::runtime_error("DNS lookups not supported in offline mode");
        }
    }
}
