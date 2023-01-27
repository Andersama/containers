#pragma once
#include <cassert>
#include <iterator>
#include <type_traits>

#if defined(_MSC_VER)
#define MUST_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define MUST_INLINE __attribute__((always_inline))
#else
#define MUST_INLINE
#endif

#if defined(_MSVC_LANG)
#define __cplusplus_version _MSVC_LANG
#elif defined(__cplusplus) // ^^^ use _MSVC_LANG / use __cplusplus vvv
#define __cplusplus_version __cplusplus
#else // ^^^ use __cplusplus / no C++ support vvv
#define __cplusplus_version 0L
#endif // ^^^ no C++ support ^^^

#if __cplusplus_version >= 201703L && __cpp_lib_array_constexpr >= 201811L
#include <array>
#endif

/*
The MIT License (MIT)

Copyright (c) 2020 Alex Anderson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

namespace containers {
    template <typename Ty, size_t N> struct plain_array {
      public:
        using element_type           = Ty;
        using value_type             = typename ::std::remove_cv<Ty>::type;
        using const_reference        = const value_type &;
        using size_type              = ::std::size_t;
        using difference_type        = ::std::ptrdiff_t;
        using pointer                = element_type *;
        using const_pointer          = const value_type *;
        using reference              = value_type &;
        using iterator               = pointer;
        using const_iterator         = const_pointer;
        using reverse_iterator       = ::std::reverse_iterator<iterator>;
        using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

        struct storage {
            value_type                  _values[N]{};
            constexpr const value_type *data() const {
                return _values;
            }
            constexpr value_type *data() {
                return _values;
            }
        };

        struct empty_storage {
            constexpr const value_type *data() const {
                return nullptr;
            }
            constexpr value_type *data() {
                return nullptr;
            }
        };

        using data_t = typename ::std::conditional<(N > 0), storage, empty_storage>::type;

        static constexpr MUST_INLINE void reverse(iterator first, iterator last) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            ::std::reverse(first, last); // cross fingers this should be optimized
#else
            for (; first != last && first != --last; ++first) {
                value_type tmp = *first;
                *first         = *last;
                *last          = tmp;
            }
#endif
        }

        static constexpr MUST_INLINE void rotate(iterator first, iterator mid, iterator last) {
            if constexpr (false) {
                const size_t p     = mid - first;
                const size_t n     = last - first;
                size_t       m     = 0;
                size_t       count = 0;
                for (m = 0, count = 0; count != n; m++) {
                    value_type t = *(first + m);
                    size_t     i = m;
                    size_t     j = m + p;
                    for (; j != m; i = j, j = ((j + p) < n) ? (j + p) : (j + p - n), count++) {
                        *(first + i) = *(first + j);
                    }
                    *(first + i) = t;
                    count++;
                }
            } else {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
                ::std::rotate(first, mid, last); // cross fingers this should be optimized
#else
                reverse(first, mid);
                reverse(mid, last);
                reverse(first, last);
#endif
            }
        }

        static constexpr MUST_INLINE iterator copy(iterator first, iterator last, iterator d_first) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            return ::std::copy(first, last, d_first); // cross fingers this should be optimized
#else
            while (first != last) { // fallback
                *d_first++ = *first++;
            }
            return d_first;
#endif
        }

        static constexpr MUST_INLINE iterator copy_backward(iterator first, iterator last, iterator d_last) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            return ::std::copy_backward(first, last, d_last); // cross fingers this should be optimized
#else
            // fallback
            while (first != last) {
                *(--d_last) = *(--last);
            }
            return d_last;
#endif
        }

        static constexpr MUST_INLINE void swap(value_type &left, value_type &right) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            ::std::swap(left, right); // cross fingers this should be optimized
#else
            // fallback
            value_type tmp = left;
            left           = right;
            right          = tmp;
#endif
        }

        static constexpr MUST_INLINE void swap(value_type (&left)[N], value_type (&right)[N]) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            ::std::swap(left, right); // cross fingers this should be optimized
#else
            if (&left != &right) {
                value_type *first1 = left;
                value_type *last1  = first1 + N;
                value_type *first2 = right;
                for (; first1 != last1; ++first1, ++first2) {
                    swap(*first1, *first2);
                }
            }
#endif
        }

        static constexpr MUST_INLINE void fill(iterator first, iterator last, const value_type &value) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            ::std::fill(first, last, value); // cross fingers this should be optimized
#else
            for (; first != last; ++first) {
                *first = value;
            }
#endif
        }

      private:
        data_t _data;
        size_t _size{0};

      public:
        constexpr plain_array() noexcept : _size{0} {
        }
        constexpr plain_array(size_type count, const value_type &value) {
            assign(count, value);
        }
        constexpr explicit plain_array(size_type count) {
            assign(count);
        }
        template <typename It, typename It2> constexpr plain_array(It first, It last) {
            assign(first, last);
        }
        constexpr plain_array(const plain_array &other) {
            assign(other.begin(), other.end());
        }
        constexpr plain_array(plain_array &&other) noexcept {
            assign(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
        }
        constexpr plain_array(::std::initializer_list<value_type> init) {
            assign(init.begin(), init.end());
        }

#if __cpp_lib_array_constexpr >= 201811L
        template <size_t N0> constexpr plain_array(const std::array<value_type, N0> &arr) noexcept {
            assign(arr.begin(), arr.end());
        }
#endif

        constexpr void assign(size_type count, const value_type &value) {
            size_t       i  = 0;
            const size_t mx = count >= N ? N : count;
            _size           = mx;
            for (; i < mx; i++)
                data()[i] = value;
            for (; i < count; i++)
                data()[i] = value_type();
        }

        constexpr void assign(size_type count) {
            size_t       i  = 0;
            const size_t mx = count >= N ? N : count;
            _size           = mx;
            for (; i < N; i++)
                data()[i] = value_type();
        }

        template <typename It, typename It2> constexpr void assign(It first, It2 last) {
            _size = 0;
            for (; _size < N && first != last; ++first) {
                data()[_size] = *first;
                _size++;
            }
        }

        //[]'s
        [[nodiscard]] constexpr reference operator[](size_type pos) {
            assert(pos < _size);
            return data()[pos];
        };

        [[nodiscard]] constexpr const_reference operator[](size_type pos) const {
            assert(pos < _size);
            return data()[pos];
        };
        // front
        [[nodiscard]] constexpr reference front() {
            assert(_size > 0);
            return data()[0];
        };
        [[nodiscard]] constexpr const_reference front() const {
            assert(_size > 0);
            return data()[0];
        };
        // back's
        [[nodiscard]] constexpr reference back() {
            assert(_size > 0);
            return data()[_size - 1];
        };
        [[nodiscard]] constexpr const_reference back() const {
            assert(_size > 0);
            return data()[_size - 1];
        };

        constexpr plain_array &operator=(const plain_array &other) {
            if (this == &other)
                return *this;
            plain_array(other.begin(), other.end());
            return *this;
        }

        constexpr plain_array &operator=(plain_array &&other) noexcept {
            if (this == &other)
                return *this;
            plain_array(std::forward<plain_array &&>(other));
            return *this;
        }

        // data's
        [[nodiscard]] constexpr value_type *data() noexcept {
            return _data.data();
        };
        [[nodiscard]] constexpr const value_type *data() const noexcept {
            return _data.data();
        };
        // begin's
        [[nodiscard]] constexpr iterator begin() noexcept {
            return data();
        };
        [[nodiscard]] constexpr const_iterator begin() const noexcept {
            return data();
        };
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
            return data();
        };
        // rbegin's
        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        };
        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        };
        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator(end());
        };
        // end's
        [[nodiscard]] constexpr iterator end() noexcept {
            return data() + _size;
        };
        [[nodiscard]] constexpr const_iterator end() const noexcept {
            return data() + _size;
        };
        [[nodiscard]] constexpr const_iterator cend() const noexcept {
            return data() + _size;
        };
        // rend's
        [[nodiscard]] constexpr reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        };
        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        };
        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator(begin());
        };

        template <typename... Args> constexpr reference emplace_back(Args &&...args) {
            if (_size < N) {
                size_t idx  = _size;
                data()[idx] = value_type(std::forward<Args>(args)...);
                _size++;
                return data()[idx];
            } else {
                return back();
            }
        }

        // writes data to last index without checking on the size, quick but unsafe
        template <typename... Args> constexpr reference unchecked_emplace_back(Args &&...args) {
            size_t idx  = _size;
            data()[idx] = value_type(std::forward<Args>(args)...);
            _size++;
            return data()[idx];
        }

        // push_back's
        constexpr void push_back(const value_type &value) {
            emplace_back(::std::forward<const value_type &>(value));
        }

        constexpr void push_back(value_type &&value) {
            emplace_back(::std::forward<value_type &&>(value));
        };

        constexpr void unchecked_push_back(const value_type &value) {
            unchecked_emplace_back(::std::forward<const value_type &>(value));
        }

        constexpr void unchecked_push_back(value_type &&value) {
            unchecked_emplace_back(::std::forward<value_type &&>(value));
        }

        // pop_back's
        constexpr void pop_back() {
            if (_size) {
                _size--;
            }
        }
        constexpr void unchecked_pop_back() {
            _size--;
        }

        // clear
        constexpr void clear() {
            _size = 0;
        }

        // empty
        constexpr bool empty() {
            return _size == 0;
        }
        constexpr bool empty() const {
            return _size == 0;
        }

        // capacity
        constexpr size_t capacity() {
            return N;
        }
        constexpr size_t capacity() const {
            return N;
        }
        // max_size
        constexpr size_t max_size() {
            return N;
        }
        constexpr size_t max_size() const {
            return N;
        }
        // size
        constexpr size_t size() {
            return _size;
        }
        constexpr size_t size() const {
            return _size;
        }

        // pop_front's (non-standard)
        constexpr void pop_front() {
            if (_size) {
                copy(begin() + 1, end(), begin());
                _size--;
            }
        }
        constexpr void unchecked_pop_front() {
            copy(begin() + 1, end(), begin());
            _size--;
        }

        constexpr iterator erase(const_iterator pos) {
            if (_size) {
                size_t erase_idx = pos - cbegin();
                if (erase_idx < _size) {
                    copy(begin() + erase_idx + 1, end(), begin() + erase_idx);
                    _size--;
                    return begin() + erase_idx;
                }
                erase_idx = _size;
                return begin() + erase_idx;
            } else {
                return end();
            }
        }

        constexpr iterator erase(const_iterator first, const_iterator last) {
            if (first == last) {
                size_t erase_idx = last - cbegin();
                return begin() + erase_idx;
            }
            if (_size) {
                size_t erase_idx = first - cbegin();
                size_t last_idx  = last - cbegin();
                if (erase_idx < last_idx && erase_idx < _size && last_idx <= _size) {
                    iterator f = begin() + erase_idx;
                    iterator l = begin() + last_idx;
                    // a b c d - - - h i j k _ _ _ _
                    // a b c d h i j k _ _ _ _ _ _ _
                    iterator c = copy(l, end(), f);
                    _size      = c - begin();
                    return begin() + erase_idx;
                }
                erase_idx = _size;
                return begin() + erase_idx;
            } else {
                return end();
            }
        }

        // emplace
        template <typename... Args> constexpr iterator emplace(const_iterator pos, Args &&...args) {
            if (_size < N) {
                size_t insert_idx = pos - cbegin();
                // clamp insertion point
                insert_idx   = insert_idx <= N ? insert_idx : N;
                iterator ret = begin() + insert_idx;
                if (insert_idx == _size) {
                    unchecked_emplace_back(std::forward<Args>(args)...);
                    return ret;
                }
                // move backwards
                copy_backward(begin() + insert_idx, end(), end() + 1);
                // construct* inplace
                data()[insert_idx] = value_type(std::forward<Args>(args)...);
                _size += 1;
                return ret;
            } else {
                return end();
            }
        };

        // insert
        constexpr iterator insert(const_iterator pos, const value_type &value) {
            return emplace(pos, value);
        };
        constexpr iterator insert(const_iterator pos, value_type &&value) {
            return emplace(pos, ::std::move(value));
        };
        constexpr iterator insert(const_iterator pos, size_type count, const value_type &value) {
            return insert_backwards(pos, count, value);
        }

        constexpr MUST_INLINE iterator insert_backwards(const_iterator pos, size_type count,
                                                        const value_type &value) {
            if (_size < N) { //
                size_t insert_idx = pos - cbegin();
                // clamp insertion point
                insert_idx   = insert_idx <= N ? insert_idx : N;
                iterator ret = begin() + insert_idx;
                if (insert_idx == _size) {
                    size_t remaining    = N - _size;
                    size_t insert_count = count <= remaining ? count : remaining;

                    fill(end(), end() + insert_count, value);
                    _size += insert_count;
                    return ret;
                }
                size_t remaining    = N - _size;
                size_t insert_count = count <= remaining ? count : remaining;
                copy_backward(begin() + insert_idx, begin() + _size, begin() + (insert_count + _size));
                fill(begin() + insert_idx, begin() + (insert_idx + insert_count), value);
                _size += insert_count;
                return ret;
            } else {
                return end();
            }
        }

        constexpr MUST_INLINE iterator insert_rotate(const_iterator pos, size_type count,
                                                     const value_type &value) {
            if (_size < N) { //
                size_t insert_idx = pos - cbegin();
                // clamp insertion point
                insert_idx   = insert_idx <= N ? insert_idx : N;
                iterator ret = begin() + insert_idx;
                if (insert_idx == _size) {
                    size_t remaining    = N - _size;
                    size_t insert_count = count <= remaining ? count : remaining;

                    fill(end(), end() + insert_count, value);
                    _size += insert_count;
                    return ret;
                }
                size_t remaining    = N - _size;
                size_t insert_count = count <= remaining ? count : remaining;

                size_t mid_idx = _size;
                fill(end(), end() + insert_count, value);
                _size += insert_count;
                rotate(begin() + insert_idx, begin() + mid_idx, begin() + _size);

                return ret;
            } else {
                return end();
            }
        }

        template <typename It, typename It2,
                  typename std::enable_if<!std::is_convertible<It, size_type>::value>::type>
        constexpr iterator insert(const_iterator pos, It first, It2 last) {
            if (_size < N) { //
                size_t insert_idx = pos - cbegin();
                // clamp insertion point
                insert_idx   = insert_idx <= N ? insert_idx : N;
                iterator ret = begin() + insert_idx;
                if (insert_idx == _size) {
                    // no need to rotate
                    for (; _size < N && first != last; ++first)
                        unchecked_emplace_back(*first);
                    return ret;
                }
                size_t mid_insert = _size;
                for (; _size < N && first != last; ++first)
                    unchecked_emplace_back(*first);
                // size_t last_insert = _size;
                // rotate algorithm
                rotate(begin() + insert_idx, begin() + mid_insert, begin() + _size);
                return ret;
            } else {
                return end();
            }
        }

        // append (non-standard)
        template <typename It, typename It2,
                  typename std::enable_if<
                      !std::is_same<typename std::iterator_traits<It>::value_type, void>::value>::type>
        constexpr iterator append(It first, It2 last) {
            if (_size < N) {
                const size_t idx = _size;
                for (; _size < N && first != last; ++first)
                    unchecked_emplace_back(*first);
                return begin() + idx;
            } else {
                return end();
            }
        }
        constexpr iterator append(size_t count, const value_type &value) {
            if (_size < N) {
                const size_t idx = _size;
                for (size_t i = 0; _size < N && i < count; i++)
                    unchecked_emplace_back(value);
                return begin() + idx;
            } else {
                return end();
            }
        }
        constexpr iterator append(size_t count) {
            if (_size < N) {
                const size_t idx = _size;
                for (size_t i = 0; _size < N && i < count; i++)
                    unchecked_emplace_back();
                return begin() + idx;
            } else {
                return end();
            }
        }

        constexpr void swap(plain_array &other) noexcept(true) {
            if constexpr (N > 0) {
                swap(_data._values, other._data._values);
                size_t tmp  = _size;
                _size       = other._size;
                other._size = tmp;
            }
        }
    };

    template <typename Ty, size_t N> struct plain_array_safe {
      public:
        using element_type           = Ty;
        using value_type             = typename ::std::remove_cv<Ty>::type;
        using const_reference        = const value_type &;
        using size_type              = ::std::size_t;
        using difference_type        = ::std::ptrdiff_t;
        using pointer                = element_type *;
        using const_pointer          = const value_type *;
        using reference              = value_type &;
        using iterator               = pointer;
        using const_iterator         = const_pointer;
        using reverse_iterator       = ::std::reverse_iterator<iterator>;
        using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

        struct storage {
            value_type                  _values[N + 1]{};
            constexpr const value_type *data() const {
                return _values;
            }
            constexpr value_type *data() {
                return _values;
            }
        };

        using data_t = storage;

        static constexpr MUST_INLINE void reverse(iterator first, iterator last) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            ::std::reverse(first, last); // cross fingers this should be optimized
#else
            for (; first != last && first != --last; ++first) {
                value_type tmp = *first;
                *first         = *last;
                *last          = tmp;
            }
#endif
        }

        static constexpr MUST_INLINE void rotate(iterator first, iterator mid, iterator last) {
            if constexpr (false) {
                const size_t p     = mid - first;
                const size_t n     = last - first;
                size_t       m     = 0;
                size_t       count = 0;
                for (m = 0, count = 0; count != n; m++) {
                    value_type t = *(first + m);
                    size_t     i = m;
                    size_t     j = m + p;
                    for (; j != m; i = j, j = ((j + p) < n) ? (j + p) : (j + p - n), count++) {
                        *(first + i) = *(first + j);
                    }
                    *(first + i) = t;
                    count++;
                }
            } else {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
                ::std::rotate(first, mid, last); // cross fingers this should be optimized
#else
                reverse(first, mid);
                reverse(mid, last);
                reverse(first, last);
#endif
            }
        }

        static constexpr MUST_INLINE iterator copy(iterator first, iterator last, iterator d_first) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            return ::std::copy(first, last, d_first); // cross fingers this should be optimized
#else
            while (first != last) { // fallback
                *d_first++ = *first++;
            }
            return d_first;
#endif
        }

        static constexpr MUST_INLINE iterator copy_backward(iterator first, iterator last, iterator d_last) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            return ::std::copy_backward(first, last, d_last); // cross fingers this should be optimized
#else
            // fallback
            while (first != last) {
                *(--d_last) = *(--last);
            }
            return d_last;
#endif
        }

        static constexpr MUST_INLINE void swap(value_type &left, value_type &right) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            ::std::swap(left, right); // cross fingers this should be optimized
#else
            // fallback
            value_type tmp = left;
            left           = right;
            right          = tmp;
#endif
        }

        static constexpr MUST_INLINE void swap(value_type (&left)[N], value_type (&right)[N]) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            ::std::swap(left, right); // cross fingers this should be optimized
#else
            if (&left != &right) {
                value_type *first1 = left;
                value_type *last1  = first1 + N;
                value_type *first2 = right;
                for (; first1 != last1; ++first1, ++first2) {
                    swap(*first1, *first2);
                }
            }
#endif
        }

        static constexpr MUST_INLINE void fill(iterator first, iterator last, const value_type &value) {
#if __cplusplus_version > 201703L && __cpp_lib_constexpr_algorithms >= 201806L
            ::std::fill(first, last, value); // cross fingers this should be optimized
#else
            for (; first != last; ++first) {
                *first = value;
            }
#endif
        }

      private:
        data_t _data;
        size_t _size{0};
        bool   _overrun = false;

      public:
        constexpr plain_array_safe() noexcept : _size{0} {
        }
        constexpr plain_array_safe(size_type count, const value_type &value) {
            assign(count, value);
        }
        constexpr explicit plain_array_safe(size_type count) {
            assign(count);
        }
        template <typename It, typename It2> constexpr plain_array_safe(It first, It last) {
            assign(first, last);
        }
        constexpr plain_array_safe(const plain_array_safe &other) {
            assign(other.begin(), other.end());
        }
        constexpr plain_array_safe(plain_array_safe &&other) noexcept {
            assign(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
        }
        constexpr plain_array_safe(::std::initializer_list<value_type> init) {
            assign(init.begin(), init.end());
        }

#if __cpp_lib_array_constexpr >= 201811L
        template <size_t N0> constexpr plain_array_safe(const std::array<value_type, N0> &arr) noexcept {
            assign(arr.begin(), arr.end());
        }
#endif

        constexpr void assign(size_type count, const value_type &value) {
            size_t       i  = 0;
            const size_t mx = count >= N ? N : count;
            _size           = mx;
            for (; i < mx; i++)
                data()[i] = value;
            for (; i < count; i++)
                data()[i] = value_type();
        }

        constexpr void assign(size_type count) {
            size_t       i  = 0;
            const size_t mx = count >= N ? N : count;
            _size           = mx;
            for (; i < N; i++)
                data()[i] = value_type();
        }

        template <typename It, typename It2> constexpr void assign(It first, It2 last) {
            _size = 0;
            for (; _size < N && first != last; ++first) {
                data()[_size] = *first;
                _size++;
            }
        }

        //[]'s
        [[nodiscard]] constexpr reference operator[](size_type pos) {
            assert(pos < _size);
            return data()[pos];
        };

        [[nodiscard]] constexpr const_reference operator[](size_type pos) const {
            assert(pos < _size);
            return data()[pos];
        };
        // front
        [[nodiscard]] constexpr reference front() {
            assert(_size > 0);
            return data()[0];
        };
        [[nodiscard]] constexpr const_reference front() const {
            assert(_size > 0);
            return data()[0];
        };
        // back's
        [[nodiscard]] constexpr reference back() {
            assert(_size > 0);
            return data()[_size - 1];
        };
        [[nodiscard]] constexpr const_reference back() const {
            assert(_size > 0);
            return data()[_size - 1];
        };

        constexpr plain_array_safe &operator=(const plain_array_safe &other) {
            if (this == &other)
                return *this;
            plain_array_safe(other.begin(), other.end());
            return *this;
        }

        constexpr plain_array_safe &operator=(plain_array_safe &&other) noexcept {
            if (this == &other)
                return *this;
            plain_array_safe(std::forward<plain_array_safe &&>(other));
            return *this;
        }

        // data's
        [[nodiscard]] constexpr value_type *data() noexcept {
            return _data.data();
        };
        [[nodiscard]] constexpr const value_type *data() const noexcept {
            return _data.data();
        };
        // begin's
        [[nodiscard]] constexpr iterator begin() noexcept {
            return data();
        };
        [[nodiscard]] constexpr const_iterator begin() const noexcept {
            return data();
        };
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
            return data();
        };
        // rbegin's
        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
            return reverse_iterator(end());
        };
        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
            return const_reverse_iterator(end());
        };
        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
            return const_reverse_iterator(end());
        };
        // end's
        [[nodiscard]] constexpr iterator end() noexcept {
            return data() + _size;
        };
        [[nodiscard]] constexpr const_iterator end() const noexcept {
            return data() + _size;
        };
        [[nodiscard]] constexpr const_iterator cend() const noexcept {
            return data() + _size;
        };
        // rend's
        [[nodiscard]] constexpr reverse_iterator rend() noexcept {
            return reverse_iterator(begin());
        };
        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
            return const_reverse_iterator(begin());
        };
        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
            return const_reverse_iterator(begin());
        };

        template <typename... Args> constexpr reference emplace_back(Args &&...args) {
            if (_size < N) {
                size_t idx  = _size;
                data()[idx] = value_type(std::forward<Args>(args)...);
                _size++;
                return data()[idx];
            } else {
                return back();
            }
        }

        // writes data to last index without checking on the size, quick but unsafe
        template <typename... Args> constexpr reference unchecked_emplace_back(Args &&...args) {
            size_t idx  = _size;
            data()[idx] = value_type(std::forward<Args>(args)...);
            _size += (_size < N);
            _overrun = _overrun || (_size >= N);
            return data()[idx];
        }

        // push_back's
        constexpr void push_back(const value_type &value) {
            emplace_back(::std::forward<const value_type &>(value));
        }

        constexpr void push_back(value_type &&value) {
            emplace_back(::std::forward<value_type &&>(value));
        };

        constexpr void unchecked_push_back(const value_type &value) {
            unchecked_emplace_back(::std::forward<const value_type &>(value));
        }

        constexpr void unchecked_push_back(value_type &&value) {
            unchecked_emplace_back(::std::forward<value_type &&>(value));
        }

        // pop_back's
        constexpr void pop_back() {
            if (_size) {
                _size--;
            }
        }
        constexpr void unchecked_pop_back() {
            _size--;
        }

        // clear
        constexpr void clear() {
            _size = 0;
        }

        // empty
        constexpr bool empty() {
            return _size == 0;
        }
        constexpr bool empty() const {
            return _size == 0;
        }

        // capacity
        constexpr size_t capacity() {
            return N;
        }
        constexpr size_t capacity() const {
            return N;
        }
        // max_size
        constexpr size_t max_size() {
            return N;
        }
        constexpr size_t max_size() const {
            return N;
        }
        // size
        constexpr size_t size() {
            return _size;
        }
        constexpr size_t size() const {
            return _size;
        }

        // pop_front's (non-standard)
        constexpr void pop_front() {
            if (_size) {
                copy(begin() + 1, end(), begin());
                _size--;
            }
        }
        constexpr void unchecked_pop_front() {
            copy(begin() + 1, end(), begin());
            _size--;
        }

        constexpr iterator erase(const_iterator pos) {
            if (_size) {
                size_t erase_idx = pos - cbegin();
                if (erase_idx < _size) {
                    copy(begin() + erase_idx + 1, end(), begin() + erase_idx);
                    _size--;
                    return begin() + erase_idx;
                }
                erase_idx = _size;
                return begin() + erase_idx;
            } else {
                return end();
            }
        }

        constexpr iterator erase(const_iterator first, const_iterator last) {
            if (first == last) {
                size_t erase_idx = last - cbegin();
                return begin() + erase_idx;
            }
            if (_size) {
                size_t erase_idx = first - cbegin();
                size_t last_idx  = last - cbegin();
                if (erase_idx < last_idx && erase_idx < _size && last_idx <= _size) {
                    iterator f = begin() + erase_idx;
                    iterator l = begin() + last_idx;
                    // a b c d - - - h i j k _ _ _ _
                    // a b c d h i j k _ _ _ _ _ _ _
                    iterator c = copy(l, end(), f);
                    _size      = c - begin();
                    return begin() + erase_idx;
                }
                erase_idx = _size;
                return begin() + erase_idx;
            } else {
                return end();
            }
        };

        // emplace
        template <typename... Args> constexpr iterator emplace(const_iterator pos, Args &&...args) {
            if (_size < N) {
                size_t insert_idx = pos - cbegin();
                // clamp insertion point
                insert_idx   = insert_idx <= N ? insert_idx : N;
                iterator ret = begin() + insert_idx;
                if (insert_idx == _size) {
                    unchecked_emplace_back(std::forward<Args>(args)...);
                    return ret;
                }
                // move backwards
                copy_backward(begin() + insert_idx, end(), end() + 1);
                // construct* inplace
                data()[insert_idx] = value_type(std::forward<Args>(args)...);
                _size += 1;
                return ret;
            } else {
                return end();
            }
        }

        // insert
        constexpr iterator insert(const_iterator pos, const value_type &value) {
            return emplace(pos, value);
        };
        constexpr iterator insert(const_iterator pos, value_type &&value) {
            return emplace(pos, ::std::move(value));
        };
        constexpr iterator insert(const_iterator pos, size_type count, const value_type &value) {
            return insert_backwards(pos, count, value);
        }

        constexpr MUST_INLINE iterator insert_backwards(const_iterator pos, size_type count,
                                                        const value_type &value) {
            if (_size < N) { //
                size_t insert_idx = pos - cbegin();
                // clamp insertion point
                insert_idx   = insert_idx <= N ? insert_idx : N;
                iterator ret = begin() + insert_idx;
                if (insert_idx == _size) {
                    size_t remaining    = N - _size;
                    size_t insert_count = count <= remaining ? count : remaining;

                    fill(end(), end() + insert_count, value);
                    _size += insert_count;
                    return ret;
                }
                size_t remaining    = N - _size;
                size_t insert_count = count <= remaining ? count : remaining;
                copy_backward(begin() + insert_idx, begin() + _size, begin() + (insert_count + _size));
                fill(begin() + insert_idx, begin() + (insert_idx + insert_count), value);
                _size += insert_count;
                return ret;
            } else {
                return end();
            }
        }

        constexpr MUST_INLINE iterator insert_rotate(const_iterator pos, size_type count,
                                                     const value_type &value) {
            if (_size < N) { //
                size_t insert_idx = pos - cbegin();
                // clamp insertion point
                insert_idx   = insert_idx <= N ? insert_idx : N;
                iterator ret = begin() + insert_idx;
                if (insert_idx == _size) {
                    size_t remaining    = N - _size;
                    size_t insert_count = count <= remaining ? count : remaining;

                    fill(end(), end() + insert_count, value);
                    _size += insert_count;
                    return ret;
                }
                size_t remaining    = N - _size;
                size_t insert_count = count <= remaining ? count : remaining;

                size_t mid_idx = _size;
                fill(end(), end() + insert_count, value);
                _size += insert_count;
                rotate(begin() + insert_idx, begin() + mid_idx, begin() + _size);

                return ret;
            } else {
                return end();
            }
        }

        template <typename It, typename It2,
                  typename std::enable_if<!std::is_convertible<It, size_type>::value>::type>
        constexpr iterator insert(const_iterator pos, It first, It2 last) {
            if (_size < N) { //
                size_t insert_idx = pos - cbegin();
                // clamp insertion point
                insert_idx   = insert_idx <= N ? insert_idx : N;
                iterator ret = begin() + insert_idx;
                if (insert_idx == _size) {
                    // no need to rotate
                    for (; _size < N && first != last; ++first)
                        unchecked_emplace_back(*first);
                    return ret;
                }
                size_t mid_insert = _size;
                for (; _size < N && first != last; ++first)
                    unchecked_emplace_back(*first);
                // size_t last_insert = _size;
                // rotate algorithm
                rotate(begin() + insert_idx, begin() + mid_insert, begin() + _size);
                return ret;
            } else {
                return end();
            }
        }

        // append (non-standard)
        template <typename It, typename It2,
                  typename std::enable_if<
                      !std::is_same<typename std::iterator_traits<It>::value_type, void>::value>::type>
        constexpr iterator append(It first, It2 last) {
            if (_size < N) {
                const size_t idx = _size;
                for (; _size < N && first != last; ++first)
                    unchecked_emplace_back(*first);
                return begin() + idx;
            } else {
                return end();
            }
        }
        constexpr iterator append(size_t count, const value_type &value) {
            if (_size < N) {
                const size_t idx = _size;
                for (size_t i = 0; _size < N && i < count; i++)
                    unchecked_emplace_back(value);
                return begin() + idx;
            } else {
                return end();
            }
        }
        constexpr iterator append(size_t count) {
            if (_size < N) {
                const size_t idx = _size;
                for (size_t i = 0; _size < N && i < count; i++)
                    unchecked_emplace_back();
                return begin() + idx;
            } else {
                return end();
            }
        }

        constexpr void swap(plain_array_safe &other) noexcept(true) {
            if constexpr (N > 0) {
                swap(_data._values, other._data._values);
                size_t tmp  = _size;
                _size       = other._size;
                other._size = tmp;
            }
        }
    };
} // namespace containers
