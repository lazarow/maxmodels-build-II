#pragma once

#include <memory>

#include "internal_representation.hpp"
#include "solving.hpp"
#include "wcnf.hpp"

void clark_completion(
    const Program &program,
    unique_ptr<WCNF> &wcnf,
    unordered_map<BodyIndex, unsigned int> &body_to_variable,
    AtomMapper &atom_mapper);

void lp2sat_like(
    const Program &program,
    unique_ptr<WCNF> &wcnf,
    unordered_map<BodyIndex, unsigned int> &body_to_variable,
    AtomMapper &atom_mapper);