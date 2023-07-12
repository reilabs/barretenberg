#pragma once
#include "barretenberg/plonk/proof_system/types/proof.hpp"
#include "barretenberg/stdlib/types/ultra.hpp"
#include "barretenberg/srs/factories/mem_crs_factory.hpp"

using namespace proof_system::plonk;
using namespace stdlib::types;

srs::factories::MemCrsFactory create_prover_factory();

extern "C" {
std::vector<barretenberg::fr>* create_input(size_t exponent);
void commit(std::vector<barretenberg::fr>* input, size_t n);
void free_crs(srs::factories::MemCrsFactory* ptr)
{
    delete ptr;
}
}