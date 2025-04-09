#pragma once

#include <ots.hpp>
#include <rapidjson/document.h>

/**
 * Construct TxDescription from a JSON string.
 *
 * @param jsonString JSON string containing transaction description.
 * @return A constructed TxDescription object or an empty optional if construction fails.
 */
ots::TxDescription txDescriptionFromJson(
    const std::string& jsonString,
    const std::string& unsignedTxSet = ""
);

/**
 * Construct TxDescription from a JSON file.
 *
 * @param filename Path to the JSON file containing transaction description.
 * @return A constructed TxDescription object or an empty optional if construction fails.
 */
ots::TxDescription txDescriptionFromJsonFile(
    const std::string& filename,
    const std::string& unsignedTxSet = ""
);

/**
 * Construct TxDescription from a JSON document.
 *
 * @param document A rapidjson::Document object containing the transaction description.
 * @return A constructed TxDescription object or an empty optional if construction fails.
 */
ots::TxDescription txDescriptionFromJsonDocument(
    const rapidjson::Document& document,
    const std::string& unsignedTxSet = ""
);
