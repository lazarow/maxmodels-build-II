#pragma once

#include "internal_representation.hpp"

using PositiveRule = vector<Literal>;
using PositiveRules = vector<PositiveRule>;

bool is_answer_set(const Program &program, const Model &supporting_model);