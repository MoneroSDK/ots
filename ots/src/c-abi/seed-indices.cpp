#include "ots-internal.h"

using namespace ots::internal;

extern "C" {
    ots_result_t* ots_seed_indices_create(uint16_t* indices, size_t size) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(result, OTS_HANDLE_SEED_INDICES, new ots::SeedIndices(indices, size));
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_create_from_string(const char* str, const char* separator) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(
                result,
                OTS_HANDLE_SEED_INDICES,
                new ots::SeedIndices(ots::SeedIndices::fromNumeric(str, separator))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_create_from_hex(const char* hex, const char* separator) {
        ots_result_t* result = new ots_result_t();
        try {
            set_handle(
                result,
                OTS_HANDLE_SEED_INDICES,
                new ots::SeedIndices(ots::SeedIndices::fromHex(hex, separator))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    const uint16_t* ots_seed_indices_values(const ots_handle_t* handle) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return nullptr;
        return *static_cast<ots::SeedIndices*>(handle->ptr);
    }

    size_t ots_seed_indices_count(const ots_handle_t* handle) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return 0;
        return static_cast<ots::SeedIndices*>(handle->ptr)->size();
    }

    void ots_seed_indices_clear(const ots_handle_t* handle) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return;
        static_cast<ots::SeedIndices*>(handle->ptr)->clear();
    }

    void ots_seed_indices_append(const ots_handle_t* handle, uint16_t index) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return;
        static_cast<ots::SeedIndices*>(handle->ptr)->emplace_back(index);
    }

    char* ots_seed_indices_numeric(const ots_handle_t* handle, const char* separator) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return nullptr;
        return create_string_copy(static_cast<ots::SeedIndices*>(handle->ptr)->numeric(separator));
    }

    char* ots_seed_indices_hex(const ots_handle_t* handle, const char* separator) {
        if(handle->type != OTS_HANDLE_SEED_INDICES)
            return nullptr;
        return create_string_copy(static_cast<ots::SeedIndices*>(handle->ptr)->hex(separator));
    }

    ots_result_t* ots_seed_indices_merge_values(
            const ots_handle_t* seed_indices1,
            const ots_handle_t* seed_indices2
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(
                seed_indices1->type != OTS_HANDLE_SEED ||
                seed_indices2->type != OTS_HANDLE_SEED_INDICES
            )
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_SEED_INDICES,
                new ots::SeedIndices(
                    ots::Seed::mergeValues(
                        static_cast<const std::vector<uint16_t>>(
                            *static_cast<ots::SeedIndices*>(seed_indices1->ptr)),
                        static_cast<const std::vector<uint16_t>>(
                            *static_cast<ots::SeedIndices*>(seed_indices2->ptr))
                    )
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_merge_with_password(
            const char* password,
            const ots_handle_t* seed_indices
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            if(
                seed_indices->type != OTS_HANDLE_SEED_INDICES
            )
                throw ots::exception::InvalidArgument("Invalid handle type");
            set_handle(
                result,
                OTS_HANDLE_SEED_INDICES,
                new ots::SeedIndices(
                    ots::Seed::mergeWithPassword(
                        password,
                        static_cast<const std::vector<uint16_t>>(
                            *static_cast<ots::SeedIndices*>(seed_indices->ptr))
                    )
                )
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_merge_multiple_values(
            const ots_handle_t* seed_indices[],
            size_t elements,
            size_t count
            ) {
        ots_result_t* result = new ots_result_t();
        try {
            std::vector<std::vector<uint16_t>> indices;
            for(size_t i = 0; i < count; ++i) {
                if(seed_indices[i]->type != OTS_HANDLE_SEED_INDICES)
                    throw ots::exception::InvalidArgument("Invalid handle type");
                indices.emplace_back(static_cast<const std::vector<uint16_t>>(
                            *static_cast<ots::SeedIndices*>(seed_indices[i]->ptr)));
            }
            set_handle(
                result,
                OTS_HANDLE_SEED_INDICES,
                new ots::SeedIndices(ots::Seed::mergeValues(indices))
            );
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_merge_values_and_zero(
            const ots_handle_t* seed_indices1,
            const ots_handle_t* seed_indices2,
            bool delete_after
            ) {
        ots_result_t* result = ots_seed_indices_merge_values(seed_indices1, seed_indices2);
        if(ots_is_error(result))
            return result;
        try {
            if(delete_after) {
                ots_free_handle(const_cast<ots_handle_t**>(&seed_indices1));
                ots_free_handle(const_cast<ots_handle_t**>(&seed_indices2));
            }
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_merge_with_password_and_zero(
            const char* password,
            const ots_handle_t* seed_indices,
            bool delete_after
            ) {
        ots_result_t* result = ots_seed_indices_merge_with_password(password, seed_indices);
        if(ots_is_error(result))
            return result;
        try {
            if(delete_after)
                ots_free_handle(const_cast<ots_handle_t**>(&seed_indices));
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }

    ots_result_t* ots_seed_indices_merge_multiple_values_and_zero(
            const ots_handle_t* seed_indices[],
            size_t elements,
            size_t count,
            bool delete_after
            ) {
        ots_result_t* result = ots_seed_indices_merge_multiple_values(seed_indices, elements, count);
        if(ots_is_error(result))
            return result;
        try {
            if(delete_after)
                for(size_t i = 0; i < count; ++i)
                    ots_free_handle(const_cast<ots_handle_t**>(&seed_indices[i]));
        } catch(const ots::exception::Exception& e) {
            set_error(result, e);
        }
        return result;
    }
}
