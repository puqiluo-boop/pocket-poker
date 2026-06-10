#pragma once
#include "../PlayerRegistry/PlayerRegistry.h"
#include <Arduino.h>
#include <vector>
#include <optional>
#include <tuple>

struct PlayerState {
    const String playerMAC;
    const uint8_t holeCards[2];

    uint32_t remainingChips;
    uint32_t activeBet; // Amount currently in the pot for the current street
    uint32_t inactiveBet; // Amount in the pot from previous streets.

    bool hasActed;
    bool hasFolded;

    PlayerState(String mac, uint8_t cards[2], uint32_t chips) : playerMAC(mac), holeCards{cards[0], cards[1]}, remainingChips(chips), activeBet(0), inactiveBet(0), hasActed(false), hasFolded(false) {}
};
struct PlayerResult {
    const String playerMAC;
    const double handValue;
    const int32_t netGain;

    PlayerResult(String mac, double value, int32_t gain) : playerMAC(mac), handValue(value), netGain(gain) {}
};

struct Pot {
    const uint32_t size;
    std::vector<const PlayerState&> eligiblePlayers;

    Pot(uint32_t s, std::vector<const PlayerState&> players) : size(s), eligiblePlayers(players) {}
};
class GameState {
private:
    const uint8_t communityCards[5];
    const uint32_t smallBlind;
    const uint32_t bigBlind;

    std::vector<PlayerState> players;

    uint8_t playerTurn; // Vector index of player whose turn it is to act
    uint8_t street; // 0, 1, 2, 3, 4 (game finished)

    uint32_t callAmount; // Amount player must call to stay in hand. 0 if no bet has been made this street.
    uint32_t roundCutoff; // Vector index of last player to raise, minRaise met or not. -1 if no raises this street.
    uint32_t minRaiseIncrement; // Size of last bet or raise. Used to calculate minimum raise amount. 0 if no bet has been made this street

    bool findActivePlayer(uint8_t start, bool forwards = true);
    bool incTurn();
    bool lastAction();
    bool nextStreet();

    bool gameActive();
    uint8_t numNonfoldedPlayers();
public:
    GameState(const int deck[52], const uint32_t smallBlind, const uint32_t bigBlind, const std::vector<PlayerInfo>& playerInfo) : communityCards{deck[0], deck[1], deck[2], deck[3], deck[4]}, smallBlind(smallBlind), bigBlind(bigBlind) {}

    static bool validPlayers(const std::vector<PlayerInfo>& players) {
        for(const PlayerInfo& player : players) {
            if(player.chipCount == 0) {
                return false;
            }
        }
        return players.size() >= 2; // Need at least 2 players to play
    }

    bool check();
    bool call();
    bool betRaise(uint32_t amount);
    bool fold();

    bool canCheck();
    bool canCall();
    // getCallInfo()
    bool canBet();
    bool canRaise();
    // getBetRaiseInfo()
    bool canFold();

    /**
    @returns a vector of the current pots, including side pots if applicable. The first element is the main pot.
    */
    std::vector<Pot> getPots();
    
    // Returns nullopt if game is not yet finished. Otherwise, returns a vector of PlayerResult (no particular order).
    std::optional<std::vector<PlayerResult>> getResults();

    uint8_t getPlayerTurn() {
        return playerTurn;
    }

    uint8_t getStreet() {
        return street;
    }
};
