#include "ots-internal.h"
#include <cstring>
#include <unordered_map>

namespace ots::internal {
    static std::unordered_map<Seed*, std::string> seedJar = {};

    extern "C" {
        ots_result_t* ots_seed_jar_add_seed(ots_handle_t* seed, const char* name) {
            ots_result_t* result = new ots_result_t();
            try {
                if(seed->type != OTS_HANDLE_SEED)
                    throw ots::exception::InvalidArgument("Invalid handle type");
                auto r = seedJar.find(static_cast<Seed*>(seed->ptr));
                if(r != seedJar.end() && r->second != "" && r->second != name)
                    throw ots::exception::InvalidArgument("Seed already in jar with other name");
                if(r == seedJar.end())
                    seedJar[static_cast<Seed*>(seed->ptr)] = name;
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
                auto r = seedJar.find(static_cast<Seed*>((*seed)->ptr));
                if(r == seedJar.end())
                    throw ots::exception::InvalidArgument("Seed not in jar");
                seedJar.erase(r);
                (*seed)->reference = false; // we transfer ownership back to the handle to free it
                ots_free_handle(seed);
                set_boolean(result, true); // we only confirm the removal
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_transfer_seed_in(ots_handle_t** seed, const char* name) {
            ots_result_t* result = new ots_result_t();
            try {
                if((*seed)->type != OTS_HANDLE_SEED)
                    throw ots::exception::InvalidArgument("Invalid handle type");
                auto r = seedJar.find(static_cast<Seed*>((*seed)->ptr));
                if(r != seedJar.end() && r->second != "" && r->second != name)
                    throw ots::exception::InvalidArgument("Seed already in jar with other name");
                if(r == seedJar.end())
                    seedJar[static_cast<Seed*>((*seed)->ptr)] = name;
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
                auto r = seedJar.find(static_cast<Seed*>((*seed)->ptr));
                if(r == seedJar.end())
                    throw ots::exception::InvalidArgument("Seed not in jar");
                set_handle(result, OTS_HANDLE_SEED, const_cast<void*>((*seed)->ptr)); // ownership is transferred back to the new handle
                seedJar.erase(r);
                (*seed)->reference = false; // we transfer ownership back to the handle to free it
                ots_free_handle(seed);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_clear(void) {
            ots_result_t* result = new ots_result_t();
            try {
                for(const auto& s : seedJar)
                    delete static_cast<const Seed*>(s.first); // we free all seeds
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
                size_t size = seedJar.size();
                ots_handle_t* seeds = new ots_handle_t[size];
                size_t i = 0;
                for(auto s : seedJar)
                    seeds[i++] = create_handle_reference(OTS_HANDLE_SEED, (void*)s.first);
                set_array(result, seeds, size, OTS_DATA_HANDLE, false);
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

        ots_result_t* ots_seed_jar_seed_for_fingerprint(const char* fingerprint) {
            ots_result_t* result = new ots_result_t();
            try {
                for(const auto& s : seedJar)
                    if(s.first->fingerprint() == fingerprint)
                        set_handle_reference(result, OTS_HANDLE_SEED, (void*)s.first);
                if(!ots_result_is_type(result, OTS_RESULT_HANDLE))
                    throw ots::exception::OutOfRange("Seed not found");
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seed_for_address(const char* address) {
            ots_result_t* result = new ots_result_t();
            try {
                for(const auto& s : seedJar)
                    if(s.first->address() == address)
                        set_handle_reference(result, OTS_HANDLE_SEED, (void*)s.first);
                if(!ots_result_is_type(result, OTS_RESULT_HANDLE))
                    throw ots::exception::OutOfRange("Seed not found");
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seed_for_name(const char* name) {
            ots_result_t* result = new ots_result_t();
            try {
                for(const auto& s : seedJar)
                    if(seedJar[s.first] == name)
                        set_handle_reference(result, OTS_HANDLE_SEED, (void*)s.first);
                if(!ots_result_is_type(result, OTS_RESULT_HANDLE))
                    throw ots::exception::OutOfRange("Seed not found");
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seed_name(const ots_handle_t* seed) {
            ots_result_t* result = new ots_result_t();
            try {
                auto r = seedJar.find(static_cast<Seed*>(seed->ptr));
                if(r == seedJar.end())
                    throw ots::exception::OutOfRange("Seed not found");
                set_string(result, r->second);
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }

        ots_result_t* ots_seed_jar_seed_rename(const ots_handle_t* seed, const char* name) {
            ots_result_t* result = new ots_result_t();
            try {
                auto r = seedJar.find(static_cast<Seed*>(seed->ptr));
                if(r == seedJar.end())
                    throw ots::exception::OutOfRange("Seed not found");
                seedJar[static_cast<Seed*>(seed->ptr)] = name;
                set_boolean(result, true); // we only confirm the renaming
            } catch(const ots::exception::Exception& e) {
                set_error(result, e);
            }
            return result;
        }
    }
}
