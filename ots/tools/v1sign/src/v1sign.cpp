#include <string>  // for std::stoul
#include <stdexcept>  // for std::invalid_argument, std::out_of_range
#include "string_tools.h" // for epee::string_tools::pod_to_hex
#include "common/base58.h" // For tools::base58::decode
#include "cryptonote_basic/cryptonote_format_utils.h" // for cryptonote::decrypt_key
#include "cryptonote_basic/cryptonote_basic.h" // For cryptonote::address_parse_info
#include "cryptonote_basic/account.h" // for cryptonote::account_base
                                      //
// needed to deactivate easy logging
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP
                                               //
/**
 * @file v1sign.cpp
 * @brief Sign a message with a Monero private key V1 (legacy)
 * @note I build this only to test how I could not find another way to generate a V1 signature
 */


/**
 * @brief Generate a Monero account from a secret spend key
 * @param key the private key hex encoded
 * @return the account object
 */
cryptonote::account_base account(const std::string& key) {
    cryptonote::account_base account;
    crypto::secret_key sk;
    epee::string_tools::hex_to_pod(key, sk);
    crypto::secret_key secretSpendKey = account.generate(sk, true, false);
    return account;
}

/**
 * @brief Generate a Monero address from an account
 * @param account the account object
 * @param nettype the network type
 * @return the address string
 */
std::string address(const cryptonote::account_base& account, const cryptonote::network_type& nettype) {
    return account.get_public_address_str(nettype);
}

/**
 * @brief Generate a Monero address from an account and a subaddress index
 * @param account the account object
 * @param nettype the network type
 * @param index the subaddress index
 * @return the address string
 */
std::string address(
        const cryptonote::account_base& account,
        const cryptonote::network_type& nettype,
        const cryptonote::subaddress_index& index
    ) {
    cryptonote::account_public_address subaddr = account.get_device().get_subaddress(account.get_keys(), index);
    return cryptonote::get_account_address_as_str(nettype, true, subaddr);
}

/**
 * @brief Sign a message with a Monero private spend key
 * @param data the message to sign
 * @param account the account object
 * @param index the subaddress index
 * @return the V1 signature string
 */
std::string sign(
        const std::string &data,
        const cryptonote::account_base& account,
        const cryptonote::subaddress_index& index) {
    const cryptonote::account_keys& keys = account.get_keys();
    crypto::secret_key skey;
    crypto::public_key pkey;
    if(index.is_zero()) { // Use the base address
        skey = keys.m_spend_secret_key;
        pkey = keys.m_account_address.m_spend_public_key;
    } else { // Use a subaddress
        crypto::secret_key skey_spend = keys.m_spend_secret_key;
        crypto::secret_key m = account.get_device().get_subaddress_secret_key(
                keys.m_view_secret_key, index
        );
        sc_add((unsigned char*)&skey_spend, (unsigned char*)&m, (unsigned char*)&skey_spend);
        skey = skey_spend;
        secret_key_to_public_key(skey, pkey);
    }
    crypto::hash hash;
    crypto::cn_fast_hash(data.data(), data.size(), hash);
    crypto::signature signature;
    crypto::generate_signature(hash, pkey, skey, signature);
    return std::string("SigV1") + tools::base58::encode(std::string((const char *)&signature, sizeof(signature)));
}

/**
 * @brief Main function
 * @param argc the number of arguments
 * @param argv the arguments
 * @return 0 on success, 1 on failure
 * @note Usage: ./v1sign <private key> <data> [subaddress index]
 * @note To compile you need to a `-DBUILD_V1SIGN=on` for cmake
 * @note Only mainnet is supported how it is only for the unit tests
 */
int main(int argc, char *argv[]) {
    if(argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <private key> <data> [subaddress index]" << std::endl;
        return 1;
    }
    cryptonote::network_type nettype = cryptonote::MAINNET;
    cryptonote::account_base acc = account(std::string(argv[1]));
    cryptonote::subaddress_index index{0};
    std::string data = std::string(argv[2]);
    if(argc > 3) {
        std::string str = std::string(argv[3]);
        size_t pos = str.find(':');
        if(pos == std::string::npos) {
            std::cerr << "Invalid subaddress index" << std::endl;
            return 1;
        }
        index = cryptonote::subaddress_index{
            (unsigned int)std::stoul(str.substr(0, pos)),
            (unsigned int)std::stoul(str.substr(pos + 1))
        };
    }
    std::string addr = index.is_zero() ? address(acc, nettype) : address(acc, nettype, index);
    std::string sig = sign(data, acc, index);
    std::cout << "Address    : " << addr << std::endl;
    std::cout << "Signature  : " << sig << std::endl;
}
