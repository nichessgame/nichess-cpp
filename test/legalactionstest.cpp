#include "nichess/nichess.hpp"
#include "nichess/util.hpp"
#include <iostream>
#include <chrono>

using namespace nichess;

int legalActionsTest1() {
  Game g = Game();

  std::vector<PlayerAction> legalActions = g.generateLegalActions();
  std::cout << legalActions.size() << "\n";
  if(legalActions.size() == 20) {
    return 0;
  } else {
    return -1;
  }
}

int legalActionsTest2() {
  Game g = Game();
  auto start = std::chrono::high_resolution_clock::now();
  uint64_t numNodes = perft(g, 4);
  auto stop = std::chrono::high_resolution_clock::now();
  
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  std::cout << "perft 4 took: " << duration.count() << " microseconds\n";
  std::cout << "numNodes: " << numNodes << "\n";
  // Not verified, but this is likely correct.
  if(numNodes == 204934) {
    return 0;
  } else {
    return -1;
  }
}

int legalactionstest(int argc, char* argv[]) {
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
    return legalActionsTest1();
  case 2:
    return legalActionsTest2();
  default:
    printf("\nInvalid test number.\n");
    return -1;
  }

  return -1;
}
