#include "nichess/nichess.hpp"
#include "nichess/util.hpp"
#include "nichess/zobrist.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include <tuple>
#include <chrono>
#include <algorithm>

using namespace nichess;

int nichess::coordinatesToBoardIndex(int column, int row) {
  return column + row * NUM_COLUMNS;
}

std::tuple<int, int> nichess::boardIndexToCoordinates(int squareIndex) {
  int x = squareIndex -  (floor(squareIndex / NUM_ROWS) * NUM_ROWS);
  int y = floor(squareIndex / NUM_ROWS);
  return std::tuple<int, int>(x, y);
}

PlayerAction::PlayerAction() { }

PlayerAction::PlayerAction(int srcIdx, int dstIdx, ActionType actionType): srcIdx(srcIdx), dstIdx(dstIdx), actionType(actionType) { }

Piece::Piece(): type(PieceType::NO_PIECE), healthPoints(0), squareIndex(0) { }

Piece::Piece(PieceType type, int healthPoints, int squareIndex):
  type(type),
  healthPoints(healthPoints),
  squareIndex(squareIndex)
{ }

Piece::Piece(const Piece& other): type(other.type), healthPoints(other.healthPoints), squareIndex(other.squareIndex) { }

bool Piece::operator==(const Piece& other) const {
  const auto* other_cs = dynamic_cast<const Piece*>(&other);
  if (other_cs == nullptr) {
    return false;
  }
  return (other_cs->type == type && other_cs->healthPoints == healthPoints && other_cs->squareIndex == squareIndex);
}

bool Piece::operator!=(const Piece& other) const {
  const auto* other_cs = dynamic_cast<const Piece*>(&other);
  if (other_cs == nullptr) {
    return true;
  }
  return (other_cs->type != type || other_cs->healthPoints != healthPoints || other_cs->squareIndex != squareIndex);
}

UndoInfo::UndoInfo() {
  this->affectedPieces.reserve(9);
}

UndoInfo::UndoInfo(PlayerAction playerAction) {
  this->affectedPieces.reserve(9);
  this->action = playerAction;
}

GameCache::GameCache() {}

std::string intToDirectionString(int i) {
  switch(i) {
    case Direction::NORTH:
      return "NORTH";
    case Direction::NORTHEAST:
      return "NORTHEAST";
    case Direction::EAST:
      return "EAST";
    case Direction::SOUTHEAST:
      return "SOUTHEAST";
    case Direction::SOUTH:
      return "SOUTH";
    case Direction::SOUTHWEST:
      return "SOUTHWEST";
    case Direction::WEST:
      return "WEST";
    case Direction::NORTHWEST:
      return "NORTHWEST";
    case Direction::INVALID:
      return "INVALID";
    default:
      return "OUT_OF_BOUNDS";
  }
}

template<typename T>
std::string convert2DVectorToString(const std::vector<std::vector<T>>& vec2D) {
    std::stringstream ss;
    
    // Start with outer vector declaration
    ss << "{\n";
    
    // Iterate through rows
    for (size_t i = 0; i < vec2D.size(); ++i) {
        // Skip empty vectors
        if (vec2D[i].empty()) {
            ss << "    {}";
            if (i < vec2D.size() - 1) {
                ss << ",";
            }
            ss << "\n";
            continue;
        }
        
        // Add inner vector opening brace
        ss << "    {";
        
        // First element of row
        if(typeid(T) == typeid(Direction)) {
          ss << intToDirectionString(vec2D[i][0]);
        } else {
          ss << vec2D[i][0];
        }
        
        // Remaining elements of row
        for (size_t j = 1; j < vec2D[i].size(); ++j) {
          if(typeid(T) == typeid(Direction)) {
            ss << ", " << intToDirectionString(vec2D[i][j]);
          } else {
            ss << ", " << vec2D[i][j];
          }
        }
        
        // Close inner vector
        ss << "}";
        
        // Add comma after all rows except last
        if (i < vec2D.size() - 1) {
            ss << ",";
        }
        
        // Newline after each row
        ss << "\n";
    }
    
    // Add closing brace and semicolon
    ss << "\n};\n";
    
    return ss.str();
}

template<typename T>
std::string convert3DVectorToString(const std::vector<std::vector<std::vector<T>>>& vec) {
    std::ostringstream oss;
    
    // Opening brace
    oss << "{\n";
    
    // Iterate through outermost vector
    for(size_t i = 0; i < vec.size(); ++i) {
        // Add comma and newline if not first element
        if(i > 0) oss << ",\n";
        
        // Indentation for readability
        oss << "    ";
        
        // Second level opening brace
        oss << "{";
        
        // Iterate through middle dimension
        for(size_t j = 0; j < vec[i].size(); ++j) {
            // Add comma and space if not first element
            if(j > 0) oss << ", ";
            
            // Third level opening brace
            oss << "{";
            
            // Iterate through innermost dimension
            for(size_t k = 0; k < vec[i][j].size(); ++k) {
                // Add comma and space if not first element
                if(k > 0) oss << ", ";
                
                // Output actual value
                oss << vec[i][j][k];
            }
            
            // Close third level brace
            oss << "}";
        }
        
        // Close second level brace
        oss << "}";
    }
    
    // Newline and closing brace
    oss << "\n}";
    
    return oss.str();
}

void GameCache::print() {
  // TODO: Make this print everything nicely so that it can be copy pasted into a header file.
  std::string s1 = "std::vector<std::vector<int>> squareToP1PawnMoveSquares = " + convert2DVectorToString(generateSquareToP1PawnMoveSquares());
  std::cout << s1 << "\n\n\n";
  std::string s2 = "std::vector<std::vector<int>> squareToP2PawnMoveSquares = " + convert2DVectorToString(generateSquareToP2PawnMoveSquares());
  std::cout << s2 << "\n\n\n";
  std::string s3 = "std::vector<std::vector<int>> squareToP1PawnAbilitySquares = " + convert2DVectorToString(generateSquareToP1PawnAbilitySquares());
  std::cout << s3 << "\n\n\n";
  std::string s4 = "std::vector<std::vector<int>> squareToP2PawnAbilitySquares = " + convert2DVectorToString(generateSquareToP2PawnAbilitySquares());
  std::cout << s4 << "\n\n\n";
}

void Game::reset() {
  moveNumber = 0;
  currentPlayer = Player::PLAYER_1;
  // Create starting position
  board[coordinatesToBoardIndex(0,0)] = new Piece(PieceType::P1_WARRIOR, WARRIOR_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(0,0));
  board[coordinatesToBoardIndex(1,0)] = new Piece(PieceType::P1_KNIGHT, KNIGHT_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(1,0));
  board[coordinatesToBoardIndex(2,0)] = new Piece(PieceType::P1_ASSASSIN, ASSASSIN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(2,0));
  board[coordinatesToBoardIndex(3,0)] = new Piece(PieceType::P1_MAGE, MAGE_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(3,0));
  board[coordinatesToBoardIndex(4,0)] = new Piece(PieceType::P1_KING, KING_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(4,0));
board[coordinatesToBoardIndex(5,0)] = new Piece(PieceType::P1_ASSASSIN, ASSASSIN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(5,0));
  board[coordinatesToBoardIndex(6,0)] = new Piece(PieceType::P1_KNIGHT, KNIGHT_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(6,0));
  board[coordinatesToBoardIndex(7,0)] = new Piece(PieceType::P1_WARRIOR, WARRIOR_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(7,0));

  board[coordinatesToBoardIndex(0,1)] = new Piece(PieceType::P1_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(0,1));
  board[coordinatesToBoardIndex(1,1)] = new Piece(PieceType::P1_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(1,1));
  board[coordinatesToBoardIndex(2,1)] = new Piece(PieceType::P1_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(2,1));
  board[coordinatesToBoardIndex(3,1)] = new Piece(PieceType::P1_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(3,1));
  board[coordinatesToBoardIndex(4,1)] = new Piece(PieceType::P1_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(4,1));
  board[coordinatesToBoardIndex(5,1)] = new Piece(PieceType::P1_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(5,1));
  board[coordinatesToBoardIndex(6,1)] = new Piece(PieceType::P1_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(6,1));
  board[coordinatesToBoardIndex(7,1)] = new Piece(PieceType::P1_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(7,1));

  board[coordinatesToBoardIndex(0,7)] = new Piece(PieceType::P2_WARRIOR, WARRIOR_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(0,7));
  board[coordinatesToBoardIndex(1,7)] = new Piece(PieceType::P2_KNIGHT, KNIGHT_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(1,7));
  board[coordinatesToBoardIndex(2,7)] = new Piece(PieceType::P2_ASSASSIN, ASSASSIN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(2,7));
  board[coordinatesToBoardIndex(3,7)] = new Piece(PieceType::P2_MAGE, MAGE_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(3,7));
  board[coordinatesToBoardIndex(4,7)] = new Piece(PieceType::P2_KING, KING_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(4,7));
   board[coordinatesToBoardIndex(5,7)] = new Piece(PieceType::P2_ASSASSIN, ASSASSIN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(5,7));
   board[coordinatesToBoardIndex(6,7)] = new Piece(PieceType::P2_KNIGHT, KNIGHT_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(6,7));
  board[coordinatesToBoardIndex(7,7)] = new Piece(PieceType::P2_WARRIOR, WARRIOR_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(7,7));

  board[coordinatesToBoardIndex(0,6)] = new Piece(PieceType::P2_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(0,6));
  board[coordinatesToBoardIndex(1,6)] = new Piece(PieceType::P2_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(1,6));
  board[coordinatesToBoardIndex(2,6)] = new Piece(PieceType::P2_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(2,6));
  board[coordinatesToBoardIndex(3,6)] = new Piece(PieceType::P2_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(3,6));
  board[coordinatesToBoardIndex(4,6)] = new Piece(PieceType::P2_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(4,6));
  board[coordinatesToBoardIndex(5,6)] = new Piece(PieceType::P2_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(5,6));
  board[coordinatesToBoardIndex(6,6)] = new Piece(PieceType::P2_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(6,6));
  board[coordinatesToBoardIndex(7,6)] = new Piece(PieceType::P2_PAWN, PAWN_STARTING_HEALTH_POINTS, coordinatesToBoardIndex(7,6));

  board[coordinatesToBoardIndex(0,2)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(0,2));
  board[coordinatesToBoardIndex(0,3)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(0,3));
  board[coordinatesToBoardIndex(0,4)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(0,4));
  board[coordinatesToBoardIndex(0,5)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(0,5));
  board[coordinatesToBoardIndex(1,2)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(1,2));
  board[coordinatesToBoardIndex(1,3)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(1,3));
  board[coordinatesToBoardIndex(1,4)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(1,4));
  board[coordinatesToBoardIndex(1,5)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(1,5));
  board[coordinatesToBoardIndex(2,2)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(2,2));
  board[coordinatesToBoardIndex(2,3)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(2,3));
  board[coordinatesToBoardIndex(2,4)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(2,4));
  board[coordinatesToBoardIndex(2,5)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(2,5));
  board[coordinatesToBoardIndex(3,2)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(3,2));
  board[coordinatesToBoardIndex(3,3)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(3,3));
  board[coordinatesToBoardIndex(3,4)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(3,4));
  board[coordinatesToBoardIndex(3,5)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(3,5));
  board[coordinatesToBoardIndex(4,2)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(4,2));
  board[coordinatesToBoardIndex(4,3)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(4,3));
  board[coordinatesToBoardIndex(4,4)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(4,4));
  board[coordinatesToBoardIndex(4,5)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(4,5));
  board[coordinatesToBoardIndex(5,2)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(5,2));
  board[coordinatesToBoardIndex(5,3)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(5,3));
  board[coordinatesToBoardIndex(5,4)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(5,4));
  board[coordinatesToBoardIndex(5,5)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(5,5));
  board[coordinatesToBoardIndex(6,2)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(6,2));
  board[coordinatesToBoardIndex(6,3)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(6,3));
  board[coordinatesToBoardIndex(6,4)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(6,4));
  board[coordinatesToBoardIndex(6,5)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(6,5));
  board[coordinatesToBoardIndex(7,2)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(7,2));
  board[coordinatesToBoardIndex(7,3)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(7,3));
  board[coordinatesToBoardIndex(7,4)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(7,4));
  board[coordinatesToBoardIndex(7,5)] = new Piece(PieceType::NO_PIECE, 0, coordinatesToBoardIndex(7,5));

  std::vector<Piece*> p1Pieces{NUM_STARTING_PIECES};
  p1Pieces[0] = board[coordinatesToBoardIndex(0,0)];
  p1Pieces[1] = board[coordinatesToBoardIndex(1,0)];
  p1Pieces[2] = board[coordinatesToBoardIndex(2,0)];
  p1Pieces[3] = board[coordinatesToBoardIndex(3,0)];
  p1Pieces[4] = board[coordinatesToBoardIndex(4,0)];
  p1Pieces[5] = board[coordinatesToBoardIndex(5,0)];
  p1Pieces[6] = board[coordinatesToBoardIndex(6,0)];
  p1Pieces[7] = board[coordinatesToBoardIndex(7,0)];

  p1Pieces[8] = board[coordinatesToBoardIndex(0,1)];
  p1Pieces[9] = board[coordinatesToBoardIndex(1,1)];
  p1Pieces[10] = board[coordinatesToBoardIndex(2,1)];
  p1Pieces[11] = board[coordinatesToBoardIndex(3,1)];
  p1Pieces[12] = board[coordinatesToBoardIndex(4,1)];
  p1Pieces[13] = board[coordinatesToBoardIndex(5,1)];
  p1Pieces[14] = board[coordinatesToBoardIndex(6,1)];
  p1Pieces[15] = board[coordinatesToBoardIndex(7,1)];
  playerToPieces[Player::PLAYER_1] = p1Pieces;
  playerToKing[Player::PLAYER_1] = p1Pieces[4];

  std::vector<Piece*> p2Pieces{NUM_STARTING_PIECES};
  p2Pieces[0] = board[coordinatesToBoardIndex(0,7)];
  p2Pieces[1] = board[coordinatesToBoardIndex(1,7)];
  p2Pieces[2] = board[coordinatesToBoardIndex(2,7)];
  p2Pieces[3] = board[coordinatesToBoardIndex(3,7)];
  p2Pieces[4] = board[coordinatesToBoardIndex(4,7)];
  p2Pieces[5] = board[coordinatesToBoardIndex(5,7)];
  p2Pieces[6] = board[coordinatesToBoardIndex(6,7)];
  p2Pieces[7] = board[coordinatesToBoardIndex(7,7)];

  p2Pieces[8] = board[coordinatesToBoardIndex(0,6)];
  p2Pieces[9] = board[coordinatesToBoardIndex(1,6)];
  p2Pieces[10] = board[coordinatesToBoardIndex(2,6)];
  p2Pieces[11] = board[coordinatesToBoardIndex(3,6)];
  p2Pieces[12] = board[coordinatesToBoardIndex(4,6)];
  p2Pieces[13] = board[coordinatesToBoardIndex(5,6)];
  p2Pieces[14] = board[coordinatesToBoardIndex(6,6)];
  p2Pieces[15] = board[coordinatesToBoardIndex(7,6)];

  playerToPieces[Player::PLAYER_2] = p2Pieces;
  playerToKing[Player::PLAYER_2] = p2Pieces[4];

  long int zh = zobristHash();
  repetitions.insert({zh, 1});
  repetitionsDraw = false;
}

Game::Game() {
  reset();
}

Game::Game(const Game& other) {
  this->moveNumber = other.moveNumber;
  this->currentPlayer = other.currentPlayer;
  for(int i = 0; i < NUM_SQUARES; i++) {
    board[i] = new Piece(*other.board[i]);
  }

  std::vector<Piece*> otherP1Pieces = other.playerToPieces[PLAYER_1];
  std::vector<Piece*> otherP2Pieces = other.playerToPieces[PLAYER_2];
  std::vector<Piece*> p1Pieces;
  std::vector<Piece*> p2Pieces;

  for(int i = 0; i < otherP1Pieces.size(); i++) {
    Piece* currentPiece = otherP1Pieces[i];
    if(currentPiece->healthPoints > 0) {
      p1Pieces.push_back(board[currentPiece->squareIndex]);
    } else {
      p1Pieces.push_back(new Piece(currentPiece->type, currentPiece->healthPoints, currentPiece->squareIndex));
    }

  }
  Piece* otherP1King = other.playerToKing[Player::PLAYER_1];
  playerToKing[Player::PLAYER_1] = board[otherP1King->squareIndex];
  playerToPieces[Player::PLAYER_1] = p1Pieces;

  for(int i = 0; i < otherP2Pieces.size(); i++) {
    Piece* currentPiece = otherP2Pieces[i];
    if(currentPiece->healthPoints > 0) {
      p2Pieces.push_back(board[currentPiece->squareIndex]);
    } else {
      p2Pieces.push_back(new Piece(currentPiece->type, currentPiece->healthPoints, currentPiece->squareIndex));
    }

  }
  Piece* otherP2King = other.playerToKing[Player::PLAYER_2];
  playerToKing[Player::PLAYER_2] = board[otherP2King->squareIndex];
  playerToPieces[Player::PLAYER_2] = p2Pieces;
  repetitions = other.repetitions;
  repetitionsDraw = other.repetitionsDraw;
}

Game::Game(const std::string encodedBoard) {
  boardFromString(encodedBoard);
  long int zh = zobristHash();
  repetitions.insert({zh, 1});
  repetitionsDraw = false;
}

Game::~Game() {
  for(Piece* p: playerToPieces[PLAYER_1]) {
    if(p->healthPoints <= 0) {
      delete p;
    }
  }
  for(Piece* p: playerToPieces[PLAYER_2]) {
    if(p->healthPoints <= 0) {
      delete p;
    }
  }
  
  for(int i = 0; i < NUM_SQUARES; i++) {
      delete board[i];
  }
}


/*
 * Assumes the action is legal.
 */
UndoInfo Game::makeAction(PlayerAction playerAction) {
  UndoInfo undoInfo = UndoInfo(playerAction);
  if(playerAction.actionType != ActionType::SKIP) {
    int srcIdx = playerAction.srcIdx;
    int dstIdx = playerAction.dstIdx;
    Piece* abilitySrcPiece = board[srcIdx];
    Piece* abilityDstPiece = board[dstIdx];
    Piece *currentPiece;
    int currentSquare;
    Direction direction;
    const std::vector<int> *directionLine;
    const std::vector<int> *squares;
    int idx;
    Player opponentPlayer;
    switch(playerAction.actionType) {
      case ActionType::MOVE_REGULAR:
        delete board[dstIdx];
        board[dstIdx] = board[srcIdx];
        board[dstIdx]->squareIndex = dstIdx;
        board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
        break;
      case ActionType::MOVE_CASTLE:
        // move king
        delete board[dstIdx];
        board[dstIdx] = this->board[srcIdx];
        board[dstIdx]->squareIndex = dstIdx;
        board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);

        // move warrior
        // based on king's destination we can determine which castle it is
        if(dstIdx == 6) {
          //p1 short castle
          delete board[5];
          board[5] = this->board[7];
          board[5]->squareIndex = 5;
          board[7] = new Piece(PieceType::NO_PIECE, 0, 7);
        } else if(dstIdx == 2) {
          //p1 long castle
          delete board[3];
          board[3] = this->board[0];
          board[3]->squareIndex = 3;
          board[0] = new Piece(PieceType::NO_PIECE, 0, 0);
        } else if(dstIdx == 62) {
          //p2 short castle
          delete board[61];
          board[61] = this->board[63];
          board[61]->squareIndex = 61;
          board[63] = new Piece(PieceType::NO_PIECE, 0, 63);
        } else { // p2 long castle
          delete board[59];
          board[59] = this->board[56];
          board[59]->squareIndex = 59;
          board[56] = new Piece(PieceType::NO_PIECE, 0, 56);
        }
        break;
      case ActionType::MOVE_PROMOTE_P1_PAWN:
        undoInfo.t1 = abilitySrcPiece->healthPoints;
        delete board[dstIdx];
        board[dstIdx] = board[srcIdx];
        board[dstIdx]->squareIndex = dstIdx;
        board[dstIdx]->type = PieceType::P1_WARRIOR;
        board[dstIdx]->healthPoints = WARRIOR_STARTING_HEALTH_POINTS;
        board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
        break;
      case ActionType::MOVE_PROMOTE_P2_PAWN:
        undoInfo.t1 = abilitySrcPiece->healthPoints;
        delete board[dstIdx];
        board[dstIdx] = board[srcIdx];
        board[dstIdx]->squareIndex = dstIdx;
        board[dstIdx]->type = PieceType::P2_WARRIOR;
        board[dstIdx]->healthPoints = WARRIOR_STARTING_HEALTH_POINTS;
        board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
        break;
      // king does single target damage
      case ActionType::ABILITY_KING_DAMAGE:
        abilityDstPiece->healthPoints -= KING_ABILITY_POINTS;
        undoInfo.affectedPieces.push_back(abilityDstPiece);
        if(abilityDstPiece->healthPoints <= 0) {
          // move piece to the destroyed piece's location
          this->board[dstIdx] = abilitySrcPiece;
          this->board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
          abilitySrcPiece->squareIndex = dstIdx;
        }
        break;
      case ActionType::ABILITY_MAGE_DAMAGE:
        abilityDstPiece->healthPoints -= MAGE_ABILITY_POINTS;
        undoInfo.affectedPieces.push_back(abilityDstPiece);
        if(abilityDstPiece->healthPoints <= 0) {
          // move piece to the destroyed piece's location
          this->board[dstIdx] = abilitySrcPiece;
          this->board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
          abilitySrcPiece->squareIndex = dstIdx;
        } else {
          direction = GameCache::srcSquareToDstSquareToDirection[srcIdx][dstIdx];
          directionLine = &GameCache::squareToDirectionToLine[srcIdx][direction];
          idx = 0;
          while(true) {
            currentPiece = board[(*directionLine)[idx]];
            if(!(currentPiece->type == PieceType::NO_PIECE)) break;
            idx++;
          }
          if(idx != 0) {
            // leap
            currentSquare = (*directionLine)[idx-1];
            undoInfo.t1 = currentSquare;
            
            delete board[currentSquare];
            board[currentSquare] = board[srcIdx];
            board[currentSquare]->squareIndex = currentSquare;
            board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
          }
        }
        break;
      case ActionType::ABILITY_MAGE_THROW_ASSASSIN:
        if(abilitySrcPiece->type == P1_MAGE) {
          opponentPlayer = Player::PLAYER_2;
        } else {
          opponentPlayer = Player::PLAYER_1;
        }
        direction = GameCache::srcSquareToDstSquareToDirection[srcIdx][dstIdx];
        directionLine = &GameCache::squareToDirectionToLine[srcIdx][direction];
        idx = (*directionLine)[0]; // assassin to be thrown is at this index
        undoInfo.t1 = idx;
        undoInfo.affectedPieces.push_back(board[idx]);

        abilityDstPiece->healthPoints -= MAGE_THROW_DAMAGE_1;
        undoInfo.affectedPieces.push_back(abilityDstPiece);

        // throw assassin
        board[dstIdx] = board[idx];
        board[dstIdx]->squareIndex = dstIdx;
        board[idx] = new Piece(PieceType::NO_PIECE, 0, idx);


        // AOE damage
        squares = &GameCache::squareToNeighboringSquares[dstIdx];
        for(int i = 0; i < squares->size(); i++) {
          currentPiece = board[(*squares)[i]];
          if(pieceBelongsToPlayer(currentPiece->type, opponentPlayer)) {
            currentPiece->healthPoints -= MAGE_THROW_DAMAGE_2;
            undoInfo.affectedPieces.push_back(currentPiece);
            if(currentPiece->healthPoints <= 0) {
              board[currentPiece->squareIndex] = new Piece(PieceType::NO_PIECE, 0, currentPiece->squareIndex);
            }
          }
        }

        break;
      case ActionType::ABILITY_PAWN_DAMAGE:
        abilityDstPiece->healthPoints -= PAWN_ABILITY_POINTS;
        undoInfo.affectedPieces.push_back(abilityDstPiece);
        if(abilityDstPiece->healthPoints <= 0) {
          // move piece to the destroyed piece's location
          this->board[dstIdx] = abilitySrcPiece;
          this->board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
          abilitySrcPiece->squareIndex = dstIdx;
        }
        break;
      case ActionType::ABILITY_P1_PAWN_DAMAGE_AND_PROMOTION:
        undoInfo.t1 = abilitySrcPiece->healthPoints;

        abilityDstPiece->healthPoints -= PAWN_ABILITY_POINTS;
        undoInfo.affectedPieces.push_back(abilityDstPiece);

        board[dstIdx] = board[srcIdx];
        board[dstIdx]->squareIndex = dstIdx;
        board[dstIdx]->type = PieceType::P1_WARRIOR;
        board[dstIdx]->healthPoints = WARRIOR_STARTING_HEALTH_POINTS;
        board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
        break;
      case ActionType::ABILITY_P2_PAWN_DAMAGE_AND_PROMOTION:
        undoInfo.t1 = abilitySrcPiece->healthPoints;

        abilityDstPiece->healthPoints -= PAWN_ABILITY_POINTS;
        undoInfo.affectedPieces.push_back(abilityDstPiece);

        board[dstIdx] = board[srcIdx];
        board[dstIdx]->squareIndex = dstIdx;
        board[dstIdx]->type = PieceType::P2_WARRIOR;
        board[dstIdx]->healthPoints = WARRIOR_STARTING_HEALTH_POINTS;
        board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
        break;
      case ActionType::ABILITY_WARRIOR_DAMAGE:
        abilityDstPiece->healthPoints -= WARRIOR_ABILITY_POINTS;
        undoInfo.affectedPieces.push_back(abilityDstPiece);
        if(abilityDstPiece->healthPoints <= 0) {
          // move piece to the destroyed piece's location
          this->board[dstIdx] = abilitySrcPiece;
          this->board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
          abilitySrcPiece->squareIndex = dstIdx;
        } else {
          direction = GameCache::srcSquareToDstSquareToDirection[srcIdx][dstIdx];
          directionLine = &GameCache::squareToDirectionToLine[srcIdx][direction];
          idx = 0;
          while(true) {
            currentPiece = board[(*directionLine)[idx]];
            if(!(currentPiece->type == PieceType::NO_PIECE)) break;
            idx++;
          }
          if(idx != 0) {
            // leap
            currentSquare = (*directionLine)[idx-1];
            undoInfo.t1 = currentSquare;
            
            delete board[currentSquare];
            board[currentSquare] = board[srcIdx];
            board[currentSquare]->squareIndex = currentSquare;
            board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
          }
        }
        break;
      case ActionType::ABILITY_ASSASSIN_DAMAGE:
        abilityDstPiece->healthPoints -= ASSASSIN_ABILITY_POINTS;
        undoInfo.affectedPieces.push_back(abilityDstPiece);
        if(abilityDstPiece->healthPoints <= 0) {
          // move piece to the destroyed piece's location
          this->board[dstIdx] = abilitySrcPiece;
          this->board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
          abilitySrcPiece->squareIndex = dstIdx;
        } else {
          direction = GameCache::srcSquareToDstSquareToDirection[srcIdx][dstIdx];
          directionLine = &GameCache::squareToDirectionToLine[srcIdx][direction];
          idx = 0;
          while(true) {
            currentPiece = board[(*directionLine)[idx]];
            if(!(currentPiece->type == PieceType::NO_PIECE)) break;
            idx++;
          }
          if(idx != 0) {
            // leap
            currentSquare = (*directionLine)[idx-1];
            undoInfo.t1 = currentSquare;
            
            delete board[currentSquare];
            board[currentSquare] = board[srcIdx];
            board[currentSquare]->squareIndex = currentSquare;
            board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
          }
        }
        break;
      case ActionType::ABILITY_WARRIOR_THROW_WARRIOR:
        if(abilitySrcPiece->type == P1_WARRIOR) {
          opponentPlayer = Player::PLAYER_2;
        } else {
          opponentPlayer = Player::PLAYER_1;
        }
        direction = GameCache::srcSquareToDstSquareToDirection[srcIdx][dstIdx];
        directionLine = &GameCache::squareToDirectionToLine[srcIdx][direction];
        idx = (*directionLine)[0]; // warrior to be thrown is at this index
        undoInfo.t1 = idx;
        undoInfo.affectedPieces.push_back(board[idx]);

        abilityDstPiece->healthPoints -= WARRIOR_THROW_DAMAGE_1;
        undoInfo.affectedPieces.push_back(abilityDstPiece);

        // throw warrior
        board[dstIdx] = board[idx];
        board[dstIdx]->squareIndex = dstIdx;
        board[idx] = new Piece(PieceType::NO_PIECE, 0, idx);

        // AOE damage
        squares = &GameCache::squareToNeighboringSquares[dstIdx];
        for(int i = 0; i < squares->size(); i++) {
          currentPiece = board[(*squares)[i]];
          if(pieceBelongsToPlayer(currentPiece->type, opponentPlayer)) {
            currentPiece->healthPoints -= WARRIOR_THROW_DAMAGE_2;
            undoInfo.affectedPieces.push_back(currentPiece);
            if(currentPiece->healthPoints <= 0) {
              board[currentPiece->squareIndex] = new Piece(PieceType::NO_PIECE, 0, currentPiece->squareIndex);
            }
          }
        }
        break;
      case ActionType::ABILITY_KNIGHT_DAMAGE:
        abilityDstPiece->healthPoints -= KNIGHT_ABILITY_POINTS;
        undoInfo.affectedPieces.push_back(abilityDstPiece);
        if(abilityDstPiece->healthPoints <= 0) {
          // move piece to the destroyed piece's location
          this->board[dstIdx] = abilitySrcPiece;
          this->board[srcIdx] = new Piece(PieceType::NO_PIECE, 0, srcIdx);
          abilitySrcPiece->squareIndex = dstIdx;
        }
        break;
    }
  } 
  this->moveNumber += 1;
  this->currentPlayer = ~currentPlayer;

  long int zh = zobristHash();
  auto it = repetitions.find(zh);
  if(it != repetitions.end()) {
    if(it->second == 2) {
      repetitionsDraw = true;
    }
    repetitions[zh] = it->second + 1;
  } else {
    repetitions.insert({zh, 1});
  }
  return undoInfo;
}

void Game::undoAction(UndoInfo undoInfo) {
  long int zh = zobristHash();
  auto it = repetitions.find(zh);
  if(it == repetitions.end()) {
    throw std::runtime_error("Attempted to undo position that's not in the repetitions map.");
  } else {
    if(it->second == 3) {
      repetitionsDraw = false;
    }
    repetitions[zh] = it->second - 1;
  }

  int srcIdx = undoInfo.action.srcIdx;
  int dstIdx = undoInfo.action.dstIdx;
  Piece *affectedPiece, *currentPiece;
  std::vector<Piece*> *pieces;
  std::vector<Piece*>::iterator position;
  switch(undoInfo.action.actionType) {
    case ActionType::MOVE_REGULAR:
      undoMove(undoInfo.action);
      break;
    case ActionType::MOVE_CASTLE:
      undoMove(undoInfo.action);
      break;
    case ActionType::MOVE_PROMOTE_P1_PAWN:
      delete board[srcIdx];
      board[srcIdx] = board[dstIdx];
      board[srcIdx]->squareIndex = srcIdx;
      board[srcIdx]->type = PieceType::P1_PAWN;
      board[srcIdx]->healthPoints = undoInfo.t1;
      board[dstIdx] = new Piece(PieceType::NO_PIECE, 0, dstIdx);
      break;
    case ActionType::MOVE_PROMOTE_P2_PAWN:
      delete board[srcIdx];
      board[srcIdx] = board[dstIdx];
      board[srcIdx]->squareIndex = srcIdx;
      board[srcIdx]->type = PieceType::P2_PAWN;
      board[srcIdx]->healthPoints = undoInfo.t1;
      board[dstIdx] = new Piece(PieceType::NO_PIECE, 0, dstIdx);
      break;
    case ActionType::ABILITY_KING_DAMAGE:
      affectedPiece = undoInfo.affectedPieces[0];
      affectedPiece->healthPoints += KING_ABILITY_POINTS;
      if(this->board[affectedPiece->squareIndex] != affectedPiece) {
        // Piece was destroyed.
        // Move king to previous location
        delete this->board[srcIdx];
        this->board[srcIdx] = this->board[dstIdx];
        this->board[srcIdx]->squareIndex = srcIdx;
        this->board[dstIdx] = affectedPiece;
      }
      if(this->board[affectedPiece->squareIndex]->type == PieceType::NO_PIECE) {
        delete this->board[affectedPiece->squareIndex];
      }
      this->board[affectedPiece->squareIndex] = affectedPiece;
      break;
    case ActionType::ABILITY_MAGE_DAMAGE:
      affectedPiece = undoInfo.affectedPieces[0];
      affectedPiece->healthPoints += MAGE_ABILITY_POINTS;
      if(this->board[affectedPiece->squareIndex] != affectedPiece) {
        // Piece was destroyed.
        // Move mage to previous location
        delete this->board[srcIdx];
        this->board[srcIdx] = this->board[dstIdx];
        this->board[srcIdx]->squareIndex = srcIdx;
        this->board[dstIdx] = affectedPiece;
      } else if(undoInfo.t1 != -1) {
        // undo leap
        delete board[srcIdx];
        board[srcIdx] = board[undoInfo.t1];
        board[srcIdx]->squareIndex = srcIdx;
        board[undoInfo.t1] = new Piece(PieceType::NO_PIECE, 0, undoInfo.t1);
      }
      break;
    case ActionType::ABILITY_MAGE_THROW_ASSASSIN:
      affectedPiece = undoInfo.affectedPieces[0];  // thrown assassin is always at 0th index.
      affectedPiece->squareIndex = undoInfo.t1;
      // This is always true, but if someone were to change the rules so that throws don't always
      // destroy the enemy piece, things would break without this check.
      // TODO: Find other places where it's assumed that a piece will be destroyed and add similar checks.
      if(this->board[affectedPiece->squareIndex]->type == PieceType::NO_PIECE) {
        delete this->board[affectedPiece->squareIndex];
      }
      this->board[affectedPiece->squareIndex] = affectedPiece;

      affectedPiece = undoInfo.affectedPieces[1];  // destroyed piece is always at 1st index
      affectedPiece->healthPoints += MAGE_THROW_DAMAGE_1;
      this->board[affectedPiece->squareIndex] = affectedPiece;

      // AOE
      for(int i = 2; i < undoInfo.affectedPieces.size(); i++) {
        affectedPiece = undoInfo.affectedPieces[i];
        affectedPiece->healthPoints += MAGE_THROW_DAMAGE_2;
        if(this->board[affectedPiece->squareIndex]->type == PieceType::NO_PIECE) {
          delete this->board[affectedPiece->squareIndex];
        }
        this->board[affectedPiece->squareIndex] = affectedPiece;
      }
      break;
    case ActionType::ABILITY_WARRIOR_DAMAGE:
      affectedPiece = undoInfo.affectedPieces[0];
      affectedPiece->healthPoints += WARRIOR_ABILITY_POINTS;
      if(this->board[affectedPiece->squareIndex] != affectedPiece) {
        // Piece was destroyed.
        // Move warrior to previous location
        delete this->board[srcIdx];
        this->board[srcIdx] = this->board[dstIdx];
        this->board[srcIdx]->squareIndex = srcIdx;
        this->board[dstIdx] = affectedPiece;
      } else if(undoInfo.t1 != -1) {
        // undo leap
        delete board[srcIdx];
        board[srcIdx] = board[undoInfo.t1];
        board[srcIdx]->squareIndex = srcIdx;
        board[undoInfo.t1] = new Piece(PieceType::NO_PIECE, 0, undoInfo.t1);
      }
      break;
    case ActionType::ABILITY_ASSASSIN_DAMAGE:
      affectedPiece = undoInfo.affectedPieces[0];
      affectedPiece->healthPoints += ASSASSIN_ABILITY_POINTS;
      if(this->board[affectedPiece->squareIndex] != affectedPiece) {
        // Piece was destroyed.
        // Move assassin to previous location
        delete this->board[srcIdx];
        this->board[srcIdx] = this->board[dstIdx];
        this->board[srcIdx]->squareIndex = srcIdx;
        this->board[dstIdx] = affectedPiece;
      } else if(undoInfo.t1 != -1) {
        // undo leap
        delete board[srcIdx];
        board[srcIdx] = board[undoInfo.t1];
        board[srcIdx]->squareIndex = srcIdx;
        board[undoInfo.t1] = new Piece(PieceType::NO_PIECE, 0, undoInfo.t1);
      }
      break;
    case ActionType::ABILITY_WARRIOR_THROW_WARRIOR:
      affectedPiece = undoInfo.affectedPieces[0];  // thrown warrior is always at 0th index.
      affectedPiece->squareIndex = undoInfo.t1;
      // This is always true, but if someone were to change the rules so that throws don't always
      // destroy the enemy piece, things would break without this check.
      // TODO: Find other places where it's assumed that a piece will be destroyed and add similar checks.
      if(this->board[affectedPiece->squareIndex]->type == PieceType::NO_PIECE) {
        delete this->board[affectedPiece->squareIndex];
      }
      this->board[affectedPiece->squareIndex] = affectedPiece;

      affectedPiece = undoInfo.affectedPieces[1];  // destroyed piece is always at 1st index
      affectedPiece->healthPoints += WARRIOR_THROW_DAMAGE_1;
      this->board[affectedPiece->squareIndex] = affectedPiece;

      // AOE
      for(int i = 2; i < undoInfo.affectedPieces.size(); i++) {
        affectedPiece = undoInfo.affectedPieces[i];
        affectedPiece->healthPoints += WARRIOR_THROW_DAMAGE_2;
        if(this->board[affectedPiece->squareIndex]->type == PieceType::NO_PIECE) {
          delete this->board[affectedPiece->squareIndex];
        }
        this->board[affectedPiece->squareIndex] = affectedPiece;
      }
      break;
    case ActionType::ABILITY_KNIGHT_DAMAGE:
      affectedPiece = undoInfo.affectedPieces[0];
      affectedPiece->healthPoints += KNIGHT_ABILITY_POINTS;
      if(this->board[affectedPiece->squareIndex] != affectedPiece) {
        // Piece was destroyed.
        // Move knight to previous location
        delete this->board[srcIdx];
        this->board[srcIdx] = this->board[dstIdx];
        this->board[srcIdx]->squareIndex = srcIdx;
        this->board[dstIdx] = affectedPiece;
      }
      if(this->board[affectedPiece->squareIndex]->type == PieceType::NO_PIECE) {
        delete this->board[affectedPiece->squareIndex];
      }
      this->board[affectedPiece->squareIndex] = affectedPiece;
      break;
    case ActionType::ABILITY_PAWN_DAMAGE:
      affectedPiece = undoInfo.affectedPieces[0];
      affectedPiece->healthPoints += PAWN_ABILITY_POINTS;
      if(this->board[affectedPiece->squareIndex] != affectedPiece) {
        // Piece was destroyed.
        // Move pawn to previous location
        delete this->board[srcIdx];
        this->board[srcIdx] = this->board[dstIdx];
        this->board[srcIdx]->squareIndex = srcIdx;
        this->board[dstIdx] = affectedPiece;
      }
      if(this->board[affectedPiece->squareIndex]->type == PieceType::NO_PIECE) {
        delete this->board[affectedPiece->squareIndex];
      }
      this->board[affectedPiece->squareIndex] = affectedPiece;
      break;
    case ActionType::ABILITY_P1_PAWN_DAMAGE_AND_PROMOTION:
      // transform warrior -> pawn and move back
      delete board[srcIdx];
      board[srcIdx] = board[dstIdx];
      board[srcIdx]->squareIndex = srcIdx;
      board[srcIdx]->healthPoints = undoInfo.t1;
      board[srcIdx]->type = PieceType::P1_PAWN;

      // restore captured piece
      affectedPiece = undoInfo.affectedPieces[0];
      affectedPiece->healthPoints += PAWN_ABILITY_POINTS;
      board[dstIdx] = affectedPiece;
      break;
    case ActionType::ABILITY_P2_PAWN_DAMAGE_AND_PROMOTION:
      // transform warrior -> pawn and move back
      delete board[srcIdx];
      board[srcIdx] = board[dstIdx];
      board[srcIdx]->squareIndex = srcIdx;
      board[srcIdx]->healthPoints = undoInfo.t1;
      board[srcIdx]->type = PieceType::P2_PAWN;

      // restore captured piece
      affectedPiece = undoInfo.affectedPieces[0];
      affectedPiece->healthPoints += PAWN_ABILITY_POINTS;
      board[dstIdx] = affectedPiece;
      break;
    case ActionType::SKIP:
      break;
    default:
      break;
  }
  this->moveNumber -= 1;
  this->currentPlayer = ~currentPlayer;
}

std::string Game::dump() const {
  std::string retval = "";
  retval += std::string("------------------------------------------\n");
  for(int i = NUM_ROWS-1; i >= 0; i--) {
    retval += std::to_string(i) + std::string("   ");
    for(int j = 0; j < NUM_COLUMNS; j++) {
      if(board[coordinatesToBoardIndex(j, i)]->type != PieceType::NO_PIECE) {
        retval += pieceTypeToString(board[coordinatesToBoardIndex(j, i)]->type) + std::to_string(board[coordinatesToBoardIndex(j, i)]->healthPoints) + std::string(" ");
      } else {
        retval += pieceTypeToString(board[coordinatesToBoardIndex(j, i)]->type) + std::string("   ") + std::string(" ");
      }
    }
    retval += std::string("\n");
  } 
  retval += std::string("\n");
  retval += std::string("   ");
  for(int i = 0; i < NUM_COLUMNS; i++) {
    retval += std::string(" ") + std::to_string(i) + std::string("   ");
  }
  retval += std::string("\n------------------------------------------\n");

  return retval;
}

/*
 * Since move is being reverted, goal here is to move from "destination" to "source".
 */
void Game::undoMove(PlayerAction action) {
  if(action.actionType == ActionType::MOVE_REGULAR) {
    delete board[action.srcIdx];
    board[action.srcIdx] = board[action.dstIdx];
    board[action.srcIdx]->squareIndex = action.srcIdx;
    board[action.dstIdx] = new Piece(PieceType::NO_PIECE, 0, action.dstIdx);
  } else { // castle
    // move king back
    delete board[action.srcIdx];
    board[action.srcIdx] = this->board[action.dstIdx];
    board[action.srcIdx]->squareIndex = action.srcIdx;
    board[action.dstIdx] = new Piece(PieceType::NO_PIECE, 0, action.dstIdx);
    // move warrior back
    if(action.dstIdx == 6) {
      // p1 short castle
      delete board[7];
      board[7] = board[5];
      board[7]->squareIndex = 7;
      board[5] = new Piece(PieceType::NO_PIECE, 0, 5);
    } else if(action.dstIdx == 2) {
       // p1 long castle
      delete board[0];
      board[0] = this->board[3];
      board[0]->squareIndex = 0;
      board[3] = new Piece(PieceType::NO_PIECE, 0, 3);
     } else if(action.dstIdx == 62) {
      // p2 short castle
      delete board[63];
      board[63] = this->board[61];
      board[63]->squareIndex = 63;
      board[61] = new Piece(PieceType::NO_PIECE, 0, 61);
     } else if(action.dstIdx == 58) {
      // p2 long castle
      delete board[56];
      board[56] = this->board[59];
      board[56]->squareIndex = 56;
      board[59] = new Piece(PieceType::NO_PIECE, 0, 59);
    }
  }
}

std::vector<PlayerAction> Game::_p1PawnActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  int squareIdx;
  const std::vector<int> *moveSquares = &GameCache::squareToP1PawnMoveSquares[piece->squareIndex];
  for(int i = 0; i < moveSquares->size(); i++) {
    squareIdx = (*moveSquares)[i];
    if(board[squareIdx]->type != NO_PIECE) continue;
    if(squareIdx > 55) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_PROMOTE_P1_PAWN));
      continue;
    }
    // Is p1 pawn trying to jump over another piece?
    if(piece->squareIndex - squareIdx == -2 * NUM_COLUMNS ) {
      // checks whether the square in front of the p1 pawn is empty
      if(board[piece->squareIndex + NUM_COLUMNS]->type != NO_PIECE) continue;
    }
    retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
  }

  const std::vector<int> *abilitySquares = &GameCache::squareToP1PawnAbilitySquares[piece->squareIndex];
  for(int i = 0; i < abilitySquares->size(); i++) {
    squareIdx = (*abilitySquares)[i];
    Piece* destinationSquarePiece = board[squareIdx];
    if(pieceBelongsToPlayer(destinationSquarePiece->type, Player::PLAYER_2)) {
      if(squareIdx > 55 && PAWN_ABILITY_POINTS >= destinationSquarePiece->healthPoints) {
        retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_P1_PAWN_DAMAGE_AND_PROMOTION));
      } else {
        retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_PAWN_DAMAGE));
      }
    }
  }
  return retval;
}

std::vector<PlayerAction> Game::_p2PawnActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  int squareIdx;
  const std::vector<int> *moveSquares = &GameCache::squareToP2PawnMoveSquares[piece->squareIndex];
  for(int i = 0; i < moveSquares->size(); i++) {
    squareIdx = (*moveSquares)[i];
    if(board[squareIdx]->type != NO_PIECE) continue;
    if(squareIdx < 8) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_PROMOTE_P2_PAWN));
      continue;
    }
    // Is p2 pawn trying to jump over another piece?
    if(piece->squareIndex - squareIdx == 2 * NUM_COLUMNS ) {
      // checks whether square in front of the p2 pawn is empty
      if(board[piece->squareIndex - NUM_COLUMNS]->type != NO_PIECE) continue;
    }
    retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
  }

  const std::vector<int> *abilitySquares = &GameCache::squareToP2PawnAbilitySquares[piece->squareIndex];
  for(int i = 0; i < abilitySquares->size(); i++) {
    squareIdx = (*abilitySquares)[i];
    Piece* destinationSquarePiece = board[squareIdx];
    if(pieceBelongsToPlayer(destinationSquarePiece->type, Player::PLAYER_1)) {
      if(squareIdx < 8 && PAWN_ABILITY_POINTS >= destinationSquarePiece->healthPoints) {
        retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_P2_PAWN_DAMAGE_AND_PROMOTION));
      } else {
        retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_PAWN_DAMAGE));
      }
    }
  }
  return retval;
}

std::vector<PlayerAction> Game::_p1KingActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  const std::vector<int> *squares = &GameCache::squareToNeighboringSquares[piece->squareIndex];
  for(int i = 0; i < squares->size(); i++) {
    int squareIdx = (*squares)[i];
    if(board[squareIdx]->type == NO_PIECE)  {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(board[squareIdx]->type, Player::PLAYER_2)) {
        retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_KING_DAMAGE));
    }
  }

  if(piece->squareIndex == 4) {
    // short castle
    if(
      board[5]->type == PieceType::NO_PIECE &&
      board[6]->type == PieceType::NO_PIECE &&
      board[7]->type == PieceType::P1_WARRIOR
      ) {
      retval.push_back(PlayerAction(4, 6, ActionType::MOVE_CASTLE));
    }
    // long castle
    if(
      board[3]->type == PieceType::NO_PIECE &&
      board[2]->type == PieceType::NO_PIECE &&
      board[1]->type == PieceType::NO_PIECE &&
      board[0]->type == PieceType::P1_WARRIOR
      ) {
      retval.push_back(PlayerAction(4, 2, ActionType::MOVE_CASTLE));
    }
  }

  return retval;
}

std::vector<PlayerAction> Game::_p2KingActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  const std::vector<int> *squares = &GameCache::squareToNeighboringSquares[piece->squareIndex];
  for(int i = 0; i < squares->size(); i++) {
    int squareIdx = (*squares)[i];
    if(board[squareIdx]->type == NO_PIECE)  {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(board[squareIdx]->type, Player::PLAYER_1)) {
        retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_KING_DAMAGE));
    }
  }
  if(piece->squareIndex == 60) {
    // short castle
    if(
      board[61]->type == PieceType::NO_PIECE &&
      board[62]->type == PieceType::NO_PIECE &&
      board[63]->type == PieceType::P2_WARRIOR
      ) {
      retval.push_back(PlayerAction(60, 62, ActionType::MOVE_CASTLE));
    }
    // long castle
    if(
      board[59]->type == PieceType::NO_PIECE &&
      board[58]->type == PieceType::NO_PIECE &&
      board[57]->type == PieceType::NO_PIECE &&
      board[56]->type == PieceType::P2_WARRIOR
      ) {
      retval.push_back(PlayerAction(60, 58, ActionType::MOVE_CASTLE));
    }
  }

  return retval;
}

std::vector<PlayerAction> Game::_p1MageActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  int squareIdx;

  const std::vector<int> *vertical1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTH];
  for(int i = 0; i < vertical1->size(); i++) {
    squareIdx = (*vertical1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTHEAST];
  for(int i = 0; i < diagonal1->size(); i++) {
    squareIdx = (*diagonal1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *horizontal1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::EAST];
  for(int i = 0; i < horizontal1->size(); i++) {
    squareIdx = (*horizontal1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTHEAST];
  for(int i = 0; i < diagonal2->size(); i++) {
    squareIdx = (*diagonal2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *vertical2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTH];
  for(int i = 0; i < vertical2->size(); i++) {
    squareIdx = (*vertical2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal3 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTHWEST];
  for(int i = 0; i < diagonal3->size(); i++) {
    squareIdx = (*diagonal3)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *horizontal2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::WEST];
  for(int i = 0; i < horizontal2->size(); i++) {
    squareIdx = (*horizontal2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal4 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTHWEST];
  for(int i = 0; i < diagonal4->size(); i++) {
    squareIdx = (*diagonal4)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  // mage throw assassin
  for(int k = 0; k < NUM_DIAGONAL_DIRECTIONS; k++) {
    const std::vector<int> *directionLine = &GameCache::squareToDirectionToLine[piece->squareIndex][DIAGONAL_DIRECTIONS[k]];
    for(int i = 0; i < directionLine->size(); i++) {
      Piece* p = board[(*directionLine)[i]];
      if(p->type != PieceType::P1_ASSASSIN) {
        break;
      } else {
        // is there a valid target?
        for(int j = i+1; j < directionLine->size(); j++) {
          Piece* p2 = board[(*directionLine)[j]];
          if(pieceBelongsToPlayer(p2->type, Player::PLAYER_2)) {
            PlayerAction currentAbility = PlayerAction(piece->squareIndex, p2->squareIndex, ActionType::ABILITY_MAGE_THROW_ASSASSIN);
            retval.push_back(currentAbility);
            break;
          } else if(pieceBelongsToPlayer(p2->type, Player::PLAYER_1)) {
            break;
          }
        }
      }
      break;
    }
  }
  return retval;
}

std::vector<PlayerAction> Game::_p2MageActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  int squareIdx;

  const std::vector<int> *vertical1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTH];
  for(int i = 0; i < vertical1->size(); i++) {
    squareIdx = (*vertical1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTHEAST];
  for(int i = 0; i < diagonal1->size(); i++) {
    squareIdx = (*diagonal1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *horizontal1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::EAST];
  for(int i = 0; i < horizontal1->size(); i++) {
    squareIdx = (*horizontal1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTHEAST];
  for(int i = 0; i < diagonal2->size(); i++) {
    squareIdx = (*diagonal2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *vertical2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTH];
  for(int i = 0; i < vertical2->size(); i++) {
    squareIdx = (*vertical2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal3 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTHWEST];
  for(int i = 0; i < diagonal3->size(); i++) {
    squareIdx = (*diagonal3)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *horizontal2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::WEST];
  for(int i = 0; i < horizontal2->size(); i++) {
    squareIdx = (*horizontal2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal4 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTHWEST];
  for(int i = 0; i < diagonal4->size(); i++) {
    squareIdx = (*diagonal4)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_MAGE_DAMAGE));
      break;
    } else {
      break;
    }
  }

  // mage throw assassin
  for(int k = 0; k < NUM_DIAGONAL_DIRECTIONS; k++) {
    const std::vector<int> *directionLine = &GameCache::squareToDirectionToLine[piece->squareIndex][DIAGONAL_DIRECTIONS[k]];
    for(int i = 0; i < directionLine->size(); i++) {
      Piece* p = board[(*directionLine)[i]];
      if(p->type != PieceType::P2_ASSASSIN) {
        break;
      } else {
        // is there a valid target?
        for(int j = i+1; j < directionLine->size(); j++) {
          Piece* p2 = board[(*directionLine)[j]];
          if(pieceBelongsToPlayer(p2->type, Player::PLAYER_1)) {
            PlayerAction currentAbility = PlayerAction(piece->squareIndex, p2->squareIndex, ActionType::ABILITY_MAGE_THROW_ASSASSIN);
            retval.push_back(currentAbility);
            break;
          } else if(pieceBelongsToPlayer(p2->type, Player::PLAYER_2)) {
            break;
          }
        }
      }
      break;
    }
  }

  return retval;
}

std::vector<PlayerAction> Game::_p1WarriorActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  // moves
  int squareIdx;

  const std::vector<int> *vertical1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTH];
  for(int i = 0; i < vertical1->size(); i++) {
    squareIdx = (*vertical1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_WARRIOR_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *horizontal1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::EAST];
  for(int i = 0; i < horizontal1->size(); i++) {
    squareIdx = (*horizontal1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_WARRIOR_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *vertical2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTH];
  for(int i = 0; i < vertical2->size(); i++) {
    squareIdx = (*vertical2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_WARRIOR_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *horizontal2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::WEST];
  for(int i = 0; i < horizontal2->size(); i++) {
    squareIdx = (*horizontal2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_WARRIOR_DAMAGE));
      break;
    } else {
      break;
    }
  }

  // warrior throw warrior
  for(int k = 0; k < 4; k++) {
    const std::vector<int> *directionLine = &GameCache::squareToDirectionToLine[piece->squareIndex][NON_DIAGONAL_DIRECTIONS[k]];
    for(int i = 0; i < directionLine->size(); i++) {
      Piece* p = board[(*directionLine)[i]];
      if(p->type != PieceType::P1_WARRIOR) {
        break;
      } else {
        // is there a valid target?
        for(int j = i+1; j < directionLine->size(); j++) {
          Piece* p2 = board[(*directionLine)[j]];
          if(pieceBelongsToPlayer(p2->type, Player::PLAYER_2)) {
            PlayerAction currentAbility = PlayerAction(piece->squareIndex, p2->squareIndex, ActionType::ABILITY_WARRIOR_THROW_WARRIOR);
            retval.push_back(currentAbility);
            break;
          } else if(pieceBelongsToPlayer(p2->type, Player::PLAYER_1)) {
            break;
          }
        }
      }
      break;
    }
  }
  return retval;
}

std::vector<PlayerAction> Game::_p2WarriorActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  int squareIdx;

  const std::vector<int> *vertical1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTH];
  for(int i = 0; i < vertical1->size(); i++) {
    squareIdx = (*vertical1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_WARRIOR_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *horizontal1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::EAST];
  for(int i = 0; i < horizontal1->size(); i++) {
    squareIdx = (*horizontal1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_WARRIOR_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *vertical2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTH];
  for(int i = 0; i < vertical2->size(); i++) {
    squareIdx = (*vertical2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_WARRIOR_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *horizontal2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::WEST];
  for(int i = 0; i < horizontal2->size(); i++) {
    squareIdx = (*horizontal2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_WARRIOR_DAMAGE));
      break;
    } else {
      break;
    }
  }

  // warrior throw warrior
  for(int k = 0; k < 4; k++) {
    const std::vector<int> *directionLine = &GameCache::squareToDirectionToLine[piece->squareIndex][NON_DIAGONAL_DIRECTIONS[k]];
    for(int i = 0; i < directionLine->size(); i++) {
      Piece* p = board[(*directionLine)[i]];
      if(p->type != PieceType::P2_WARRIOR) {
        break;
      } else {
        // is there a valid target?
        for(int j = i+1; j < directionLine->size(); j++) {
          Piece* p2 = board[(*directionLine)[j]];
          if(pieceBelongsToPlayer(p2->type, Player::PLAYER_1)) {
            PlayerAction currentAbility = PlayerAction(piece->squareIndex, p2->squareIndex, ActionType::ABILITY_WARRIOR_THROW_WARRIOR);
            retval.push_back(currentAbility);
            break;
          } else if(pieceBelongsToPlayer(p2->type, Player::PLAYER_2)) {
            break;
          }
        }
      }
      break;
    }
  }
  return retval;
}


std::vector<PlayerAction> Game::_p1KnightActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  const std::vector<int> *squares = &GameCache::squareToKnightActionSquares[piece->squareIndex];
  for(int i = 0; i < squares->size(); i++) {
    int s = (*squares)[i];
    Piece* currentPiece = board[s];
    if(currentPiece->type == NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, s, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(currentPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, s, ActionType::ABILITY_KNIGHT_DAMAGE));
    }
  }
  return retval;
}

std::vector<PlayerAction> Game::_p2KnightActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  const std::vector<int> *squares = &GameCache::squareToKnightActionSquares[piece->squareIndex];
  for(int i = 0; i < squares->size(); i++) {
    int s = (*squares)[i];
    Piece* currentPiece = board[s];
    if(currentPiece->type == NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, s, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(currentPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, s, ActionType::ABILITY_KNIGHT_DAMAGE));
    }
  }
  return retval;
}

std::vector<PlayerAction> Game::_p1AssassinActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  int squareIdx;

  const std::vector<int> *squares = &GameCache::squareToNeighboringNonDiagonalSquares[piece->squareIndex];
  for(int i = 0; i < squares->size(); i++) {
    squareIdx = (*squares)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_ASSASSIN_DAMAGE));
      continue;
    } else {
      continue;
    }
  }

  const std::vector<int> *diagonal1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTHEAST];
  for(int i = 0; i < diagonal1->size(); i++) {
    squareIdx = (*diagonal1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_ASSASSIN_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTHEAST];
  for(int i = 0; i < diagonal2->size(); i++) {
    squareIdx = (*diagonal2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_ASSASSIN_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal3 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTHWEST];
  for(int i = 0; i < diagonal3->size(); i++) {
    squareIdx = (*diagonal3)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_ASSASSIN_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal4 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTHWEST];
  for(int i = 0; i < diagonal4->size(); i++) {
    squareIdx = (*diagonal4)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_2)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_ASSASSIN_DAMAGE));
      break;
    } else {
      break;
    }
  }
  return retval;
}

std::vector<PlayerAction> Game::_p2AssassinActions(Piece* piece) {
  std::vector<PlayerAction> retval;
  int squareIdx;

  const std::vector<int> *squares = &GameCache::squareToNeighboringNonDiagonalSquares[piece->squareIndex];
  for(int i = 0; i < squares->size(); i++) {
    squareIdx = (*squares)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_ASSASSIN_DAMAGE));
      continue;
    } else {
      continue;
    }
  }

  const std::vector<int> *diagonal1 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTHEAST];
  for(int i = 0; i < diagonal1->size(); i++) {
    squareIdx = (*diagonal1)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_ASSASSIN_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal2 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTHEAST];
  for(int i = 0; i < diagonal2->size(); i++) {
    squareIdx = (*diagonal2)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_ASSASSIN_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal3 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::SOUTHWEST];
  for(int i = 0; i < diagonal3->size(); i++) {
    squareIdx = (*diagonal3)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_ASSASSIN_DAMAGE));
      break;
    } else {
      break;
    }
  }

  const std::vector<int> *diagonal4 = &GameCache::squareToDirectionToLine[piece->squareIndex][Direction::NORTHWEST];
  for(int i = 0; i < diagonal4->size(); i++) {
    squareIdx = (*diagonal4)[i];
    Piece *dstPiece = board[squareIdx];
    if(dstPiece->type == PieceType::NO_PIECE) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::MOVE_REGULAR));
    } else if(pieceBelongsToPlayer(dstPiece->type, Player::PLAYER_1)) {
      retval.push_back(PlayerAction(piece->squareIndex, squareIdx, ActionType::ABILITY_ASSASSIN_DAMAGE));
      break;
    } else {
      break;
    }
  }
  return retval;
}

std::vector<PlayerAction> Game::legalActionsByPiece(Piece* piece) {
  switch(piece->type) {
    case P1_KING:
      return _p1KingActions(piece);
    case P1_MAGE:
      return _p1MageActions(piece);
    case P1_PAWN:
      return _p1PawnActions(piece);
    case P1_WARRIOR:
      return _p1WarriorActions(piece);
    case P1_ASSASSIN:
      return _p1AssassinActions(piece);
    case P1_KNIGHT:
      return _p1KnightActions(piece);
    case P2_KING:
      return _p2KingActions(piece);
    case P2_MAGE:
      return _p2MageActions(piece);
    case P2_PAWN:
      return _p2PawnActions(piece);
    case P2_WARRIOR:
      return _p2WarriorActions(piece);
    case P2_ASSASSIN:
      return _p2AssassinActions(piece);
    case P2_KNIGHT:
      return _p2KnightActions(piece);
    default:
      return std::vector<PlayerAction>();
  }
}

std::vector<PlayerAction> Game::generateLegalActions() {
  std::vector<PlayerAction> retval;
  if(playerToKing[currentPlayer]->healthPoints <= 0) {
    return retval;
  }
  std::vector<Piece*> currentPlayerPieces = playerToPieces[currentPlayer];
  for(int i = 0; i < currentPlayerPieces.size(); i++) {
    Piece* currentPiece = currentPlayerPieces[i];
    if(currentPiece->healthPoints <= 0) continue; // dead pieces don't move

    auto legalActions = legalActionsByPiece(currentPiece);
    retval.insert(retval.end(), legalActions.begin(), legalActions.end());
  }

  // TODO: No longer needed. Remove?
  // player can also skip the action
  //PlayerAction p = PlayerAction(ACTION_SKIP, ACTION_SKIP, ActionType::SKIP);
  //retval.push_back(p);
  return retval;
}

long int Game::zobristHash() {
    long int hash = 0;
    std::vector<Piece*> p1Pieces = playerToPieces[Player::PLAYER_1];
    for(int i = 0; i < p1Pieces.size(); i++) {
      Piece* currentPiece = p1Pieces[i];
      if(currentPiece->healthPoints <= 0) continue;
      // dividing hp by 10 because health points are { 10, 20, 30, 40, 50, 60 } and 
      // indexes are { 1, 2, 3, 4, 5, 6 }
      hash ^= Zobrist::pieceTypeToSquareToHPToKey[currentPiece->type][currentPiece->squareIndex][currentPiece->healthPoints/10];
    }
    std::vector<Piece*> p2Pieces = playerToPieces[Player::PLAYER_2];
    for(int i = 0; i < p2Pieces.size(); i++) {
      Piece* currentPiece = p2Pieces[i];
      if(currentPiece->healthPoints <= 0) continue;
      // dividing hp by 10 because health points are { 10, 20, 30, 40, 50, 60 } and 
      // indexes are { 1, 2, 3, 4, 5, 6 }
      hash ^= Zobrist::pieceTypeToSquareToHPToKey[currentPiece->type][currentPiece->squareIndex][currentPiece->healthPoints/10];
    }
    if(currentPlayer == Player::PLAYER_2) {
      hash ^= Zobrist::p2Key;
    }
    return hash;
}

/*
 * Checks whether values are in the right range.
 */
bool isActionValid(int srcIdx, int dstIdx) {
  if(srcIdx == ACTION_SKIP && dstIdx == ACTION_SKIP) {
    return true;
  } else if((!isOffBoard(srcIdx)) && (!isOffBoard(dstIdx))) {
    return true;
  }
  return false;
}

bool Game::isActionLegal(int srcIdx, int dstIdx) {
  // TODO
  return false;
}

Player Game::getCurrentPlayer() {
  return currentPlayer;
}

Piece Game::getPieceByCoordinates(int x, int y) {
  return *board[coordinatesToBoardIndex(x, y)]; 
}

Piece Game::getPieceBySquareIndex(int squareIndex) {
  return *board[squareIndex]; 
}

bool Game::isGameOver() {
  Piece* p1King = playerToKing[PLAYER_1];
  Piece* p2King = playerToKing[PLAYER_2];
  if(p1King->healthPoints <= 0 || p2King->healthPoints <= 0 || repetitionsDraw || moveNumber >= 333) {
    return true;
  } else {
    return false;
  }
}

bool Game::isGameDraw() {
  if(repetitionsDraw || moveNumber >= 333) {
    return true;
  } else {
    return false;
  }
}

std::optional<Player> Game::winner() {
  Piece* p1King = playerToKing[PLAYER_1];
  Piece* p2King = playerToKing[PLAYER_2];
  if(p1King->healthPoints <= 0) {
    return PLAYER_2;
  } else if(p2King->healthPoints <= 0) {
    return PLAYER_1;
  }
  return std::nullopt;
}

std::string Game::boardToString() {
  std::stringstream retval;
  retval << currentPlayer << "|";
  Piece* currentPiece;
  for(int i = 0; i < NUM_SQUARES; i++) {
    currentPiece = board[i];
    if(currentPiece->type == NO_PIECE) {
      retval << "empty,";
    } else {
      switch(currentPiece->type) {
        case P1_KING:
          retval << "0-king-";
          break;
        case P1_MAGE:
          retval << "0-mage-";
          break;
        case P1_PAWN:
          retval << "0-pawn-";
          break;
        case P1_WARRIOR:
          retval << "0-warrior-";
          break;
        case P1_ASSASSIN:
          retval << "0-assassin-";
          break;
        case P1_KNIGHT:
          retval << "0-knight-";
          break;
        case P2_KING:
          retval << "1-king-";
          break;
        case P2_MAGE:
          retval << "1-mage-";
          break;
        case P2_PAWN:
          retval << "1-pawn-";
          break;
        case P2_WARRIOR:
          retval << "1-warrior-";
          break;
        case P2_ASSASSIN:
          retval << "1-assassin-";
          break;
        case P2_KNIGHT:
          retval << "1-knight-";
          break;
      }        
      retval << currentPiece->healthPoints << ",";
    }
  }
  return retval.str();
}

void Game::boardFromString(std::string encodedBoard) {
  currentPlayer = (Player)(std::stoi(encodedBoard.substr(0, encodedBoard.find("|"))));
  moveNumber = 0;
  std::vector<Piece*> p1Pieces;
  std::vector<Piece*> p2Pieces;

  std::string b1 = encodedBoard.substr(2);
  std::string delimiter1 = ",";
  std::string delimiter2 = "-";
  std::string token1;
  std::string s, tmp;
  size_t pos = 0;
  int boardIdx = 0;
  while((pos = b1.find(delimiter1)) != std::string::npos) {
    token1 = b1.substr(0, pos);
    if(token1 == "empty") {
      board[boardIdx] = new Piece(PieceType::NO_PIECE, 0, boardIdx);
    } else {
      std::stringstream ss(token1);
      std::vector<std::string> words;
      while(std::getline(ss, tmp, '-')) {
        words.push_back(tmp);
      }
      int healthPoints = std::stoi(words[2]);
      s = words[0] + words[1];
      if(s == "0king") {
        board[boardIdx] = new Piece(PieceType::P1_KING, healthPoints, boardIdx);
        p1Pieces.push_back(board[boardIdx]);
        playerToKing[PLAYER_1] = board[boardIdx];
      } else if(s == "0pawn") {
        board[boardIdx] = new Piece(PieceType::P1_PAWN, healthPoints, boardIdx);
        p1Pieces.push_back(board[boardIdx]);
      } else if(s == "0mage") {
        board[boardIdx] = new Piece(PieceType::P1_MAGE, healthPoints, boardIdx);
        p1Pieces.push_back(board[boardIdx]);
      } else if(s == "0assassin") {
        board[boardIdx] = new Piece(PieceType::P1_ASSASSIN, healthPoints, boardIdx);
        p1Pieces.push_back(board[boardIdx]);
      } else if(s == "0knight") {
        board[boardIdx] = new Piece(PieceType::P1_KNIGHT, healthPoints, boardIdx);
        p1Pieces.push_back(board[boardIdx]);
      } else if(s == "0warrior") {
        board[boardIdx] = new Piece(PieceType::P1_WARRIOR, healthPoints, boardIdx);
        p1Pieces.push_back(board[boardIdx]);
      } else if(s == "1king") {
        board[boardIdx] = new Piece(PieceType::P2_KING, healthPoints, boardIdx);
        p2Pieces.push_back(board[boardIdx]);
        playerToKing[PLAYER_2] = board[boardIdx];
      } else if(s == "1pawn") {
        board[boardIdx] = new Piece(PieceType::P2_PAWN, healthPoints, boardIdx);
        p2Pieces.push_back(board[boardIdx]);
      } else if(s == "1mage") {
        board[boardIdx] = new Piece(PieceType::P2_MAGE, healthPoints, boardIdx);
        p2Pieces.push_back(board[boardIdx]);
      } else if(s == "1assassin") {
        board[boardIdx] = new Piece(PieceType::P2_ASSASSIN, healthPoints, boardIdx);
        p2Pieces.push_back(board[boardIdx]);
      } else if(s == "1knight") {
        board[boardIdx] = new Piece(PieceType::P2_KNIGHT, healthPoints, boardIdx);
        p2Pieces.push_back(board[boardIdx]);
      } else if(s == "1warrior") {
        board[boardIdx] = new Piece(PieceType::P2_WARRIOR, healthPoints, boardIdx);
        p2Pieces.push_back(board[boardIdx]);
      }
    }
    b1.erase(0, pos + delimiter1.length());
    boardIdx += 1;
  }

  playerToPieces[PLAYER_1] = p1Pieces;
  playerToPieces[PLAYER_2] = p2Pieces;
}

std::vector<Piece*> Game::getAllPiecesByPlayer(Player player) {
  return playerToPieces[player];
}

/* 
 * performance test - https://www.chessprogramming.org/Perft
 * with bulk counting
 */
unsigned long long nichess::perft(Game& game, int depth) {
  unsigned long long nodes = 0;
  std::vector<PlayerAction> legalActions = game.generateLegalActions();
  int numLegalActions = legalActions.size();
  if(depth == 1) {
    return (unsigned long long) numLegalActions;
  }

  UndoInfo ui;
  PlayerAction pa;
  for(int i = 0; i < numLegalActions; i++) {
    PlayerAction pa = legalActions[i];
    ui = game.makeAction(pa);
    nodes += perft(game, depth-1);
    game.undoAction(ui);
  }
  return nodes;
}
