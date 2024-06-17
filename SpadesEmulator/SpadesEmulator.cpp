#include <iostream>

#include "utils.h"
#include "BaseGame.h"
#include "HybGame.h"

#include <chrono>
using namespace std::chrono;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> distr(0, (DECK_SIZE * 4) - 1);

uint randUInt() {
    return ((rand() & 255) << 24) ^ ((rand() & 255) << 16) ^ ((rand() & 255) << 8) ^ (rand() & 255);
}

constexpr int ceilNum(float num) {
    int inum = static_cast<int>(num);
    return inum + (num != inum);
}
constexpr int divUp(int a, int b) {
    return ceilNum(static_cast<float>(a) / b);
}
// What is the minimum byte amount to store the necessary bit number
constexpr int bitsToBytes(int bitNum) {
    return divUp(bitNum, BYTE_SIZE);
}

int main()
{
    const int TEST_CASES = 50000;   

    const int arcType = 64;

    const int pNumPerGame = 2;

    // round to the nearest number of bits needed that is a multiple of 8
    constexpr int cardSizeByteRound = BYTE_SIZE * bitsToBytes(CARD_SIZE);

    std::cout << "Emulating " << TEST_CASES << " games with " << CARD_NUM << " cards per game, each of size " << cardSizeByteRound << " bits \n";
    std::cout << "-------------------------------------------------------------------------" << std::endl;


    //per player we need a player ref, an enemy ref and a game ref
    const int baseRefs = 3 * arcType;
    //per player, we need 2 bytes for the card played each turn, 2 bytes for each players points and a final byte for the game turn 
    const int baseAdditionalStuff = (2*pNumPerGame + 1)*BYTE_SIZE;
    const int baseCardInfo = CARD_NUM * cardSizeByteRound;

    const int memBaseBits = baseRefs + baseAdditionalStuff + baseCardInfo;


    //HYBRID MEM
    const int lambda = 128; //security param
    const int hSize = 256; //hash output size

    const int memHybInitBits = lambda + hSize*3;
    const int memHybGameNoRelayBits = lambda;
    const int memHybGameRelayBits = lambda + 1;


    constexpr float memBaseBytes = static_cast<float>(bitsToBytes(memBaseBits) * TEST_CASES) / 1024/ 1024;
    constexpr float memHybInitBytes = static_cast<float>(bitsToBytes(memHybInitBits) * TEST_CASES) / 1024 / 1024;
    constexpr float memHybGameNoRelayBytes = static_cast<float>(bitsToBytes(memHybGameNoRelayBits) * TEST_CASES) / 1024 / 1024;
    constexpr float memHybGameRelayBytes = static_cast<float>(bitsToBytes(memHybGameRelayBits) * TEST_CASES) / 1024 / 1024;
    std::cout << "Total Base Memory needed for initialization:                  " << memBaseBytes << " (MB)\n";
    std::cout << "Total Base Memory needed for game:                            " << memBaseBytes << " (MB)\n";
    std::cout << "Total Hybrid Memory needed for initialization:                " << memHybInitBytes << " (MB)\n";
    std::cout << "Total Hybrid Memory needed for game with NO relay fallback:   " << memHybGameNoRelayBytes << " (MB)\n";
    std::cout << "Total Hybrid Memory needed for game with relay fallback:      " << memHybGameRelayBytes << " (MB)\n";
    std::cout << "-------------------------------------------------------------------------" << std::endl;


    constexpr float netBaseFlowBytes = static_cast<float>(TURN_NUM * bitsToBytes(cardSizeByteRound) * TEST_CASES) / 1024 / 1024;
    const int netHybInitBits = hSize * 2 + lambda;
    constexpr float netHybInitBytes = static_cast<float>(bitsToBytes(netHybInitBits) * TEST_CASES) / 1024 / 1024;
    //We asume relay is activated in the middle of the game (on avarage) and for one player only
    constexpr float netHybRelayFlow = netBaseFlowBytes / 2 / 2; 

    std::cout << "Total Base Network (in) for initialization:                   " << 0 << " (MB)\n";
    std::cout << "Total Base Network (out) for initialization:                  " << netBaseFlowBytes << " (MB)\n";
    std::cout << "Total Base Network (in) for game:                             " << netBaseFlowBytes << " (MB)\n";
    std::cout << "Total Base Network (out) for game:                            " << netBaseFlowBytes << " (MB)\n";
    std::cout << "Total Hybrid Network (in) for initialization:                 " << 0 << " (MB)\n";
    std::cout << "Total Hybrid Network (out) for initialization:                " << netHybInitBytes << " (MB)\n";
    std::cout << "Total Hybrid Network (in) for game with NO relay fallback:    " << 0 << " (MB)\n";
    std::cout << "Total Hybrid Network (out) for game with NO relay fallback:   " << 0 << " (MB)\n";
    std::cout << "Total Hybrid Network (in) for game with relay fallback:       " << netHybRelayFlow << " (MB)\n";
    std::cout << "Total Hybrid Network (out) for game with relay fallback:      " << netHybRelayFlow << " (MB)\n";
    std::cout << "-------------------------------------------------------------------------" << std::endl;



    //emulate inputs from player
    uint* randArr = new uint[TEST_CASES * CARD_NUM];
    for (int i = 0; i < TEST_CASES * CARD_NUM; i++) {
        randArr[i] = randUInt() % CARD_NUM;
    }


    //test Base CPU times
    auto start = high_resolution_clock::now(); 
    Game** gameArr = new Game * [TEST_CASES];
    for (int i = 0; i < TEST_CASES; i++) {
        gameArr[i] = new Game();
        testBaseInit(gameArr[i]);
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    tLong durationCast = duration.count();
    std::cout << "Total CPU Base Initialization time:                           " << durationCast << " (ms)" << "\n";

    start = high_resolution_clock::now();
    for (int i = 0; i < TEST_CASES; i++) {
        testBaseGame(gameArr[i], randArr + (i*13));
    }
    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);
    durationCast = duration.count();
    std::cout << "Total CPU Base Game time:                                     " << durationCast << " (ms)" << "\n";


    //test Hybrid CPU times
    start = high_resolution_clock::now();
    for (int i = 0; i < TEST_CASES; i++) {
        testHybInit();
    }
    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);
    durationCast = duration.count();
    std::cout << "Total CPU Hybrid Initialization time:                         " << durationCast << " (ms)" << "\n";

    std::cout << "Total CPU Hybrid Game with NO relay fallback time:            " << 0 << " (ms)\n";
    std::cout << "Total CPU Hybrid Game with relay fallback time:               " << durationCast << " (ms)\n";
    std::cout << "-------------------------------------------------------------------------" << std::endl;



    delete[] randArr;
    for (int i = 0; i < TEST_CASES; ++i) {
        delete gameArr[i];
    }
    delete[] gameArr;

}
