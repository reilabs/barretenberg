#include "bench_bindings.hpp"

#include <barretenberg/plonk/proof_system/types/proof.hpp>
#include <barretenberg/plonk/proof_system/proving_key/serialize.hpp>
#include <barretenberg/srs/global_crs.hpp>

using namespace barretenberg::srs;
using namespace grumpkin;

std::shared_ptr<barretenberg::srs::factories::CrsFactory> create_prover_factory()
{
    auto g2_point = barretenberg::g2::one * barretenberg::fr::random_element();

    std::vector<barretenberg::g1::affine_element> g1_points;

    auto scalar = barretenberg::fr::random_element();
    const auto element = barretenberg::g1::affine_element(barretenberg::g1::one * scalar);
    g1_points.push_back(element);

    init_crs_factory(g1_points, g2_point);
    return get_crs_factory();
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

    std::shared_ptr<barretenberg::polynomial> coeffs(br_fr_to_poly(input));

    transcript::StandardTranscript inp_tx = transcript::StandardTranscript(transcript::Manifest());
    plonk::KateCommitmentScheme<turbo_settings> newKate;

    auto circuit_proving_key = std::make_shared<proving_key>(n, 0, crs, ComposerType::STANDARD);
    work_queue queue(circuit_proving_key.get(), &inp_tx);

    newKate.commit(coeffs->data(), "F_COMM", n, queue);
    queue.process_queue();
}
}
