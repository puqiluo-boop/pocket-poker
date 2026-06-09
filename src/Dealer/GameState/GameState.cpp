#include "../CardUtils/CardUtils.h"
#include "GameState.h"
#include <Arduino.h>
#include "../PlayerRegistry/PlayerRegistry.h"

#include <vector>
#include <optional>
#include <algorithm>

//Add parameter for button postion
GameState::GameState(const int deck[52], const uint32_t smallBlind, const uint32_t bigBlind, const std::vector<std::tuple<String, uint32_t>> playerInfo) :
communityCards{deck[0], deck[1], deck[2], deck[3], deck[4]},
smallBlind(smallBlind),
bigBlind(bigBlind) {
    assert(playerInfo.size() >= 2); // Need at least 2 players to play

    PlayerState smallBlindPlayer = PlayerState(std::get<0>(playerInfo[0]), (uint8_t*)deck[5], std::get<1>(playerInfo[0]));
    if(smallBlindPlayer.remainingChips == 0) smallBlindPlayer.hasFolded  = true;
    smallBlindPlayer.activeBet = std::min(smallBlind, smallBlindPlayer.remainingChips);
    smallBlindPlayer.remainingChips -= std::min(smallBlind, smallBlindPlayer.remainingChips);
    players.push_back(smallBlindPlayer);

    PlayerState bigBlindPlayer = PlayerState(std::get<0>(playerInfo[1]), (uint8_t*)deck[7], std::get<1>(playerInfo[1]));
    if(bigBlindPlayer.remainingChips == 0) bigBlindPlayer.hasFolded  = true;
    bigBlindPlayer.activeBet = std::min(bigBlind, bigBlindPlayer.remainingChips);
    bigBlindPlayer.remainingChips -= std::min(bigBlind, bigBlindPlayer.remainingChips);
    players.push_back(bigBlindPlayer);

    for (size_t i = 2; i < playerInfo.size(); i++) {
        PlayerState player = PlayerState(std::get<0>(playerInfo[i]), (uint8_t*)deck[5 + (i * 2)], std::get<1>(playerInfo[i]));
        if(player.remainingChips == 0) player.hasFolded = true;
        players.push_back(player);
    }

    playerTurn = 2 % players.size(); // Start with player to the left of big blind
    street = 0;
    callAmount = bigBlind;
    lastAnyRaiserIndex = -1;
    minRaiseIncrement = bigBlind;
}

/**
 * Increments playerTurn to the next player. Should be called after a player takes a non-fold action.
 */
void GameState::incTurn() {
    if(lastAction()) {
        nextStreet();
    } else {
        playerTurn = (playerTurn + 1) % players.size();

        PlayerState& player = players[playerTurn];
        if(player.remainingChips == 0 || player.hasFolded) { // Skip players who are all in or have folded
            incTurn();
        }
    }
}

/**
 * @return true if the current player's action will end the round
 */
bool GameState::lastAction() {
    return (lastAnyRaiserIndex == -1 && playerTurn == players.size() - 1) || (playerTurn + 1) % players.size() == lastAnyRaiserIndex;
}

void GameState::nextStreet() {
    street++;
    playerTurn = 0; //Need to fix this for folds and all-ins
    callAmount = 0;
    lastAnyRaiserIndex = -1;
    minRaiseIncrement = bigBlind;
    for(PlayerState& player : players) {
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
        players[playerTurn].hasActed = true;
        incTurn();
        return true;
    }
    return false;
}

bool GameState::call() {
    if(callAmount == 0) {
        return false;
    }
    PlayerState& player = players[playerTurn];
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
    PlayerState& player = players[playerTurn];
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

uint8_t GameState::getNumActivePlayers() {
    uint8_t count = 0;
    for(const PlayerState& player : players) {
        if(!player.hasFolded) {
            count++;
        }
    }
    return count;
}

bool GameState::fold() {
    players[playerTurn].hasFolded = true;
    if(getNumActivePlayers() == 1) {
         // End game if only one player remains
    }
    incTurn();
    return true;
}

std::vector<Pot> GameState::getPots() {
    std::vector<Pot> pots;
    std::vector<std::tuple<String, uint32_t, bool, bool>> incBetAllInPriority; // Vector of (playerMAC, totalBet) sorted in ascending order of totalBet, with all-in players sorted before non all-in players with the same totalBet
    for(PlayerState& player : players) {
        if(player.activeBet > 0 || player.inactiveBet > 0) {
            incBetAllInPriority.push_back(std::make_tuple(player.playerMAC, player.activeBet + player.inactiveBet, player.remainingChips == 0, player.hasFolded));
        }
    }
    std::sort(incBetAllInPriority.begin(), incBetAllInPriority.end(), [](const std::tuple<String, uint32_t, bool, bool>& a, const std::tuple<String, uint32_t, bool, bool>& b) {
        if (std::get<1>(a) == std::get<1>(b)) { // If totalBet is the same, sort all-in players first
            bool aAllIn = std::get<2>(a);
            bool bAllIn = std::get<2>(b);
            if (aAllIn && !bAllIn) {
                return true;
            } else if (!aAllIn && bAllIn) {
                return false;
            }
        }
        return std::get<1>(a) < std::get<1>(b);
    });
    
    std::vector<String> activePlayers;
    for(const PlayerState& player : players) {
        // If they haven't folded and aren't all-in (chips > 0), they are active
        if(!player.hasFolded && player.remainingChips > 0) {
            activePlayers.push_back(player.playerMAC);
        }
    }

    uint32_t potTracker = 0;
    std::vector<String> currentPotPlayers = activePlayers;
    for(size_t i = 0; i < incBetAllInPriority.size(); i++) {
        auto& [playerMAC, totalBet, allIn, folded] = incBetAllInPriority[i];
        if(totalBet == 0) continue;
        if(allIn) {
            assert(!folded); // A folded player cannot be all-in
            currentPotPlayers.push_back(playerMAC);
            potTracker += totalBet;
            for(size_t j = i + 1; j < incBetAllInPriority.size(); j++) {
                auto& [otherPlayerMAC, otherTotalBet, otherAllIn, otherFolded] = incBetAllInPriority[j];
                potTracker += totalBet;
                otherTotalBet -= totalBet;
                if(otherAllIn) {
                    assert(!otherFolded);
                    currentPotPlayers.push_back(otherPlayerMAC);
                }
            }
            totalBet = 0;

            pots.push_back(Pot(potTracker, currentPotPlayers));
            potTracker = 0;
            currentPotPlayers = activePlayers;
        } else {
            potTracker += totalBet;
            totalBet = 0;
            if(i + 1 == incBetAllInPriority.size()) {
                pots.push_back(Pot(potTracker, currentPotPlayers));
            }
        }
    }
    
    if(pots.empty()) { // No bets were made
        std::vector<String> validPlayers;
        for(const PlayerState& player : players) {
            if(!player.hasFolded) {
                validPlayers.push_back(player.playerMAC);
            }
        }
        pots.push_back(Pot(0, validPlayers));
    }

    return pots;
}

std::optional<std::vector<PlayerResult>> GameState::getResults() {
    if(getNumActivePlayers() == 1) {
        std::vector<PlayerResult> results;
        for(const PlayerState& player : players) {
            if(!player.hasFolded) {
                int32_t netGain = player.activeBet + player.inactiveBet;
                for(const PlayerState& otherPlayer : players) {
                    if(otherPlayer.hasFolded) {
                        int32_t otherTotalBet = otherPlayer.activeBet + otherPlayer.inactiveBet;
                        results.push_back(PlayerResult(otherPlayer.playerMAC, 0, -otherTotalBet));
                        netGain += otherTotalBet;
                    }
                }
                results.push_back(PlayerResult(player.playerMAC, 0, netGain));
                break;
            }
        }
        return results;
    } else if(street == 4) { 
        std::vector<PlayerResult> results;
        /*
        for(const PlayerState& player : players) {
            if(!player.hasFolded) {
                double handValue = evaluateHand(player.holeCards, communityCards);
                int32_t netGain = player.activeBet + player.inactiveBet;
                for(const PlayerState& otherPlayer : players) {
                    if(otherPlayer.hasFolded) {
                        netGain += otherPlayer.activeBet + otherPlayer.inactiveBet;
                    }
                }
                results.push_back(PlayerResult(player.playerMAC, handValue, netGain));
            }
        }
        std::sort(results.begin(), results.end(), [](const PlayerResult& a, const PlayerResult& b) {
            return a.handValue > b.handValue; // Sort in descending order of hand strength
        });
        return results;
        */
    }
    return std::nullopt;
}