#pragma once

#define DS_CONCAT(a, b) a##b

#define DS_ASSIGN_OPERATOR(e, type, name, op)                                                                                                                  \
    inline e& name(e& left, const e right) {                                                                                                                   \
        left = (e)((type)left op(type) right);                                                                                                                 \
        return left;                                                                                                                                           \
    }

#define DS_OPERATOR(e, type, name, op)                                                                                                                         \
    inline e name(e left, const e right) { return (e)((type)left op(type) right); }

#define DS_BITFLAGS(e, type)                                                                                                                                   \
    DS_OPERATOR(e, type, operator&, &);                                                                                                                        \
    DS_ASSIGN_OPERATOR(e, type, operator&=, &);                                                                                                                \
    DS_OPERATOR(e, type, operator|, |);                                                                                                                        \
    DS_ASSIGN_OPERATOR(e, type, operator|=, |);                                                                                                                \
    inline e operator~(const e left) { return (e)(~(type)left); }

namespace ds {

template <typename T>
bool HasAnyFlag(T check, T flags) {
    return (check & flags) != static_cast<T>(0);
}

template <typename T>
bool HasAllFlags(T check, T flags) {
    return (check & flags) == flags;
}

template <typename T>
void SetFlags(T& out, T flags, bool set) {
    if (set) {
        out |= flags;
    } else {
        out &= ~flags;
    }
}

} // namespace ds