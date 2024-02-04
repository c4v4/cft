#ifndef SCP_INCLUDE_COLUMNS_HPP_
#define SCP_INCLUDE_COLUMNS_HPP_
#include <cassert>
#include <vector>

#include "cft.hpp"

#define GROWTH_FACTOR 1.618

template <typename Elem>
class CollectionOf;

template <typename Elem>
class IdxList {
public:
    static constexpr size_t MY_SIZE = sizeof(Elem);

protected:
    IdxList(idx_t const* beg_, idx_t const* end_)
        : sz(end_ - beg_) {
        std::copy(beg_, end_, begin());
    }

    IdxList(IdxList const& other)
        : sz(other.sz) {
        std::copy(other.begin(), other.end(), begin());
    }

public:
    IdxList()
        : sz(0U) {
    }

    IdxList(IdxList&&)                 = delete;
    IdxList& operator=(IdxList const&) = delete;
    IdxList& operator=(IdxList&&)      = delete;
    ~IdxList()                         = default;

    Elem& operator=(Elem const&) = delete;
    Elem& operator=(Elem&&)      = delete;

    [[nodiscard]] inline idx_t* begin() {
        return reinterpret_cast<idx_t*>(reinterpret_cast<char*>(this) + MY_SIZE);
    }

    [[nodiscard]] inline idx_t* end() {
        return begin() + sz;
    }

    [[nodiscard]] inline idx_t& operator[](idx_t i) {
        return begin()[i];
    }

    [[nodiscard]] inline idx_t size() const {
        return sz;
    }

    [[nodiscard]] inline bool empty() const {
        return sz == 0;
    }

    [[nodiscard]] inline idx_t const* begin() const {
        return reinterpret_cast<idx_t const*>(reinterpret_cast<char const*>(this) + MY_SIZE);
    }

    [[nodiscard]] inline idx_t const* end() const {
        return begin() + sz;
    }

    [[nodiscard]] inline idx_t operator[](idx_t i) const {
        return begin()[i];
    }

protected:
    idx_t sz;
};

class RowTempName : public IdxList<RowTempName> {
    friend class CollectionOf<RowTempName>;

protected:
    template <typename iter>
    RowTempName(iter beg_, iter end_)
        : IdxList<RowTempName>(beg_, end_) {
    }

    RowTempName(RowTempName const& other)
        : IdxList<RowTempName>(other) {
    }

public:
    RowTempName() {
    }
};

class SubInstCol : public IdxList<SubInstCol> {
    friend class CollectionOf<SubInstCol>;

protected:
    SubInstCol(idx_t const* beg_, idx_t const* end_, real_t c_ = 0.0)
        : IdxList<SubInstCol>(beg_, end_)
        , c(c_)
        , c_u(c_) {
    }

    SubInstCol(SubInstCol const& other)
        : IdxList<SubInstCol>(other)
        , c(other.c)
        , c_u(other.c_u) {
    }

public:
    SubInstCol(real_t c_ = 0.0)
        : c(c_)
        , c_u(c_) {
    }

    [[nodiscard]] inline real_t get_cost() const {
        return c;
    }

    inline void set_cost(real_t new_c) {
        c_u = c = new_c;
    }

    [[nodiscard]] inline real_t get_cu() const {
        return c_u;
    }

    inline void set_cu(real_t new_c) {
        c_u = new_c;
    }

    inline real_t update_cu(const real_t delta_u) {
        return c_u -= delta_u;
    }

    [[nodiscard]] inline real_t compute_lagr_cost(std::vector<real_t> const& u) const {
        real_t local_c_u = c;
        for (idx_t i : *this) {
            assert(i < u.size());
            local_c_u -= u[i];
        }
        return local_c_u;
    }

    inline real_t compute_lagr_cost(std::vector<real_t> const& u) {
        c_u = c;
        for (idx_t i : *this) {
            assert(i < u.size());
            c_u -= u[i];
        }
        return c_u;
    }

protected:
    real_t c;
    real_t c_u;
};

class InstCol : public IdxList<InstCol> {
    friend class CollectionOf<InstCol>;

protected:
    InstCol(idx_t const* beg_, idx_t const* end_, real_t c_ = 0.0, real_t sol_c_ = 0.0)
        : IdxList<InstCol>(beg_, end_)
        , c(c_)
        , sol_c(sol_c_) {
    }

    InstCol(InstCol const& other)
        : IdxList<InstCol>(other)
        , c(other.c)
        , sol_c(other.sol_c) {
    }

public:
    InstCol(real_t c_ = 0.0, real_t sol_c_ = 0.0)
        : c(c_)
        , sol_c(sol_c_) {
    }

    [[nodiscard]] inline real_t get_cost() const {
        return c;
    }

    inline void set_cost(real_t new_c) {
        c = new_c;
    }

    [[nodiscard]] inline real_t get_solcost() const {
        return sol_c;
    }

    inline void set_solcost(real_t new_c) {
        sol_c = new_c;
    }

    [[nodiscard]] inline real_t compute_lagr_cost(std::vector<real_t> const& u) const {
        real_t local_c_u = c;
        for (idx_t i : *this) {
            assert(i < u.size());
            local_c_u -= u[i];
        }
        return local_c_u;
    }

protected:
    real_t c;
    real_t sol_c;
};

static_assert(sizeof(IdxList<SubInstCol>) == sizeof(IdxList<InstCol>));
static_assert(sizeof(IdxList<InstCol>) == sizeof(idx_t));

static_assert(sizeof(SubInstCol) == SubInstCol::MY_SIZE);
static_assert(SubInstCol::MY_SIZE == IdxList<SubInstCol>::MY_SIZE);
static_assert(IdxList<SubInstCol>::MY_SIZE ==
              std::max(alignof(real_t), sizeof(idx_t)) + sizeof(real_t) * 2U);

static_assert(sizeof(InstCol) == InstCol::MY_SIZE);
static_assert(InstCol::MY_SIZE == IdxList<InstCol>::MY_SIZE);
static_assert(IdxList<InstCol>::MY_SIZE ==
              std::max(alignof(real_t), sizeof(idx_t)) + sizeof(real_t) * 2U);

template <typename Elem>
class CollectionOf {
    static_assert(std::is_same_v<Elem, SubInstCol> || std::is_same_v<Elem, InstCol> ||
                      std::is_same_v<Elem, RowTempName>,
                  "CollectionOf works only with Col or RowTempName as base element.");

public:
    class CollectionIter {
    public:
        CollectionIter(Elem* base_)
            : base(base_) {
        }

        [[nodiscard]] inline auto& operator*() {
            return *base;
        }

        inline auto& operator++() {
            base = reinterpret_cast<Elem*>(reinterpret_cast<char*>(base) + sizeof(Elem) +
                                           base->size() * sizeof(idx_t));
            return *this;
        }

        inline auto operator++(int) {
            Elem* old_base = base;
            base           = reinterpret_cast<Elem*>(reinterpret_cast<char*>(base) + sizeof(Elem) +
                                           base->size() * sizeof(idx_t));
            return CollectionIter(old_base);
        }

        [[nodiscard]] inline bool operator!=(CollectionIter& it2) const {
            return base != it2.base;
        }

        [[nodiscard]] inline bool operator==(CollectionIter& it2) const {
            return base == it2.base;
        }

    private:
        Elem* base;
    };


public:
    CollectionOf(idx_t allocated_elem = 100) {
        buffer_allocate(allocated_elem * (sizeof(Elem) + sizeof(idx_t) * 10));
        offsets.reserve(allocated_elem);
    }

    ~CollectionOf() {
        free(start);
        start = finish = end_of_storage = nullptr;
    }

    [[nodiscard]] inline CollectionIter begin() {
        return CollectionIter(reinterpret_cast<Elem*>(start));
    }

    [[nodiscard]] inline CollectionIter end() {
        return CollectionIter(reinterpret_cast<Elem*>(finish));
    }

    [[nodiscard]] inline Elem& operator[](idx_t j) {
        return *reinterpret_cast<Elem*>(start + offsets[j]);
    }

    [[nodiscard]] inline Elem& back() {
        return *reinterpret_cast<Elem*>(start + offsets.back());
    }

    [[nodiscard]] inline Elem* data() {
        return reinterpret_cast<Elem*>(start + offsets[0]);
    }

    [[nodiscard]] inline idx_t size() const {
        return offsets.size();
    }

    [[nodiscard]] inline bool empty() const {
        return offsets.empty();
    }

    [[nodiscard]] inline const CollectionIter begin() const {
        return CollectionIter(reinterpret_cast<Elem*>(start));
    }

    [[nodiscard]] inline const CollectionIter end() const {
        return CollectionIter(reinterpret_cast<Elem*>(finish));
    }

    [[nodiscard]] inline Elem const& operator[](idx_t j) const {
        return *reinterpret_cast<Elem*>(start + offsets[j]);
    }

    [[nodiscard]] inline Elem const& back() const {
        return *reinterpret_cast<Elem*>(start + offsets.back());
    }

    [[nodiscard]] inline Elem const* data() const {
        return reinterpret_cast<Elem*>(start);
    }

    inline void clear() {
        finish = start;
        offsets.clear();
    }

    inline void reserve(idx_t new_ncols) {
        idx_t ncols = offsets.size();
        if (new_ncols <= ncols)
            return;

        double avg_col_size = ncols > 0
                                  ? static_cast<double>(finish - start) / static_cast<double>(ncols)
                                  : 10.0;

        buffer_allocate(new_ncols * avg_col_size + 1);
        offsets.reserve(new_ncols);
    }

    template <typename iter, typename... Args>
    inline Elem& emplace_back(iter beg_, iter end_, Args&&... args) {

        idx_t col_size    = end_ - beg_;
        idx_t col_storage = sizeof(Elem) + col_size * sizeof(idx_t);
        char* new_finish  = finish + col_storage;

        buffer_check_size(new_finish);

        Elem* elem = new (finish) Elem(beg_, end_, std::forward<Args>(args)...);
        assert(reinterpret_cast<char*>(elem) == finish);

        offsets.push_back(finish - start);
        finish = new_finish;

        return *elem;
    }

    inline Elem& emplace_back(Elem const& other) {

        idx_t col_size    = other.size();
        idx_t col_storage = sizeof(Elem) + col_size * sizeof(idx_t);
        char* new_finish  = finish + col_storage;

        buffer_check_size(new_finish);

        Elem* elem = new (finish) Elem(other);
        assert(reinterpret_cast<char*>(elem) == finish);

        offsets.push_back(finish - start);
        finish = new_finish;

        return *elem;
    }

    inline void push_back(Elem const& elem) {
        emplace_back(elem);
    }

    // Construct in place a new column
    template <typename... Args>
    inline Elem& new_col_create(Args&&... args) {
        Elem& elem = buffer_emplace_back<Elem>(std::forward<Args>(args)...);
        offsets.push_back(reinterpret_cast<char*>(std::addressof(elem)) - start);
        return elem;
    }

    inline void new_col_push_back(idx_t idx) {
        buffer_emplace_back<idx_t>(idx);
        ++back().sz;
    }

    inline void new_col_discard() {
        finish = reinterpret_cast<char*>(offsets.back());
        offsets.pop_back();
    }


private:
    template <typename T, typename... Args>
    inline T& buffer_emplace_back(Args&&... args) {
        char* new_finish = finish + sizeof(T);

        buffer_check_size(new_finish);

        T* elem = new (finish) T(std::forward<Args>(args)...);
        assert(reinterpret_cast<char*>(elem) == finish);

        finish = new_finish;

        return *elem;
    }

    inline void buffer_check_size(char*& new_finish) {
        if (new_finish > end_of_storage) {
            size_t new_size  = std::max<size_t>(new_finish - start,
                                               GROWTH_FACTOR * (end_of_storage - start));
            char*  old_start = start;
            buffer_allocate(new_size);
            new_finish += start - old_start;
        }
    }

    inline void buffer_allocate(size_t new_size) {
        assert(!start == !finish && !start == !end_of_storage);

        size_t prev_size = finish - start;
        start            = reinterpret_cast<char*>(realloc(start, new_size));
        finish           = start + prev_size;
        end_of_storage   = start + new_size;

        assert(start && finish && end_of_storage);
    }

    char* start          = nullptr;
    char* finish         = nullptr;
    char* end_of_storage = nullptr;

    std::vector<size_t> offsets;
};

using Rows        = CollectionOf<RowTempName>;
using Cols        = CollectionOf<InstCol>;
using SubInstCols = CollectionOf<SubInstCol>;

#endif