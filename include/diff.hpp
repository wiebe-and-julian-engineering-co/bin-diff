#pragma once

#include <cstdint>

#include <vector>
#include <memory>
#include <list>
#include <string>
#include <algorithm>
#include <iterator>

/**
 * @brief sequence_view type
 * 
 * @details
 * Used for recursive algorithms that process sequences of data recursively.
 * Useful for implementing algorithms which use divide
 * and conquer techniques, like diff or quicksort.
 * Current implementation only works for linear storage containers, like
 * vectors and strings. sequence_view is not able to mutate the viewed type.
 * 
 * Future specializations for mutable sequences and non-linear stored sequences
 * are possible but currently not supported.
 * 
 * @tparam t_sequenced_type Type in sequence viewed as a sequence_view
 */
template <class t_sequenced_type>
class sequence_view {
    t_sequenced_type* m_seq;
    size_t m_view_index_begin;
    size_t m_view_index_end;

public:
    /**
     * @brief Construct a null sequence_view
     * 
     * @details
     * No member function should be used on null sequences except for size() 
     */
    sequence_view() : m_seq(nullptr), m_view_index_begin(0), m_view_index_end(0) { }

    /**
     * @brief Construct a sequence_view viewing seq from begin to end
     * 
     * @details
     * Begin becomes the 0th element of the sequence_view, end becomes the
     * Nth element. Which is like containers not the last element, but the one
     * after the last element.
     * 
     * @param seq Sequence to view
     * @param begin Begin of view
     * @param end End of view
     */
    sequence_view(t_sequenced_type* seq, uint32_t begin, uint32_t end) : m_seq(seq), m_view_index_begin(begin), m_view_index_end(end) { }

    /**
     * @brief Perform a recursion of a sequence_view
     * 
     * @details
     * Copies all traits of seq and points begin and end to the approriate
     * elements in seq. Useful for recursive algorithms while maintaining low
     * memory overhead.
     * 
     * @param seq Sequence to view
     * @param begin Begin of view in seq
     * @param end End of view in seq
     */
    sequence_view(const sequence_view& seq, uint32_t begin, uint32_t end) :
        m_seq(seq.m_seq),
        m_view_index_begin(seq.m_view_index_begin + begin),
        m_view_index_end(seq.m_view_index_begin + end)
    { }

    /**
     * @brief Combines this with other
     * 
     * @addtogroup Untested
     * 
     * @details
     * When two sequences are consecutive or overlapping, they may be combined.
     * Combining two uncombinable sequences raises an error
     * 
     * @param other Sequence view of the same type to combine
     * @return sequence_view Newly constructed view from this and other
     */
    sequence_view combine(const sequence_view& other) const {
        if (m_seq != other.m_seq) {
            // Cannot combine sequences with different super sequences
            throw std::exception();
        } else if (m_view_index_end >= other.m_view_index_begin || other.m_view_index_end >= m_view_index_begin) {
            return sequence_view(m_seq,
                std::min(
                    m_view_index_begin,
                    other.m_view_index_begin
                ),
                std::max(
                    m_view_index_end,
                    other.m_view_index_end
                )
            );
        } else {
            // Cannot combine sequences that do not succeed eachother
            throw std::exception();
        }
    }

    /**
     * @brief Performs a lookup in subsequenced part of m_seq
     * 
     * @param i Index to lookup
     * @return const t_sequenced_type& const-reference value in sequences
     */
    const t_sequenced_type& operator[] (size_t i) const {
        return m_seq[m_view_index_begin + i];
    }

    /**
     * @brief Returns the index of the first subsequenced element
     * 
     * @return size_t Index of element
     */
    size_t index_begin() const {
        return m_view_index_begin;
    }
    
    /**
     * @brief Returns the index+1 of the last subsequenced element
     * 
     * @return size_t Index of element
     */
    size_t index_end() const {
        return m_view_index_end;
    }

    /**
     * @brief Returns an iterable pointer to the first subsequenced element
     * 
     * @return const t_sequenced_type* Pointer as iterator
     */
    const t_sequenced_type* begin() const noexcept {
        return &m_seq[m_view_index_begin];
    }
    
    /**
     * @brief Returns an iterable pointer to the last+1 subsequenced element
     * 
     * @return const t_sequenced_type* Pointer as iterator
     */
    const t_sequenced_type* end() const noexcept {
        return &m_seq[m_view_index_end];
    }

    /**
     * @brief Returns the size of the sequence_view
     * 
     * @return size_t Amount of elements viewable via sequence_view
     */
    size_t size() const {
        return m_view_index_end - m_view_index_begin;
    }

    /**
     * @brief Prints the sequence_view to a t_os
     * 
     * @tparam t_os ostream type
     * @param os ostream
     * @param seq sequence_view
     * @return t_os& Chainable ostream
     */
    template <class t_os>
    friend t_os& operator<< (t_os& os, const sequence_view& seq) {
        if (seq.m_seq != nullptr) {
            for (size_t i = seq.m_view_index_begin; i < seq.m_view_index_end; i++) {
                os << seq.m_seq[i];
            }
        } else {
            os << "Null sequence";
        }

        return os;
    }
};

enum class PatchOperation {
    Addition,
    Deletion,
    Undefined
};

class Patch {
    PatchOperation m_op;
    size_t m_begin;
    std::string m_seq;

    Patch(PatchOperation op, size_t begin, std::string&& str) :
        m_op(op),
        m_begin(begin),
        m_seq(str)
    { }

    Patch(PatchOperation op, size_t begin, const std::string& str) :
        m_op(op),
        m_begin(begin),
        m_seq(str)
    { }
  public:

    template <class t_sequenced_type>
    static Patch make_addition(const sequence_view<t_sequenced_type>& seq) {
        auto str = std::string(std::cbegin(seq), seq.size());
        return Patch(PatchOperation::Addition, seq.index_begin(), std::move(str));
    }

    template <class t_sequenced_type>
    static Patch make_deletion(const sequence_view<t_sequenced_type>& seq) {
        auto str = std::string(std::cbegin(seq), seq.size());
        return Patch(PatchOperation::Deletion, seq.index_begin(), std::move(str));
    }

    PatchOperation GetOperation() {
        return m_op;
    }

    template <class t_os>
    friend t_os& operator<< (t_os& os, const Patch& p) {
        os << "Patch ";

        if (p.m_op == PatchOperation::Addition) {
            os << "Addition";
        } else if (p.m_op == PatchOperation::Deletion) {
            os << "Deletion";
        } else {
            os << "Undefined";
        }

        os << ": [" << p.m_begin << ':' << p.m_begin + p.m_seq.size() << "] - " << p.m_seq;

        return os;
    }
};

class Diff {
    template <class t_sequenced_type>
    void _diff(const sequence_view<t_sequenced_type>& lhs_seq, const sequence_view<t_sequenced_type>& rhs_seq, std::vector<Patch>& patches) const {
        int32_t lhs_size = lhs_seq.size();
        int32_t rhs_size = rhs_seq.size();
        int32_t max_len = lhs_size + rhs_size;
        int32_t x_values_len = 2 * std::min(lhs_size, rhs_size) + 2;

        int32_t x = 0;
        int32_t y = 0;
        int32_t z = 0;
        int32_t x_initial = 0;
        int32_t y_initial = 0;
        int32_t u = 0;
        int32_t v = 0;

        if (lhs_size > 0 && rhs_size > 0) {
            int32_t w = lhs_size - rhs_size;

            std::vector<int32_t> best_forward_x_values(x_values_len, 0);
            std::vector<int32_t> best_backward_x_values(x_values_len, 0);

            for (int32_t D = 0; D < (max_len / 2 + (max_len % 2 != 0)) + 1; D++) {
                // forward snake
                for (int32_t k = -(D - 2 * std::max(0, D - rhs_size)); k < D - 2 * std::max(0, D - lhs_size) + 1; k += 2) {
                    if (k == -D || (k != D && best_forward_x_values[(k - 1) % x_values_len] < best_forward_x_values[(k + 1) % x_values_len])) {
                        x = best_forward_x_values[(k + 1) % x_values_len];
                    } else {
                        x = best_forward_x_values[(k - 1) % x_values_len] + 1;
                    }

                    y = x - k;
                    x_initial = x;
                    y_initial = y;

                    while (x < lhs_size && y < rhs_size && lhs_seq[x] == rhs_seq[y]) { // evaluate diagonals
                        x++;
                        y++;
                    }

                    best_forward_x_values[k % x_values_len] = x;
                    z = -(k - w);

                    if (max_len % 2 == 1 && z >= -(D - 1) && z <= D - 1 && best_forward_x_values[k % x_values_len] + best_backward_x_values[z % x_values_len] >= lhs_size) {
                        D = 2 * D - 1;
                        u = x;
                        v = y;
                        x = x_initial;
                        y = y_initial;

                        if (D > 1 || (x != u && y != v)) {
                            // forward subdiff
                            _diff(
                                sequence_view(lhs_seq, 0, x),
                                sequence_view(rhs_seq, 0, y),
                                patches
                            );

                            // backward subdiff
                            _diff(
                                sequence_view(lhs_seq, u, lhs_size),
                                sequence_view(rhs_seq, v, rhs_size),
                                patches
                            );

                            return;
                        }
                        else if (rhs_size > lhs_size) {
                            _diff(sequence_view<t_sequenced_type>(), sequence_view(rhs_seq, lhs_size, rhs_size), patches);
                            return;
                        } else if (rhs_size < lhs_size) {
                            _diff(sequence_view(lhs_seq, rhs_size, lhs_size), sequence_view<t_sequenced_type>(), patches);
                            return;
                        } else {
                            return;
                        }
                    }
                }

                // backward snake
                for (int32_t k = -(D - 2 * std::max(0, D - rhs_size)); k < D - 2 * std::max(0, D - lhs_size) + 1; k += 2) {
                    if (k == -D || (k != D && best_backward_x_values[(k - 1) % x_values_len] < best_backward_x_values[(k + 1) % x_values_len])) {
                        x = best_backward_x_values[(k + 1) % x_values_len];
                    } else {
                        x = best_backward_x_values[(k - 1) % x_values_len] + 1;
                    }

                    y = x - k;
                    x_initial = x;
                    y_initial = y;

                    while (x < lhs_size && y < rhs_size && lhs_seq[lhs_size - x - 1] == rhs_seq[rhs_size - y - 1]) { // evaluate diagonals
                        x++;
                        y++;
                    }

                    best_backward_x_values[k % x_values_len] = x;
                    z = -(k - w);

                    if (max_len % 2 == 0 && z >= -D && z <= D && best_backward_x_values[k % x_values_len] + best_forward_x_values[z % x_values_len] >= lhs_size) {
                        D = 2 * D;
                        u = lhs_size - x_initial;
                        v = rhs_size - y_initial;
                        x = lhs_size - x;
                        y = rhs_size - y;

                        if (D > 1 || (x != u && y != v)) {
                            // forward subdiff
                            _diff(
                                sequence_view(lhs_seq, 0, x),
                                sequence_view(rhs_seq, 0, y),
                                patches
                            );

                            // backward subdiff
                            _diff(
                                sequence_view(lhs_seq, u, lhs_size),
                                sequence_view(rhs_seq, v, rhs_size),
                                patches
                            );

                            return;
                        } else if (rhs_size > lhs_size) {
                            _diff(sequence_view<t_sequenced_type>(), sequence_view(rhs_seq, lhs_size, rhs_size), patches);
                            return;
                        } else if (rhs_size < lhs_size) {
                            _diff(sequence_view(lhs_seq, rhs_size, lhs_size), sequence_view<t_sequenced_type>(), patches);
                            return;
                        } else {
                            return;
                        }
                    }
                }
            }
        }  else if (lhs_size > 0) {
            patches.push_back(Patch::make_deletion(lhs_seq));
            return;
        } else if (rhs_size > 0) {
            patches.push_back(Patch::make_addition(rhs_seq));
            return;
        }
    }

  public:
    Diff() { }

    const auto diff(const std::string& lhs, const std::string& rhs) {
        std::vector<Patch> patches;
        _diff(
            sequence_view(lhs.c_str(), 0, lhs.size()),
            sequence_view(rhs.c_str(), 0, rhs.size()),
            patches
        );
        patches.shrink_to_fit();
        return patches;
    }
};