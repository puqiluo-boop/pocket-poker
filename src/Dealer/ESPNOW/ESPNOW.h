#pragma once
#include "../PlayerRegistry/PlayerRegistry.h"

void broadcastConnectionCheck();

void broadcastCards(std::tuple<uint8_t, uint8_t> cards, String playerMAC);