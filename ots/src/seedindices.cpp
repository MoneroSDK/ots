#include "ots.hpp"
#include "ots-internal.hpp"
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>

namespace ots {

    SeedIndices& SeedIndices::operator=(const SeedIndices& other) {
        if(this != &other) {
            wipe();
            m_vec = other.m_vec;
        }
        return *this;
    }

    SeedIndices& SeedIndices::operator=(SeedIndices&& other) noexcept {
        if(this != &other) {
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
        if(m_vec.size() > 0) {
            volatile uint16_t* p = const_cast<volatile uint16_t*>(m_vec.data());
            for(size_t i = 0; i < m_vec.size(); i++)
                p[i] = 0;  // Zero initialize
        }
    }

    const std::string SeedIndices::numeric(const std::string& separator) const noexcept {
        std::ostringstream strStream;
        for(size_t i = 0; i < m_vec.size(); i++) {
            strStream << std::setw(4) << std::setfill('0') << m_vec[i];
            if(!separator.empty() && i < m_vec.size() - 1)
                strStream << separator;
        }
        return strStream.str();
    }

    const std::string SeedIndices::hex(const std::string& separator) const noexcept {
        std::ostringstream strStream;
        for(size_t i = 0; i < m_vec.size(); i++) {
            strStream << std::hex << std::setw(4) << std::setfill('0') << m_vec[i];
            if(!separator.empty() && i < m_vec.size() - 1)
                strStream << separator;
        }
        return std::move(strStream.str());
    }

    SeedIndices::operator const std::string() const noexcept {
        return numeric();
    }

    SeedIndices::operator const uint8_t*() const noexcept {
        return reinterpret_cast<const uint8_t*>(m_vec.data());
    }

    SeedIndices::operator const char*() const noexcept {
        return reinterpret_cast<const char*>(m_vec.data());
    }

    bool SeedIndices::operator==(const SeedIndices& other) const noexcept {
        return m_vec == other.m_vec;
    }

    bool SeedIndices::operator==(const std::vector<uint16_t>& other) const noexcept {
        return m_vec == other;
    }

    bool SeedIndices::operator!=(const SeedIndices& other) const noexcept {
        return m_vec != other.m_vec;
    }

    bool SeedIndices::operator!=(const std::vector<uint16_t>& other) const noexcept {
        return m_vec != other;
    }

    SeedIndices SeedIndices::fromHex(const std::string& hex, const std::string& separator) {
        SeedIndices indices;
        std::vector<std::string> hexParts = ots::splitString(hex, separator);
        for(const auto& part : hexParts)
            if(!part.empty())
                indices.push_back(std::stoi(part, nullptr, 16));
        return indices;
    }

    SeedIndices SeedIndices::fromNumeric(const std::string& numeric, const std::string& separator) {
        SeedIndices indices;
        std::vector<std::string> numParts = ots::splitString(numeric, separator);
        for(const auto& part : numParts)
            if(!part.empty())
                indices.push_back(std::stoi(part));
        return indices;
    }
}
