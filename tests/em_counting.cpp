#include "../src/em_counting.hpp"

#include <stxxl/bits/stream/stream.h>

#include <iostream>
#include <random>
#include <vector>

using T = uint32_t;
std::mt19937_64 prng;

void ensure(bool cond) {
    if (!cond) {
        std::cerr << "Fail\n";
        abort();
    }
}


std::vector<T> generate_random_vector(size_t n, T max_value) {
    std::uniform_int_distribution<T> dist(0, max_value);

    std::vector<T> result;
    result.reserve(n);

    for(size_t i=0; i<n; i++)
        result.push_back(dist(prng));

    return result;
}

template <typename It>
std::vector< std::pair<T, size_t> > distribution(It begin, It end) {
    std::sort(begin, end);
    std::vector< std::pair<T, size_t> > distr;

    size_t count = 1;
    T last = *begin;
    for(auto i = begin + 1; i != end; i++) {
        if (last != *i) {
            distr.emplace_back(last, count);
            last = *i;
            count = 0;
        }

        count++;
    }

    distr.emplace_back(last, count);

    return distr;
}

void check_counting(size_t n, T max_value) {
    auto vec = generate_random_vector(n, max_value);
    std::uniform_int_distribution<size_t> dist(1, n / 10);

    size_t i = 0;

    em_counting<T, size_t> counter;

    while(i < n) {
        const size_t chunk = std::min<size_t>(dist(prng), n - i);

        auto begin = vec.begin() + i;
        auto end = begin + chunk;

        std::cout << i << ":" << chunk << "\n";

        {
            std::sort(begin, end);
            auto str = stxxl::stream::streamify(begin, end);
            counter.update(str);
        }

        i += chunk;
    }

    auto distr = distribution(vec.begin(), vec.end());

    for(int it = 0; it < 2; it++) {
        counter.rewind();

        for(auto ref : distr) {
            ensure(!counter.empty());

            auto& citem = *counter;

            ensure(citem.first == ref.first);
            ensure(citem.second == ref.second);

            ++counter;
        }

        ensure(counter.empty());
    }
}

int main() {
    for(int i=0; i<10; i++) {
        check_counting(10000, 10); // dense
        check_counting(10000, 1000);
        check_counting(10000, 100000);
    }

    std::cout << "Ok\n";

    return 0;
}
