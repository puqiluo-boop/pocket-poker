#include "../CardUtils/CardUtils.h"
#include "GameState.h"
#include <Arduino.h>
#include "../PlayerRegistry/PlayerRegistry.h"

#include <vector>
#include <optional>
#include <algorithm>

//Add parameter for button postion
GameState::GameState(const int deck[52], const uint32_t smallBlind, const uint32_t bigBlind, const std::vector<String> playerMACS) :
communityCards{deck[0], deck[1], deck[2], deck[3], deck[4]},
smallBlind(smallBlind),
bigBlind(bigBlind) {
    PlayerState smallBlindPlayer = PlayerState(playerMACS[0], (uint8_t*)deck[5], (uint32_t)1000);
    smallBlindPlayer.activeBet = std::min(smallBlind, smallBlindPlayer.remainingChips);
    smallBlindPlayer.remainingChips -= std::min(smallBlind, smallBlindPlayer.remainingChips);
    playersInHand.push_back(smallBlindPlayer);

    PlayerState bigBlindPlayer = PlayerState(playerMACS[1], (uint8_t*)deck[7], (uint32_t)1000);
    bigBlindPlayer.activeBet = std::min(bigBlind, bigBlindPlayer.remainingChips);
    bigBlindPlayer.remainingChips -= std::min(bigBlind, bigBlindPlayer.remainingChips);
    playersInHand.push_back(bigBlindPlayer);

    for (size_t i = 2; i < playerMACS.size(); i++) {
        PlayerState player = PlayerState(playerMACS[i], (uint8_t*)deck[5 + (i * 2)], (uint32_t)1000);
        playersInHand.push_back(player);
    }

    playerTurn = 2 % playersInHand.size(); // Start with player to the left of big blind
    street = 0;
    callAmount = bigBlind;
    lastAnyRaiserIndex = -1;
    minRaiseIncrement = bigBlind;
}

/**
 * Increments playerTurn to the next player. Should be called after a player takes a non-fold action.
 */
void GameState::incTurn(bool folding = false) {
    if(lastAction()) {
        nextStreet();
    } else {
        playerTurn = (playerTurn + folding ? 1 : 0) % playersInHand.size();
        if(playersInHand[playerTurn].remainingChips == 0) { // Skip players who are all in
            incTurn();
        }
    }
}

/**
 * @return true if the current player's action will end the round
 */
bool GameState::lastAction() {
    return (lastAnyRaiserIndex == -1 && playerTurn == playersInHand.size() - 1) || (playerTurn + 1) % playersInHand.size() == lastAnyRaiserIndex;
}

void GameState::nextStreet() {
    street++;
    playerTurn = 0;
    callAmount = 0;
    lastAnyRaiserIndex = -1;
    minRaiseIncrement = bigBlind;
    for(PlayerState& player : playersInHand) {
        player.hasActed = false;
        player.inactiveBet += player.activeBet;
        player.activeBet = 0;
    }
    if(street == 4) {
        // TODO: If game is over
    }
}

bool GameState::check() {
    if(callAmount == 0) {
        playersInHand[playerTurn].hasActed = true;
        incTurn();
        return true;
    }
    return false;
}

bool GameState::call() {
    if(callAmount == 0) {
        return false;
    }
    PlayerState& player = playersInHand[playerTurn];
    if(player.remainingChips < callAmount - player.activeBet) { // All-in call
        player.activeBet += player.remainingChips;
        player.remainingChips = 0;
    } else {
        player.remainingChips -= callAmount - player.activeBet;
        player.activeBet += callAmount - player.activeBet;
    }
    player.hasActed = true;
    incTurn();
    return true;
}

bool GameState::betRaise(uint32_t amount) {
    PlayerState& player = playersInHand[playerTurn];
    if(callAmount - player.activeBet < minRaiseIncrement && player.hasActed) { // Player is not allowed to raise (can only call or fold) due to a short all-in
        return false;
    }
    if(amount == player.remainingChips) { // All in
        if(amount + player.activeBet <= callAmount) { // Constitutes as a call, not a raise
            return false;
        }
        player.activeBet += player.remainingChips;
        player.remainingChips = 0;
    } else if(amount + player.activeBet - callAmount >= minRaiseIncrement && amount <= player.remainingChips) { // "Normal" bet or raise
        player.remainingChips -= amount;
        player.activeBet += amount;
    } else {
        return false;
    }
    // player.activeBet should now be the total amount the player has bet for this street.
    if(player.activeBet - callAmount > minRaiseIncrement) {
        minRaiseIncrement = player.activeBet - callAmount;
    }
    assert(player.activeBet > callAmount);
    assert(minRaiseIncrement >= bigBlind);
    callAmount = player.activeBet;
    lastAnyRaiserIndex = playerTurn;

    player.hasActed = true;
    incTurn();
    return true;
}

