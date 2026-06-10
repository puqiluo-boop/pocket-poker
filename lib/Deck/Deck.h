#pragma once

#include <Arduino.h>

// ==========================================
// 1. CARD DATA DECLARATIONS
// Avoid including all individual card headers here to prevent include-path issues
// The actual card data headers should define these symbols; declare them as extern
// so this header can be used even when the include path for the card files is not set.

#define CARD_WIDTH 64
#define CARD_HEIGHT 96

// Spades
extern const uint16_t AceSpades[];
extern const uint16_t TwoSpades[];
extern const uint16_t ThreeSpades[];
extern const uint16_t FourSpades[];
extern const uint16_t FiveSpades[];
extern const uint16_t SixSpades[];
extern const uint16_t SevenSpades[];
extern const uint16_t EightSpades[];
extern const uint16_t NineSpades[];
extern const uint16_t TenSpades[];
extern const uint16_t JackSpades[];
extern const uint16_t QueenSpades[];
extern const uint16_t KingSpades[];

// Hearts
extern const uint16_t AceHearts[];
extern const uint16_t TwoHearts[];
extern const uint16_t ThreeHearts[];
extern const uint16_t FourHearts[];
extern const uint16_t FiveHearts[];
extern const uint16_t SixHearts[];
extern const uint16_t SevenHearts[];
extern const uint16_t EightHearts[];
extern const uint16_t NineHearts[];
extern const uint16_t TenHearts[];
extern const uint16_t JackHearts[];
extern const uint16_t QueenHearts[];
extern const uint16_t KingHearts[];

// Clubs
extern const uint16_t AceClubs[];
extern const uint16_t TwoClubs[];
extern const uint16_t ThreeClubs[];
extern const uint16_t FourClubs[];
extern const uint16_t FiveClubs[];
extern const uint16_t SixClubs[];
extern const uint16_t SevenClubs[];
extern const uint16_t EightClubs[];
extern const uint16_t NineClubs[];
extern const uint16_t TenClubs[];
extern const uint16_t JackClubs[];
extern const uint16_t QueenClubs[];
extern const uint16_t KingClubs[];

// Diamonds
extern const uint16_t AceDiamonds[];
extern const uint16_t TwoDiamonds[];
extern const uint16_t ThreeDiamonds[];
extern const uint16_t FourDiamonds[];
extern const uint16_t FiveDiamonds[];
extern const uint16_t SixDiamonds[];
extern const uint16_t SevenDiamonds[];
extern const uint16_t EightDiamonds[];
extern const uint16_t NineDiamonds[];
extern const uint16_t TenDiamonds[];
extern const uint16_t JackDiamonds[];
extern const uint16_t QueenDiamonds[];
extern const uint16_t KingDiamonds[];

// Backside
extern const uint16_t Backside[];

// --- TRIM ---
const int Trim[24][2] = {
    {0, 0},
    {0, 1},
    {0, 2},
    {0, 61},
    {0, 62},
    {0, 63},
    {1, 0},
    {1, 1},
    {1, 62},
    {1, 63},
    {2, 0},
    {2, 63},
    {95, 0},
    {95, 1},
    {95, 2},
    {95, 61},
    {95, 62},
    {95, 63},
    {94, 0},
    {94, 1},
    {94, 62},
    {94, 63},
    {93, 0},
    {93, 63},
};

// ==========================================
// 2. THE MASTER DECK ARRAY
// ==========================================
// Uses PascalCase variable names (e.g. AceSpades)

const uint16_t* deck[52] = {
    // Spades (0-12)
    TwoSpades, ThreeSpades, FourSpades, FiveSpades, 
    SixSpades, SevenSpades, EightSpades, NineSpades, TenSpades, 
    JackSpades, QueenSpades, KingSpades, AceSpades,

    // Hearts (13-25)
    TwoHearts, ThreeHearts, FourHearts, FiveHearts, 
    SixHearts, SevenHearts, EightHearts, NineHearts, TenHearts, 
    JackHearts, QueenHearts, KingHearts, AceHearts,

    // Clubs (26-38)
    TwoClubs, ThreeClubs, FourClubs, FiveClubs, 
    SixClubs, SevenClubs, EightClubs, NineClubs, TenClubs, 
    JackClubs, QueenClubs, KingClubs, AceClubs,

    // Diamonds (39-51)
    TwoDiamonds, ThreeDiamonds, FourDiamonds, FiveDiamonds, 
    SixDiamonds, SevenDiamonds, EightDiamonds, NineDiamonds, TenDiamonds, 
    JackDiamonds, QueenDiamonds, KingDiamonds, AceDiamonds
};

