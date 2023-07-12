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

extern "C" {

barretenberg::polynomial create_input(size_t exponent)
{
    size_t n = 1 << exponent;
    auto coeffs = polynomial(n + 1);
    for (size_t i = 0; i < n; ++i) {
        coeffs[i] = barretenberg::fr::random_element();
    }

    return coeffs;
}

std::shared_ptr<barretenberg::srs::factories::ProverCrs<curve::BN254>> create_prover_crs(size_t n)
{
    auto prover_crs = create_prover_factory();
    return prover_crs->get_prover_crs(n);
}

void commit(barretenberg::polynomial input,
            size_t n,
            std::shared_ptr<barretenberg::srs::factories::ProverCrs<curve::BN254>> crs)
{
    transcript::StandardTranscript inp_tx = transcript::StandardTranscript(transcript::Manifest());
    plonk::KateCommitmentScheme<turbo_settings> newKate;

    auto circuit_proving_key = std::make_shared<proving_key>(n, 0, crs, ComposerType::STANDARD);
    work_queue queue(circuit_proving_key.get(), &inp_tx);

    newKate.commit(input.data(), "F_COMM", n, queue);
    queue.process_queue();
}
}
