#include "nichess/util.hpp"
#include <cmath>

std::string playerToString(Player p) {
  switch(p) {
    case PLAYER_1:
      return "PLAYER_1";
    case PLAYER_2:
      return "PLAYER_2";
    default:
      return "default";
  }
}

std::string pieceTypeToString(PieceType pt) {
  switch(pt) {
    case P1_KING:
      return "K";
    case P1_MAGE:
      return "M";
    case P1_PAWN:
      return "P";
    case P1_WARRIOR:
      return "W";
    case P1_ASSASSIN:
      return "A";
    case P1_KNIGHT:
      return "N";
    case P2_KING:
      return "k";
    case P2_MAGE:
      return "m";
    case P2_PAWN:
      return "p";
    case P2_WARRIOR:
      return "w";
    case P2_ASSASSIN:
      return "a";
    case P2_KNIGHT:
      return "n";
    case NO_PIECE:
      return ".";
    default:
      return "default";
  }
}

bool player1OrEmpty(PieceType pt) {
  switch(pt) {
    case P1_KING:
      return true;
    case P1_MAGE:
      return true;
    case P1_PAWN:
      return true;
    case P1_WARRIOR:
      return true;
    case P1_ASSASSIN:
      return true;
    case P1_KNIGHT:
      return true;
    case P2_KING:
      return false;
    case P2_MAGE:
      return false;
    case P2_PAWN:
      return false;
    case P2_WARRIOR:
      return false;
    case P2_ASSASSIN:
      return false;
    case P2_KNIGHT:
      return false;
    case NO_PIECE:
      return true;
  }
  return false;
}

bool player2OrEmpty(PieceType pt) {
  switch(pt) {
    case P1_KING:
      return false;
    case P1_MAGE:
      return false;
    case P1_PAWN:
      return false;
    case P1_WARRIOR:
      return false;
    case P1_ASSASSIN:
      return false;
    case P1_KNIGHT:
      return false;
    case P2_KING:
      return true;
    case P2_MAGE:
      return true;
    case P2_PAWN:
      return true;
    case P2_WARRIOR:
      return true;
    case P2_ASSASSIN:
      return true;
    case P2_KNIGHT:
      return true;
    case NO_PIECE:
      return true;
  }
  return false;
}

bool pieceBelongsToPlayer(PieceType pt, Player player) {
  switch(pt) {
    case P1_KING:
        return player == PLAYER_1;
      case P1_MAGE:
        return player == PLAYER_1;
      case P1_PAWN:
        return player == PLAYER_1;
      case P1_WARRIOR:
        return player == PLAYER_1;
      case P1_ASSASSIN:
        return player == PLAYER_1;
      case P1_KNIGHT:
        return player == PLAYER_1;
      case P2_KING:
        return player == PLAYER_2;
      case P2_MAGE:
        return player == PLAYER_2;
      case P2_PAWN:
        return player == PLAYER_2;
      case P2_WARRIOR:
        return player == PLAYER_2;
      case P2_ASSASSIN:
        return player == PLAYER_2;
      case P2_KNIGHT:
        return player == PLAYER_2;
      case NO_PIECE:
        return false;
    }
  return false;
}

bool isOffBoard(int x, int y) {
  if(x >= NUM_COLUMNS || x < 0 || y >= NUM_ROWS || y < 0)
    return true;
  else
    return false;
}

bool isOffBoard(int squareIndex) {
  if(squareIndex < 0 || squareIndex > NUM_ROWS * NUM_COLUMNS - 1)
    return true;
  else
    return false;
}

std::vector<std::vector<int>> generateSquareToNeighboringDiagonalSquares() {
  std::vector<std::vector<int>> squareToNeighboringDiagonalSquares{NUM_SQUARES};
  for(int srcY = 0; srcY < NUM_ROWS; srcY++) {
    for(int srcX = 0; srcX < NUM_COLUMNS; srcX++) {
      std::vector<int> neighboringDiagonalSquares;
      int srcIndex = coordinatesToBoardIndex(srcX, srcY);
      for(int k = -1; k < 2; k++) {
        for(int l = -1; l < 2; l++) {
          if(k == 0 || l == 0) continue;
          int newX = srcX + k;
          int newY = srcY + l;
          if(isOffBoard(newX, newY)) continue;
          int newIndex = coordinatesToBoardIndex(newX, newY);
          neighboringDiagonalSquares.push_back(newIndex);
        }
      }
      squareToNeighboringDiagonalSquares[srcIndex] = neighboringDiagonalSquares;  
    }
  }
  return squareToNeighboringDiagonalSquares;
}

std::vector<std::vector<int>> generateSquareToNeighboringNonDiagonalSquares() {
  std::vector<std::vector<int>> squareToNeighboringNonDiagonalSquares{NUM_SQUARES};
  for(int srcY = 0; srcY < NUM_ROWS; srcY++) {
    for(int srcX = 0; srcX < NUM_COLUMNS; srcX++) {
      std::vector<int> neighboringNonDiagonalSquares;
      int srcIndex = coordinatesToBoardIndex(srcX, srcY);
      for(int k = -1; k < 2; k++) {
        for(int l = -1; l < 2; l++) {
          if(k == 0 && l == 0) continue;
          if(std::abs(k) == 1 && std::abs(l) == 1) continue;
          int newX = srcX + k;
          int newY = srcY + l;
          if(isOffBoard(newX, newY)) continue;
          int newIndex = coordinatesToBoardIndex(newX, newY);
          neighboringNonDiagonalSquares.push_back(newIndex);
        }
      }
      squareToNeighboringNonDiagonalSquares[srcIndex] = neighboringNonDiagonalSquares;  
    }
  }
  return squareToNeighboringNonDiagonalSquares;
}

std::vector<std::vector<int>> generateSquareToNeighboringSquares() {
  std::vector<std::vector<int>> squareToNeighboringSquares{NUM_SQUARES};
  for(int srcY = 0; srcY < NUM_ROWS; srcY++) {
    for(int srcX = 0; srcX < NUM_COLUMNS; srcX++) {
      std::vector<int> neighboringSquares;
      int srcIndex = coordinatesToBoardIndex(srcX, srcY);
      for(int k = -1; k < 2; k++) {
        for(int l = -1; l < 2; l++) {
          if(k == 0 && l == 0) continue;
          int newX = srcX + k;
          int newY = srcY + l;
          if(isOffBoard(newX, newY)) continue;
          int newIndex = coordinatesToBoardIndex(newX, newY);
          neighboringSquares.push_back(newIndex);
        }
      }
      squareToNeighboringSquares[srcIndex] = neighboringSquares;  
    }
  }
  return squareToNeighboringSquares;
}

std::vector<std::vector<std::vector<int>>> generateSquareToDirectionToLine() {
  std::vector<std::vector<std::vector<int>>> squareToDirectionToLine{NUM_SQUARES};
  for(int srcY = 0; srcY < NUM_ROWS; srcY++) {
    for(int srcX = 0; srcX < NUM_COLUMNS; srcX++) {
      int squareIndex = coordinatesToBoardIndex(srcX, srcY);
      std::vector<std::vector<int>> directionToLine{NUM_DIRECTIONS};

      std::vector<int> north;
      for(int d = 1; d < NUM_ROWS; d++) {
        int newX = srcX;
        int newY = srcY + d;
        if(isOffBoard(newX, newY)) break;
        int newIndex = coordinatesToBoardIndex(newX, newY);
        north.push_back(newIndex);
      }

      std::vector<int> northeast;
      for(int d = 1; d < NUM_COLUMNS; d++) {
        int newX = srcX + d;
        int newY = srcY + d;
        if(isOffBoard(newX, newY)) break;
        int newIndex = coordinatesToBoardIndex(newX, newY);
        northeast.push_back(newIndex);
      }

      std::vector<int> east;
      for(int d = 1; d < NUM_COLUMNS; d++) {
        int newX = srcX + d;
        int newY = srcY;
        if(isOffBoard(newX, newY)) break;
        int newIndex = coordinatesToBoardIndex(newX, newY);
        east.push_back(newIndex);
      }

      std::vector<int> southeast;
      for(int d = 1; d < NUM_COLUMNS; d++) {
        int newX = srcX + d;
        int newY = srcY - d;
        if(isOffBoard(newX, newY)) break;
        int newIndex = coordinatesToBoardIndex(newX, newY);
        southeast.push_back(newIndex);
      }

      std::vector<int> south;
      for(int d = 1; d < NUM_ROWS; d++) {
        int newX = srcX;
        int newY = srcY - d;
        if(isOffBoard(newX, newY)) break;
        int newIndex = coordinatesToBoardIndex(newX, newY);
        south.push_back(newIndex);
      }

      std::vector<int> southwest;
      for(int d = 1; d < NUM_COLUMNS; d++) {
        int newX = srcX - d;
        int newY = srcY - d;
        if(isOffBoard(newX, newY)) break;
        int newIndex = coordinatesToBoardIndex(newX, newY);
        southwest.push_back(newIndex);
      }

      std::vector<int> west;
      for(int d = 1; d < NUM_COLUMNS; d++) {
        int newX = srcX - d;
        int newY = srcY;
        if(isOffBoard(newX, newY)) break;
        int newIndex = coordinatesToBoardIndex(newX, newY);
        west.push_back(newIndex);
      }

      std::vector<int> northwest;
      for(int d = 1; d < NUM_COLUMNS; d++) {
        int newX = srcX - d;
        int newY = srcY + d;
        if(isOffBoard(newX, newY)) break;
        int newIndex = coordinatesToBoardIndex(newX, newY);
        northwest.push_back(newIndex);
      }

      std::vector<int> invalid;

      directionToLine[Direction::NORTH] = north;
      directionToLine[Direction::NORTHEAST] = northeast;
      directionToLine[Direction::EAST] = east;
      directionToLine[Direction::SOUTHEAST] = southeast;
      directionToLine[Direction::SOUTH] = south;
      directionToLine[Direction::SOUTHWEST] = southwest;
      directionToLine[Direction::WEST] = west;
      directionToLine[Direction::NORTHWEST] = northwest;
      directionToLine[Direction::INVALID] = invalid;

      squareToDirectionToLine[squareIndex] = directionToLine;
    }
  }
  return squareToDirectionToLine;
}

std::vector<std::vector<Direction>> generateSrcSquareToDstSquareToDirection() {
  std::vector<std::vector<Direction>> srcSquareToDstSquareToDirection{NUM_SQUARES};
  for(int srcY = 0; srcY < NUM_ROWS; srcY++) {
    for(int srcX = 0; srcX < NUM_COLUMNS; srcX++) {
      int srcIndex = coordinatesToBoardIndex(srcX, srcY);
      std::vector<Direction> dstSquareToDirection{NUM_SQUARES};
      for(int dstY = 0; dstY < NUM_ROWS; dstY++) {
        for(int dstX = 0; dstX < NUM_COLUMNS; dstX++) {
          int dstIndex = coordinatesToBoardIndex(dstX, dstY);
          int dx = dstX - srcX;
          int dy = dstY - srcY;
          if(dx == 0 && dy > 0) {
            dstSquareToDirection[dstIndex] = Direction::NORTH;
          } else if(dx == dy && dx > 0) {
            dstSquareToDirection[dstIndex] = Direction::NORTHEAST;
          } else if(dx > 0 && dy == 0) {
            dstSquareToDirection[dstIndex] = Direction::EAST;
          } else if(dx == (-dy) && dx > 0) {
            dstSquareToDirection[dstIndex] = Direction::SOUTHEAST;
          } else if(dx == 0 && dy < 0) {
            dstSquareToDirection[dstIndex] = Direction::SOUTH;
          } else if(dx == dy && dx < 0) {
            dstSquareToDirection[dstIndex] = Direction::SOUTHWEST;
          } else if(dx < 0 && dy == 0) {
            dstSquareToDirection[dstIndex] = Direction::WEST;
          } else if(dx == -(dy) && dx < 0) {
            dstSquareToDirection[dstIndex] = Direction::NORTHWEST;
          } else {
            dstSquareToDirection[dstIndex] = Direction::INVALID;
          }
        }
      }
      srcSquareToDstSquareToDirection[srcIndex] = dstSquareToDirection;
    }
  }
  return srcSquareToDstSquareToDirection;
}

std::vector<std::vector<int>> generateSquareToP1PawnMoveSquares() {
  std::vector<std::vector<int>> squareToP1PawnMoves{NUM_SQUARES};
  for(int srcY = 0; srcY < NUM_ROWS; srcY++) {
    for(int srcX = 0; srcX < NUM_COLUMNS; srcX++) {
      std::vector<int> p1PawnSquares;
      int srcIndex = coordinatesToBoardIndex(srcX, srcY);
      int newX = srcX;
      int newY = srcY + 1;
      if(!isOffBoard(newX, newY)) {
        int newIndex = coordinatesToBoardIndex(newX, newY);
        p1PawnSquares.push_back(newIndex);
      }

      if(srcY == 1) {
        // p1 pawn can also go 2 squares north
        int newX = srcX;
        int newY = srcY + 2;
        if(!isOffBoard(newX, newY)) {
          int newIndex = coordinatesToBoardIndex(newX, newY);
          p1PawnSquares.push_back(newIndex);
        }
      }
      squareToP1PawnMoves[srcIndex] = p1PawnSquares;  
    }
  }
  return squareToP1PawnMoves;
}

std::vector<std::vector<int>> generateSquareToP2PawnMoveSquares() {
  std::vector<std::vector<int>> squareToP2PawnMoves{NUM_SQUARES};
  for(int srcY = 0; srcY < NUM_ROWS; srcY++) {
    for(int srcX = 0; srcX < NUM_COLUMNS; srcX++) {
      std::vector<int> p2PawnSquares;
      int srcIndex = coordinatesToBoardIndex(srcX, srcY);
      int newX = srcX;
      int newY = srcY - 1;
      if(!isOffBoard(newX, newY)) {
        int newIndex = coordinatesToBoardIndex(newX, newY);
        p2PawnSquares.push_back(newIndex);
      }

      if(srcY == 6) {
        // p2 pawn can also go 2 squares south
        int newX = srcX;
        int newY = srcY - 2;
        if(!isOffBoard(newX, newY)) {
          int newIndex = coordinatesToBoardIndex(newX, newY);
          p2PawnSquares.push_back(newIndex);
        }
      }
      squareToP2PawnMoves[srcIndex] = p2PawnSquares;  
    }
  }
  return squareToP2PawnMoves;
}

std::vector<std::vector<int>> generateSquareToP1PawnAbilitySquares() {
  std::vector<std::vector<int>> squareToP1PawnAbilities{NUM_SQUARES};
  for(int srcY = 0; srcY < NUM_ROWS; srcY++) {
    for(int srcX = 0; srcX < NUM_COLUMNS; srcX++) {
      std::vector<int> p1PawnSquares;
      int srcIndex = coordinatesToBoardIndex(srcX, srcY);
      int newX = srcX - 1;
      int newY = srcY + 1;
      if(!isOffBoard(newX, newY)) {
        int newIndex = coordinatesToBoardIndex(newX, newY);
        p1PawnSquares.push_back(newIndex);
      }
      newX = srcX + 1;
      newY = srcY + 1;
      if(!isOffBoard(newX, newY)) {
        int newIndex = coordinatesToBoardIndex(newX, newY);
        p1PawnSquares.push_back(newIndex);
      }
      squareToP1PawnAbilities[srcIndex] = p1PawnSquares;  
    }
  }
  return squareToP1PawnAbilities;
}

std::vector<std::vector<int>> generateSquareToP2PawnAbilitySquares() {
  std::vector<std::vector<int>> squareToP2PawnAbilities{NUM_SQUARES};
  for(int srcY = 0; srcY < NUM_ROWS; srcY++) {
    for(int srcX = 0; srcX < NUM_COLUMNS; srcX++) {
      std::vector<int> p2PawnSquares;
      int srcIndex = coordinatesToBoardIndex(srcX, srcY);
      int newX = srcX - 1;
      int newY = srcY - 1;
      if(!isOffBoard(newX, newY)) {
        int newIndex = coordinatesToBoardIndex(newX, newY);
        p2PawnSquares.push_back(newIndex);
      }
      newX = srcX + 1;
      newY = srcY - 1;
      if(!isOffBoard(newX, newY)) {
        int newIndex = coordinatesToBoardIndex(newX, newY);
        p2PawnSquares.push_back(newIndex);
      }
      squareToP2PawnAbilities[srcIndex] = p2PawnSquares;  
    }
  }
  return squareToP2PawnAbilities;
}

std::vector<std::vector<int>> generateSquareToKnightActionSquares() {
  std::vector<std::vector<int>> squareToKnightSquares{NUM_SQUARES};
  for(int row = 0; row < NUM_ROWS; row++) {
    for(int column = 0; column < NUM_COLUMNS; column++) {
      int srcSquareIndex = coordinatesToBoardIndex(column, row);
      std::vector<int> squares;

      int move_dst_x1 = column + 2;
      int move_dst_y1 = row + 1;
      int move_dst_x2 = column + 2;
      int move_dst_y2 = row - 1;
      int move_dst_x3 = column + 1;
      int move_dst_y3 = row - 2;
      int move_dst_x4 = column - 1;
      int move_dst_y4 = row - 2;
      int move_dst_x5 = column - 2;
      int move_dst_y5 = row - 1;
      int move_dst_x6 = column - 2;
      int move_dst_y6 = row + 1;
      int move_dst_x7 = column - 1;
      int move_dst_y7 = row + 2;
      int move_dst_x8 = column + 1;
      int move_dst_y8 = row + 2;

      if(!isOffBoard(move_dst_x1, move_dst_y1)){
        squares.push_back(coordinatesToBoardIndex(move_dst_x1, move_dst_y1));
      }
      if(!isOffBoard(move_dst_x2, move_dst_y2)){
        squares.push_back(coordinatesToBoardIndex(move_dst_x2, move_dst_y2));
      }
      if(!isOffBoard(move_dst_x3, move_dst_y3)){
        squares.push_back(coordinatesToBoardIndex(move_dst_x3, move_dst_y3));
      }
      if(!isOffBoard(move_dst_x4, move_dst_y4)){
        squares.push_back(coordinatesToBoardIndex(move_dst_x4, move_dst_y4));
      }

      if(!isOffBoard(move_dst_x5, move_dst_y5)){
        squares.push_back(coordinatesToBoardIndex(move_dst_x5, move_dst_y5));
      }
      if(!isOffBoard(move_dst_x6, move_dst_y6)){
        squares.push_back(coordinatesToBoardIndex(move_dst_x6, move_dst_y6));
      }
      if(!isOffBoard(move_dst_x7, move_dst_y7)){
        squares.push_back(coordinatesToBoardIndex(move_dst_x7, move_dst_y7));
      }
      if(!isOffBoard(move_dst_x8, move_dst_y8)){
        squares.push_back(coordinatesToBoardIndex(move_dst_x8, move_dst_y8));
      }
      squareToKnightSquares[srcSquareIndex] = squares;
    }
  }
  return squareToKnightSquares;
}
