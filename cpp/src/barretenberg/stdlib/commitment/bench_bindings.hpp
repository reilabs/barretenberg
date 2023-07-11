#pragma once
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/stdlib/types/ultra.hpp"

using namespace proof_system::plonk;
using namespace stdlib::types;

std::shared_ptr<barretenberg::srs::factories::CrsFactory> create_prover_factory();
void build_circuit(UltraPlonkComposer& composer, size_t circuit_size);

extern "C" {
UltraPlonkComposer* create_composer(size_t circuit_size);
void commit(UltraPlonkComposer* composer, size_t length);
}