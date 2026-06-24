#include <vector>
#include "ots.hpp"

struct PolyseedTestCase {
    const std::string name;
    const std::string phrase;
    const uint64_t height = 0;
    const uint64_t time = 0;
    const uint64_t expected_height = 0;
    const uint64_t expected_time = 0;
    const ots::Network network = ots::Network::MAIN;
    const std::string password = "";
    const bool encrypted = false;
    const std::string passphrase = "";
    const bool offset = false;
    const std::string address = "";
    const std::string fingerprint = "";
    const bool valid = true;
    const std::string secret_spend_key = "";
    const std::string public_spend_key = "";
    const std::string secret_view_key = "";
    const std::string public_view_key = "";
    const std::string lang_code = "en";
};

const std::vector<PolyseedTestCase> polyseed_test_cases = {
    {"valid mainnet seed",
     "october embark potato lecture make pair reunion deliver energy soccer trim film monitor ethics gesture grab",
     0, 0, 0, 0,
     ots::Network::MAIN,
     "", false,
     "", false,
     "47RFWVVwhcYA7qzVeUw8Cb2rttxS17GtABzyK5fDFQp3QV3EJLYfo6GF6Qmwe82hioY3ebrhNDq8UM7Bz13LrjFuSHrBXpy", "0896CB",
     true,
     "98df06f3eb6eeb01768afbd682859a3a8e5822fe7898e11a0c4ad31ee17b9103", "98ef6893842d9d368355efb872195c0b1f29fbb1b82a8341c2fef16c0459948c",
     "5950d2d3e7db2335df6c99834802c00182b0b55a5faa0493132722e407ae4300", "65f09c9a3cf829544218371797bd50b99c20c6209fd1e17835ce9064c33ce0e0"
    }
};
