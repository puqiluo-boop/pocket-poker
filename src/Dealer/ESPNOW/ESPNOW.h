#pragma once
#include "../PlayerRegistry/PlayerRegistry.h"

void broadcastConnectionCheck();

void broadcastCards(const int (&deck)[52], const PlayerRegistry& playerRegistry);