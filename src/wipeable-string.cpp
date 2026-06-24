#include "ots.hpp"
#include "ots-exceptions.hpp"

namespace ots {
    WipeableString::WipeableString(const char* s)
        : m_str(s) {
    }

    WipeableString::WipeableString(const char* s, size_t n)
        : m_str(s, n) {
    }

    WipeableString::WipeableString(const WipeableString& other)
        : m_str(other.m_str) {
    }

    WipeableString::WipeableString(WipeableString&& other) noexcept
        : m_str(std::move(other.m_str)) {
    }

    WipeableString& WipeableString::operator=(const WipeableString& other) {
        if (this != &other) {
            wipe();
            m_str = other.m_str;
        }
        return *this;
    }

    WipeableString& WipeableString::operator=(WipeableString&& other) noexcept {
        if (this != &other) {
            wipe();
            m_str = std::move(other.m_str);
        }
        return *this;
    }

    WipeableString& WipeableString::operator=(const char* s) {
        wipe();
        m_str = s;
        return *this;
    }

    WipeableString::operator std::string() const {
        throw ots::exception::wipeablestring::UnsafeConversion();
    }

    WipeableString::~WipeableString() {
        wipe();
    }

    std::string WipeableString::insecure() const noexcept {
        return m_str;
    }

    const char* WipeableString::c_str() const noexcept {
        return m_str.c_str();
    }

    const char* WipeableString::data() const noexcept {
        return m_str.data();
    }

    size_t WipeableString::size() const noexcept {
        return m_str.size();
    }

    bool WipeableString::empty() const noexcept {
        return m_str.empty();
    }

    void WipeableString::clear() noexcept {
        wipe();
        m_str.clear();
    }

    size_t WipeableString::capacity() const noexcept {
        return m_str.capacity();
    }

    void WipeableString::reserve(size_t n) {
        m_str.reserve(n);
    }

    WipeableString WipeableString::substr(size_t pos, size_t len) const {
        return WipeableString(m_str.substr(pos, len).c_str());
    }

    size_t WipeableString::find(const WipeableString& str, size_t pos) const noexcept {
        return m_str.find(str.m_str, pos);
    }

    size_t WipeableString::find(const char* s, size_t pos) const noexcept {
        return m_str.find(s, pos);
    }

    WipeableString& WipeableString::append(const WipeableString& str) {
        m_str.append(str.m_str);
        return *this;
    }

    WipeableString& WipeableString::append(const char* s) {
        m_str.append(s);
        return *this;
    }

    WipeableString& WipeableString::operator+=(const WipeableString& str) {
        return append(str);
    }

    WipeableString& WipeableString::operator+=(const char* s) {
        return append(s);
    }

    bool WipeableString::operator==(const WipeableString& other) const noexcept {
        return m_str == other.m_str;
    }

    bool WipeableString::operator!=(const WipeableString& other) const noexcept {
        return m_str != other.m_str;
    }

    int WipeableString::compare(const WipeableString& other) const noexcept {
        return m_str.compare(other.m_str);
    }

    std::ostream& operator<<(std::ostream& os, const WipeableString& str) {
        return os << str.m_str;
    }

    void WipeableString::wipe() noexcept {
        if (m_str.size() > 0) {
            volatile char* p = const_cast<volatile char*>(m_str.data());
            for (size_t i = 0; i < m_str.size(); ++i) {
                p[i] = 0;
            }
        }
    }
}
