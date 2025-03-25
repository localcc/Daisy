#pragma once
#include <Daisy/Assert.hpp>

#include <cstdint>
#include <utility>
#include <vector>

template <typename Obj>
class Handle {
public:
    static constexpr int32_t InvalidIndex = -1;

public:
    Handle() : index(InvalidIndex) {}
    Handle(int32_t index) : index(index) {}

    bool operator==(const Handle& other) const { return index == other.index; }
    bool operator!=(const Handle& other) const { return !(*this == other); }

    [[nodiscard]] bool IsValid() const { return index != InvalidIndex; }
    [[nodiscard]] int32_t Index() const { return index; }

private:
    int32_t index;
};

template <typename T>
class HandleVec {
private:
    struct VecSlot {
        T data = {};
        bool isFree = true;
    };

public:
    template <bool Const = false>
    struct Iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = std::conditional_t<Const, std::pair<Handle<T>, T const*>, std::pair<Handle<T>, T*>>;
        using reference = std::conditional_t<Const, std::pair<Handle<T>, T const&>, std::pair<Handle<T>, T&>>;
        using data_ptr = std::conditional_t<Const, std::vector<VecSlot> const*, std::vector<VecSlot>*>;

        Iterator() = default;
        explicit Iterator(std::vector<VecSlot>* data) : data(data) {}

        reference operator*() const { return {Handle<T>(index), data->at(index).data}; }
        pointer operator->() const { return {Handle<T>(index), &data->at(index)->data}; }

        Iterator& operator++() {
            index++;

            int32_t foundIndex = 0;
            bool found = false;
            const int32_t dataSize = static_cast<int32_t>(data->size());
            for (int32_t i = index; i < dataSize; i++) {
                if (data->at(i).isFree)
                    continue;

                foundIndex = i;
                found = true;
            }

            if (!found) { // convert into end sentinel
                data = nullptr;
                index = 0;
            }

            index = foundIndex;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        constexpr bool operator==(const Iterator& other) const { return data == other.data && index == other.index; }
        constexpr bool operator!=(const Iterator& other) const { return !(*this == other); }

    private:
        data_ptr data = nullptr;
        int32_t index = 0;
    };

public:
    HandleVec() = default;

    Handle<T> Add(T&& element) {
        if (!freeHandles.empty()) {
            Handle<T> handle = freeHandles.back();
            freeHandles.pop_back();

            list[handle.Index()] = {std::move(element), false};
            return handle;
        }

        Handle<T> handle(static_cast<int32_t>(list.size()));
        list.push_back({std::move(element), false});
        return handle;
    }

    T Remove(Handle<T> handle) {
        DS_ASSERT(static_cast<int32_t>(list.size()) > handle.Index(), "Invalid handle");
        freeHandles.push_back(handle);

        T value = std::move(list[handle.Index()].data);
        list[handle.Index()] = std::move(VecSlot{});
        return value;
    }

    [[nodiscard]] bool Contains(Handle<T> handle) {
        if (handle.Index() >= static_cast<int32_t>(list.size()))
            return false;
        return !list[handle.Index()].isFree;
    }
    [[nodiscard]] size_t Size() const { return list.size(); }

    void Clear() {
        freeHandles.clear();
        freeHandles.resize(list.size());
        for (size_t i = 0; i < list.size(); i++) {
            freeHandles[i] = i;
            list[i] = std::move(VecSlot{});
        }
    }

    [[nodiscard]] T& Get(Handle<T> handle) {
        DS_ASSERT(static_cast<int32_t>(list.size()) > handle.Index(), "Invalid handle");
        return list[handle.Index()].data;
    }
    [[nodiscard]] const T& Get(Handle<T> handle) const {
        DS_ASSERT(static_cast<int32_t>(list.size()) > handle.Index(), "Invalid handle");
        return list[handle.Index()].data;
    }

    T& operator[](Handle<T> handle) { return this->Get(handle); }
    const T& operator[](Handle<T> handle) const { return this->Get(handle); }

    Iterator<true> begin() const {
        if (list.empty())
            return Iterator{};
        return Iterator(&list);
    }
    Iterator<true> end() const { return Iterator{}; }

    Iterator<false> begin() {
        if (list.empty())
            return Iterator{};
        return Iterator(&list);
    }
    Iterator<false> end() { return Iterator{}; }

private:
    std::vector<VecSlot> list;
    std::vector<Handle<T>> freeHandles;
};