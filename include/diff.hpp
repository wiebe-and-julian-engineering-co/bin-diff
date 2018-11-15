#pragma once

#include <vector>
#include <string>

#include "patch.hpp"
#include "sequence_view.hpp"

namespace diff {
    template <class t_sequenced_type>
    class _diff {
        const sequence_view<t_sequenced_type>& lhs_seq;
        const sequence_view<t_sequenced_type>& rhs_seq;
        std::vector<patch>& patches;
        // int32_t lhs_size = lhs_seq.size();
        // int32_t rhs_size = rhs_seq.size();
        // int32_t max_len = lhs_size + rhs_size;
        // int32_t x_values_len = 2 * std::min(lhs_size, rhs_size) + 2;
        int32_t lhs_size;
        int32_t rhs_size;
        int32_t max_len;
        int32_t x_values_len;

        int32_t x = 0;
        int32_t y = 0;
        int32_t z = 0;
        int32_t x_initial = 0;
        int32_t y_initial = 0;
        int32_t u = 0;
        int32_t v = 0;
        int32_t w = 0;

    public:
        /**
         * @brief Optimal diff algorithm
         * 
         * @details
         * Based off of Robert Elder's optimized Myer's diff
         * See more: http://blog.robertelder.org/diff-algorithm/
         * 
         * @tparam t_sequenced_type Type contained in the sequence
         * @param lhs_seq Source sequence
         * @param rhs_seq Destination sequence to generate patches to
         * @param patches patches holding vector
         */
        _diff(const sequence_view<t_sequenced_type>& lhs_seq, const sequence_view<t_sequenced_type>& rhs_seq, std::vector<patch>& patches) :
            lhs_seq(lhs_seq),
            rhs_seq(rhs_seq),
            patches(patches),
            lhs_size(lhs_seq.size()),
            rhs_size(rhs_seq.size()),
            max_len(lhs_size + rhs_size),
            x_values_len(2 * std::min(lhs_size, rhs_size) + 2)
        {
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
                patches.push_back(patch::make_deletion(lhs_seq));
                return;
            } else if (rhs_size > 0) {
                patches.push_back(patch::make_addition(rhs_seq));
                return;
            }
        }
    };

    /**
     * @brief Optimal diff algorithm
     * 
     * @details
     * Based off of Robert Elder's optimized Myer's diff
     * See more: http://blog.robertelder.org/diff-algorithm/
     * 
     * @param lhs_seq Source sequence
     * @param rhs_seq Destination sequence to generate patches to
     * @return std::vector<patch> Vector of calculated patches
     */
    inline const auto diff(const std::string& lhs, const std::string& rhs) {
        std::vector<patch> patches;
        _diff(
            sequence_view(lhs.c_str(), 0, lhs.size()),
            sequence_view(rhs.c_str(), 0, rhs.size()),
            patches
        );
        patches.shrink_to_fit();
        return patches;
    }
}