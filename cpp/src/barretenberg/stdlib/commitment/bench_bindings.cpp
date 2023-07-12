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

std::shared_ptr<srs::factories::MemCrsFactory> create_prover_factory()
{
    auto g2_point = barretenberg::g2::one * barretenberg::fr::random_element();

    std::vector<barretenberg::g1::affine_element> g1_points;

    auto scalar = barretenberg::fr::random_element();
    const auto element = barretenberg::g1::affine_element(barretenberg::g1::one * scalar);
    g1_points.push_back(element);

    return std::make_shared<srs::factories::MemCrsFactory>(srs::factories::MemCrsFactory(g1_points, g2_point));
}

barretenberg::polynomial* br_fr_to_poly(std::vector<barretenberg::fr>* input)
{
    barretenberg::polynomial* coeffs = new barretenberg::Polynomial<barretenberg::fr>(*input);

    delete input;
    return coeffs;
}

extern "C" {

std::vector<barretenberg::fr>* create_input(size_t exponent)
{
    size_t n = 1 << exponent;
    auto elements = new std::vector<barretenberg::fr>();
    for (size_t i = 0; i < n; ++i) {
        elements->push_back(barretenberg::fr::random_element());
    }

    return elements;
}

barretenberg::srs::factories::ProverCrs<curve::BN254>* create_prover_crs(size_t n)
{
    auto prover_crs = create_prover_factory();
    return prover_crs->get_prover_crs(n).get();
}

void commit(std::vector<barretenberg::fr>* input, size_t n, barretenberg::srs::factories::ProverCrs<curve::BN254>* crs)
{
    auto coeffs = br_fr_to_poly(input);
    auto crs_sp = std::make_shared<MemProverCrs>(crs);

    transcript::StandardTranscript inp_tx = transcript::StandardTranscript(transcript::Manifest());
    plonk::KateCommitmentScheme<turbo_settings> newKate;

    auto circuit_proving_key = std::make_shared<proving_key>(n, 0, crs, ComposerType::STANDARD);
    work_queue queue(circuit_proving_key.get(), &inp_tx);

    newKate.commit(coeffs->data(), "F_COMM", n, queue);
    queue.process_queue();
    delete coeffs;
}
}
