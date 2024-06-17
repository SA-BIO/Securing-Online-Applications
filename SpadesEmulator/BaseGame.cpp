#include "BaseGame.h"

// This codification doesn't follow the one specified in the paper. This is just an emulation of CPU usage not a real implementation.
inline int getSuit(byte card) {
    if (card < 13) {
        return 0;
    }
    else if (card < 26) {
        return 1;
    }
    else if (card < 39) {
        return 2;
    }
    else {
        return 3;    //Spades
    }
}



void testBaseInit(Game* game) {
    Player* p1 = new Player();
    Player* p2 = new Player();

    game->player1 = p1;
    game->player2 = p2;

    p1->currentGame = game;
    p2->currentGame = game;

    p1->generateDeck();
    p2->generateDeck();
}


void testBaseGame(Game* game, uint* randVals) {
    for (int i = 0; i < TURN_NUM; i++) {
        game->playerCardP1 = game->player1->cards[randVals[i]];
        game->playerCardP2 = game->player2->cards[randVals[i]];

        int p1Suit = getSuit(game->playerCardP1);
        int p2Suit = getSuit(game->playerCardP2);

        if (game->turnCount % 2 == 0) {
            if (p1Suit == p2Suit) {
                if (game->playerCardP1 > game->playerCardP2) {
                    game->scoreP1++;
                }
                else {
                    game->scoreP2++;
                }
            }
            else {
                if (p2Suit == 3) {
                    game->scoreP2++;
                }
                else {
                    game->scoreP1++;
                }
            }
        }
        else {
            if (p1Suit == p2Suit) {
                if (game->playerCardP2 > game->playerCardP1) {
                    game->scoreP2++;
                }
                else {
                    game->scoreP1++;
                }
            }
            else {
                if (p1Suit == 3) {
                    game->scoreP1++;
                }
                else {
                    game->scoreP2++;
                }
            }
        }
        game->turnCount++;
    }
}