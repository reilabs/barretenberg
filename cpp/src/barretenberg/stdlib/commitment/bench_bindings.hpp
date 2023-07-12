#pragma once
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/stdlib/types/ultra.hpp"
#include "barretenberg/srs/factories/mem_crs_factory.hpp"

using namespace proof_system::plonk;
using namespace stdlib::types;

std::shared_ptr<srs::factories::MemCrsFactory> create_prover_factory();

extern "C" {
std::vector<barretenberg::fr>* create_input(size_t exponent);
barretenberg::srs::factories::ProverCrs<curve::BN254>* create_prover_crs(size_t n);
void commit(std::vector<barretenberg::fr>* input, size_t n, barretenberg::srs::factories::MemCrsFactory* crs);
void free_crs(barretenberg::srs::factories::ProverCrs<curve::BN254>* ptr)
{
    delete ptr;
}
}