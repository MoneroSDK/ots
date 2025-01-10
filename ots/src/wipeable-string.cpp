#include "ots.hpp"
#include "ots-exceptions.hpp"

namespace ots {
    WipeableString::WipeableString(const WipeableString& other) 
        : std::string(other) {
    }

    WipeableString::WipeableString(WipeableString&& other) noexcept 
        : std::string(std::move(other)) {
    }

    WipeableString& WipeableString::operator=(const WipeableString& other) {
        wipe();
        std::string::operator=(other);
        return *this;
    }

    WipeableString& WipeableString::operator=(WipeableString&& other) noexcept {
        wipe();
        std::string::operator=(std::move(other));
        return *this;
    }

    WipeableString::~WipeableString() {
        wipe();
    }

    WipeableString::operator std::string() const {
        throw ots::exception::wipeablestring::UnsafeConversion();
    }

    std::string WipeableString::insecure() const noexcept {
        return std::string(*this);
    }

    void WipeableString::wipe() noexcept {
        if (size() > 0) {
            volatile char* p = const_cast<volatile char*>(data());
            for (size_t i = 0; i < size(); ++i) {
                p[i] = 0;
            }
        }
    }
}
