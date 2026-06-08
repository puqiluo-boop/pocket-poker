#include "../CardUtils/CardUtils.h"
#include "GameState.h"
#include <Arduino.h>
#include "../PlayerRegistry/PlayerRegistry.h"

#include <vector>
#include <optional>

GameState::GameState(const int deck[52], std::vector<String> playerMACS) : communityCards{deck[0], deck[1], deck[2], deck[3], deck[4]} {
    for (size_t i = 0; i < playerMACS.size(); i++) {
        PlayerState player = PlayerState(playerMACS[i], (uint32_t)1000, (uint8_t*)deck[i * 2]);
        players.push_back(player);
    }
    playerTurn = 0;
    street = 0;
}

