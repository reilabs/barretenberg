#include <gtest/gtest.h>

#include "merkle_tree.hpp"
#include "membership.hpp"
#include "memory_store.hpp"
#include "memory_tree.hpp"

#include "barretenberg/plonk/composer/ultra_plonk_composer.hpp"
#include "barretenberg/stdlib/primitives/bool/bool.hpp"
#include "barretenberg/stdlib/primitives/field/field.hpp"
#include "barretenberg/stdlib/primitives/witness/witness.hpp"

namespace {
auto& engine = numeric::random::get_debug_engine();
}

namespace proof_system::stdlib_merkle_test {

using namespace barretenberg;
using namespace proof_system::plonk::stdlib::merkle_tree;
using namespace plonk::stdlib;

using Composer = proof_system::UltraCircuitConstructor;

using bool_ct = bool_t<Composer>;
using field_ct = field_t<Composer>;
using witness_ct = witness_t<Composer>;

TEST(stdlib_merkle_tree, test_check_membership)
{
    MemoryStore store;
    auto db = MerkleTree(store, 3);
    auto composer = Composer();

    // Check membership at index 0.
    auto zero = field_ct(witness_ct(&composer, fr::zero())).decompose_into_bits();
    field_ct root = witness_ct(&composer, db.root());
    bool_ct is_member =
        check_membership(root, create_witness_hash_path(composer, db.get_hash_path(0)), field_ct(0), zero);

    // Check membership at index 7 after inserting 1.
    db.update_element(7, 1);
    auto seven = field_ct(witness_ct(&composer, fr(7))).decompose_into_bits();
    root = witness_ct(&composer, db.root());
    bool_ct is_member_ =
        check_membership(root, create_witness_hash_path(composer, db.get_hash_path(1)), field_ct(1), seven);

    printf("composer gates = %zu\n", composer.get_num_gates());

    bool result = composer.check_circuit();
    EXPECT_EQ(is_member.get_value(), true);
    EXPECT_EQ(is_member_.get_value(), true);
    EXPECT_EQ(result, true);
}

TEST(stdlib_merkle_tree, test_batch_update_membership)
{
    MemoryStore store;
    MerkleTree db(store, 4);
    auto composer = Composer();
    // Fill in an arbitrary value at i = 2.
    db.update_element(2, fr::random_element());
    // Define old state.
    field_ct old_root = witness_ct(&composer, db.root());
    auto old_hash_path_1 = create_witness_hash_path(composer, db.get_hash_path(5));
    auto old_hash_path_2 = create_witness_hash_path(composer, db.get_hash_path(7));
    auto values = std::vector<field_ct>(4);
    for (size_t i = 4; i < 8; i++) {
        values[i - 4] = field_ct(&composer, i * 2);
        db.update_element(i, fr(i * 2));
    }
    // Define new state. Batch update must verify with any old hash path in the subtree.
    field_ct new_root = witness_ct(&composer, db.root());
    field_ct start_idx = field_ct(witness_ct(&composer, fr(4)));
    batch_update_membership(new_root, old_root, old_hash_path_1, values, start_idx);
    batch_update_membership(new_root, old_root, old_hash_path_2, values, start_idx);
    printf("composer gates = %zu\n", composer.get_num_gates());
    bool result = composer.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(stdlib_merkle_tree, test_assert_check_membership)
{
    MemoryStore store;
    auto db = MerkleTree(store, 3);
    auto composer = Composer();

    auto zero = field_ct(witness_ct(&composer, fr::zero())).decompose_into_bits();
    field_ct root = witness_ct(&composer, db.root());

    assert_check_membership(root, create_witness_hash_path(composer, db.get_hash_path(0)), field_ct(0), zero);

    printf("composer gates = %zu\n", composer.get_num_gates());

    bool result = composer.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(stdlib_merkle_tree, test_assert_check_membership_fail)
{
    MemoryStore store;
    auto db = MerkleTree(store, 3);

    auto composer = Composer();

    auto zero = field_ct(witness_ct(&composer, fr::zero())).decompose_into_bits();
    field_ct root = witness_ct(&composer, db.root());

    assert_check_membership(root, create_witness_hash_path(composer, db.get_hash_path(0)), field_ct(1), zero);

    printf("composer gates = %zu\n", composer.get_num_gates());

    bool result = composer.check_circuit();
    EXPECT_EQ(result, false);
}
// To test whether both old hash path and new hash path works for the same Merkle tree
TEST(stdlib_merkle_tree, test_update_members)
{
    {
        MemoryStore store;
        auto db = MerkleTree(store, 3);

        auto composer = Composer();

        auto zero = field_ct(witness_ct(&composer, fr::zero())).decompose_into_bits();

        auto old_value = field_ct(0);
        hash_path<Composer> old_path = create_witness_hash_path(composer, db.get_hash_path(0));
        field_ct old_root = witness_ct(&composer, db.root());

        auto new_value = field_ct(1);
        auto new_path_fr = get_new_hash_path(db.get_hash_path(0), 0, new_value.get_value());
        hash_path<Composer> new_path = create_witness_hash_path(composer, new_path_fr);
        field_ct new_root = witness_ct(&composer, get_hash_path_root(new_path_fr));

        update_membership(new_root, new_value, old_root, old_path, old_value, zero);

        printf("composer gates = %zu\n", composer.get_num_gates());

        bool result = composer.check_circuit();
        EXPECT_EQ(result, true);
    }
    {
        MemoryStore store;
        auto db = MerkleTree(store, 3);

        auto composer = Composer();

        auto zero = field_ct(witness_ct(&composer, fr::zero())).decompose_into_bits();

        auto old_value = field_ct(0);
        hash_path<Composer> old_path = create_witness_hash_path(composer, db.get_hash_path(0));
        field_ct old_root = witness_ct(&composer, db.root());

        auto new_value = field_ct(1);
        auto new_path_fr = get_new_hash_path(db.get_hash_path(0), 0, new_value.get_value());
        hash_path<Composer> new_path = create_witness_hash_path(composer, new_path_fr);
        field_ct new_root = witness_ct(&composer, get_hash_path_root(new_path_fr));

        update_membership(new_root, new_value, old_root, new_path, old_value, zero);

        printf("composer gates = %zu\n", composer.get_num_gates());

        bool result = composer.check_circuit();
        EXPECT_EQ(result, true);
    }
}

TEST(stdlib_merkle_tree, test_tree)
{
    size_t depth = 3;
    size_t num = 1UL << depth;
    MemoryStore store;
    MerkleTree db(store, depth);
    MemoryTree mem_tree(depth);

    auto composer = Composer();

    auto zero_field = field_ct(witness_ct(&composer, fr::zero()));
    auto values = std::vector<field_ct>(num, zero_field);
    auto root = field_ct(&composer, mem_tree.root());

    assert_check_tree(root, values);

    printf("composer gates = %zu\n", composer.get_num_gates());

    bool result = composer.check_circuit();
    EXPECT_EQ(result, true);
}

TEST(stdlib_merkle_tree, test_update_memberships)
{
    constexpr size_t depth = 4;
    MemoryStore store;
    MerkleTree tree(store, depth);

    auto composer = Composer();

    constexpr size_t filled = (1UL << depth) / 2;
    std::vector<fr> filled_values;
    for (size_t i = 0; i < filled; i++) {
        uint256_t val = fr::random_element();
        tree.update_element(i, val);
        filled_values.push_back(val);
    }

    // old state
    fr old_root = tree.root();
    std::vector<size_t> old_indices = { 0, 2, 5, 7 };

    std::vector<fr> old_values;
    std::vector<fr_hash_path> old_hash_paths;
    for (size_t i = 0; i < old_indices.size(); i++) {
        old_values.push_back(filled_values[old_indices[i]]);
    }

    // new state
    std::vector<fr> new_values;
    std::vector<fr> new_roots;
    for (size_t i = 0; i < old_indices.size(); i++) {
        uint256_t val = fr::random_element();
        new_values.push_back(val);
        old_hash_paths.push_back(tree.get_hash_path(old_indices[i]));
        new_roots.push_back(tree.update_element(old_indices[i], new_values[i]));
    }

    // old state circuit types
    field_ct old_root_ct = witness_ct(&composer, old_root);
    std::vector<bit_vector<Composer>> old_indices_ct;
    std::vector<field_ct> old_values_ct;
    std::vector<hash_path<Composer>> old_hash_paths_ct;

    // new state circuit types
    std::vector<field_ct> new_values_ct;
    std::vector<field_ct> new_roots_ct;

    for (size_t i = 0; i < old_indices.size(); i++) {
        auto idx_vec = field_ct(witness_ct(&composer, uint256_t(old_indices[i]))).decompose_into_bits(depth);
        old_indices_ct.push_back(idx_vec);
        old_values_ct.push_back(witness_ct(&composer, old_values[i]));
        old_hash_paths_ct.push_back(create_witness_hash_path(composer, old_hash_paths[i]));

        new_values_ct.push_back(witness_ct(&composer, new_values[i]));
        new_roots_ct.push_back(witness_ct(&composer, new_roots[i]));
    }

    update_memberships(old_root_ct, new_roots_ct, new_values_ct, old_values_ct, old_hash_paths_ct, old_indices_ct);

    printf("composer gates = %zu\n", composer.get_num_gates());
    bool result = composer.check_circuit();
    EXPECT_EQ(result, true);
}
} // namespace proof_system::stdlib_merkle_test