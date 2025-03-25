#pragma once
#include <cstdint>

namespace ds {

struct Result {
    enum { OK, INVALID_PARAMETER, NOTIFICATION_REGISTER, DEVICE_ENUMERATION, CONTROLLER_NOT_FOUND, USB_COMMUNICATION, UNKNOWN_INPUT_REPORT, TIMEOUT } code;
    uint32_t additionalInfo;

    Result(decltype(code) c) : code(c) {}
    Result(decltype(code) c, uint32_t additionalInfo) : code(c), additionalInfo(additionalInfo) {}

    bool operator==(decltype(code) cmp) const { return this->code == cmp; }
    bool operator!=(decltype(code) cmp) const { return this->code != cmp; }
};

} // namespace ds