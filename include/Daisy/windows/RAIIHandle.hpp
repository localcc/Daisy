#pragma once
#include <Daisy/windows/WindowsFwd.hpp>

#include <functional>
#include <utility>

namespace ds {

struct EndFunc {
    EndFunc(std::function<void()>&& call) : call(call) {}
    ~EndFunc() { call(); }

private:
    std::function<void()> call;
};

/// @brief Emit a variable that will call a function at the end of the scope
inline EndFunc raii(std::function<void()>&& call) { return EndFunc(std::move(call)); }

template <typename T, typename Traits>
struct RAIIHandle {
public:
    RAIIHandle() = default;
    RAIIHandle(T handle) : handle(handle) {}

    operator T() { return handle; }

    RAIIHandle(const RAIIHandle& other) = delete;
    RAIIHandle& operator=(const RAIIHandle& other) = delete;

    RAIIHandle(RAIIHandle&& other) noexcept : handle(std::exchange(other.handle, Traits::INVALID)) {}
    RAIIHandle& operator=(RAIIHandle&& other) noexcept {
        std::swap(this->handle, other.handle);
        return *this;
    }

    ~RAIIHandle() { Traits{}(handle); }

public:
    T handle = Traits::INVALID;
};

struct CloseWin {
    static constexpr wt::HANDLE INVALID = nullptr;
    void operator()(wt::HANDLE& handle) const;
};
using WinHandle = RAIIHandle<wt::HANDLE, CloseWin>;

struct CloseNotification {
    static constexpr wt::HCMNOTIFICATION INVALID = nullptr;
    void operator()(wt::HCMNOTIFICATION& handle) const;
};
using NotificationHandle = RAIIHandle<wt::HCMNOTIFICATION, CloseNotification>;

struct CloseDevInfo {
    static constexpr wt::HDEVINFO INVALID = nullptr;
    void operator()(wt::HDEVINFO& handle) const;
};
using DevInfoHandle = RAIIHandle<wt::HDEVINFO, CloseDevInfo>;

} // namespace ds