// main.cpp - Player Module
// Just displays UI and forwards actions to dealer

#include <Arduino.h>
#include "UIManager.h"
#include "PlayerComms.h"
#include "Structs.h"
#include "CardDisplay.h"

#define PLAYER_ID 2

// Simple player states
enum PlayerState {
    DISCONNECTED,
    WAITING_FOR_GAME,
    JOIN_CONFIRMATION,
    IN_GAME_ACTIVE,      // My turn
    IN_GAME_INACTIVE     // Not my turn
};

PlayerState currentState = DISCONNECTED;

// Called when dealer sends us a message
void handleIncomingNetworkData(BaseMessage* msg) {
    
    if(msg->recieverID != PLAYER_ID && msg->recieverID != BROADCAST_ID) {return;}

    switch (msg->msgType) {
        
        case MSG_CONNECTION:
            ConnectionMsg* connMsg = (ConnectionMsg*)msg;
            if (connMsg->senderID == 0) { // If it's from the dealer
                Serial.println("Dealer is checking connection. Replying...");
                ConnectionMsg packet;
                packet.msgType = MSG_CONNECTION;
                packet.senderID = PLAYER_ID;
                packet.recieverID = 0;

                sendMessage(packet);
            }
            break;

        case MSG_HOLE_CARDS:
            HoleCardsMsg* hcsMsg = (HoleCardsMsg*)msg;
            setHoleCards(hcsMsg->card1, hcsMsg->card2);
            displayHoleCards();
            break;

        /*
        case MSG_REQUEST_JOIN:
            // Dealer is asking if we want to join
            currentState = JOIN_CONFIRMATION;
            showJoinPopup(msg.buyInAmount);
            break;
            
        case MSG_TURN_START:
            // It's our turn to act
            currentState = IN_GAME_ACTIVE;
            enableButtons();
            break;
            
        case MSG_TURN_END:
            // Our turn is over, wait for others
            currentState = IN_GAME_INACTIVE;
            disableButtons();
            break;
            
        case MSG_HAND_END:
            // Hand is over, show results
            showHandResult(msg.result);
            currentState = WAITING_FOR_GAME;
            break;
            */
    }
}

// Called when user clicks a button
void onPlayerAction(PlayerActionType action, uint32_t betRaiseSize) {
    PlayerActionMsg packet;
    packet.senderID = PLAYER_ID;
    packet.recieverID = 0;
    packet.playerActionType = action;
    packet.betRaiseSize = betRaiseSize;
    sendMessage(packet);
    
    // Disable buttons until dealer responds
    currentState = IN_GAME_INACTIVE;
    disableButtons();
}

void setup() {
    Serial.begin(115200);
    
    // Initialize display
    initUI();
    
    // Show "Connecting..." screen
    //showConnectingScreen();
    
    // Initialize ESP-NOW
    if (initPlayerComms(PLAYER_ID, handleIncomingNetworkData)) {
        currentState = WAITING_FOR_GAME;
        //showWaitingScreen();
    } else {
        //showErrorScreen("Connection failed");
    }
}

void loop() {
    updateUI();  // Just update LVGL, that's it!
}