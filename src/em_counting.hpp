#ifndef HEADER_EM_COUNTING_HPP
#define HEADER_EM_COUNTING_HPP

#include <stxxl/sequence>
#include <cassert>
#include <memory>
#include <utility>

template <typename Key, typename Count>
class em_counting {
public:
    using key_type = Key;
    using count_type = Count;
    using value_type = std::pair<key_type, count_type>;

private:
    using sequence_type = stxxl::sequence<value_type>;
    using stream_type = typename sequence_type::stream;
    using pool_type = stxxl::read_write_pool<typename sequence_type::block_type>;

public:

    em_counting()
        : _pool(2 * stxxl::config::get_instance()->disks_number(),
                2 * stxxl::config::get_instance()->disks_number() + 4),
          _sequence(_pool)
    {}

    em_counting(const em_counting&) = delete;
    em_counting& operator=(const em_counting&) = delete;

    // Updates counts and invalidates streaming interface. Call rewind
    // before using it again
    template<typename Stream>
    void update(Stream& in) {
        if (in.empty())
            return;

        auto count_next = [&in] () -> value_type {
            if (in.empty()) return {Key{}, 0};

            auto current = *in;
            count_type count = 1;
            for(++in; !in.empty() && *in == current; ++count, ++in);

            return {current, count};
        };

        auto in_to_stream = [&in, count_next] (sequence_type& result) {
            while(true) {
                auto count = count_next();
                if (!count.second) break;
                result.push_back(count);
            }
        };

        // first run:  _sequence is empty and we do not have to merge
        if (_sequence.empty()) {
            in_to_stream(_sequence);
            return;
        }

        // both input sources contain data, so we have to merge
        sequence_type result(_pool);
        {
            auto sstream = _sequence.get_stream();

            while (!(sstream.empty() || in.empty())) {
                auto count = count_next();
                if (!count.second) break;

                // copy elements from sequence that are not contained in in
                for (; !sstream.empty() &&
                       (*sstream).first < count.first; ++sstream)
                    result.push_back(*sstream);

                // if current element is already in sequence, we add counts
                if (!sstream.empty() && (*sstream).first == count.first) {
                    count.second += (*sstream).second;
                    ++sstream;
                }

                result.push_back(count);
            }

            // copy remainder from _sequence
            for (; !sstream.empty(); ++sstream)
                result.push_back(*sstream);
        }

        // copy remainder from in
        in_to_stream(result);

        _stream.reset(nullptr);
        _sequence.swap(result);
    }

// stream
    // todo: we could implement the stream s.t. the last update is unnecessary

    void rewind() {
        _stream.reset(nullptr); // free blocks
        _stream.reset(new stream_type(_sequence));
    }

    const value_type& operator*() const {
        assert(_stream);
        return **_stream;
    }

    bool empty() const {
        assert(_stream);
        return _stream->empty();
    }

    em_counting& operator++() {
        assert(_stream);
        ++(*_stream);
    }


private:
    pool_type _pool;
    sequence_type _sequence;
    std::unique_ptr<stream_type> _stream;

};

#endif // HEADER_EM_COUNTING_HPP
