#pragma once

#include "internal_representation.hpp"

using PositiveRule = vector<Literal>;
using PositiveRules = vector<PositiveRule>;

bool is_stable_model(const Program &program, const Model &supporting_model);