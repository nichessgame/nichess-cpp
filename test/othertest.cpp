#include "nichess/nichess.hpp"
#include "nichess/util.hpp"

using namespace nichess;

int copyTest1() {
  Game g1 = Game();
  Game g2 = Game(g1);

  std::string b1 = g1.boardToString();
  std::string b2 = g2.boardToString();

  if(b1 == b2) {
    return 0;
  } else {
    return -1;
  }
}

int copyTest2() {
  Game g1 = Game();
  Game g2 = Game(g1);
  std::vector<PlayerAction> legalActions = g1.generateLegalActions();
  PlayerAction pa = legalActions[0];

  g1.makeAction(pa);
  g2.makeAction(pa);
  std::string b1 = g1.boardToString();
  std::string b2 = g2.boardToString();

  if(b1 == b2) {
    return 0;
  } else {
    return -1;
  }
}

// two fold repetition is not a draw
int zobristDrawTest3() {
  Game g = Game();
  g.makeAction(PlayerAction(1, 18, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(57, 42, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(18, 1, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(42, 57, ActionType::MOVE_REGULAR));

  if(g.isGameOver() || g.repetitionsDraw) {
    return -1;
  } else {
    return 0;
  }
}

// three fold repetition is a draw
int zobristDrawTest4() {
  Game g = Game();
  g.makeAction(PlayerAction(1, 18, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(57, 42, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(18, 1, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(42, 57, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(1, 18, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(57, 42, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(18, 1, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(42, 57, ActionType::MOVE_REGULAR));

  std::optional<Player> winner = g.winner();
  if(g.isGameOver() && g.repetitionsDraw && (!winner)) {
    return 0;
  } else {
    return -1;
  }
}

// three fold repetition -> undoAction is not a draw
int zobristDrawTest5() {
  Game g = Game();
  g.makeAction(PlayerAction(1, 18, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(57, 42, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(18, 1, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(42, 57, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(1, 18, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(57, 42, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(18, 1, ActionType::MOVE_REGULAR));
  UndoInfo ui = g.makeAction(PlayerAction(42, 57, ActionType::MOVE_REGULAR));
  g.undoAction(ui);

  if(g.isGameOver() || g.repetitionsDraw) {
    return -1;
  } else {
    return 0;
  }
}

// three fold repetition is a draw
// testing whether zobrist hashing works after a capture
int zobristDrawTest6() {
  Game g = Game();
  g.makeAction(PlayerAction(12, 28, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(51, 35, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(28, 35, ActionType::ABILITY_PAWN_DAMAGE));

  g.makeAction(PlayerAction(1, 18, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(57, 42, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(18, 1, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(42, 57, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(1, 18, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(57, 42, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(18, 1, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(42, 57, ActionType::MOVE_REGULAR));

  std::optional<Player> winner = g.winner();
  if(g.isGameOver() && g.repetitionsDraw && (!winner)) {
    return 0;
  } else {
    return -1;
  }
}

// Zobrist hash before and after undoAction should be the same
int zobristTest7() {
  Game g = Game();
  g.makeAction(PlayerAction(12, 28, ActionType::MOVE_REGULAR));
  g.makeAction(PlayerAction(51, 35, ActionType::MOVE_REGULAR));
  long int zhBefore = g.zobristHash();
  UndoInfo ui = g.makeAction(PlayerAction(28, 35, ActionType::ABILITY_PAWN_DAMAGE));
  g.undoAction(ui);
  long int zhAfter = g.zobristHash();


  if(zhBefore == zhAfter) {
    return 0;
  } else {
    return -1;
  }
}

int othertest(int argc, char* argv[]) {
  int defaultchoice = 1;
  int choice = defaultchoice;

  if (argc > 1) {
    if(sscanf(argv[1], "%d", &choice) != 1) {
      printf("Couldn't parse that input as a number\n");
      return -1;
    }
  }

  switch(choice) {
  case 1:
    return copyTest1();
  case 2:
    return copyTest2();
  case 3:
    return zobristDrawTest3();
  case 4:
    return zobristDrawTest4();
  case 5:
    return zobristDrawTest5();
  case 6:
    return zobristDrawTest6();
  case 7:
    return zobristTest7();
  default:
    printf("\nInvalid test number.\n");
    return -1;
  }

  return -1;
}
