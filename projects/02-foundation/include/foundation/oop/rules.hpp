#pragma once
#include <cstddef>
#include <cstring>
#include <utility>
#include <vector>
#include <string>
#include <memory>

namespace foundation {

// ---- Rule of Five ----
// Buffer owns a heap allocation. All five special members defined explicitly.
class Buffer {
    std::byte* data_{nullptr};
    std::size_t size_{0};

public:
    Buffer() = default;

    explicit Buffer(std::size_t size)
        : data_{size ? new std::byte[size]() : nullptr}, size_{size} {}

    ~Buffer() { delete[] data_; }

    // Copy constructor — deep copy
    Buffer(const Buffer& other)
        : data_{other.size_ ? new std::byte[other.size_] : nullptr}
        , size_{other.size_}
    { if (data_) std::memcpy(data_, other.data_, size_); }

    // Copy assignment — copy-and-swap
    Buffer& operator=(Buffer other) noexcept { swap(other); return *this; }

    // Move constructor — O(1) ownership transfer
    Buffer(Buffer&& other) noexcept : data_{other.data_}, size_{other.size_} {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    void swap(Buffer& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
    }

    std::byte*  data() const noexcept { return data_; }
    std::size_t size() const noexcept { return size_; }
};

// ---- Rule of Zero ----
// Uses RAII types exclusively. No explicit special members needed.
struct RuleOfZeroExample {
    std::vector<int>       data;   // handles copy/move
    std::unique_ptr<int>   ptr;    // move-only
    std::string            name;   // handles copy/move
};

} // namespace foundation
