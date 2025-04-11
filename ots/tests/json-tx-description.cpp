#include "json-tx-description.hpp"
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <rapidjson/error/en.h>

ots::TxDescription txDescriptionFromJsonFile(
    const std::string& filename,
    const std::string& unsignedTxSet
) {
    std::ifstream file(filename);
    if(!file.is_open())
        throw std::runtime_error("Failed to open file " + filename);
    rapidjson::IStreamWrapper isw(file);
    rapidjson::Document document;
    document.ParseStream(isw);
    if(document.HasParseError())
        throw std::runtime_error("Failed to parse JSON from file: " + std::string(rapidjson::GetParseError_En(document.GetParseError())));
    return txDescriptionFromJsonDocument(document, unsignedTxSet);
}

ots::TxDescription txDescriptionFromJson(
    const std::string& jsonString,
    const std::string& unsignedTxSet
) {
    rapidjson::Document document;
    document.Parse(jsonString.c_str());
    if(document.HasParseError())
        throw std::runtime_error("Failed to parse JSON: " + std::string(rapidjson::GetParseError_En(document.GetParseError())));
    return txDescriptionFromJsonDocument(document, unsignedTxSet);
}

ots::TxDescription txDescriptionFromJsonDocument(
    const rapidjson::Document& document,
    const std::string& unsignedTxSet
) {
    if(
        !document.IsObject()
        || !document.HasMember("result")
        || !document["result"].IsObject()
        || !document["result"].HasMember("desc")
        || !document["result"]["desc"].IsArray()
        || !document["result"].HasMember("summary")
        || !document["result"]["summary"].IsObject()
        || !document["result"]["summary"].HasMember("recipients")
        || !document["result"]["summary"]["recipients"].IsArray()
    )
        throw std::runtime_error("Invalid JSON structure");
    ots::TxDescription txDesc{unsignedTxSet};
    const auto& summary = document["result"]["summary"];
    if(summary.HasMember("amount_in") && summary["amount_in"].IsUint64())
        txDesc.amountIn = summary["amount_in"].GetUint64();
    if(summary.HasMember("amount_out") && summary["amount_out"].IsUint64())
        txDesc.amountOut = summary["amount_out"].GetUint64();
    if(summary.HasMember("recipients")) {
        const auto& recipients = summary["recipients"];
        for (const auto& recipient : recipients.GetArray()) {
            if (recipient.HasMember("address") && recipient["address"].IsString() &&
                    recipient.HasMember("amount") && recipient["amount"].IsUint64()) {
                txDesc.flows.emplace_back(ots::FlowVector{ots::Address(recipient["address"].GetString()), recipient["amount"].GetUint64()});
            }
        }
    }
    if(
            summary.HasMember("change_address")
            && summary["change_address"].IsString()
            && !std::string(summary["change_address"].GetString()).empty()
            && summary.HasMember("change_amount")
            && summary["change_amount"].IsUint64()
      ) {
        txDesc.change = ots::FlowVector{ots::Address(summary["change_address"].GetString()), summary["change_amount"].GetUint64()};
    }
    if(summary.HasMember("fee") && summary["fee"].IsUint64())
        txDesc.fee = summary["fee"].GetUint64();
    for(const auto& desc: document["result"]["desc"].GetArray()) {
        ots::TransferDescription transferDesc;
        if(
                desc.HasMember("amount_in")
                && desc["amount_in"].IsUint64()
          )
            transferDesc.amountIn = desc["amount_in"].GetUint64();
        if(
                desc.HasMember("amount_out")
                && desc["amount_out"].IsUint64()
          )
            transferDesc.amountOut = desc["amount_out"].GetUint64();
        if(desc.HasMember("recipients")) {
            for(const auto& recipient: desc["recipients"].GetArray()) {
                if(recipient.HasMember("address") && recipient["address"].IsString() &&
                        recipient.HasMember("amount") && recipient["amount"].IsUint64()) {
                    transferDesc.flows.emplace_back(ots::FlowVector{ots::Address(recipient["address"].GetString()), recipient["amount"].GetUint64()});
                }
            }
        }
        if(
                desc.HasMember("change_address")
                && desc["change_address"].IsString()
                && !std::string(desc["change_address"].GetString()).empty()
                && desc.HasMember("change_amount")
                && desc["change_amount"].IsUint64()
          ) {
            transferDesc.change = ots::FlowVector{ots::Address(desc["change_address"].GetString()), desc["change_amount"].GetUint64()};
        }
        if(desc.HasMember("fee") && desc["fee"].IsUint64())
            transferDesc.fee = desc["fee"].GetUint64();
        if(desc.HasMember("payment_id") && desc["payment_id"].IsString())
            transferDesc.paymentId = desc["payment_id"].GetString();
        if(desc.HasMember("dummy_outputs") && desc["dummy_outputs"].IsUint())
            transferDesc.dummyOutputs = static_cast<uint32_t>(desc["dummy_outputs"].GetUint());
        if(desc.HasMember("extra") && desc["extra"].IsString())
            transferDesc.extra = desc["extra"].GetString();
        if(desc.HasMember("ring_size") && desc["ring_size"].IsUint())
            transferDesc.ringSize = static_cast<uint32_t>(desc["ring_size"].GetUint());
        if(desc.HasMember("unlock_time") && desc["unlock_time"].IsUint64())
            transferDesc.unlockTime = desc["unlock_time"].GetUint64();
        txDesc.transfers.push_back(transferDesc);
    }
    return txDesc;
}
