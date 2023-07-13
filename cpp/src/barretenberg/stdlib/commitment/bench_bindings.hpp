#pragma once
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/stdlib/types/ultra.hpp"
#include "barretenberg/srs/factories/mem_crs_factory.hpp"
#include "barretenberg/ecc/curves/grumpkin/grumpkin.hpp"

using namespace proof_system::plonk;
using namespace stdlib::types;

// srs::factories::MemCrsFactory create_prover_factory();

extern "C" {
std::vector<grumpkin::fq>* create_input(size_t n);
srs::factories::MemCrsFactory* create_prover_factory();
void commit(std::vector<grumpkin::fq>* input, size_t n, srs::factories::MemCrsFactory* prover_factory);
void free_crs(srs::factories::MemCrsFactory* ptr)
{
    delete ptr;
}
void free_input(std::vector<barretenberg::fr>* ptr)
{
    // 123
    delete ptr;
}
}