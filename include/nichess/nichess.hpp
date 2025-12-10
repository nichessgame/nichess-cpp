#pragma once 

#include "constants.hpp"
#include "gamecache.hpp"

#include <vector>
#include <optional>
#include <tuple>
#include <map>
#include <string>

namespace nichess {

enum Player: int {
  PLAYER_1, PLAYER_2
};

constexpr Player operator~(Player p) {
  return Player(p ^ 1);
}

enum PieceType: int {
  P1_KING, P1_MAGE, P1_WARRIOR, P1_ASSASSIN, P1_KNIGHT, P1_PAWN,
  P2_KING, P2_MAGE, P2_WARRIOR, P2_ASSASSIN, P2_KNIGHT, P2_PAWN,
  NO_PIECE
};

enum class ActionType: int {
  MOVE_REGULAR, MOVE_CASTLE, MOVE_PROMOTE_P1_PAWN, MOVE_PROMOTE_P2_PAWN, ABILITY_KING_DAMAGE, ABILITY_MAGE_DAMAGE, ABILITY_P1_PAWN_DAMAGE_AND_PROMOTION, ABILITY_P2_PAWN_DAMAGE_AND_PROMOTION, ABILITY_MAGE_THROW_ASSASSIN, ABILITY_WARRIOR_DAMAGE, ABILITY_ASSASSIN_DAMAGE, ABILITY_KNIGHT_DAMAGE, ABILITY_PAWN_DAMAGE, ABILITY_WARRIOR_THROW_WARRIOR, SKIP
};

class Piece {
  public:
    PieceType type;
    int healthPoints;
    int squareIndex;
    Piece();
    Piece(PieceType type, int healthPoints, int squareIndex);
    Piece(const Piece& other);
    bool operator==(const Piece& other) const;
    bool operator!=(const Piece& other) const;
};

class PlayerAction {
  public:
    int srcIdx, dstIdx;
    ActionType actionType;
    PlayerAction();
    PlayerAction(int srcIdx, int dstIdx, ActionType actionType);
};

class UndoInfo {
  public:
    std::vector<Piece*> affectedPieces;
    PlayerAction action;
    // Some actions require saving extra values, like previous position of an affected piece or its
    // health points. t1 and t2 are used for that.
    int t1 = -1;
    int t2 = -1;
    UndoInfo();
    UndoInfo(PlayerAction playerAction);
};

class Game {
  public:
    Piece* board[NUM_SQUARES];
    std::vector<std::vector<Piece*>> playerToPieces{NUM_PLAYERS};
    std::vector<Piece*> playerToKing{NUM_PLAYERS};
    Player currentPlayer;
    int moveNumber;
    std::map<long int, int> repetitions;
    bool repetitionsDraw;

    Game();
    Game(const Game& other);
    Game(const std::string encodedBoard);
    ~Game();
    void undoMove(PlayerAction action);
    bool isActionLegal(int srcIdx, int dstIdx);
    UndoInfo makeAction(PlayerAction playerAction);
    void undoAction(UndoInfo undoInfo);
    long int zobristHash();
    std::vector<PlayerAction> generateLegalActions();
    std::vector<PlayerAction> _p1KingMoves(Piece* piece);
    std::vector<PlayerAction> _p2KingMoves(Piece* piece);
    std::vector<PlayerAction> _knightMoves(Piece* piece);
    std::vector<PlayerAction> _warriorMoves(Piece* piece);
    std::vector<PlayerAction> _mageMoves(Piece* piece);
    std::vector<PlayerAction> _p1PawnMoves(Piece* piece);
    std::vector<PlayerAction> _p2PawnMoves(Piece* piece);
    std::vector<PlayerAction> _assassinMoves(Piece* piece);
    std::vector<PlayerAction> legalMovesByPiece(Piece* piece);
    std::vector<PlayerAction> legalMovesBySquare(int srcSquareIdx);
    std::vector<PlayerAction> _defaultAbilities(Piece* piece);
    std::vector<PlayerAction> _p1KnightAbilities(Piece* piece);
    std::vector<PlayerAction> _p2KnightAbilities(Piece* piece);
    std::vector<PlayerAction> _p1AssassinAbilities(Piece* piece);
    std::vector<PlayerAction> _p2AssassinAbilities(Piece* piece);
    std::vector<PlayerAction> _p1MageAbilities(Piece* piece);
    std::vector<PlayerAction> _p2MageAbilities(Piece* piece);
    std::vector<PlayerAction> legalAbilitiesByPiece(Piece* piece);
    std::vector<PlayerAction> _p1KingActions(Piece* piece);
    std::vector<PlayerAction> _p2KingActions(Piece* piece);
    std::vector<PlayerAction> _p1KnightActions(Piece* piece);
    std::vector<PlayerAction> _p2KnightActions(Piece* piece);
    std::vector<PlayerAction> _p1WarriorActions(Piece* piece);
    std::vector<PlayerAction> _p2WarriorActions(Piece* piece);
    std::vector<PlayerAction> _p1MageActions(Piece* piece);
    std::vector<PlayerAction> _p2MageActions(Piece* piece);
    std::vector<PlayerAction> _p1PawnActions(Piece* piece);
    std::vector<PlayerAction> _p2PawnActions(Piece* piece);
    std::vector<PlayerAction> _p1AssassinActions(Piece* piece);
    std::vector<PlayerAction> _p2AssassinActions(Piece* piece);
    std::vector<PlayerAction> legalActionsByPiece(Piece* piece);

    Player getCurrentPlayer();
    Piece getPieceByCoordinates(int x, int y);
    Piece getPieceBySquareIndex(int squareIndex);
    std::vector<Piece*> getAllPiecesByPlayer(Player player);
    std::string boardToString();
    void boardFromString(std::string encodedBoard);
    bool isGameOver();
    bool isGameDraw();
    std::optional<Player> winner();
    std::string dump() const;
    void reset();
};

int coordinatesToBoardIndex(int column, int row);
std::tuple<int, int> boardIndexToCoordinates(int squareIndex);
unsigned long long perft(Game& game, int depth);

} // namespace nichess
