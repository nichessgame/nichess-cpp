#pragma once 

#include <vector>

/*
 * Used for hashing of game states.
 */
class Zobrist {
  public:
    static const std::vector<std::vector<std::vector<long int>>> pieceTypeToSquareToHPToKey;
    static const long int p2Key;
    Zobrist(); 
};
