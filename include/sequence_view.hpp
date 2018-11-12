#pragma once
#include <cstddef>

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