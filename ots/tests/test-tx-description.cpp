#include "test-tx-description.hpp"
#include "json-tx-description.h"
#include <iostream>

bool equalTxDescriptions(
    const ots::TxDescription& a,
    const ots::TxDescription& b,
    bool includeUnisignedTxSet
) {
    if(includeUnisignedTxSet && a != b)
            return false;
    if(
        a.amountIn != b.amountIn
        || a.amountOut != b.amountOut
        || a.flows.size() != b.flows.size()
        || (
            a.change.has_value()
            && b.change.has_value()
            && a.change.value().address != b.change.value().address
            && a.change.value().amount != b.change.value().amount
           )
        || a.fee != b.fee
        || a.transfers.size() != b.transfers.size()
    )
        return false;
    for(size_t i = 0; i < a.transfers.size(); ++i) {
        if(
            a.transfers[i].paymentId != b.transfers[i].paymentId
            || a.transfers[i].dummyOutputs != b.transfers[i].dummyOutputs
            || a.transfers[i].extra != b.transfers[i].extra
            || a.transfers[i].ringSize != b.transfers[i].ringSize
            || (
                a.transfers[i].change.has_value()
                && b.transfers[i].change.has_value()
                && a.transfers[i].change.value().address != b.transfers[i].change.value().address
                && a.transfers[i].change.value().amount != b.transfers[i].change.value().amount
               )
        ) {
            std::cerr << "TxTransfer mismatch: " << a.transfers[i].dummyOutputs << " != " << b.transfers[i].dummyOutputs << std::endl;
            std::cerr << "TxTransfer mismatch: " << a.transfers[i].ringSize << " != " << b.transfers[i].ringSize << std::endl;
            return false;
        }
    }
    return true;
}

extern "C" {
    bool equalTxDescriptions(
        const void* a,
        const void* b
    ) {
        if(a == nullptr || b == nullptr)
            return false;
        return equalTxDescriptions(
            *static_cast<const ots::TxDescription*>(a),
            *static_cast<const ots::TxDescription*>(b)
        );
    }
}
