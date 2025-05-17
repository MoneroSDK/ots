#include "ots-internal.h"
#include <cstring>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace ots::internal {
    class SeedJar {
        private:
            // Using a map for fast key-based lookup and ordered storage
            std::unordered_map<Seed*, std::string> seedNameMap;
            // Using a vector to maintain insertion order and provide index-based access
            std::vector<Seed*> seedOrder;

        public:
            // Add a seed to the jar
            bool add(Seed* seed, const std::string& name) {
                if(name.empty() || contains(seed))
                    return false;
                seedNameMap[seed] = name;
                seedOrder.push_back(seed);
                return true;
            }

            // Remove a seed from the jar
            bool remove(Seed* seed) {
                if(!contains(seed))
                    return false;
                auto vecIt = std::find(seedOrder.begin(), seedOrder.end(), seed);
                if(vecIt != seedOrder.end())
                    seedOrder.erase(vecIt);
                seedNameMap.erase(seedNameMap.find(seed));
                return true;
            }

            // Clear the jar
            void clear() {
                for(auto* seed : seedOrder)
                    delete seed;
                seedOrder.clear();
                seedNameMap.clear();
            }

            // Get seed count
            size_t size() const {
                return seedOrder.size();
            }

            // Get seed by index
            Seed* seedByIndex(size_t index) const {
                if(index >= seedOrder.size())
                    return nullptr;
                return seedOrder[index];
            }

            // Get seed by name
            Seed* seedByName(const std::string& name) const {
                for(const auto& [seed, seedName] : seedNameMap)
                    if (seedName == name)
                        return seed;
                return nullptr;
            }

            // Get seed by fingerprint
            Seed* seedByFingerprint(const std::string& fingerprint) const {
                for(const auto* seed : seedOrder)
                    if(seed->fingerprint() == fingerprint)
                        return const_cast<Seed*>(seed);
                return nullptr;
            }

            // Get seed by address
            Seed* seedByAddress(const std::string& address) const {
                for(const auto* seed : seedOrder)
                    if(seed->address() == address)
                        return const_cast<Seed*>(seed);
                return nullptr;
            }

            // Get name of a seed
            std::string nameOf(const Seed* seed) const {
                auto it = seedNameMap.find(const_cast<Seed*>(seed));
                if(it != seedNameMap.end())
                    return it->second;
                return "";
            }

            // Rename a seed
            bool rename(Seed* seed, const std::string& name) {
                auto it = seedNameMap.find(seed);
                if(it == seedNameMap.end())
                    return false;
                it->second = name;
                return true;
            }

            // Get all seeds
            std::vector<Seed*>& allSeeds() {
                return seedOrder;
            }

            // Check if a seed exists
            bool contains(const Seed* seed) const {
                return seedNameMap.find(const_cast<Seed*>(seed)) != seedNameMap.end();
            }

            bool containsName(const std::string& name) const {
                return std::any_of(
                    seedNameMap.begin(),
                    seedNameMap.end(),
                    [&name](const auto& pair) {
                        return pair.second == name;
                    }
                );
            }
    };
    static SeedJar seedJar;

    extern "C" {
        ots_result_t* ots_seed_jar_add_seed(ots_handle_t* seed, const char* name) {
            ots_result_t* result = new ots_result_t();
            try {
                if(seed->type != OTS_HANDLE_SEED)
                    throw ots::exception::InvalidArgument("Invalid handle type");
                if(seedJar.contains(static_cast<Seed*>(seed->ptr)))
                    throw ots::exception::InvalidArgument("Seed already in jar with other name");
                if(name == nullptr || name[0] == '\0')
                    throw ots::exception::InvalidArgument("Invalid empty name");
                if(seedJar.containsName(name))
                    throw ots::exception::InvalidArgument("Name already in use");
                seedJar.add(static_cast<Seed*>(seed->ptr), name);
                set_handle_reference(result, OTS_HANDLE_SEED, seed->ptr);
                seed->reference = true; // we take ownership of the handle
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_remove_seed(ots_handle_t** seed) {
            ots_result_t* result = new ots_result_t();
            try {
                if((*seed)->type != OTS_HANDLE_SEED)
                    throw ots::exception::InvalidArgument("Invalid handle type");
                if(!seedJar.contains(static_cast<Seed*>((*seed)->ptr)))
                    throw ots::exception::InvalidArgument("Seed not in jar");
                bool removed = seedJar.remove(static_cast<Seed*>((*seed)->ptr));
                (*seed)->reference = false; // we transfer ownership back to the handle to free it
                ots_free_handle(seed);
                set_boolean(result, removed); // we only confirm the removal
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_purge_seed_for_index(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                bool removed = seedJar.remove(seed);
                delete seed;
                set_boolean(result, removed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_purge_seed_for_name(const char* name) {
            ots_result_t* result = new ots_result_t();
            try {
                Seed* seed = seedJar.seedByName(name);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                bool removed = seedJar.remove(seed);
                delete seed;
                set_boolean(result, removed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_purge_seed_for_fingerprint(const char* fingerprint) {
            ots_result_t* result = new ots_result_t();
            try {
                Seed* seed = seedJar.seedByFingerprint(fingerprint);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                bool removed = seedJar.remove(seed);
                delete seed;
                set_boolean(result, removed); // we only confirm the removal
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_purge_seed_for_address(const char* address) {
            ots_result_t* result = new ots_result_t();
            try {
                Seed* seed = seedJar.seedByAddress(address);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                bool removed = seedJar.remove(seed);
                delete seed;
                set_boolean(result, removed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_transfer_seed_in(
            ots_handle_t** seed,
            const char* name
        ) {
            ots_result_t* result = new ots_result_t();
            try {
                if((*seed)->type != OTS_HANDLE_SEED)
                    throw ots::exception::InvalidArgument("Invalid handle type");
                if(seedJar.contains(static_cast<Seed*>((*seed)->ptr)))
                    throw ots::exception::InvalidArgument("Seed already in jar with other name");
                if(name == nullptr || name[0] == '\0')
                    throw ots::exception::InvalidArgument("Invalid empty name");
                if(seedJar.containsName(name))
                    throw ots::exception::InvalidArgument("Name already in use");
                seedJar.add(static_cast<Seed*>((*seed)->ptr), name);
                set_handle_reference(result, OTS_HANDLE_SEED, (*seed)->ptr); // we return the same handle but as reference
                (*seed)->reference = true; // we take ownership away from the handle, important to not free seed itself
                ots_free_handle(seed); // we free the handle itself
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_transfer_seed_out(ots_handle_t** seed) {
            ots_result_t* result = new ots_result_t();
            try {
                if((*seed)->type != OTS_HANDLE_SEED)
                    throw ots::exception::InvalidArgument("Invalid handle type");
                if(!seedJar.contains(static_cast<Seed*>((*seed)->ptr)))
                    throw ots::exception::InvalidArgument("Seed not in jar");
                set_handle(result, OTS_HANDLE_SEED, const_cast<void*>((*seed)->ptr)); // ownership is transferred back to the new handle
                seedJar.remove(static_cast<Seed*>((*seed)->ptr));
                (*seed)->reference = true; // we remove ownership from the handle to free it
                ots_free_handle(seed); // we free the handle itself
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_transfer_seed_out_for_index(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_handle(result, OTS_HANDLE_SEED, static_cast<void*>(seed));
                seedJar.remove(seed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_transfer_seed_out_for_name(const char* name) {
            ots_result_t* result = new ots_result_t();
            try {
                Seed* seed = seedJar.seedByName(name);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_handle(result, OTS_HANDLE_SEED, static_cast<void*>(seed));
                seedJar.remove(seed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_transfer_seed_out_for_fingerprint(const char* fingerprint) {
            ots_result_t* result = new ots_result_t();
            try {
                Seed* seed = seedJar.seedByFingerprint(fingerprint);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_handle(result, OTS_HANDLE_SEED, static_cast<void*>(seed));
                seedJar.remove(seed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_transfer_seed_out_for_address(const char* address) {
            ots_result_t* result = new ots_result_t();
            try {
                Seed* seed = seedJar.seedByAddress(address);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_handle(result, OTS_HANDLE_SEED, static_cast<void*>(seed));
                seedJar.remove(seed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_clear(void) {
            ots_result_t* result = new ots_result_t();
            try {
                seedJar.clear();
                set_boolean(result, true); // we only confirm the removal
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seeds(void) {
            ots_result_t* result = new ots_result_t();
            try {
                ots_handle_t* seeds = new ots_handle_t[seedJar.size()];
                size_t i = 0;
                for(Seed* s : seedJar.allSeeds())
                    seeds[i++] = create_handle_reference(OTS_HANDLE_SEED, (void*)s);
                set_array(result, seeds, seedJar.size(), OTS_DATA_HANDLE, false);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seed_count(void) {
            ots_result_t* result = new ots_result_t();
            try {
                set_number(result, seedJar.size());
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seed_for_index(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_handle_reference(result, OTS_HANDLE_SEED, (void*)seed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seed_for_fingerprint(const char* fingerprint) {
            ots_result_t* result = new ots_result_t();
            try {
                Seed* seed = seedJar.seedByFingerprint(fingerprint);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_handle_reference(result, OTS_HANDLE_SEED, (void*)seed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seed_for_address(const char* address) {
            ots_result_t* result = new ots_result_t();
            try {
                Seed* seed = seedJar.seedByAddress(address);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_handle_reference(result, OTS_HANDLE_SEED, (void*)seed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seed_for_name(const char* name) {
            ots_result_t* result = new ots_result_t();
            try {
                Seed* seed = seedJar.seedByName(name);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_handle_reference(result, OTS_HANDLE_SEED, (void*)seed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seed_name(const ots_handle_t* seed) {
            ots_result_t* result = new ots_result_t();
            try {
                if(!seedJar.contains(static_cast<Seed*>(seed->ptr)))
                    throw ots::exception::OutOfRange("Seed not found");
                set_string(result, seedJar.nameOf(static_cast<Seed*>(seed->ptr)));
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seed_rename(
            const ots_handle_t* seed,
            const char* name
        ) {
            ots_result_t* result = new ots_result_t();
            try {
                if(!seedJar.contains(static_cast<Seed*>(seed->ptr)))
                    throw ots::exception::OutOfRange("Seed not found");
                if(name == nullptr || name[0] == '\0')
                    throw ots::exception::InvalidArgument("Invalid empty name");
                if(seedJar.containsName(name))
                    throw ots::exception::InvalidArgument("Name already in use");
                set_boolean(
                    result,
                    seedJar.rename(static_cast<Seed*>(seed->ptr), name)
                );
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_name(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_string(result, seedJar.nameOf(seed));
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_fingerprint(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_string(result, seed->fingerprint());
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_address(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_handle(
                        result,
                        OTS_HANDLE_ADDRESS,
                        new ots::Address(seed->address())
                        );
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_address_string(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_string(result, seed->address());
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_seed_type(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_seed_type(result, seed->type());
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_seed_type_string(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                std::vector<std::string> types = {
                    "Monero",
                    "Polyseed",
                };
                set_string(result, types[static_cast<int>(seed->type())]);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_is_legacy(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_boolean(result, seed->isLegacy());
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_network(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_network(result, seed->network());
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_network_string(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                std::vector<std::string> networks = {
                    "Main",
                    "Test",
                    "Stage",
                };
                set_string(result, networks[static_cast<int>(seed->network())]);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_height(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_number(result, seed->height());
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_timestamp(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_number(result, seed->timestamp());
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_item_wallet(size_t index) {
            ots_result_t* result = new ots_result_t();
            try {
                if(index >= seedJar.size())
                    throw ots::exception::OutOfRange("Index out of range");
                Seed* seed = seedJar.seedByIndex(index);
                if(seed == nullptr)
                    throw ots::exception::OutOfRange("Seed not found");
                set_handle_reference(
                    result,
                    OTS_HANDLE_WALLET,
                    seed->wallet().get()
                );
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }
    }
}
