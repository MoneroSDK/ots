#pragma once

#include "ots.hpp"
#include "ots-exceptions.hpp"
#include <vector>

namespace ots {
    template<typename T>
    WipeableVector<T>& WipeableVector<T>::operator=(const WipeableVector& other) {
        if (this != &other) {
            wipe();
            m_vec = other.m_vec;
        }
        return *this;
    }

    template<typename T>
    WipeableVector<T>& WipeableVector<T>::operator=(WipeableVector&& other) noexcept {
        if (this != &other) {
            wipe();
            m_vec = std::move(other.m_vec);
        }
        return *this;
    }

    template<typename T>
    WipeableVector<T>::~WipeableVector() {
        wipe();
    }

    template<typename T>
    WipeableVector<T>::operator std::vector<T>() const {
        throw ots::exception::wipeablevector::UnsafeConversion();
    }

    template<typename T>
    std::vector<T> WipeableVector<T>::insecure() const noexcept {
        return m_vec;
    }

    template<typename T>
    const T* WipeableVector<T>::data() const noexcept {
        return m_vec.data();
    }

    template<typename T>
    T* WipeableVector<T>::data() noexcept {
        return m_vec.data();
    }

    template<typename T>
    size_t WipeableVector<T>::size() const noexcept {
        return m_vec.size();
    }

    template<typename T>
    bool WipeableVector<T>::empty() const noexcept {
        return m_vec.empty();
    }

    template<typename T>
    void WipeableVector<T>::clear() noexcept {
        wipe();
        m_vec.clear();
    }

    template<typename T>
    void WipeableVector<T>::reserve(size_t n) {
        m_vec.reserve(n);
    }

    template<typename T>
    void WipeableVector<T>::resize(size_t n) {
        m_vec.resize(n);
    }

    template<typename T>
    T& WipeableVector<T>::operator[](size_t pos) {
        return m_vec[pos];
    }

    template<typename T>
    const T& WipeableVector<T>::operator[](size_t pos) const {
        return m_vec[pos];
    }

    template<typename T>
    T& WipeableVector<T>::at(size_t pos) {
        return m_vec.at(pos);
    }

    template<typename T>
    const T& WipeableVector<T>::at(size_t pos) const {
        return m_vec.at(pos);
    }

    template<typename T>
    void WipeableVector<T>::push_back(const T& value) {
        m_vec.push_back(value);
    }

    template<typename T>
    void WipeableVector<T>::push_back(T&& value) {
        m_vec.push_back(std::move(value));
    }

    template<typename T>
    void WipeableVector<T>::wipe() noexcept {
        if (m_vec.size() > 0) {
            volatile T* p = const_cast<volatile T*>(m_vec.data());
            for (size_t i = 0; i < m_vec.size(); ++i) {
                p[i] = T{};  // Zero initialize
            }
        }
    }
}
