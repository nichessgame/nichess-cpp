#pragma once

#include <vector>
#include "nichess/constants.hpp"

namespace nichess {
/*
 * Used for faster generation and validation of actions.
 */
class GameCache {
  public:
    static const std::vector<std::vector<int>> squareToNeighboringSquares;
    static const std::vector<std::vector<int>> squareToNeighboringNonDiagonalSquares;
    static const std::vector<std::vector<std::vector<int>>> squareToDirectionToLine;
    static const std::vector<std::vector<Direction>> srcSquareToDstSquareToDirection;
    static const std::vector<std::vector<int>> squareToKnightActionSquares;
    static const std::vector<std::vector<int>> squareToP1PawnMoveSquares;
    static const std::vector<std::vector<int>> squareToP2PawnMoveSquares;
    static const std::vector<std::vector<int>> squareToP1PawnAbilitySquares;
    static const std::vector<std::vector<int>> squareToP2PawnAbilitySquares;
    void print();
    
    GameCache(); 
};

} // namespace nichess
