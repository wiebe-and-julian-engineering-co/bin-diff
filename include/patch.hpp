#pragma once
#include <string>

#include "sequence_view.hpp"

enum class patch_operation {
    Addition,
    Deletion
};

/**
 * @brief patch
 * 
 * @details
 * Patches are differences between sequences. If there is a patch between
 * sequence `A` and sequence `B`, applying a patch onto sequence `A` will,
 * possibly partially, turn sequence `A` into `B`
 */
class patch {
    patch_operation m_op;
    size_t m_begin;
    std::string m_seq;

    patch(patch_operation op, size_t begin, std::string&& str) :
        m_op(op),
        m_begin(begin),
        m_seq(str)
    { }

    patch(patch_operation op, size_t begin, const std::string& str) :
        m_op(op),
        m_begin(begin),
        m_seq(str)
    { }

public:
    template <class t_sequenced_type>
    static patch make_addition(const sequence_view<t_sequenced_type>& seq) {
        auto str = std::string(std::cbegin(seq), seq.size());
        return patch(patch_operation::Addition, seq.index_begin(), std::move(str));
    }

    template <class t_sequenced_type>
    static patch make_deletion(const sequence_view<t_sequenced_type>& seq) {
        auto str = std::string(std::cbegin(seq), seq.size());
        return patch(patch_operation::Deletion, seq.index_begin(), std::move(str));
    }

    patch_operation GetOperation() {
        return m_op;
    }

    template <class t_os>
    friend t_os& operator<< (t_os& os, const patch& p) {
        os << "Patch ";

        if (p.m_op == patch_operation::Addition) {
            os << "Addition";
        } else if (p.m_op == patch_operation::Deletion) {
            os << "Deletion";
        } else {
            os << "Undefined";
        }

        os << ": [" << p.m_begin << ':' << p.m_begin + p.m_seq.size() << "] - " << p.m_seq;

        return os;
    }
};