#include "../CardUtils/CardUtils.h"
#include "GameState.h"
#include <Arduino.h>
#include "../PlayerRegistry/PlayerRegistry.h"

#include <vector>
#include <optional>
#include <algorithm>

bool GameState::findActivePlayer(uint8_t start, bool forwards = true) {
    uint8_t savedTurn = start;
    while(players[playerTurn].hasFolded || players[playerTurn].remainingChips == 0) {
        playerTurn = (playerTurn + (forwards ? 1 : -1)) % players.size();
        if(playerTurn == savedTurn) { // We've looped through all players and found no active players
            return false;
        }
    }
    return true;
}

bool GameState::gameActive() {
    if(street == 4) {
        return false;
    }
    uint8_t activePlayers = 0;
    uint8_t nonFoldedPlayers = 0;
    bool lastActivePlayerHasCalled = false;
    for(const PlayerState& player : players) {
        if(!player.hasFolded) {
            nonFoldedPlayers++;
            if(player.remainingChips > 0) {
                activePlayers++;
                lastActivePlayerHasCalled = player.activeBet >= callAmount;
            }
        }
    }
    
    if (nonFoldedPlayers == 1) {
        return false;
    } else if(activePlayers == 1 && lastActivePlayerHasCalled) {
        return false;
    } else if(activePlayers == 0) {
        return false;
    } else {
        return true;
    }
}

GameState::GameState(const int deck[52], const uint32_t smallBlind, const uint32_t bigBlind, const std::vector<PlayerInfo>& playerInfo) :
communityCards{deck[0], deck[1], deck[2], deck[3], deck[4]},
smallBlind(smallBlind),
bigBlind(bigBlind) {
    assert(validPlayers(playerInfo));

    PlayerState smallBlindPlayer = PlayerState(playerInfo[0].playerMAC, (uint8_t*)deck[5], playerInfo[0].chipCount);
    smallBlindPlayer.activeBet = std::min(smallBlind, smallBlindPlayer.remainingChips);
    smallBlindPlayer.remainingChips -= std::min(smallBlind, smallBlindPlayer.remainingChips);
    players.push_back(smallBlindPlayer);

    PlayerState bigBlindPlayer = PlayerState(playerInfo[1].playerMAC, (uint8_t*)deck[7], playerInfo[1].chipCount);
    bigBlindPlayer.activeBet = std::min(bigBlind, bigBlindPlayer.remainingChips);
    bigBlindPlayer.remainingChips -= std::min(bigBlind, bigBlindPlayer.remainingChips);
    players.push_back(bigBlindPlayer);

    for (size_t i = 2; i < playerInfo.size(); i++) {
        PlayerState player = PlayerState(playerInfo[i].playerMAC, (uint8_t*)deck[5 + (i * 2)], playerInfo[i].chipCount);
        players.push_back(player);
    }

    playerTurn = 2 % players.size(); // Start with player to the left of big blind
    street = 0;
    callAmount = bigBlind;
    roundCutoff = 2 % players.size();
    minRaiseIncrement = bigBlind;
}

/**
 * Increments playerTurn to the next player. Can end rounds and the game entirely.
 */
bool GameState::incTurn() {
    if(!gameActive()) return false;
    if(lastAction()) {
        return nextStreet();
    } else {
        playerTurn = (playerTurn + 1) % players.size();

        PlayerState& player = players[playerTurn];
        if(player.remainingChips == 0 || player.hasFolded) { // Skip players who are all in or have folded
            return incTurn();
        }
    }
    return true;
}

/**
 * @return true if the current player's (in)action will end the round
 */
bool GameState::lastAction() {
    return (playerTurn + 1) % players.size() == roundCutoff;
}

bool GameState::nextStreet() {
    if(!gameActive()) return false;
    street++;
    playerTurn = 0;
    findActivePlayer(0);
    callAmount = 0;
    roundCutoff = 0;
    minRaiseIncrement = bigBlind;
    for(PlayerState& player : players) {
        player.hasActed = false;
        player.inactiveBet += player.activeBet;
        player.activeBet = 0;
    }
    return true;
}

bool GameState::check() {
    if(!gameActive()) {return false;}
    PlayerState& player = players[playerTurn];
    if(callAmount == player.activeBet) {
        player.hasActed = true;
        incTurn();
        return true;
    }
    return false;
}

bool GameState::call() {
    if(!gameActive() || callAmount == 0) return false;
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
    if(!gameActive()) {return false;}
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
    roundCutoff = playerTurn;

    player.hasActed = true;
    incTurn();
    return true;
}

bool GameState::fold() {
    assert(players[playerTurn].remainingChips > 0); // Player cannot fold if they are already all-in
    if(!gameActive()) {return false;}
    players[playerTurn].hasFolded = true;
    incTurn();
    return true;
}

std::vector<Pot> GameState::getPots() {
    std::vector<Pot> pots;
    std::vector<std::tuple<const PlayerState&, uint32_t>> incBetAllInPriority; // Vector of (playerState, totalBet) sorted in ascending order of totalBet, with all-in players sorted before non all-in players with the same totalBet
    for(const PlayerState& player : players) {
        if(player.activeBet > 0 || player.inactiveBet > 0) {
            incBetAllInPriority.push_back(std::make_tuple(player, player.activeBet + player.inactiveBet));
        }
    }
    std::sort(incBetAllInPriority.begin(), incBetAllInPriority.end(), [](const std::tuple<const PlayerState&, uint32_t>& a, const std::tuple<const PlayerState&, uint32_t>& b) {
        if (std::get<1>(a) == std::get<1>(b)) { // If totalBet is the same, sort all-in players first
            bool aAllIn = std::get<0>(a).remainingChips == 0;
            bool bAllIn = std::get<0>(b).remainingChips == 0;
            if (aAllIn && !bAllIn) {
                return true;
            } else if (!aAllIn && bAllIn) {
                return false;
            }
        }
        return std::get<1>(a) < std::get<1>(b);
    });
    
    std::vector<const PlayerState&> activePlayers;
    for(const PlayerState& player : players) {
        // If they haven't folded and aren't all-in (chips > 0), they are active
        if(!player.hasFolded && player.remainingChips > 0) {
            activePlayers.push_back(player);
        }
    }

    uint32_t potTracker = 0;
    std::vector<const PlayerState&> currentPotPlayers = activePlayers;
    for(size_t i = 0; i < incBetAllInPriority.size(); i++) {
        auto& [player, totalBet] = incBetAllInPriority[i];
        if(totalBet == 0) continue;
        if(player.remainingChips == 0) {
            assert(player.hasFolded); // A folded player cannot be all-in
            currentPotPlayers.push_back(player);
            potTracker += totalBet;
            for(size_t j = i + 1; j < incBetAllInPriority.size(); j++) {
                auto& [otherPlayer, otherTotalBet] = incBetAllInPriority[j];
                potTracker += totalBet;
                otherTotalBet -= totalBet;
                if(otherPlayer.remainingChips == 0) {
                    assert(!otherPlayer.hasFolded); // A folded player cannot be all-in
                    currentPotPlayers.push_back(otherPlayer);
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
        std::vector<const PlayerState&> validPlayers;
        validPlayers.reserve(players.size());
        for(const PlayerState& player : players) { // Inefficient loop for asserting folded status.
            assert(!player.hasFolded); // If no bets were made, no players should have folded
            validPlayers.push_back(player);
        }
        pots.push_back(Pot(0, validPlayers));
    }

    return pots;
}

uint8_t GameState::numNonfoldedPlayers() {
    uint8_t count = 0;
    for(const PlayerState& player : players) {
        if(!player.hasFolded) {
            count++;
        }
    }
    return count;
}

std::optional<std::vector<PlayerResult>> GameState::getResults() {
    if(gameActive()) return std::nullopt;
    if(numNonfoldedPlayers() == 1) {
        std::vector<PlayerResult> results;
        results.reserve(players.size());

        std::vector<Pot> pots = getPots();
        assert(pots.size() == 1); // If there is only one non-folded player, there should only be the main pot
        Pot& mainPot = pots[0];
        assert(mainPot.eligiblePlayers.size() == 1); // If there is only one non-folded player, they should be the only eligible player for the main pot
        const PlayerState& winner = mainPot.eligiblePlayers[0];

        results.push_back(PlayerResult(winner.playerMAC, 0, mainPot.size - winner.inactiveBet - winner.activeBet - winner.remainingChips)); // TODO: Subtract total starting chips from pot size to get net gain.
        for(const PlayerState& player : players) {
            if(!player.hasFolded) {
                int32_t netGain = -player.inactiveBet - player.activeBet;
                results.push_back(PlayerResult(player.playerMAC, 0, netGain)); // Hand value is 0 since it doesn't matter in this scenario
            }
        }

        return results;
    } else { 
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
}

uint8_t GameState::getPlayerTurn() {
    return playerTurn;
}

uint8_t GameState::getStreet() {
    return street;
}