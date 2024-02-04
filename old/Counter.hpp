#ifndef SCP_INCLUDE_COUNTER_HPP_
#define SCP_INCLUDE_COUNTER_HPP_

#include "cft.hpp"

class Counter {

public:

    explicit Counter(idx_t max) : m(max), i(0) { }

    inline void inc() {
        i++;
    }

    [[nodiscard]] inline auto reached() const {
        return i >= m;
    }

    inline void restart() {
        i = 0;
    }

    inline void set_max(idx_t new_max) {
        m = new_max;
    }

    [[nodiscard]] inline idx_t get_max() const {
        return m;
    }

    [[nodiscard]] inline auto get() const {
        return i;
    }

private:

    idx_t m; // max value
    idx_t i; // current value

};


#endif  // SCP_INCLUDE_COUNTER_HPP_
