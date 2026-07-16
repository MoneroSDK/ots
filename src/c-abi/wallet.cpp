#include "ots-internal.h"
#include <cstring>
#include <iostream>

using namespace ots::internal;

extern "C" {
    ots_result_t* ots_wallet_create(
            const uint8_t key[32],
            uint64_t height,
            OTS_NETWORK network
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            std::array<uint8_t, 32> key_array;
            memcpy(key_array.data(), key, 32);
            set_handle(
                result,
                OTS_HANDLE_WALLET,
                new ots::Wallet(key_array, height, to_cpp_network(network))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_height(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_number(
                result,
                static_cast<int64_t>(static_cast<ots::Wallet*>(wallet->ptr)->height())
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_address(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_ADDRESS,
                new ots::Address(static_cast<ots::Wallet*>(wallet->ptr)->address())
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_subaddress(
            const ots_handle_t* wallet,
            uint32_t account,
            uint32_t index
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_ADDRESS,
                new ots::Address(static_cast<ots::Wallet*>(wallet->ptr)->address(account, index))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_accounts(
            const ots_handle_t* wallet,
            uint32_t max,
            uint32_t offset
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::vector<ots::Address> addresses = static_cast<ots::Wallet*>(wallet->ptr)->accounts(max, offset);
            ots_handle_t* handles = new ots_handle_t[addresses.size()];
            for(size_t i = 0; i < addresses.size(); i++) {
                ots::Address* address = new ots::Address(addresses[i]);
                handles[i] = create_handle(OTS_HANDLE_ADDRESS, address);
            }
            set_array(
                result,
                handles,
                addresses.size(),
                OTS_DATA_HANDLE,
                true
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_subaddresses(
            const ots_handle_t* wallet,
            uint32_t account,
            uint32_t max,
            uint32_t offset
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::vector<ots::Address> addresses = static_cast<ots::Wallet*>(wallet->ptr)->subAddresses(account, max, offset);
            ots_handle_t* handles = new ots_handle_t[addresses.size()];
            for(size_t i = 0; i < addresses.size(); i++) {
                ots::Address* address = new ots::Address(addresses[i]);
                handles[i] = create_handle(OTS_HANDLE_ADDRESS, address);
            }
            set_array(
                result,
                handles,
                addresses.size(),
                OTS_DATA_HANDLE,
                true
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_has_address(
            const ots_handle_t* wallet,
            const ots_handle_t* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET || address->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->hasAddress(
                    *static_cast<ots::Address*>(address->ptr),
                    max_account_depth,
                    max_index_depth
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_has_address_string(
        const ots_handle_t* wallet,
        const char* address,
        uint32_t max_account_depth,
        uint32_t max_index_depth
    ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->hasAddress(
                    address,
                    max_account_depth,
                    max_index_depth
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_address_index(
            const ots_handle_t* wallet,
            const ots_handle_t* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET || address->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::pair<uint32_t, uint32_t> idx = static_cast<ots::Wallet*>(wallet->ptr)->addressIndex(
                        *static_cast<ots::Address*>(address->ptr),
                        max_account_depth,
                        max_index_depth
                    );

            uint32_t* arr = new uint32_t[2];
            arr[0] = idx.first;
            arr[1] = idx.second;
            set_address_index(result, arr);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_address_string_index(
            const ots_handle_t* wallet,
            const char* address,
            uint32_t max_account_depth,
            uint32_t max_index_depth
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::pair<uint32_t, uint32_t> idx = static_cast<ots::Wallet*>(wallet->ptr)->addressIndex(
                        address,
                        max_account_depth,
                        max_index_depth
                    );

            uint32_t* arr = new uint32_t[2];
            arr[0] = idx.first;
            arr[1] = idx.second;
            set_address_index(result, arr);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_secret_view_key(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_WIPEABLE_STRING,
                new ots::WipeableString(
                    static_cast<ots::Wallet*>(wallet->ptr)->secretViewKey()
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_public_view_key(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_WIPEABLE_STRING,
                new ots::WipeableString(
                    static_cast<ots::Wallet*>(wallet->ptr)->publicViewKey()
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_secret_spend_key(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_WIPEABLE_STRING,
                new ots::WipeableString(
                    static_cast<ots::Wallet*>(wallet->ptr)->secretSpendKey()
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_public_spend_key(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_WIPEABLE_STRING,
                new ots::WipeableString(
                    static_cast<ots::Wallet*>(wallet->ptr)->publicSpendKey()
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_import_outputs(
            const ots_handle_t* wallet,
            const char* outputs,
            const size_t outputs_size
    ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            uint64_t imported = static_cast<ots::Wallet*>(wallet->ptr)->importOutputs(
                std::string(outputs, outputs_size)
            );
            if(imported & 0x8000000000000000) // int64_t max would be 9,223,372,036,854,775,807 (should never happen IMO)
                throw ots::exception::RangeError("Imported outputs count is too large to convert to int64_t: " + std::to_string(imported));
            set_number(result, static_cast<int64_t>(imported));
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_export_key_images(const ots_handle_t* wallet) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            const std::string& key_images = static_cast<ots::Wallet*>(wallet->ptr)->exportKeyImages();
            set_binary_string(
                result,
                key_images,
                key_images.size()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_describe_tx(
        const ots_handle_t* wallet,
        const char* tx,
        const size_t tx_size
    ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_TX_DESCRIPTION,
                new ots::TxDescription(
                    static_cast<ots::Wallet*>(wallet->ptr)->describeTransaction(
                        std::string(tx, tx_size)
                    )
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_check_tx(const ots_handle_t* wallet, const ots_handle_t* tx) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET || tx->type != OTS_HANDLE_TX_DESCRIPTION)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::vector<ots::TxWarning> warnings = static_cast<ots::Wallet*>(wallet->ptr)->checkTransaction(*static_cast<ots::TxDescription*>(tx->ptr));
            ots_handle_t* handles = new ots_handle_t[warnings.size()];
            for(size_t i = 0; i < warnings.size(); i++)
                handles[i] = create_handle(OTS_HANDLE_TX_WARNING, new ots::TxWarning(warnings[i]));
            set_array(result, handles, warnings.size(), OTS_DATA_HANDLE, false);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_check_tx_string(
        const ots_handle_t* wallet,
        const char* tx,
        const size_t tx_size
    ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            std::vector<ots::TxWarning> warnings = static_cast<ots::Wallet*>(wallet->ptr)->checkTransaction(std::string(tx, tx_size));
            ots_handle_t* handles = new ots_handle_t[warnings.size()];
            for(size_t i = 0; i < warnings.size(); i++)
                handles[i] = create_handle(OTS_HANDLE_TX_WARNING, new ots::TxWarning(warnings[i]));
            set_array(result, handles, warnings.size(), OTS_DATA_HANDLE, false);
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_sign_transaction(
            const ots_handle_t* wallet,
            const char* tx,
            const size_t tx_size
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            const std::string unsigned_tx = std::string(tx, tx_size);
            const std::string& signed_tx = static_cast<ots::Wallet*>(wallet->ptr)->signTransaction(unsigned_tx);
            set_binary_string(
                result,
                signed_tx,
                signed_tx.size()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_sign_data(
            const ots_handle_t* wallet,
            const char* data,
            size_t data_size
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            const std::string& signed_data = static_cast<ots::Wallet*>(wallet->ptr)->signData(std::string(data, data_size));
            set_binary_string(
                result,
                signed_data,
                signed_data.size()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_sign_data_with_index(
            const ots_handle_t* wallet,
            const char* data,
            size_t data_size,
            uint32_t account,
            uint32_t index
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            const std::string& signed_data = static_cast<ots::Wallet*>(wallet->ptr)->signData(
                    std::string(data, data_size),
                    std::pair(account, index)
                    );
            set_binary_string(
                result,
                signed_data,
                signed_data.size()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_sign_data_with_address(
            const ots_handle_t* wallet,
            const char* data,
            size_t data_size,
            const ots_handle_t* address
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET || address->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            const std::string& signed_data = static_cast<ots::Wallet*>(wallet->ptr)->signData(
                std::string(data, data_size),
                *static_cast<ots::Address*>(address->ptr)
            );
            set_binary_string(
                result,
                signed_data,
                signed_data.size()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_sign_data_with_address_string(
            const ots_handle_t* wallet,
            const char* data,
            size_t data_size,
            const char* address
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            const std::string& signed_data = static_cast<ots::Wallet*>(wallet->ptr)->signData(
                std::string(data, data_size),
                address
            );
            set_binary_string(
                result,
                signed_data,
                signed_data.size()
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_verify_data(
            const ots_handle_t* wallet,
            const char* data,
            size_t data_size,
            const char* signature,
            bool legacy_fallback
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->verifyData(
                    std::string(data, data_size),
                    signature,
                    legacy_fallback
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_verify_data_with_index(
            const ots_handle_t* wallet,
            const char* data,
            size_t data_size,
            uint32_t account,
            uint32_t index,
            const char* signature,
            bool legacy_fallback
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                static_cast<ots::Wallet*>(wallet->ptr)->verifyData(
                    std::string(data, data_size),
                    std::pair(account, index),
                    signature,
                    legacy_fallback
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_verify_data_with_address(
            const ots_handle_t* wallet,
            const char* data,
            size_t data_size,
            const ots_handle_t* address,
            const char* signature,
            bool legacy_fallback
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET || address->type != OTS_HANDLE_ADDRESS)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                ots::Wallet::verifyData(
                    std::string(data, data_size),
                    *static_cast<ots::Address*>(address->ptr),
                    signature,
                    legacy_fallback
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_wallet_verify_data_with_address_string(
            const ots_handle_t* wallet,
            const char* data,
            size_t data_size,
            const char* address,
            const char* signature,
            bool legacy_fallback
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(wallet->type != OTS_HANDLE_WALLET)
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_boolean(
                result,
                ots::Wallet::verifyData(
                    std::string(data, data_size),
                    address,
                    signature,
                    legacy_fallback
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }
}
