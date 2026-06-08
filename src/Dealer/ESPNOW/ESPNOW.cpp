#include "ESPNOW_Shared.h"
#include "ESPNOW.h"
#include "../PlayerRegistry/PlayerRegistry.h"

void broadcastConnectionCheck() {
    ConnectionMsg packet;
    packet.msgType = MSG_CONNECTION;
    packet.sender = DEALER;
    packet.reciever = BROADCAST;

    sendMessage(packet);
}

void broadcastCards(const int (&deck)[52], const PlayerRegistry& playerRegistry) {
    // Send cards to players 1 through 6
    for (int player = 0; player < MAX_PLAYERS; player++) {
        if(!playerRegistry.isPlayerConnected(player)) {continue;}
        HoleCardsMsg packet;
        
        packet.msgType = MSG_HOLE_CARDS;
        packet.senderID = 0;
        packet.recieverID = player + 1; // Only this specific player should read it
        
        packet.card1 = deck[2 * player];
        packet.card2 = deck[2 * player + 1];
        
        // Send this packet using the template
        sendMessage(packet);
        
        // Small delay to avoid overwhelming the radio
        delay(10);
    }
}