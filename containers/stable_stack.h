#pragma once
#include <array>
#include <cassert>
#include <list>
#include <memory>
#include <vector>

/*
The MIT License (MIT)

Copyright (c) 2022 Alex Anderson

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

// stable stack: maintains that pointers to it's internal members are stable as the stack grows
//  warning: this implementation constructs entire blocks of memory and destructs all at once
//  pop_back() and pop() do not destruct objects in memory, be wary of stale references and accesses
//  because those objects will still be live

template <typename T, size_t N = 32> struct stable_stack {
  public:
	using array_type = std::array<T, N>;
	struct data_block {
		using reference       = T &;
		using const_reference = const T &;
		using size_type       = ::std::size_t;

		array_type                        data;
		[[nodiscard]] constexpr reference operator[](size_type pos) {
			return data[pos];
		};

		[[nodiscard]] constexpr const_reference operator[](size_type pos) const {
			return data[pos];
		}
	};
	using backing_type   = data_block;
	using smart_ptr_type = std::unique_ptr<backing_type>;
	using size_type      = ::std::size_t;
	// a fat iterator type
	struct iterator {
		using iterator_category = std::random_access_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = typename ::std::remove_cv<T>::type;
		using pointer           = value_type *;
		using reference         = value_type &;

		constexpr iterator(stable_stack<T, N> *original_struct, size_type idx) : _ptr(original_struct), _idx(idx) {
		}

		constexpr iterator &operator+=(difference_type idxs) {
			_idx += idxs;
			return *this;
		}

		constexpr iterator &operator-=(difference_type idxs) {
			_idx -= idxs;
			return *this;
		}

		constexpr iterator &operator++() {
			_idx++;
			return *this;
		}

		constexpr iterator operator++(int) {
			iterator tmp = *this;
			++(*this);
			return tmp;
		}

		constexpr iterator &operator--() {
			_idx--;
			return *this;
		}

		constexpr iterator operator--(int) {
			iterator tmp = *this;
			--(*this);
			return tmp;
		}

		friend bool operator==(const iterator &a, const iterator &b) {
			return a._ptr == b._ptr && a._idx == b._idx;
		};

		friend bool operator<(const iterator &a, const iterator &b) {
			return a._ptr == b._ptr && a._idx < b._idx;
		};

		friend bool operator>(const iterator &a, const iterator &b) {
			return a._ptr == b._ptr && a._idx > b._idx;
		};

		friend bool operator!=(const iterator &a, const iterator &b) {
			return a._ptr != b._ptr || a._idx != b._idx;
		};

		[[nodiscard]] constexpr reference operator*() const noexcept {
			assert(_ptr && "can't dereference value-initialized stable_stack iterator");
			assert(_idx < _ptr->_size && "can't dereference out of range stable_stack iterator");
			return _ptr->operator[](_idx);
		}
		

		stable_stack<T, N> *_ptr;
		size_type           _idx;
	};

  private:
	std::vector<smart_ptr_type> _data = {};
	// these are just for book-keeping, we could implement the struct w/o them
	size_t _size     = {};
	size_t _capacity = {};

  public:
	using element_type    = T;
	using value_type      = typename ::std::remove_cv<T>::type;
	using const_reference = const value_type &;
	using difference_type = ::std::ptrdiff_t;

	using pointer       = value_type *;
	using const_pointer = const value_type *;

	using reference      = element_type &;
	using const_iterator = const iterator;

	using reverse_iterator       = ::std::reverse_iterator<iterator>;
	using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

	// emplace_back's
	template <class... Args> constexpr reference emplace_back(Args &&...args) {
		size_type d = _size / N;
		size_type r = _size % N;

		if (_size == _capacity) {
			_data.emplace_back(std::make_unique<data_block>());
			_capacity += N;
		}
		_size += 1;
		// not ideal
		reference ref = _data[d]->operator[](r);
		ref           = T{std::forward<Args>(args)...};
		return ref;
	};

	constexpr iterator begin() {
		return iterator{this, 0ULL};
	}

	constexpr iterator end() {
		return iterator{this, _size};
	}

	constexpr reverse_iterator rbegin() {
		return std::reverse_iterator<iterator>(end());
	}

	constexpr reverse_iterator rend() {
		return std::reverse_iterator<iterator>(begin());
	}

	// push's
	constexpr void push(const T &value) {
		emplace_back(::std::forward<const T &>(value));
	}
	constexpr void push(T &&value) {
		emplace_back(::std::forward<T &&>(value));
	};
	// push_back's
	constexpr void push_back(const T &value) {
		emplace_back(::std::forward<const T &>(value));
	}
	constexpr void push_back(T &&value) {
		emplace_back(::std::forward<T &&>(value));
	};

	//[]'s
	[[nodiscard]] constexpr reference operator[](size_type pos) {
		// assert(pos < size());
		size_type        d = pos / N;
		size_type        r = pos % N;
		return _data[d]->operator[](r);
	};

	[[nodiscard]] constexpr const_reference operator[](size_type pos) const {
		// assert(pos < size());
		size_type        d = pos / N;
		size_type        r = pos % N;
		return _data[d]->operator[](r);
	};
	// pop_back's
	constexpr void pop_back() {
		// we don't destroy the object?
		_size -= (_size > 0);
	};
	// pop's
	constexpr void pop() {
		pop_back();
	};

	// back's
	[[nodiscard]] constexpr reference back() {
		return operator[](_size - 1);
	};
	[[nodiscard]] constexpr const_reference back() const {
		return operator[](_size - 1);
	};

	// front's
	[[nodiscard]] constexpr reference front() {
		return operator[](0);
	};
	[[nodiscard]] constexpr const_reference front() const {
		return operator[](0);
	};

	// top's
	[[nodiscard]] constexpr reference top() {
		return back();
	};
	[[nodiscard]] constexpr const_reference top() const {
		return back();
	};

	// size
	constexpr size_type size() const noexcept {
		return _size;
	};
	// capacity
	constexpr size_type capacity() const noexcept {
		return _capacity;
	};
	
	constexpr void reserve(size_type new_capacity) {
		size_type d = new_capacity / N;
		size_type r = new_capacity % N;
		
		_data.reserve(d + (r>0));
	}
	
};