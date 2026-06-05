#pragma once
#include <Arduino.h>
#include <vector>
#include <tuple>

typedef struct {
    uint8_t playerID;
    std::tuple<uint8_t, uint8_t> holeCards;
    uint32_t totalBet;
} PlayerState;

typedef struct {
    std::vector<PlayerState> playerVector; // ID, TotalBet
    uint8_t playerTurn; // Vector index of player who has action
    uint8_t street; // 0, 1, 2, 3
    uint8_t communityCards[5];
} GameObject;

class GameState {
private:
    GameObject game;

public:
    GameState(std::vector<uint8_t> playerIDs);

    uint32_t getPotSize();

    uint32_t getCallSize(uint8_t playerID);

    void nextState();
};
