#pragma once
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/stdlib/types/ultra.hpp"

using namespace proof_system::plonk;
using namespace stdlib::types;

std::shared_ptr<barretenberg::srs::factories::CrsFactory> create_prover_factory();

extern "C" {
barretenberg::polynomial create_input(size_t exponent);
std::shared_ptr<barretenberg::srs::factories::ProverCrs<curve::BN254>> create_prover_crs(size_t n);
void commit(barretenberg::polynomial input,
            size_t n,
            std::shared_ptr<barretenberg::srs::factories::ProverCrs<curve::BN254>> crs);
}