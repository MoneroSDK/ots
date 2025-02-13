#include "ots.hpp"

namespace ots {

    seed_handle_t SeedJar::store(const Seed& seed) noexcept {
        static seed_handle_t last_handle = 0;
        seed_handle_t handle = last_handle++;
        s_seeds[handle] = std::make_unique<Seed>(seed);
        return handle;
    }

    const Seed& SeedJar::get(seed_handle_t handle) {
        if(!has(handle))
            throw exception::seedjar::SeedNotFound();
        return *s_seeds[handle];
    }

    const Seed& SeedJar::get(const std::string& fingerprint) {
        for(const auto& seed : s_seeds) {
            if(seed.second->fingerprint() == fingerprint)
                return *seed.second;
        }
        throw exception::seedjar::SeedNotFound();
    }

    void SeedJar::remove(seed_handle_t handle) noexcept {
        s_seeds.erase(handle);
    }

    void SeedJar::remove(const std::string& fingerprint) noexcept {
        for(auto it = s_seeds.begin(); it != s_seeds.end(); ++it) {
            if(it->second->fingerprint() == fingerprint) {
                s_seeds.erase(it);
                return;
            }
        }
    }

    void SeedJar::clear() noexcept {
        s_seeds.clear();
    }

    uint32_t SeedJar::count() noexcept {
        return s_seeds.size();
    }

    bool SeedJar::has(seed_handle_t handle) noexcept {
        return s_seeds.find(handle) != s_seeds.end();
    }

    bool SeedJar::has(const std::string& fingerprint) noexcept {
        for(const auto& seed : s_seeds) {
            if(seed.second->fingerprint() == fingerprint)
                return true;
        }
        return false;
    }

    std::vector<std::reference_wrapper<const Seed>> SeedJar::list() const noexcept {
        std::vector<std::reference_wrapper<const Seed>> seeds;
        for(const auto& seed : s_seeds)
            seeds.emplace_back(std::cref(*seed.second));
        return seeds;
    }

    std::vector<const Seed*> SeedJar::listPtr() const noexcept {
        std::vector<const Seed*> seeds;
        for(const auto& seed : s_seeds)
            seeds.emplace_back(seed.second.get());
        return seeds;
    }
} // namespace ots
