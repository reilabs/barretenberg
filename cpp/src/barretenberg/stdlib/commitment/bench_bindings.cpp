#include "bench_bindings.hpp"

#include <barretenberg/plonk/proof_system/types/proof.hpp>
#include <barretenberg/plonk/proof_system/proving_key/serialize.hpp>
#include <barretenberg/srs/global_crs.hpp>

using namespace barretenberg::srs;

std::shared_ptr<barretenberg::srs::factories::CrsFactory> create_prover_factory()
{
    auto g2_point = barretenberg::g2::one * barretenberg::fr::random_element();

    auto g1_points = new std::vector<barretenberg::g1::affine_element>();

    auto scalar = barretenberg::fr::random_element();
    const auto element = barretenberg::g1::affine_element(barretenberg::g1::one * scalar);
    g1_points->emplace_back(element);

    init_crs_factory(*g1_points, g2_point);
    return get_crs_factory();
}

void build_circuit(UltraPlonkComposer& composer, size_t circuit_size)
{
    while (composer.get_num_gates() <= circuit_size / 2) {
        plonk::stdlib::pedersen_commitment<UltraPlonkComposer>::compress(field_ct(witness_ct(&composer, 1)),
                                                                         field_ct(witness_ct(&composer, 1)));
    }
}

extern "C" {

UltraPlonkComposer* create_composer(size_t circuit_size)
{
    auto composer = std::make_unique<UltraPlonkComposer>(create_prover_factory());
    build_circuit(*composer, circuit_size);

    if (composer->failed()) {
        std::string error = format("composer logic failed: ", composer->err());
        throw_or_abort(error);
    }

    composer->compute_proving_key();

    return composer.release();
}

void commit(UltraPlonkComposer* composer, size_t length)
{

    auto fields = new std::vector<plonk::stdlib::field_t<plonk::UltraPlonkComposer>>();

    std::cout << "hello 1" << std::endl;

    for (size_t i = 0; i < length; i++) {
        fields->push_back(plonk::stdlib::field_t<plonk::UltraPlonkComposer>(
            plonk::stdlib::witness_t(composer, barretenberg::fr::random_element())));
    }

    std::cout << "hello 2" << std::endl;

    // allegedly compress is same as commit, or so i'm ment to believe
    auto out = proof_system::plonk::stdlib::pedersen_commitment<plonk::UltraPlonkComposer>::compress(*fields);
    delete fields;
}
}

void free_composer(UltraPlonkComposer* composer)
{
    delete composer;
}