#include "bench_bindings.hpp"

#include <barretenberg/plonk/proof_system/types/proof.hpp>
#include <barretenberg/plonk/proof_system/proving_key/serialize.hpp>
#include <barretenberg/srs/global_crs.hpp>
#include "barretenberg/ecc/scalar_multiplication/point_table.hpp"
#include "barretenberg/ecc/scalar_multiplication/scalar_multiplication.hpp"

using namespace barretenberg::srs;
using namespace barretenberg;
using namespace grumpkin;

class MemProverCrs : public srs::factories::ProverCrs<curve::BN254> {
  public:
    MemProverCrs(std::vector<barretenberg::g1::affine_element> const& points)
        : num_points(points.size())
    {
        monomials_ = scalar_multiplication::point_table_alloc<barretenberg::g1::affine_element>(num_points);
        std::copy(points.begin(), points.end(), monomials_.get());
        scalar_multiplication::generate_pippenger_point_table<curve::BN254>(
            monomials_.get(), monomials_.get(), num_points);
    }

    barretenberg::g1::affine_element* get_monomial_points() override { return monomials_.get(); }

    size_t get_monomial_size() const override { return num_points; }

  private:
    size_t num_points;
    std::shared_ptr<barretenberg::g1::affine_element[]> monomials_;
};

srs::factories::MemCrsFactory* create_prover_factory()
{
    auto g2_point = barretenberg::g2::one * barretenberg::fr::random_element();

    std::vector<barretenberg::g1::affine_element> g1_points;

    for (int i = 0; i < 4; i++) {
        auto scalar = barretenberg::fr::random_element();
        const auto element = barretenberg::g1::affine_element(barretenberg::g1::one * scalar);
        g1_points.push_back(element);
    }

    return new srs::factories::MemCrsFactory(g1_points, g2_point);
}

constexpr size_t get_circuit_size(const size_t target_count_base)
{
    constexpr size_t base_gates = 2;
    constexpr size_t gates_per_hash = 262;
    return (target_count_base - base_gates) / gates_per_hash;
}

extern "C" {

std::vector<grumpkin::fq>* create_input(size_t n)
{
    auto elements = new std::vector<grumpkin::fq>();
    for (size_t i = 0; i < n; ++i) {
        elements->push_back(grumpkin::fq::random_element());
    }

    return elements;
}

void commit(std::vector<grumpkin::fq>* input, size_t n, srs::factories::MemCrsFactory* prover_factory)
{

    // auto crs = prover_factory->get_prover_crs(n);
    // auto coeffs = br_fr_to_poly(*input);

    // transcript::StandardTranscript inp_tx = transcript::StandardTranscript(transcript::Manifest());
    // plonk::KateCommitmentScheme<turbo_settings> newKate;

    // auto circuit_proving_key = std::make_shared<proving_key>(n, 0, crs, ComposerType::STANDARD);
    // work_queue queue(circuit_proving_key.get(), &inp_tx);

    // newKate.commit(coeffs->data(), "F_COMM", n, queue);
    // queue.process_queue();
    // delete coeffs;

    grumpkin::fq left = grumpkin::fq::random_element();
    grumpkin::fq out = grumpkin::fq::random_element();
    for (size_t i = 0; i < n; ++i) {
        out = crypto::pedersen_commitment::compress_native({ left, out });
    }

    // crypto::pedersen_commitment::compress_native(*input, 0);
}
}
