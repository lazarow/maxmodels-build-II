#pragma once

#include "internal_representation.hpp"

using PositiveRule = vector<Literal>;
using PositiveRules = vector<PositiveRule>;

Model compute_consequences(const Program &program, const Model &model);