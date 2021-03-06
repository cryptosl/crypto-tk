//
// libsse_crypto - An abstraction layer for high level cryptographic features.
// Copyright (C) 2015-2017 Raphael Bost
//
// This file is part of libsse_crypto.
//
// libsse_crypto is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// libsse_crypto is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with libsse_crypto.  If not, see <http://www.gnu.org/licenses/>.
//

#include <sse/crypto/random.hpp>
#include <sse/crypto/rcprf.hpp>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

#include "gtest/gtest.h"

constexpr size_t kRCPrfKeySize = 32;

TEST(rc_prf, parameters)
{
    // leaf count
    EXPECT_EQ(sse::crypto::RCPrfParams::max_leaf_index(0), 0);
    constexpr sse::crypto::RCPrfParams::depth_type max_depth = ~0;
    for (sse::crypto::RCPrfParams::depth_type i
         = sse::crypto::RCPrfParams::kMaxHeight;
         i < max_depth;
         i++) {
        ASSERT_EQ(sse::crypto::RCPrfParams::max_leaf_index(i), ~0UL);
    }
    ASSERT_EQ(sse::crypto::RCPrfParams::max_leaf_index(max_depth), ~0UL);

    // range intersection
    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(4, 7, 3, 8));
    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(4, 7, 5, 8));
    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(4, 7, 3, 6));
    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(4, 7, 5, 6));
    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(3, 8, 4, 7));
    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(5, 8, 4, 7));
    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(3, 6, 4, 7));
    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(5, 6, 4, 7));

    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(4, 7, 3, 7));
    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(4, 7, 4, 8));
    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(4, 7, 4, 6));
    EXPECT_TRUE(sse::crypto::RCPrfParams::ranges_intersect(4, 7, 5, 7));

    EXPECT_FALSE(sse::crypto::RCPrfParams::ranges_intersect(4, 7, 8, 9));
    EXPECT_FALSE(sse::crypto::RCPrfParams::ranges_intersect(4, 7, 1, 2));
    EXPECT_FALSE(sse::crypto::RCPrfParams::ranges_intersect(8, 9, 4, 7));
    EXPECT_FALSE(sse::crypto::RCPrfParams::ranges_intersect(1, 2, 4, 7));
}

TEST(rc_prf, constrain)
{
    constexpr uint8_t                  test_depth = 7;
    std::array<uint8_t, kRCPrfKeySize> k{
        {0x00}}; // fixed key for easy debugging and bug reproducing
    sse::crypto::RCPrf<16> rc_prf(sse::crypto::Key<kRCPrfKeySize>(k.data()),
                                  test_depth);

    for (uint64_t min = 0;
         min <= sse::crypto::RCPrfParams::max_leaf_index(test_depth);
         min++) {
        uint64_t maximal_range
            = sse::crypto::RCPrfParams::max_leaf_index(test_depth);
        if (min == 0) {
            // we cannot constrain the key to the range [0,
            // max_leaf_index(test_depth)]
            maximal_range--;
        }
        for (uint64_t max = min; max <= maximal_range; max++) {
            auto constrained_prf = rc_prf.constrain(min, max);
            for (uint64_t leaf = min; leaf <= max; leaf++) {
                auto out             = rc_prf.eval(leaf);
                auto out_constrained = constrained_prf.eval(leaf);
                ASSERT_EQ(out, out_constrained);
            }
        }
    }
}

TEST(rc_prf, double_constrain)
{
    constexpr uint8_t                  test_depth = 5;
    std::array<uint8_t, kRCPrfKeySize> k{
        {0x00}}; // fixed key for easy debugging and bug reproducing
    sse::crypto::RCPrf<16> rc_prf(sse::crypto::Key<kRCPrfKeySize>(k.data()),
                                  test_depth);

    for (uint64_t min = 0;
         min <= sse::crypto::RCPrfParams::max_leaf_index(test_depth);
         min++) {
        uint64_t maximal_range
            = sse::crypto::RCPrfParams::max_leaf_index(test_depth);
        if (min == 0) {
            // we cannot constrain the key to the range [0,
            // leaf_count(test_depth]
            maximal_range--;
        }
        for (uint64_t max = min; max <= maximal_range; max++) {
            auto constrained_prf = rc_prf.constrain(min, max);

            // reconstrain the PRF
            for (uint64_t subrange_min = min; subrange_min <= max;
                 subrange_min++) {
                for (uint64_t subrange_max = subrange_min; subrange_max <= max;
                     subrange_max++) {
                    auto reconstrained_prf
                        = constrained_prf.constrain(subrange_min, subrange_max);
                    for (uint64_t leaf = subrange_min; leaf <= subrange_max;
                         leaf++) {
                        auto out             = constrained_prf.eval(leaf);
                        auto out_constrained = reconstrained_prf.eval(leaf);
                        ASSERT_EQ(out, out_constrained);
                    }
                }
            }
        }
    }
}

// Exceptions that can be raised by using the normal APIs
TEST(rc_prf, eval_constrain_exceptions)
{
    constexpr uint8_t                  test_depth = 7;
    std::array<uint8_t, kRCPrfKeySize> k{{0x00}};
    sse::crypto::RCPrf<16> rc_prf(sse::crypto::Key<kRCPrfKeySize>(k.data()),
                                  test_depth);

    // Exceptions raised by RCPRF::eval
    EXPECT_THROW(rc_prf.eval(1UL << (test_depth + 1)), std::out_of_range);
    EXPECT_THROW(rc_prf.eval(1UL << test_depth), std::out_of_range);

    // Exceptions raised by RCPrf::constrain
    EXPECT_THROW(rc_prf.constrain(3, 2), std::invalid_argument);
    EXPECT_THROW(rc_prf.constrain(0, 1UL << test_depth), std::out_of_range);
    EXPECT_THROW(rc_prf.constrain(0, (1UL << (test_depth - 1)) - 1),
                 std::out_of_range);

    uint64_t range_min = 4, range_max = 9;
    auto     constrained_rc_prf = rc_prf.constrain(range_min, range_max);

    // Exceptions raised by ConstrainedRCPrf::eval
    EXPECT_THROW(constrained_rc_prf.eval(range_min - 1), std::out_of_range);
    EXPECT_THROW(constrained_rc_prf.eval(range_max + 1), std::out_of_range);

    // Exceptions raised by ConstrainedRCPrfLeafElement::eval
    std::array<uint8_t, 16> buffer = sse::crypto::random_bytes<uint8_t, 16>();
    sse::crypto::ConstrainedRCPrfLeafElement<16> leaf(buffer, test_depth, 1);
    EXPECT_THROW(leaf.eval(0), std::out_of_range);
    EXPECT_THROW(leaf.eval(2), std::out_of_range);

    // Exceptions raised by ConstrainedRCPrfInnerElement::eval
    range_min              = 4;
    range_max              = 7;
    uint8_t subtree_height = 3;

    sse::crypto::ConstrainedRCPrfInnerElement<16> elt(
        sse::crypto::Key<kRCPrfKeySize>(),
        test_depth,
        subtree_height,
        range_min,
        range_max);
    EXPECT_THROW(elt.eval(range_min - 1), std::out_of_range);
    EXPECT_THROW(elt.eval(range_max + 1), std::out_of_range);
}

// Exceptions that can be raised when re-constaining an already constrained
// RC-PRF
TEST(rc_prf, reconstrain_exceptions)
{
    constexpr uint8_t      test_depth = 7;
    sse::crypto::RCPrf<16> rc_prf(sse::crypto::Key<kRCPrfKeySize>(),
                                  test_depth);

    uint64_t range_min = 4;
    uint64_t range_max = 7;

    auto constrained_prf = rc_prf.constrain(range_min, range_max);

    EXPECT_THROW(constrained_prf.constrain(range_min, range_max + 1),
                 std::out_of_range);
    EXPECT_THROW(constrained_prf.constrain(range_min - 1, range_max),
                 std::out_of_range);


    // Test the inner node exception
    range_min = sse::crypto::RCPrfParams::max_leaf_index(test_depth - 2) + 1;
    range_max
        = 2 * sse::crypto::RCPrfParams::max_leaf_index(test_depth - 2) + 1;
    std::vector<std::unique_ptr<sse::crypto::ConstrainedRCPrfElement<16>>>
        constrained_elements;

    sse::crypto::ConstrainedRCPrfInnerElement<16> elt(
        sse::crypto::Key<kRCPrfKeySize>(),
        test_depth,
        test_depth - 2,
        range_min,
        range_max);


    EXPECT_THROW(elt.generate_constrained_subkeys(
                     range_min - 1, range_max, constrained_elements),
                 std::out_of_range);
    EXPECT_THROW(elt.generate_constrained_subkeys(
                     range_min, range_max + 1, constrained_elements),
                 std::out_of_range);

    sse::crypto::ConstrainedRCPrfLeafElement<16> leaf(
        std::array<uint8_t, 16>(), test_depth, 1);
    EXPECT_THROW(leaf.generate_constrained_subkeys(0, 1, constrained_elements),
                 std::out_of_range);
    EXPECT_THROW(leaf.generate_constrained_subkeys(1, 2, constrained_elements),
                 std::out_of_range);
}

// Exceptions raised by the constructors
TEST(rc_prf, constructors_exceptions)
{
    // Exceptions raised the RCPrf constructor

    EXPECT_THROW(
        sse::crypto::RCPrf<16> rc_prf(sse::crypto::Key<kRCPrfKeySize>(), 0),
        std::invalid_argument);

    EXPECT_THROW(
        sse::crypto::RCPrf<16> rc_prf(sse::crypto::Key<kRCPrfKeySize>(), 70),
        std::invalid_argument);

    // Exceptions raised the ConstrainedRCPrfInnerElement constructor

    constexpr uint64_t range_min      = 0;
    constexpr uint64_t range_max      = 3;
    constexpr uint8_t  subtree_height = 3;
    constexpr uint8_t  tree_height    = subtree_height + 1;
    static_assert(
        range_max - range_min
            == sse::crypto::RCPrfParams::max_leaf_index_generic(subtree_height),
        "The tested range and the subtree_height are not compatible");

    // min > max
    EXPECT_THROW(sse::crypto::ConstrainedRCPrfInnerElement<16> elt(
                     sse::crypto::Key<kRCPrfKeySize>(),
                     tree_height,
                     subtree_height,
                     range_max,
                     range_min),
                 std::invalid_argument);

    // subtree height <= 1
    EXPECT_THROW(sse::crypto::ConstrainedRCPrfInnerElement<16> elt(
                     sse::crypto::Key<kRCPrfKeySize>(),
                     tree_height,
                     1,
                     0, // the range and the height hav to be compatible
                     0),
                 std::invalid_argument);
    EXPECT_THROW(sse::crypto::ConstrainedRCPrfInnerElement<16> elt(
                     sse::crypto::Prg(sse::crypto::Key<kRCPrfKeySize>()),
                     tree_height,
                     1,
                     0, // the range and the height hav to be compatible
                     0),
                 std::invalid_argument);

    // subtree height >= tree height
    EXPECT_THROW(sse::crypto::ConstrainedRCPrfInnerElement<16> elt(
                     sse::crypto::Key<kRCPrfKeySize>(),
                     subtree_height,
                     tree_height,
                     range_min,
                     range_max),
                 std::invalid_argument);

    // range and tree height are not matching
    EXPECT_THROW(sse::crypto::ConstrainedRCPrfInnerElement<16> elt(
                     sse::crypto::Key<kRCPrfKeySize>(),
                     tree_height,
                     subtree_height - 1,
                     range_min,
                     range_max),
                 std::invalid_argument);


    // Exceptions raised by the ConstrainedRCPrf constructor
    std::vector<std::unique_ptr<sse::crypto::ConstrainedRCPrfElement<16>>>
        empty_vec;

    EXPECT_THROW(sse::crypto::ConstrainedRCPrf<16> cprf(std::move(empty_vec)),
                 std::invalid_argument);


    std::vector<std::unique_ptr<sse::crypto::ConstrainedRCPrfElement<16>>>
        leaf_vec;
    leaf_vec.emplace_back(new sse::crypto::ConstrainedRCPrfLeafElement<16>(
        std::array<uint8_t, 16>(), tree_height, 0));
    leaf_vec.emplace_back(new sse::crypto::ConstrainedRCPrfLeafElement<16>(
        std::array<uint8_t, 16>(), tree_height, 4));

    EXPECT_THROW(sse::crypto::ConstrainedRCPrf<16> cprf(std::move(leaf_vec)),
                 std::invalid_argument);


    leaf_vec.empty();
    leaf_vec.emplace_back(new sse::crypto::ConstrainedRCPrfLeafElement<16>(
        std::array<uint8_t, 16>(), tree_height, 0));
    leaf_vec.emplace_back(new sse::crypto::ConstrainedRCPrfLeafElement<16>(
        std::array<uint8_t, 16>(), tree_height + 1, 1));
    EXPECT_THROW(sse::crypto::ConstrainedRCPrf<16> cprf(std::move(leaf_vec)),
                 std::invalid_argument);
}