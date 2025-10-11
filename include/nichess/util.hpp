#pragma once

#include "nichess.hpp"

#include <string>

using namespace nichess;

std::string playerToString(Player p);
std::string pieceTypeToString(PieceType pt);
bool player1OrEmpty(PieceType pt);
bool player2OrEmpty(PieceType pt);
bool pieceBelongsToPlayer(PieceType pt, Player player);
bool isOffBoard(int x, int y);
bool isOffBoard(int squareIndex);
std::vector<std::vector<int>> generateSquareToNeighboringSquares();
std::vector<std::vector<int>> generateSquareToNeighboringDiagonalSquares();
std::vector<std::vector<int>> generateSquareToNeighboringNonDiagonalSquares();
std::vector<std::vector<std::vector<int>>> generateSquareToDirectionToLine();
std::vector<std::vector<Direction>> generateSrcSquareToDstSquareToDirection();
std::vector<std::vector<int>> generateSquareToP1PawnMoveSquares();
std::vector<std::vector<int>> generateSquareToP2PawnMoveSquares();
std::vector<std::vector<int>> generateSquareToP1PawnAbilitySquares();
std::vector<std::vector<int>> generateSquareToP2PawnAbilitySquares();
std::vector<std::vector<int>> generateSquareToKnightActionSquares();
