#pragma once
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/stdlib/types/ultra.hpp"

using namespace proof_system::plonk;
using namespace stdlib::types;

std::shared_ptr<barretenberg::srs::factories::CrsFactory> create_prover_factory();
void build_circuit(UltraPlonkComposer& composer, int circuit_size);

extern "C" {
Composer* create_composer(int circuit_size);
void commit(UltraPlonkComposer* composer);
}