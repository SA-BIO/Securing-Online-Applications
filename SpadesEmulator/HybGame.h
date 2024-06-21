#pragma once
#include <random>
#include "utils.h"

extern std::random_device rd;
extern std::uniform_int_distribution<> distr;

class GameHyb {
public:
	u128 seedP1;
	u128 seedP2;
	bool turnCount = false;
};


void initConstans();
void testHybInit(GameHyb* game);
void testHybGame_Relay(GameHyb* game, const uint avRelayTurn, byte* randVals0, byte* randVals1, byte* res);
bool testHybGame_Cheat(GameHyb* game, const uint avRelayTurn, byte ketToCheck);