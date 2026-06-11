#include "CardUtils.h"
#include <Arduino.h>
#include <algorithm>
#include <cstring>

void shuffleDeck(int deck[52]) {
    // Initialize deck in order (0-51)
    for (int i = 0; i < 52; i++) {
        deck[i] = i;
    }
    
    // Fisher-Yates shuffle algorithm
    for (int i = 51; i > 0; i--) {
        int j = random(0, i + 1);  // Random index from 0 to i
        
        // Swap cards at positions i and j
        int temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

uint32_t evaluatePlayerHand(int holeCards[2], const int communityBoard[5]) {
    int fullHand[7]; // Create a temporary 7-card array on the Stack (instantly)

    // 1. Inject the player's specific hole cards
    fullHand[0] = holeCards[0];
    fullHand[1] = holeCards[1];

    // 2. Blast the 5 community cards into the rest of the array
    // memcpy is a low-level hardware command that copies memory instantly
    memcpy(&fullHand[2], communityBoard, 5 * sizeof(int));

    // 3. Pass the assembled array to your master evaluator
    return getHandValue(fullHand);
}

uint8_t cValue(int card) {
    assert(card >= 0 && card < 52);
    uint8_t rank = (card % 13) + 2; // Ranks 2-14 (where 11=Jack, 12=Queen, 13=King, 14=Ace)
    uint8_t suit = (card / 13); // Suits 0-3 (e.g. 0=Spades, 1=Hearts, 2=Clubs, 3=Diamonds)
    return rank | (suit << 4); // Combine rank and suit into a single
}

uint8_t cRank(uint8_t cValue) {
    return cValue & 0xF;
}

uint8_t cSuit(uint8_t cValue) {
    return (cValue >> 4) & 0b0011;
}

uint32_t makeHandValue(uint8_t handRank, uint8_t cv1, uint8_t cv2=0, uint8_t cv3=0, uint8_t cv4=0, uint8_t cv5=0) {
    return (handRank << 20) | ((cv1 & 0xF) << 16) | ((cv2 & 0xF) << 12) | ((cv3 & 0xF) << 8) | ((cv4 & 0xF) << 4) | (cv5 & 0xF);
}

uint32_t getHandValue(int cards[7]) {

    uint8_t cVals[7];
    for(int i = 0; i < 7; i++) {
        cVals[i] = cValue(cards[i]);
    }
    std::sort(cVals, cVals + 7, [](uint8_t a, uint8_t b) {
        return cRank(a) > cRank(b);
    }); // Sort hand by descending rank 

    uint8_t suitCounts[4] = {0};
    for(int i = 0; i < 7; i++) {
        suitCounts[cSuit(cVals[i])]++;
    }

    // STRAIGHT FLUSH
    for(uint8_t suit = 0; suit < 4; suit++) {
        if(suitCounts[suit] < 5) continue;
        int count = 0;
        uint8_t topRank = 0;
        bool hasAce = false;
        for(int i = 0; i < 7; i++) {
            if(cSuit(cVals[i]) != suit) continue;
            uint8_t iRank = cRank(cVals[i]);
            if(iRank == 14) hasAce = true;

            if(topRank == 0) {
                topRank = iRank;
                count = 1;
            } else if(iRank == topRank - count) {
                count++;
            } else {
                count = 1;
                topRank = cRank(cVals[i]);
            }
            if(count == 4 && topRank == 5 && hasAce) count++;

            if(count == 5) return makeHandValue(9, topRank);
        }
    }

    // FOUR OF A KIND
    for(int i = 0; i < 4; i++) {
        if(cRank(cVals[i]) == cRank(cVals[i+3])) {
            return makeHandValue(8, cVals[i], cVals[(i == 0) ? 4 : 0]);
        }
    }

    // FULL HOUSE
    uint8_t threeOAK = 0;
    uint8_t topPair = 0;
    for(int i = 0; i < 6; i++) {
        uint8_t iRank = cRank(cVals[i]);
        if(!threeOAK && i + 2 < 7) {
            if(iRank == cRank(cVals[i+2])) {
                threeOAK = cVals[i];
            }
        }
        if(!topPair && iRank != cRank(threeOAK) && iRank == cRank(cVals[i+1])) {
            topPair = cVals[i];
        }
        if(threeOAK && topPair) {
            return makeHandValue(7, threeOAK, topPair);
        }
    }

    // FLUSH
    for(uint8_t s = 0; s < 4; s++) {
        if(suitCounts[s] < 5) continue;
        uint8_t fCards[5];
        int idx = 0;
        
        for(int i = 0; i < 7; i++) {
            if(cSuit(cVals[i]) != s) continue;
            fCards[idx++] = cVals[i];
            if(idx == 5) break;
        }
        
        return makeHandValue(6, fCards[0], fCards[1], fCards[2], fCards[3], fCards[4]);
    }

    // STRAIGHT
    {
        int count = 0;
        uint8_t topRank = 0;
        uint8_t prevRank = 0;
        bool hasAce = false;

        for(int i = 0; i < 7; i++) {
            uint8_t iRank = cRank(cVals[i]);
            if(iRank == 14) hasAce = true;

            if(iRank == prevRank) continue; 
            prevRank = iRank;
            
            if(topRank == 0) {
                topRank = iRank;
                count = 1;
            } else if(iRank == topRank - count) {
                count++;
            } else {
                count = 1;
                topRank = cRank(cVals[i]);
            }

            if(count == 4 && topRank == 5 && hasAce) count++;

            if(count == 5) return makeHandValue(5, topRank);
        }
    }

    // THREE OF A KIND
    if(threeOAK) {
        uint8_t kicker1 = 0;
        uint8_t kicker2 = 0;
        for(int i = 0; i < 7; i++) {
            if(cRank(cVals[i]) == cRank(threeOAK)) continue;
            if(!kicker1) {
                kicker1 = cVals[i];
            } else {
                kicker2 = cVals[i];
                break;
            }
        }
        return makeHandValue(4, threeOAK, kicker1, kicker2);
    }

    // TWO PAIR + PAIR
    if(topPair) {
        uint8_t secondPair = 0;
        uint8_t kicker1 = 0;
        uint8_t kicker2 = 0;
        uint8_t kicker3 = 0;

        for(int i = 0; i < 7; i++) {
            uint8_t iRank = cRank(cVals[i]);
            if(iRank == cRank(topPair)) continue;
            if(!secondPair && i + 1 < 7 && iRank == cRank(cVals[i+1])) {
                secondPair = cVals[i++];
            } else if(!kicker1) {
                kicker1 = cVals[i];
            } else if(!kicker2) {
                kicker2 = cVals[i];
            } else if(!kicker3) {
                kicker3 = cVals[i];
            }
        }

        if(secondPair) {
            return makeHandValue(3, topPair, secondPair, kicker1);
        } else {
            return makeHandValue(2, topPair, kicker1, kicker2, kicker3);
        }
    }

    // HIGH CARD
    return makeHandValue(1, cVals[0], cVals[1], cVals[2], cVals[3], cVals[4]);
}

uint8_t getHandRank(uint32_t handValue) {
    return (handValue >> 20) & 0xF;
}