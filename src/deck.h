#ifndef DECK_H
#define DECK_H

#include <Arduino.h>

// ==========================================
// 1. INCLUDE ALL 52 CARD FILES
// ==========================================
// Assumes filenames are like "AceSpades.h"

// --- SPADES ---
#include "cards/AceSpades.h"
#include "cards/TwoSpades.h"
#include "cards/ThreeSpades.h"
#include "cards/FourSpades.h"
#include "cards/FiveSpades.h"
#include "cards/SixSpades.h"
#include "cards/SevenSpades.h"
#include "cards/EightSpades.h"
#include "cards/NineSpades.h"
#include "cards/TenSpades.h"
#include "cards/JackSpades.h"
#include "cards/QueenSpades.h"
#include "cards/KingSpades.h"

// --- HEARTS ---
#include "cards/AceHearts.h"
#include "cards/TwoHearts.h"
#include "cards/ThreeHearts.h"
#include "cards/FourHearts.h"
#include "cards/FiveHearts.h"
#include "cards/SixHearts.h"
#include "cards/SevenHearts.h"
#include "cards/EightHearts.h"
#include "cards/NineHearts.h"
#include "cards/TenHearts.h"
#include "cards/JackHearts.h"
#include "cards/QueenHearts.h"
#include "cards/KingHearts.h"

// --- CLUBS ---
#include "cards/AceClubs.h"
#include "cards/TwoClubs.h"
#include "cards/ThreeClubs.h"
#include "cards/FourClubs.h"
#include "cards/FiveClubs.h"
#include "cards/SixClubs.h"
#include "cards/SevenClubs.h"
#include "cards/EightClubs.h"
#include "cards/NineClubs.h"
#include "cards/TenClubs.h"
#include "cards/JackClubs.h"
#include "cards/QueenClubs.h"
#include "cards/KingClubs.h"

// --- DIAMONDS ---
#include "cards/AceDiamonds.h"
#include "cards/TwoDiamonds.h"
#include "cards/ThreeDiamonds.h"
#include "cards/FourDiamonds.h"
#include "cards/FiveDiamonds.h"
#include "cards/SixDiamonds.h"
#include "cards/SevenDiamonds.h"
#include "cards/EightDiamonds.h"
#include "cards/NineDiamonds.h"
#include "cards/TenDiamonds.h"
#include "cards/JackDiamonds.h"
#include "cards/QueenDiamonds.h"
#include "cards/KingDiamonds.h"

// ==========================================
// 2. THE MASTER DECK ARRAY
// ==========================================
// Uses PascalCase variable names (e.g. AceSpades)

const uint16_t* deck[52] = {
    // Spades (0-12)
    AceSpades, TwoSpades, ThreeSpades, FourSpades, FiveSpades, 
    SixSpades, SevenSpades, EightSpades, NineSpades, TenSpades, 
    JackSpades, QueenSpades, KingSpades,

    // Hearts (13-25)
    AceHearts, TwoHearts, ThreeHearts, FourHearts, FiveHearts, 
    SixHearts, SevenHearts, EightHearts, NineHearts, TenHearts, 
    JackHearts, QueenHearts, KingHearts,

    // Clubs (26-38)
    AceClubs, TwoClubs, ThreeClubs, FourClubs, FiveClubs, 
    SixClubs, SevenClubs, EightClubs, NineClubs, TenClubs, 
    JackClubs, QueenClubs, KingClubs,

    // Diamonds (39-51)
    AceDiamonds, TwoDiamonds, ThreeDiamonds, FourDiamonds, FiveDiamonds, 
    SixDiamonds, SevenDiamonds, EightDiamonds, NineDiamonds, TenDiamonds, 
    JackDiamonds, QueenDiamonds, KingDiamonds
};

#endif