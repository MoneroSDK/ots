#include "ots.hpp"

namespace ots {

    SeedIndices& SeedIndices::operator=(const SeedIndices& other) {
        if (this != &other) {
            wipe();
            m_vec = other.m_vec;
        }
        return *this;
    }

    SeedIndices& SeedIndices::operator=(SeedIndices&& other) noexcept {
        if (this != &other) {
            wipe();
            m_vec = std::move(other.m_vec);
        }
        return *this;
    }

    SeedIndices::~SeedIndices() {
        wipe();
    }

    size_t SeedIndices::size() const noexcept {
        return m_vec.size();
    }

    bool SeedIndices::empty() const noexcept {
        return m_vec.empty();
    }

    void SeedIndices::clear() noexcept {
        wipe();
        m_vec.clear();
    }

    uint16_t& SeedIndices::operator[](size_t pos) {
        return m_vec[pos];
    }

    const uint16_t& SeedIndices::operator[](size_t pos) const {
        return m_vec[pos];
    }

    uint16_t& SeedIndices::at(size_t pos) {
        return m_vec.at(pos);
    }

    const uint16_t& SeedIndices::at(size_t pos) const {
        return m_vec.at(pos);
    }

    void SeedIndices::push_back(uint16_t value) {
        m_vec.push_back(value);
    }

    void SeedIndices::wipe() noexcept {
        if (m_vec.size() > 0) {
            volatile uint16_t* p = const_cast<volatile uint16_t*>(m_vec.data());
            for (size_t i = 0; i < m_vec.size(); ++i) {
                p[i] = 0;  // Zero initialize
            }
        }
    }
}
