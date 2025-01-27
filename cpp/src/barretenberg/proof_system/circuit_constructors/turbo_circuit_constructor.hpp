#pragma once
#include <array>
#include "circuit_constructor_base.hpp"
#include "barretenberg/proof_system/types/composer_type.hpp"
#include "barretenberg/proof_system/types/merkle_hash_type.hpp"
#include "barretenberg/proof_system/types/pedersen_commitment_type.hpp"

namespace proof_system {

inline std::vector<std::string> turbo_selector_names()
{
    std::vector<std::string> result{ "q_m", "q_c",     "q_1",          "q_2",     "q_3",    "q_4",
                                     "q_5", "q_arith", "q_fixed_base", "q_range", "q_logic" };
    return result;
}
class TurboCircuitConstructor : public CircuitConstructorBase<arithmetization::Turbo<barretenberg::fr>> {

  public:
    static constexpr ComposerType type = ComposerType::TURBO;
    static constexpr merkle::HashType merkle_hash_type = merkle::HashType::FIXED_BASE_PEDERSEN;
    static constexpr pedersen::CommitmentType commitment_type = pedersen::CommitmentType::FIXED_BASE_PEDERSEN;
    static constexpr size_t UINT_LOG2_BASE = 2;

    using WireVector = std::vector<uint32_t, barretenberg::ContainerSlabAllocator<uint32_t>>;
    using SelectorVector = std::vector<barretenberg::fr, barretenberg::ContainerSlabAllocator<barretenberg::fr>>;

    WireVector& w_l = std::get<0>(wires);
    WireVector& w_r = std::get<1>(wires);
    WireVector& w_o = std::get<2>(wires);
    WireVector& w_4 = std::get<3>(wires);

    SelectorVector& q_m = selectors.q_m;
    SelectorVector& q_c = selectors.q_c;
    SelectorVector& q_1 = selectors.q_1;
    SelectorVector& q_2 = selectors.q_2;
    SelectorVector& q_3 = selectors.q_3;
    SelectorVector& q_4 = selectors.q_4;
    SelectorVector& q_5 = selectors.q_5;
    SelectorVector& q_arith = selectors.q_arith;
    SelectorVector& q_fixed_base = selectors.q_fixed_base;
    SelectorVector& q_range = selectors.q_range;
    SelectorVector& q_logic = selectors.q_logic;

    TurboCircuitConstructor(const size_t size_hint = 0);
    // This constructor is needed to simplify switching between circuit constructor and composer
    TurboCircuitConstructor(std::string const&, const size_t size_hint = 0)
        : TurboCircuitConstructor(size_hint){};
    TurboCircuitConstructor(TurboCircuitConstructor&& other) = default;
    TurboCircuitConstructor& operator=(TurboCircuitConstructor&& other)
    {
        CircuitConstructorBase<arithmetization::Turbo<barretenberg::fr>>::operator=(std::move(other));
        constant_variable_indices = other.constant_variable_indices;
        return *this;
    };
    ~TurboCircuitConstructor() {}

    void create_add_gate(const add_triple& in);

    void create_big_add_gate(const add_quad& in);
    void create_big_add_gate_with_bit_extraction(const add_quad& in);
    void create_big_mul_gate(const mul_quad& in);
    void create_balanced_add_gate(const add_quad& in);

    void create_mul_gate(const mul_triple& in);
    void create_bool_gate(const uint32_t a);
    void create_poly_gate(const poly_triple& in);
    void create_fixed_group_add_gate(const fixed_group_add_quad& in);
    void create_fixed_group_add_gate_with_init(const fixed_group_add_quad& in, const fixed_group_init_quad& init);
    void create_fixed_group_add_gate_final(const add_quad& in);
    void fix_witness(const uint32_t witness_index, const barretenberg::fr& witness_value);

    barretenberg::fr arithmetic_gate_evaluation(const size_t index, const barretenberg::fr alpha_base);
    barretenberg::fr fixed_base_gate_evaluation(const size_t index, const std::vector<barretenberg::fr>& alpha_powers);
    barretenberg::fr logic_gate_evaluation(const size_t index,
                                           const barretenberg::fr alpha_bas,
                                           const barretenberg::fr alpha);
    barretenberg::fr range_gate_evaluation(const size_t index,
                                           const barretenberg::fr alpha_bas,
                                           const barretenberg::fr alpha);

    bool lazy_arithmetic_gate_check(const size_t gate_index);
    bool lazy_fixed_base_gate_check(const size_t gate_index);
    bool lazy_logic_gate_check(const size_t gate_index);
    bool lazy_range_gate_check(const size_t gate_index);

    bool check_circuit();

    std::vector<uint32_t> decompose_into_base4_accumulators(const uint32_t witness_index,
                                                            const size_t num_bits,
                                                            std::string const& msg);

    void create_range_constraint(const uint32_t variable_index, const size_t num_bits, std::string const& msg)
    {
        decompose_into_base4_accumulators(variable_index, num_bits, msg);
    }

    accumulator_triple create_logic_constraint(const uint32_t a,
                                               const uint32_t b,
                                               const size_t num_bits,
                                               bool is_xor_gate);
    accumulator_triple create_and_constraint(const uint32_t a, const uint32_t b, const size_t num_bits);
    accumulator_triple create_xor_constraint(const uint32_t a, const uint32_t b, const size_t num_bits);

    uint32_t put_constant_variable(const barretenberg::fr& variable);

    size_t get_num_constant_gates() const { return 0; }

    void assert_equal_constant(const uint32_t a_idx,
                               const barretenberg::fr& b,
                               std::string const& msg = "assert_equal_constant")
    {
        if (variables[a_idx] != b && !failed()) {
            failure(msg);
        }
        auto b_idx = put_constant_variable(b);
        assert_equal(a_idx, b_idx, msg);
    }

    /**
     * For any type other than uint32_t (presumed to be a witness index), we call normalize first.
     */
    template <typename T>
    void assert_equal_constant(T const& in, const barretenberg::fr& b, std::string const& msg = "assert_equal_constant")
    {
        assert_equal_constant(in.normalize().witness_index, b, msg);
    }

    // these are variables that we have used a gate on, to enforce that they are equal to a defined value
    std::map<barretenberg::fr, uint32_t> constant_variable_indices;
};
} // namespace proof_system