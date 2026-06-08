#pragma once
#include <Arduino.h>
#include <vector>
#include <optional>

struct PlayerState {
    const String playerMAC;
    const uint32_t totalChips;
    const uint8_t holeCards[2];

    uint32_t totalBet;
    bool inHand;

    PlayerState(String mac, uint32_t chips, uint8_t cards[2]) : playerMAC(mac), totalChips(chips), holeCards{cards[0], cards[1]}, totalBet(0), inHand(true) {}
};
struct PlayerResult {
    const String playerMAC;
    const double handValue;
    const int32_t netGain;

    PlayerResult(String mac, double value, int32_t gain) : playerMAC(mac), handValue(value), netGain(gain) {}
};

class GameState {
private:
    const uint8_t communityCards[5];

    std::vector<PlayerState> players;
    uint8_t playerTurn; // Vector index of player who has action
    uint8_t street; // 0, 1, 2, 3
public:
    GameState(const int deck[52], std::vector<String> playerMACs) : communityCards{deck[0], deck[1], deck[2], deck[3], deck[4]} {};

    bool addBet(String playerMAC, uint32_t amount);

    std::optional<uint32_t> getBet(String playerMAC);

    /**
    @returns a vector of the current pots, including side pots if applicable. The first element is the main pot.
    */
    std::vector<uint32_t> getPotSizes();

    /**
    @returns the amount a player must call to stay in the hand. Nullopt if player not found.
    */
    std::optional<uint32_t> getCallSize(String playerMAC);

    /**
    @returns true if the game has not yet reached a terminal state, false if the game is over and should be reset.
    */
    bool nextState();

    std::vector<PlayerResult> getResults();
};
