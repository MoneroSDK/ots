#include "ots-internal.h"
#include <cstring>
#include <string>

using namespace ots::internal;

/**
 * @brief Hack class to access the protected method unsignedTxSet()
 * @internal
 * @note usage: UnsignedTxSet::from(txDescription)
 */
class UnsignedTxSet : public ots::TxDescription {
public:
    explicit UnsignedTxSet(const ots::TxDescription& original) 
        : ots::TxDescription(original) {}
    
    operator const std::string() const {
        return this->unsignedTxSet();
    }

    static const std::string from(const ots::TxDescription& original) {
        static UnsignedTxSet instance(original);
        return instance;
    }
};

extern "C" {
    ots_tx_description_t* ots_tx_description(
        const ots_handle_t* tx_description
    ) {
        if(tx_description == nullptr)
            return nullptr;
        try {
            if(tx_description->type != OTS_HANDLE_TX_DESCRIPTION)
                throw ots::exception::InvalidArgument("Invalid handle type");
            ots::TxDescription* txDescription = static_cast<ots::TxDescription*>(tx_description->ptr);
            ots_tx_description_t* result = new ots_tx_description_t();
            const std::string txSet = UnsignedTxSet::from(*txDescription);
            result->tx_set = new char[txSet.size() + 1];
            std::strncpy(result->tx_set, txSet.c_str(), txSet.size());
            result->tx_set[txSet.size()] = '\0';
            result->tx_set_size = txSet.size();
            result->amount_in = txDescription->amountIn;
            result->amount_out = txDescription->amountOut;
            ots_flow_vector_t* flows = new ots_flow_vector_t[txDescription->flows.size()];
            for(size_t i = 0; i < txDescription->flows.size(); i++) {
                const std::string address = txDescription->flows[i].address;
                flows[i].address = strdup(address.c_str());
                flows[i].amount = txDescription->flows[i].amount;
            }
            result->flows = flows;
            result->flows_size = txDescription->flows.size();
            if(
                txDescription->change.has_value() == false
                || txDescription->change->amount == 0
            )
                result->change = nullptr;
            else {
                result->change = new ots_flow_vector_t;
                result->change->amount = txDescription->change->amount;
                std::string address = txDescription->change->address;
                result->change->address = strdup(address.c_str());
            }
            result->fee = txDescription->fee;
            ots_transfer_description_t* transfers = new ots_transfer_description_t[txDescription->transfers.size()];
            for(size_t i = 0; i < txDescription->transfers.size(); i++) {
                transfers[i].amount_in = txDescription->transfers[i].amountIn;
                transfers[i].amount_out = txDescription->transfers[i].amountOut;
                transfers[i].ring_size = txDescription->transfers[i].ringSize;
                transfers[i].unlock_time = txDescription->transfers[i].unlockTime;
                ots_flow_vector_t* transferFlows = new ots_flow_vector_t[txDescription->transfers[i].flows.size()];
                for(size_t f = 0; f < txDescription->transfers[i].flows.size(); f++) {
                    const std::string address = txDescription->transfers[i].flows[f].address;
                    transferFlows[f].address = strdup(address.c_str());
                    transferFlows[f].amount = txDescription->transfers[i].flows[f].amount;
                }
                transfers[i].flows = transferFlows;
                transfers[i].flows_size = txDescription->transfers[i].flows.size();
                if(
                    txDescription->transfers[i].change.has_value() == false
                    || txDescription->transfers[i].change->amount == 0
                )
                    transfers[i].change = nullptr;
                else {
                    transfers[i].change = new ots_flow_vector_t;
                    transfers[i].change->amount = txDescription->transfers[i].change->amount;
                    std::string address = txDescription->transfers[i].change->address;
                    transfers[i].change->address = strdup(address.c_str());
                }
                transfers[i].fee = txDescription->transfers[i].fee;
                if(txDescription->transfers[i].paymentId.empty())
                    transfers[i].payment_id = nullptr;
                else {
                    const std::string paymentId = txDescription->transfers[i].paymentId;
                    transfers[i].payment_id = new char[paymentId.size() + 1];
                    std::strncpy(transfers[i].payment_id, paymentId.c_str(), paymentId.size());
                    transfers[i].payment_id[paymentId.size()] = '\0';
                }
                transfers[i].dummy_outputs = txDescription->transfers[i].dummyOutputs;
                if(txDescription->transfers[i].extra.empty())
                    transfers[i].tx_extra = nullptr;
                else {
                    const std::string txExtra = txDescription->transfers[i].extra;
                    transfers[i].tx_extra = new char[txExtra.size() + 1];
                    std::strncpy(transfers[i].tx_extra, txExtra.c_str(), txExtra.size());
                    transfers[i].tx_extra[txExtra.size()] = '\0';
                }
            }
            result->transfers = transfers;
            result->transfers_size = txDescription->transfers.size();
            return result;
        } catch(const ots::exception::Exception& e) {
            return nullptr;
        }
    }

    const char* ots_tx_description_tx_set(
        const ots_handle_t* tx_description
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
        )
            return nullptr;
        const std::string txSet = UnsignedTxSet::from(*static_cast<ots::TxDescription*>(tx_description->ptr));
        char* result = new char[txSet.size() + 1];
        std::strncpy(result, txSet.c_str(), txSet.size());
        result[txSet.size()] = '\0';
        return result;
    }

    size_t ots_tx_description_tx_set_size(
        const ots_handle_t* tx_description
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
        )
            return 0;
        return static_cast<const std::string>(
                UnsignedTxSet::from(*static_cast<ots::TxDescription*>(tx_description->ptr))
                ).size();
    }

    uint64_t ots_tx_description_amount_in(
        const ots_handle_t* tx_description
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->amountIn;
    }

    uint64_t ots_tx_description_amount_out(
        const ots_handle_t* tx_description
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->amountOut;
    }

    size_t ots_tx_description_flows_count(
        const ots_handle_t* tx_description
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->flows.size();
    }

    const char* ots_tx_description_flow_address(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->flows.size()
        )
            return nullptr;
        return strdup(
            static_cast<const std::string&>(
                static_cast<ots::TxDescription*>(
                    tx_description->ptr
                )->flows[index].address
            ).c_str()
        );
    }

    uint64_t ots_tx_description_flow_amount(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->flows.size()
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->flows[index].amount;
    }

    uint64_t ots_tx_description_fee(
        const ots_handle_t* tx_description
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->fee;
    }

    size_t ots_tx_description_transfers_count(
        const ots_handle_t* tx_description
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size();
    }

    uint64_t ots_tx_description_transfer_amount_in(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].amountIn;
    }

    uint64_t ots_tx_description_transfer_amount_out(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].amountOut;
    }

    uint32_t ots_tx_description_transfer_ring_size(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].ringSize;
    }

    uint64_t ots_tx_description_transfer_unlock_time(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].unlockTime;
    }

    size_t ots_tx_description_transfer_flows_count(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].flows.size();
    }

    const char* ots_tx_description_transfer_flow_address(
        const ots_handle_t* tx_description,
        size_t index,
        size_t flow_index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
            || flow_index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].flows.size()
        )
            return nullptr;
        return strdup(
            static_cast<const std::string&>(
                static_cast<ots::TxDescription*>(
                    tx_description->ptr
                )->transfers[index].flows[flow_index].address
            ).c_str()
        );
    }

    uint64_t ots_tx_description_transfer_flow_amount(
        const ots_handle_t* tx_description,
        size_t index,
        size_t flow_index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
            || flow_index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].flows.size()
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].flows[flow_index].amount;
    }

    bool ots_tx_description_transfer_has_change(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return false;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].change.has_value();
    }

    const char* ots_tx_description_transfer_change_address(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return nullptr;
        return strdup(
            static_cast<const std::string&>(
                static_cast<ots::TxDescription*>(
                    tx_description->ptr
                )->transfers[index].change.value().address
            ).c_str()
        );
    }

    uint64_t ots_tx_description_transfer_change_amount(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].change.value().amount;
    }

    uint64_t ots_tx_description_transfer_fee(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].fee;
    }

    const char* ots_tx_description_transfer_payment_id(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return nullptr;
        return strdup(
            static_cast<std::string>(
                static_cast<ots::TxDescription*>(
                    tx_description->ptr
                )->transfers[index].paymentId
            ).c_str()
        );
    }

    uint32_t ots_tx_description_transfer_dummy_outputs(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return 0;
        return static_cast<ots::TxDescription*>(tx_description->ptr)->transfers[index].dummyOutputs;
    }

    const char* ots_tx_description_transfer_extra(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return nullptr;
        return strdup(
            static_cast<const std::string&>(
                static_cast<ots::TxDescription*>(
                    tx_description->ptr
                )->transfers[index].extra
            ).c_str()
        );
    }

    size_t ots_tx_description_transfer_extra_size(
        const ots_handle_t* tx_description,
        size_t index
    ) {
        if(
            tx_description == nullptr
            || tx_description->type != OTS_HANDLE_TX_DESCRIPTION
            || index >= static_cast<ots::TxDescription*>(tx_description->ptr)->transfers.size()
        )
            return 0;
        return static_cast<const std::string&>(
                    static_cast<ots::TxDescription*>(
                        tx_description->ptr
                    )->transfers[index].extra
                ).size();
    }
}
