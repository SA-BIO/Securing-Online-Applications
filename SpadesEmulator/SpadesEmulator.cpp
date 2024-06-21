#include <iostream>

#include "utils.h"
#include "BaseGame.h"
#include "HybGame.h"

#include <chrono>
using namespace std::chrono;

std::random_device rd;
std::uniform_int_distribution<> distr(0, (DECK_SIZE) - 1);

byte randByte() {
    return static_cast<byte>(rand() & 255);
}
uint randUInt() {
    return ((rand() & 255) << 24) ^ ((rand() & 255) << 16) ^ ((rand() & 255) << 8) ^ (rand() & 255);
}

void mainTest() {
    const int TEST_CASES = 100000;
    const int arcType = 64;
    const int pNumPerGame = 2;

    if (CARD_SIZE_BYTES != 1) {
        std::cout << "ERROR: This emulation only accounts for CARD_SIZE_BYTES = 1";
        return;
    }

    std::cout << "Emulating " << TEST_CASES << " games with " << CARD_NUM << " cards per game, each of size: " << CARD_SIZE_BYTES << " byte(s) \n";
    std::cout << "-------------------------------------------------------------------------" << std::endl;

    // BASE MEM
    const int gameMemBits = 2 * arcType + 3 * 32 + 2 * BYTE_SIZE;                   //2 refs to players, 3 ints and 2 bytes for internal vars
    const int playerMemBits = CARD_NUM * BYTE_SIZE + arcType + 32;                  //13 bytes for cards, a ref to the game and a uint for player seed

    const int memBaseBits = playerMemBits + gameMemBits / 2;                        //we need 1 game for every 2 players


    //HYBRID MEM
    const int lambda = 128; //security param
    const int hSize = 256; //hash output size

    const int hybGameMemBits = 2 * lambda;                                                      //2 player seeds 

    const int memHybInitBits = hybGameMemBits/2;                                                //we need 1 game for every 2 players
    const int memHybGameNoRelayBits = memHybInitBits;                                           //seed
    const int memHybGameRelayBits = memHybGameNoRelayBits + 2*BYTE_SIZE;                        //seed, 2 auxiliary integers and 2 bytes remembering each players turn and previous action
    const int memHybGameRelayCheatBits = memHybGameRelayBits;                                   //u128 and a byte as auxiliary params

    constexpr float memBaseBytes = static_cast<float>(bitsToBytes(memBaseBits) * TEST_CASES) / 1024 / 1024;
    constexpr float memHybInitBytes = static_cast<float>(bitsToBytes(memHybInitBits) * TEST_CASES) / 1024 / 1024;
    constexpr float memHybGameNoRelayBytes = static_cast<float>(bitsToBytes(memHybGameNoRelayBits) * TEST_CASES) / 1024 / 1024;
    constexpr float memHybGameRelayBytes = static_cast<float>(bitsToBytes(memHybGameRelayBits) * TEST_CASES) / 1024 / 1024;
    constexpr float memHybGameRelayCheatBytes = static_cast<float>(bitsToBytes(memHybGameRelayCheatBits) * TEST_CASES) / 1024 / 1024;
    std::cout << "Estimated Base Memory needed for initialization:                              " << memBaseBytes << " (MB)\n";
    std::cout << "Estimated Hybrid Memory needed for initialization:                            " << memHybInitBytes << " (MB)\n";
    std::cout << "Estimated Hybrid Memory needed for game with NO relay fallback:               " << memHybGameNoRelayBytes << " (MB)\n";
    std::cout << "Estimated Hybrid Memory needed for game with relay fallback (no cheating):    " << memHybGameRelayBytes << " (MB)\n";
    std::cout << "Estimated Hybrid Memory needed for game with relay fallback (cheat):          " << memHybGameRelayCheatBytes << " (MB)\n";
    std::cout << "-------------------------------------------------------------------------" << std::endl;


    constexpr float netBaseFlowBytes = static_cast<float>(TURN_NUM * CARD_SIZE_BYTES * TEST_CASES) / 1024 / 1024;
    const int netHybInitBits = hSize + lambda;
    constexpr float netHybInitBytes = static_cast<float>(bitsToBytes(netHybInitBits) * TEST_CASES) / 1024 / 1024;
    //We asume relay is activated in the middle of the game (on avarage)
    constexpr float netHybRelayFlow = netBaseFlowBytes / 2 ;

    std::cout << "Estimated Base Network (in) for initialization:                               " << 0 << " (MB)\n";
    std::cout << "Estimated Base Network (out) for initialization:                              " << netBaseFlowBytes << " (MB)\n";
    std::cout << "Estimated Base Network (in) for game:                                         " << netBaseFlowBytes << " (MB)\n";
    std::cout << "Estimated Base Network (out) for game:                                        " << netBaseFlowBytes << " (MB)\n";
    std::cout << "Estimated Hybrid Network (in) for initialization:                             " << 0 << " (MB)\n";
    std::cout << "Estimated Hybrid Network (out) for initialization:                            " << netHybInitBytes << " (MB)\n";
    std::cout << "Estimated Hybrid Network (in) for game with NO relay fallback:                " << 0 << " (MB)\n";
    std::cout << "Estimated Hybrid Network (out) for game with NO relay fallback:               " << 0 << " (MB)\n";
    std::cout << "Estimated Hybrid Network (in) for game with relay fallback:                   " << netHybRelayFlow << " (MB)\n";
    std::cout << "Estimated Hybrid Network (out) for game with relay fallback:                  " << netHybRelayFlow << " (MB)\n";
    std::cout << "-------------------------------------------------------------------------" << std::endl;



    //emulate inputs from player
    uint* randArrBase = new uint[TEST_CASES * CARD_NUM];
    for (int i = 0; i < TEST_CASES * CARD_NUM; i++) {
        randArrBase[i] = randUInt() % CARD_NUM;
    }

    //test Base CPU times
    Game** gameArr = new Game * [TEST_CASES];
    for (int i = 0; i < TEST_CASES; i++) {
        gameArr[i] = new Game();
        testBaseInit(gameArr[i], false);    //init player 2 (we only measure time for player 1)
    }

    auto start = high_resolution_clock::now();
    for (int i = 0; i < TEST_CASES; i++) {
        testBaseInit(gameArr[i], true);     //init player 1
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    tLong durationCast = duration.count();
    std::cout << "Measured CPU Base Initialization time:                                        " << durationCast << " (ms)" << "\n";

    start = high_resolution_clock::now();
    for (int i = 0; i < TEST_CASES; i++) {
        testBaseGame(gameArr[i], randArrBase + (i * 13));
    }
    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);
    durationCast = duration.count();
    std::cout << "Measured CPU Base Game time:                                                  " << durationCast << " (ms)" << "\n";

    delete[] randArrBase;
    for (int i = 0; i < TEST_CASES; ++i) {
        delete gameArr[i];
    }
    delete[] gameArr;


    //test Hybrid CPU times
    initConstans();
    GameHyb** hybGamesArr = new GameHyb * [TEST_CASES];
    for (int i = 0; i < TEST_CASES; i++) {
        hybGamesArr[i] = new GameHyb();
    }

    start = high_resolution_clock::now();
    for (int i = 0; i < TEST_CASES; i++) {
        testHybInit(hybGamesArr[i]);
    }
    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);
    durationCast = duration.count();
    std::cout << "Measured CPU Hybrid Initialization time:                                      " << durationCast << " (ms)" << "\n";

    std::cout << "Measured CPU Hybrid Game with NO relay fallback time:                         " << 0 << " (ms)\n";


    constexpr int avarageRelayTurn = divUp(TURN_NUM, 2);

    byte resultsArr[avarageRelayTurn];
    byte* randCheatKey = new byte[TEST_CASES];
    bool* randCheatResults = new bool[TEST_CASES];
    for (int i = 0; i < TEST_CASES; i++) {
        randCheatKey[i] = randByte();
        randCheatResults[i] = randByte();
    }

    byte* randArrHyb0 = new byte[TEST_CASES * avarageRelayTurn];
    byte* randArrHyb1 = new byte[TEST_CASES * avarageRelayTurn];
    for (int i = 0; i < TEST_CASES * avarageRelayTurn; i++) {
        randArrHyb0[i] = randByte();
        randArrHyb1[i] = randByte();
    }

    start = high_resolution_clock::now();
    for (int i = 0; i < TEST_CASES; i++) {
        int auxHybCounter = i * avarageRelayTurn;
        testHybGame_Relay(hybGamesArr[i], avarageRelayTurn, randArrHyb0 + auxHybCounter, randArrHyb1 + auxHybCounter, resultsArr);
    }
    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);
    tLong durationCast_NoCheat = duration.count();
    std::cout << "Measured CPU Hybrid Game with relay fallback (no cheating) time:              " << durationCast_NoCheat << " (ms)\n";

    start = high_resolution_clock::now();
    for (int i = 0; i < TEST_CASES; i++) {
        randCheatResults[i] = testHybGame_Cheat(hybGamesArr[i], avarageRelayTurn, randCheatKey[i]);
    }
    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);
    durationCast = duration.count();
    std::cout << "Measured CPU Hybrid Game with relay fallback (cheating) time:                 " << durationCast + durationCast_NoCheat << " (ms)\n";

    std::cout << "-------------------------------------------------------------------------" << std::endl;

    delete[] randCheatKey;
    delete[] randCheatResults;
    delete[] randArrHyb0;
    delete[] randArrHyb1;
    for (int i = 0; i < TEST_CASES; ++i) {
        delete hybGamesArr[i];
    }
    delete[] hybGamesArr;
}


int main() {
    mainTest();

    std::cout << "END" << std::endl;
    char stopChar = std::getchar();
    std::cout << stopChar << std::endl;

    return 0;
}
