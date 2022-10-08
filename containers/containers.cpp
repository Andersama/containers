// containers.cpp : Defines the entry point for the application.
//
#include "nanobench.h"
#include "plain_array.h"
#include <iostream>
#include <string>
#include <vector>

template <typename value> constexpr void rotate(value *first, value *mid, value *last) {
    const size_t p     = mid - first;
    const size_t n     = last - first;
    size_t       m     = 0;
    size_t       count = 0;
    for (m = 0, count = 0; count != n; m++) {
        value t = *(first + m);
        size_t     i = m;
        size_t     j = m + p;
        for (; j != m; i = j, j = ((j + p) < n) ? (j + p) : (j + p - n), count++) {
            *(first + i) = *(first + j);
        }
        *(first + i) = t;
        count++;
    }
}

int main() {
    int values[10];
    for (size_t i = 0; i < 10; i++)
        values[i] = i;
    rotate(&values[0] + 3, &values[0] + 6, &values[0] + 10);

    constexpr containers::plain_array<int, 32> test = []() {
        containers::plain_array<int, 32> values;
        for (size_t i = 0; i < values.capacity() / 2; i++)
            values.unchecked_emplace_back(i);
        for (; values.size() < (values.capacity() * 3 / 4);)
            values.append(1, values.size());
        for (; values.size() < values.capacity();)
            values.emplace_back(values.size());
        // values.append(values.capacity() - values.size(), 64);
        values.erase(values.begin() + 8, values.begin() + 24);
        values.pop_front();
        values.pop_front();
        values.erase(values.begin() + 3);
        values.pop_back();
        values.pop_back();
        values.insert(values.begin() + 6, 2, 4);
        return values;
    }();

    constexpr containers::plain_array_safe<int, 32> test_safe = []() {
        containers::plain_array_safe<int, 32> values;
        for (size_t i = 0; i < values.capacity() / 2; i++)
            values.unchecked_emplace_back(i);
        for (; values.size() < (values.capacity() * 3 / 4);)
            values.append(1, values.size());
        for (; values.size() < values.capacity();)
            values.emplace_back(values.size());
        // values.append(values.capacity() - values.size(), 64);
        values.erase(values.begin() + 8, values.begin() + 24);
        values.pop_front();
        values.pop_front();
        values.erase(values.begin() + 3);
        values.pop_back();
        values.pop_back();
        values.insert(values.begin() + 6, 2, 4);
        return values;
    }();

    std::cout << "match test\n";
    for (size_t i = 0; i < test_safe.size(); i++) {
        std::cout << test[i] << '\t' << test_safe[i] << '\n';
    }

    containers::plain_array<int, 32> swap_test;
    swap_test.emplace_back(10);
    swap_test.emplace_back(2);

    containers::plain_array<int, 32> swap_test_2;
    swap_test_2.emplace_back(3);

    swap_test.swap(swap_test_2);
    std::cout << "swap test\n";
    for (size_t i = 0; i < swap_test.size(); i++)
        std::cout << swap_test[i] << '\n';

    for (size_t i = 0; i < swap_test_2.size(); i++)
        std::cout << swap_test_2[i] << '\n';

    std::cout << "constexpr test\n";

    for (size_t i = 0; i < test.size(); i++)
        std::cout << test[i] << '\n';

    ankerl::nanobench::Bench benchmark;
    benchmark.epochs(1024);
    benchmark.minEpochIterations(128);
    benchmark.warmup(4);

    // msvc std::reverse improves as batch_count increases, roughly switches around 8*
    constexpr int batch_count = 64;

    benchmark.batch(batch_count);
    benchmark.unit("index");
    benchmark.performanceCounters(true);
    benchmark.relative(true);

    size_t                                    count = 0;
    containers::plain_array<int, batch_count> insert_test;
    insert_test.emplace_back(0);

    benchmark.run("int[] (insert move_backwards)", [&]() {
        insert_test.clear();
        size_t r = (insert_test.capacity() / 4) + ((insert_test.capacity() % 4) > 0ULL);
        count    = 0;
        for (size_t i = 0; i < r; i++) {
            insert_test.insert_backwards(insert_test.begin() + i, 4, count);
            count++;
        }
    });

    benchmark.run("int[] (insert rotate)", [&]() {
        insert_test.clear();
        size_t r = (insert_test.capacity() / 4) + ((insert_test.capacity() % 4) > 0ULL);
        count    = 0;
        for (size_t i = 0; i < r; i++) {
            insert_test.insert_rotate(insert_test.begin() + i, 4, count);
            count++;
        }
    });

    /*
    // 1 2 3 4 5 6 7 8
    // 3 4 5 6 7 8 1 2
    */

    return 0;
}
