#include "bench_bindings.hpp"

#include <barretenberg/plonk/proof_system/types/proof.hpp>
#include <barretenberg/plonk/proof_system/proving_key/serialize.hpp>
#include <barretenberg/srs/global_crs.hpp>

using namespace barretenberg::srs;

// this might fail if global crs is not initialized somewhere
std::shared_ptr<barretenberg::srs::factories::CrsFactory> create_prover_factory()
{
    return get_crs_factory();
}

void build_circuit(UltraPlonkComposer& composer, int circuit_size)
{
    while (composer.get_num_gates() <= circuit_size / 2) {
        plonk::stdlib::pedersen_commitment<UltraPlonkComposer>::compress(field_ct(witness_ct(&composer, 1)),
                                                                         field_ct(witness_ct(&composer, 1)));
    }
}

extern "C" {

UltraPlonkComposer* create_composer(int circuit_size)
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

void commit(UltraPlonkComposer* composer)
{
    plonk::stdlib::field_t<plonk::UltraPlonkComposer> left(
        plonk::stdlib::witness_t(composer, barretenberg::fr::random_element()));
    plonk::stdlib::field_t<plonk::UltraPlonkComposer> out(
        plonk::stdlib::witness_t(composer, barretenberg::fr::random_element()));

    // allegedly compress is same as commit, or so i'm ment to believe
    out = proof_system::plonk::stdlib::pedersen_commitment<plonk::UltraPlonkComposer>::compress(left, out);
}
}