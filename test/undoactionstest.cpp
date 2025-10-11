#include "nichess/nichess.hpp"
#include "nichess/util.hpp"

using namespace nichess;

int undoActionTest1() {
  Game g = Game();

  std::vector<PlayerAction> legalActions = g.generateLegalActions();
  PlayerAction pa = legalActions[0];
  std::string b1 = g.boardToString();
  UndoInfo ui = g.makeAction(pa);
  g.undoAction(ui);
  std::string b2 = g.boardToString();

  if(b1 == b2) {
    return 0;
  } else {
    return -1;
  }
}


int undoactionstest(int argc, char* argv[]) {
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
    return undoActionTest1();
  default:
    printf("\nInvalid test number.\n");
    return -1;
  }


  return -1;
}
