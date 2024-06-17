#pragma once
#include <random>
#include "utils.h"

extern std::random_device rd;
extern std::mt19937 gen;
extern std::uniform_int_distribution<> distr;

void testHybInit();
void testHybGame();