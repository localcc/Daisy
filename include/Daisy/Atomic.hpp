#pragma once
#include <atomic>

class AtomicBool {
public:
    AtomicBool(bool val) : value(val) {}

    bool Load(std::memory_order order) const { return value.load(order); }
    void Store(bool val, std::memory_order order) { this->value.store(val, order); }
    bool CompareExchangeStrong(bool& expected, bool desired, std::memory_order order) { return this->value.compare_exchange_strong(expected, desired, order); }

    AtomicBool(const AtomicBool& other) {
        const auto val = other.value.load(std::memory_order_seq_cst);
        this->value = val;
    }
    AtomicBool& operator=(const AtomicBool& other) {
        const auto currentValue = other.value.load(std::memory_order_seq_cst);
        this->value.store(currentValue, std::memory_order_seq_cst);
        return *this;
    }

    AtomicBool(AtomicBool&& other) noexcept {
        const auto currentValue = other.value.load(std::memory_order_seq_cst);
        this->value = currentValue;
    }
    AtomicBool& operator=(AtomicBool&& other) noexcept {
        const auto currentValue = other.value.load(std::memory_order_seq_cst);
        this->value.store(currentValue, std::memory_order_seq_cst);
        return *this;
    }

    ~AtomicBool() = default;

private:
    std::atomic_bool value;
};