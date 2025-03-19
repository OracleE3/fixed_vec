//
// Created by cain986 on 8/13/24.
//

#ifndef FIXED_VECTOR_HPP
#define FIXED_VECTOR_HPP

#include <array>
#include <iterator>
#include <stdexcept>
#include <cstddef>
#ifndef FIXED_VECTOR_NOEXCEPT
#include <cstring>
#endif

/**
 * @class fixed_vector
 * @brief Allows functionality like `std::vector<T>` but with no dynamic memory allocation
 *
 * Has fixed compile-time storage like `std::array<T, CAPACITY>` but may logically contain less than `CAPACITY` items
 * @tparam T The data type to store in the fixed vector
 * @tparam CAPACITY The compile-time capacity of the fixed vector
 */
template <typename T, size_t CAPACITY>
class fixed_vector {
    static_assert(CAPACITY > 0, "Capacity cannot be 0");

    /// @brief The logical capacity and actual size of the fixed vector
    const size_t _capacity;
    /// @brief The current logical size of the fixed vector
    size_t _current_size;
    /// @brief The underlying `std::array<T, CAPACITY>` that does the actual storage
    std::array<T, CAPACITY> _buf;

    /**
     * @fn unsafe_right_shift_by_one
     * @brief Blindly shift everything to the right by 1
     *
     * Should only be called after bounds checking has been done
     * @warning DOES NOT BOUNDS CHECK
     */
    void unsafe_right_shift_by_one() {
        ++this->_current_size;
        for (size_t i = this->_current_size; i > 0; --i) this->_buf[i] = this->_buf[i-1];
    }

    /**
     * @fn unsafe_left_shift_by_one
     * @brief Blindly shift everything to the left by 1
     *
     * Should only be called after bounds checking has been done
     * @warning DOES NOT BOUNDS CHECK
     */
    void unsafe_left_shift_by_one() {
        for (size_t i = 1; i < this->_current_size; ++i) this->_buf[i-1] = this->_buf[i];
        --this->_current_size;
    }

public:
    /// @brief Default constructor. Initial size will be 0
    fixed_vector()
        : _capacity(CAPACITY),
          _current_size(0),
          _buf({})
    {}

    /// @brief Copy constructor
    /// @param v The `fixed_vector` to copy. Size will be the same as `v`
    constexpr fixed_vector(const fixed_vector& v)
        : _capacity(v._capacity),
          _current_size(v._current_size),
          _buf(v._buf)
    {}

    /// @brief Move constructor: allows vectors to be moved around conveniently
    /// @param v The `fixed_vector` to move into this object
    constexpr fixed_vector(fixed_vector&& v) noexcept
        : _capacity(v._capacity),
          _current_size(v._current_size),
          _buf(std::move(v._buf))
    {}

    /// @brief Move constructor: allows a `std::array<T, CAPACITY>` to be converted into a `fixed_vector`
    /// @param a The `std::array<T, CAPACITY>` to move into this object
    explicit fixed_vector(std::array<T, CAPACITY> &&a)
        : _capacity(CAPACITY),
          _current_size(CAPACITY),
          _buf(std::move(a))
    {}

    /// @brief Initializer list constructor: allows initialization of `fixed_vector` using curly brace lists
    /// @param init_list An initializer list in curly braces, e.g. {1, 2, 3, 6, 12, ...}
    fixed_vector(std::initializer_list<T> init_list)
        : _capacity(CAPACITY),
          _current_size(init_list.size()),
          _buf({})
    {
        // Compiler should check initializer list size vs capacity for us
        std::copy(init_list.begin(), init_list.end(), _buf.begin());
    }

    /// @brief Get the fixed vector capacity
    [[nodiscard]] size_t capacity() const { return this->_capacity; }

    /// @brief Get the fixed vector current logical size
    [[nodiscard]] size_t size() const { return this->_current_size; }

    /// @brief Clear the fixed vector logical contents. Does not destory any of the contents
    void clear() { this->_current_size = 0; }

    /// @brief Get a pointer to the underlying array
    [[nodiscard]] T* data() { return &(this->_buf[0]); }

    /// @brief Get a const pointer to the underlying array
    [[nodiscard]] const T* cdata() { return &(this->_buf[0]); }

    /// @brief Get a const reference to the underlying `std::array<T, CAPACITY>`
    const std::array<T, CAPACITY>& array() const { return _buf; }

    /// @brief Get a mutable reference to the underlying `std::array<T, CAPACITY>`
    std::array<T, CAPACITY>& array_mut() { return this->_buf; }

    /// @brief Add a value to the end of the fixed vector, increasing the logical size by 1
    /// @param val The value to add
    void push_back(T val) {
        if (this->_current_size == this->_capacity) {
#ifndef FIXED_VECTOR_NOEXCEPT
            const auto fmt = "Cannot push back: vector is at capacity %zu";
            char msg[128] = {};
            std::memset(msg, 0, 128);
            std::snprintf(msg, 128, fmt, this->_capacity);
            throw std::length_error("Cannot push back: vector is at capacity");
#else
            // WARNING: Defining `FIXED_VECTOR_NOEXCEPT` does no bounds checking!!!
            // TODO: Implment some kind of no-exception bounds checking!
            return;
#endif
        }
        this->_buf[this->_current_size] = val;
        ++this->_current_size;
    }

    /// @brief Add a value to the front of the fixed vector, increasing the logical size by 1
    /// @param val The value to add
    /// @warning Shifts every value in the array to the right: could be costly for large arrays
    void push_front(T val) {
        if (this->_current_size == this->_capacity) {
#ifndef FIXED_VECTOR_NOEXCEPT
            const auto fmt = "Cannot push back: vector is at capacity %zu";
            char msg[128] = {};
            std::memset(msg, 0, 128);
            std::snprintf(msg, 128, fmt, this->_capacity);
            throw std::length_error("Cannot push back: vector is at capacity");
#else
            // WARNING: Defining `FIXED_VECTOR_NOEXCEPT` does no bounds checking!!!
            // TODO: Implment some kind of no-exception bounds checking!
            return;
#endif
        }
        // Should be fine since we checked bounds already
        this->unsafe_right_shift_by_one();
        this->_buf[0] = val;
    }

    /// @brief Add a value to the end of the fixed vector, decreasing the logical size by 1
    /// @return The value previously at the back
    [[nodiscard]] T pop_back() {
#ifndef FIXED_VECTOR_NOEXCEPT
        if (this->_current_size == 0) throw std::length_error("Cannot pop back: vector is empty");
#else
        /*
         * WARNING: Defining `FIXED_VECTOR_NOEXCEPT` does not do proper bounds checking!!!
         * If empty, it returns whatever the zeroth element is since CAPACITY is always at least 1
         */
        // TODO: Implment some kind of no-exception bounds checking!
        if (this->_current_size == 0) return this->_buf[0];
#endif
        --this->_current_size;
        return std::move(this->_buf[this->_current_size]);
    }

    /// @brief Remove a value from the front of the fixed vector, decreasing the logical size by 1
    /// @return The value previously at the front
    /// @warning Shifts every value in the array to the left: could be costly for large arrays
    [[nodiscard]] T pop_front() {
#ifndef FIXED_VECTOR_NOEXCEPT
        if (this->_current_size == 0) throw std::length_error("Cannot pop back: vector is empty");
        /*
         * WARNING: Defining `FIXED_VECTOR_NOEXCEPT` does not do proper bounds checking!!!
         * If empty, it returns whatever the zeroth element is since CAPACITY is always at least 1
         */
        // TODO: Implment some kind of no-exception bounds checking!
#endif
        T val = this->_buf[0];
        this->unsafe_left_shift_by_one();
        return std::move(val);
    }

    /// @brief Reverse the fixed vector contents in-place
    void reverse() {
        T temp;
        for (size_t i = 0; i <= this->_current_size / 2; ++i) {
            temp = this->_buf[i];
            this->_buf[i] = this->_buf[this->_current_size - 1 - i];
            this->_buf[this->_current_size - 1 - i] = temp;
        }
    }

    /// @brief Allow square-bracket indexing like a `std::vector`
    T& operator[](size_t pos) {
        if (this->_current_size == 0 || pos > this->_current_size - 1) {
#ifndef FIXED_VECTOR_NOEXCEPT
            const auto fmt = "Index %zu is out of range for current size %zu";
            char msg[128] = {};
            std::memset(msg, 0, 128);
            std::snprintf(msg, 128, fmt, pos, this->_current_size);
            throw std::out_of_range("Index is out of range for current size");
#else
            /*
             * WARNING: Defining `FIXED_VECTOR_NOEXCEPT` does not do proper bounds checking!!!
             * If empty, it returns whatever the zeroth element is since CAPACITY is always at least 1
             */
            // TODO: Implment some kind of no-exception bounds checking!
            return this->_buf[0];
#endif
        }
        if (this->_current_size > this->_capacity) {
#ifndef FIXED_VECTOR_NOEXCEPT
            // This should never happen, but fail loudly if it does
            const auto fmt = "Current size %zu is somehow larger than capacity %zu: something is very wrong!";
            char msg[128] = {};
            std::memset(msg, 0, 128);
            std::snprintf(msg, 128, fmt, this->_current_size, this->_capacity);
            throw std::runtime_error("Current size is somehow larger than capacity: something is very wrong!");
#else
            /*
             * WARNING: Defining `FIXED_VECTOR_NOEXCEPT` does not do proper bounds checking!!!
             * If empty, it returns whatever the zeroth element is since CAPACITY is always at least 1
             */
            // TODO: Implment some kind of no-exception bounds checking!
            return this->_buf[0];
#endif
        }
        return this->_buf[pos];
    }

    /// @brief Allow const square-bracket indexing like a `std::vector`
    const T& operator[](size_t pos) const {
        if (this->_current_size == 0 || pos > this->_current_size - 1) {
#ifndef FIXED_VECTOR_NOEXCEPT
            const auto fmt = "Index %zu is out of range for current size %zu";
            char msg[128] = {};
            std::memset(msg, 0, 128);
            std::snprintf(msg, 128, fmt, pos, this->_current_size);
            throw std::out_of_range("Index is out of range for current size");
#else
            /*
             * WARNING: Defining `FIXED_VECTOR_NOEXCEPT` does not do proper bounds checking!!!
             * If empty, it returns whatever the zeroth element is since CAPACITY is always at least 1
             */
            // TODO: Implment some kind of no-exception bounds checking!
            return this->_buf[0];
#endif
        }
        if (this->_current_size > this->_capacity) {
#ifndef FIXED_VECTOR_NOEXCEPT
            // This should never happen, but fail loudly if it does
            const auto fmt = "Current size %zu is somehow larger than capacity %zu: something is very wrong!";
            char msg[128] = {};
            std::memset(msg, 0, 128);
            std::snprintf(msg, 128, fmt, this->_current_size, this->_capacity);
            throw std::runtime_error("Current size is somehow larger than capacity: something is very wrong!");
#else
            /*
             * WARNING: Defining `FIXED_VECTOR_NOEXCEPT` does not do proper bounds checking!!!
             * If empty, it returns whatever the zeroth element is since CAPACITY is always at least 1
             */
            // TODO: Implment some kind of no-exception bounds checking!
            return this->_buf[0];
#endif
        }
        return this->_buf[pos];
    }

    /// @brief Allow another `fixed_vector` to be copied into this one using the `=` operator
    fixed_vector& operator= (const fixed_vector& v) noexcept {
        this->_current_size = v._current_size;
        for (size_t i = 0; i < this->_buf.size(); ++i) this->_buf[i] = v._buf[i];
        return *this;
    }

    /// @brief Allow another `fixed_vector` to be moved into this one using the `=` operator
    fixed_vector& operator= (fixed_vector&& v) noexcept {
        this->_current_size = v._current_size;
        this->_buf = std::move(v._buf);
        return *this;
    }

    /// @brief Allow two `fixed_vector`s to be compared using `==`
    bool operator== (const fixed_vector& v) noexcept {
        bool equals = false;
        if (this->_current_size == v.size())
            equals = std::equal(this->cbegin(), this->cend(), v.cbegin());
        return equals;
    }

    /// @brief Allow two `fixed_vector`s to be compared using `==`
    bool operator== (const fixed_vector& v) const noexcept {
        bool equals = false;
        if (this->_current_size == v.size())
            equals = std::equal(this->cbegin(), this->cend(), v.cbegin());
        return equals;
    }

    /// @class iterator
    /// @brief Allows `fixed_vector` to be used with C++ standard iterator-based functions
    class iterator {
        T* _ptr;
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        explicit iterator(T* ptr) : _ptr(ptr) {}

        iterator operator++() {
            ++_ptr;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++_ptr;
            return tmp;
        }

        T& operator*() {
            return *_ptr;
        }

        T* operator->() {
            return _ptr;
        }

        bool operator==(const iterator& other) const {
            return _ptr == other._ptr;
        }

        bool operator!=(const iterator& other) const {
            return _ptr != other._ptr;
        }

        friend iterator operator+(difference_type n, const iterator& it) { return it + n; }

        friend iterator operator+(const iterator& it, difference_type n) {
            iterator temp = it;
            temp += n;
            return temp;
        }

        friend iterator operator-(difference_type n, const iterator& it) {
            iterator temp = it;
            temp -= n;
            return temp;
        }

        friend difference_type operator-(const iterator& lhs, const iterator& rhs) { return lhs._ptr - rhs._ptr; }
    };

    /// @class const_iterator
    /// @brief Allows `fixed_vector` to be used with C++ standard const-iterator-based functions
    class const_iterator {
        const T* _ptr;
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        explicit const_iterator(const T* ptr) : _ptr(ptr) {}

        const_iterator operator++() {
            ++_ptr;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++_ptr;
            return tmp;
        }

        const T& operator*() {
            return *_ptr;
        }

        const T* operator->() {
            return _ptr;
        }

        bool operator==(const const_iterator& other) const {
            return _ptr == other._ptr;
        }

        bool operator!=(const const_iterator& other) const {
            return _ptr != other._ptr;
        }

        friend const_iterator operator+(difference_type n, const const_iterator& it) { return it + n; }

        friend const_iterator operator+(const const_iterator& it, difference_type n) {
            const_iterator temp = it;
            temp += n;
            return temp;
        }

        friend const_iterator operator-(difference_type n, const const_iterator& it) {
            const_iterator temp = it;
            temp -= n;
            return temp;
        }

        friend difference_type operator-(const const_iterator& lhs, const const_iterator& rhs) { return lhs._ptr - rhs._ptr; }
    };

    /// @brief Get mutable iterator to beginning
    iterator begin() {
        return iterator(_buf.data());
    }

    /// @brief Get mutable iterator to end
    iterator end() {
        return iterator(_buf.data() + _current_size);
    }

    /// @brief Get const iterator to beginning
    const_iterator cbegin() const {
        return const_iterator(_buf.data());
    }

    /// @brief Get const iterator to end
    const_iterator cend() const {
        return const_iterator(_buf.data() + _current_size);
    }

    /// @brief Overload for getting const iterator to beginning
    const_iterator begin() const {
        return cbegin();
    }

    /// @brief Overload for getting const iterator to end
    const_iterator end() const {
        return cend();
    }
};

#endif //FIXED_VECTOR_HPP
