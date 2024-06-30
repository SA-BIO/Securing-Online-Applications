#pragma once
#include <random>
#include "utils.h"

extern std::random_device rd;
extern std::uniform_int_distribution<> distr;

/*class GameHyb {
public:
	u128 seeds[2];
};*/


void initConstans();
void testHybInit(u128& seed);
void testHybGame_Relay(const uint avRelayTurn, byte* randVals0, byte* randVals1, bool* testComp, byte* res);
bool testHybGame_Cheat(u128& seed, const uint avRelayTurn, byte ketToCheck);