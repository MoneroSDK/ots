#include <vector>
#include <ots.hpp>

struct WalletVerifySignedMessageTestCase {
    const std::string name;
    const std::string message;
    const std::string signature;
    const std::string address;
    const bool valid = true;
    const bool throws = false;
    const size_t version = 2;
};

const std::vector<WalletVerifySignedMessageTestCase> wallet_verify_signed_message_test_cases = {
    {"valid signature", "Test message to sign", "SigV2EFwUsHxZyKmaTsdCAMRBRq6x8eyakkD2jHjdsYzonjHuH8mu13eo2XNdHFdZEU2EPfCVuGg7sRdug1V4zKZhoems", "47RFWVVwhcYA7qzVeUw8Cb2rttxS17GtABzyK5fDFQp3QV3EJLYfo6GF6Qmwe82hioY3ebrhNDq8UM7Bz13LrjFuSHrBXpy", true},
    {"invalid signature", "Test message to sign", "SigV28888sHxZyKmaTsdCAMRBRq6x8eyakkD2jHjdsYzonjHuH8mu13eo2XNdHFdZEU2EPfCVuGg7sRdug1V4zKZh8888", "47RFWVVwhcYA7qzVeUw8Cb2rttxS17GtABzyK5fDFQp3QV3EJLYfo6GF6Qmwe82hioY3ebrhNDq8UM7Bz13LrjFuSHrBXpy", false},
    {"invalid address", "Test message to sign", "SigV2EFwUsHxZyKmaTsdCAMRBRq6x8eyakkD2jHjdsYzonjHuH8mu13eo2XNdHFdZEU2EPfCVuGg7sRdug1V4zKZhoems", "99RFWVVwhcYA7qzVeUw8Cb2rttxS17GtABzyK5fDFQp3QV3EJLYfo6GF6Qmwe82hioY3ebrhNDq8UM7Bz13LrjFuSHrBXpy", false, true},
    {"empty message", "", "SigV2EFwUsHxZyKmaTsdCAMRBRq6x8eyakkD2jHjdsYzonjHuH8mu13eo2XNdHFdZEU2EPfCVuGg7sRdug1V4zKZhoems", "address", false, true},
    {"empty signature", "Test message to sign", "", "47RFWVVwhcYA7qzVeUw8Cb2rttxS17GtABzyK5fDFQp3QV3EJLYfo6GF6Qmwe82hioY3ebrhNDq8UM7Bz13LrjFuSHrBXpy", false, true},
    {"empty address", "Test message to sign", "SigV2EFwUsHxZyKmaTsdCAMRBRq6x8eyakkD2jHjdsYzonjHuH8mu13eo2XNdHFdZEU2EPfCVuGg7sRdug1V4zKZhoems", "", false, true},
    {"empty message and signature", "", "", "47RFWVVwhcYA7qzVeUw8Cb2rttxS17GtABzyK5fDFQp3QV3EJLYfo6GF6Qmwe82hioY3ebrhNDq8UM7Bz13LrjFuSHrBXpy", false, true},
    {"empty message and address", "", "SigV2EFwUsHxZyKmaTsdCAMRBRq6x8eyakkD2jHjdsYzonjHuH8mu13eo2XNdHFdZEU2EPfCVuGg7sRdug1V4zKZhoems", "", false, true},
    {"empty signature and address", "Test message to sign", "", "", false, true},
    {"empty message, signature, and address", "", "", "", false, true},
};
