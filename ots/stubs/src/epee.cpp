
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>

namespace epee {
    namespace net_utils {
        namespace http {
            struct url_content {
                std::string host;
                std::string uri;
                std::string port;
                std::string protocol;
                std::string user;
                std::string password;
                std::string query_string;
            };
        }
        
        bool parse_url(const std::string& url_s, http::url_content& content) {
            return false;  // Always fail in offline mode
        }
    }
}
