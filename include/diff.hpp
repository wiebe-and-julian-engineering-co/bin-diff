#pragma once

#include <cstdint>

#include <vector>
#include <memory>
#include <list>
#include <string>
#include <algorithm>
#include <iterator>

class sequence_view {
    const char* m_str;
    size_t m_view_index_begin;
    size_t m_view_index_end;

public:
    sequence_view() : m_str(nullptr), m_view_index_begin(0), m_view_index_end(0) { }

    sequence_view(const char* str, uint32_t begin, uint32_t end) : m_str(str), m_view_index_begin(begin), m_view_index_end(end) { }

    sequence_view(const sequence_view& seq, uint32_t begin, uint32_t end) :
        m_str(seq.m_str),
        m_view_index_begin(seq.m_view_index_begin + begin),
        m_view_index_end(seq.m_view_index_begin + end)
    { }

    sequence_view combine(const sequence_view& other) const {
        if (m_str != other.m_str) {
            // Cannot combine sequences with different super sequences
            throw std::exception();
        } else if (m_view_index_end >= other.m_view_index_begin || other.m_view_index_end >= m_view_index_begin) {
            return sequence_view(m_str,
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

    char operator[] (size_t i) const {
        return m_str[m_view_index_begin + i];
    }

    size_t index_begin() const {
        return m_view_index_begin;
    }
    
    size_t index_end() const {
        return m_view_index_end;
    }

    const char* begin() const noexcept {
        return &m_str[m_view_index_begin];
    }
    
    const char* end() const noexcept {
        return &m_str[m_view_index_end];
    }

    size_t size() const {
        return m_view_index_end - m_view_index_begin;
    }

    template <class t_os>
    friend t_os& operator<< (t_os& os, const sequence_view& seq) {
        for (size_t i = seq.m_view_index_begin; i < seq.m_view_index_end; i++) {
            os << seq.m_str[i];
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
    std::string m_str;

    Patch(PatchOperation op, size_t begin, std::string&& str) :
        m_op(op),
        m_begin(begin),
        m_str(str)
    { }

    Patch(PatchOperation op, size_t begin, const std::string& str) :
        m_op(op),
        m_begin(begin),
        m_str(str)
    { }
  public:

    static Patch make_addition(const sequence_view& seq) {
        auto str = std::string(std::cbegin(seq), seq.size());
        return Patch(PatchOperation::Addition, seq.index_begin(), std::move(str));
    }

    static Patch make_deletion(const sequence_view& seq) {
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

        os << ": [" << p.m_begin << ':' << p.m_begin + p.m_str.size() << "] - " << p.m_str;

        return os;
    }
};

class Diff {
    std::vector<Patch> _diff(const sequence_view& lhs_seq, const sequence_view& rhs_seq) const {
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
                            auto temp = std::vector<Patch>(); 

                            std::vector<Patch> forward_subdiff = _diff(
                                sequence_view(lhs_seq, 0, x),
                                sequence_view(rhs_seq, 0, y)
                            );

                            std::vector<Patch> backward_subdiff = _diff(
                                sequence_view(lhs_seq, u, lhs_size),
                                sequence_view(rhs_seq, v, rhs_size)
                            );
                            
                            temp.reserve(forward_subdiff.size() + backward_subdiff.size());

                            std::copy(forward_subdiff.begin(), forward_subdiff.end(), std::back_inserter(temp));
                            std::copy(backward_subdiff.begin(), backward_subdiff.end(), std::back_inserter(temp));

                            return temp;
                        }
                        else if (rhs_size > lhs_size) {
                            return _diff(sequence_view(), sequence_view(rhs_seq, lhs_size, rhs_size));
                        } else if (rhs_size < lhs_size) {
                            return _diff(sequence_view(lhs_seq, rhs_size, lhs_size), sequence_view());
                        } else {
                            return { };
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
                            auto temp = std::vector<Patch>();

                            std::vector<Patch> forward_subdiff = _diff(
                                sequence_view(lhs_seq, 0, x),
                                sequence_view(rhs_seq, 0, y)
                            );

                            std::vector<Patch> backward_subdiff = _diff(
                                sequence_view(lhs_seq, u, lhs_size),
                                sequence_view(rhs_seq, v, rhs_size)
                            );

                            temp.reserve(forward_subdiff.size() + backward_subdiff.size());

                            std::copy(forward_subdiff.begin(), forward_subdiff.end(), std::back_inserter(temp));
                            std::copy(backward_subdiff.begin(), backward_subdiff.end(), std::back_inserter(temp));

                            return temp;
                        } else if (rhs_size > lhs_size) {
                            return _diff(sequence_view(), sequence_view(rhs_seq, lhs_size, rhs_size));
                        } else if (rhs_size < lhs_size) {
                            return _diff(sequence_view(lhs_seq, rhs_size, lhs_size), sequence_view());
                        } else {
                            return { };
                        }
                    }
                }
            }
        }  else if (lhs_size > 0) {
            return { Patch::make_deletion(lhs_seq) };
        } else {
            return { Patch::make_addition(rhs_seq) };
        }

        return { };
    }

  public:
    Diff() { }

    template <class t_type>
    static const std::string_view make_string_view(const t_type& s, size_t begin, size_t end) {
        return std::string_view(&s[begin], end - begin);
    }

    auto diff(const std::string& lhs, const std::string& rhs) {
        return _diff(sequence_view(lhs.c_str(), 0, lhs.size()), sequence_view(rhs.c_str(), 0, rhs.size()));
    }
};