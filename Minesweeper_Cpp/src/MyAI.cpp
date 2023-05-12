// ======================================================================
// FILE:        MyAI.cpp
//
// AUTHOR:      Jian Li
//
// DESCRIPTION: This file contains your agent class, which you will
//              implement. You are responsible for implementing the
//              'getAction' function and any helper methods you feel you
//              need.
//
// NOTES:       - If you are having trouble understanding how the shell
//                works, look at the other parts of the code, as well as
//                the documentation.
//
//              - You are only allowed to make changes to this portion of
//                the code. Any changes to other portions of the code will
//                be lost when the tournament runs your code.
// ======================================================================

#include "MyAI.hpp"

MyAI::MyAI(int _rowDimension, int _colDimension, int _totalMines, int _agentX,
           int _agentY)
    : Agent() {
  // Initialize data in base class
  this->rowDimension = _rowDimension;
  this->colDimension = _colDimension;
  this->totalMines = _totalMines;
  this->agentX = _agentX;
  this->agentY = _agentY;

  // Populate both arrays with starting board information
  for (int i = 0; i < _colDimension; ++i) {
    vector<TileStatus> initialStatus(_rowDimension, COVERED);
    vector<int> initialValues(_rowDimension, -1);

    boardStatus->push_back(initialStatus);
    boardValues->push_back(initialValues);
  }
  printVecs();
  updateVecs(3, 1, 5);
  printVecs();
};

Agent::Action MyAI::getAction(int number) {
  // Update last action
  int x = this->agentX;
  int y = this->agentY;
  updateVecs(number, x, y);

  // Check if we already stored a next move
  if (!nextMoves.empty()) {
    Agent::Action next = nextMoves.front();
    nextMoves.pop();

    this->agentX = next.x;
    this->agentY = next.y;

    return next;
  }

  // If there are no bombs nearby, then every surrounding tile must be safe
  if (number == 0) {
    // nextMoves.push({UNCOVER, x - 1, y + 1});
    // nextMoves.push({UNCOVER, x, y + 1});
    // nextMoves.push({UNCOVER, x + 1, y + 1});
    // nextMoves.push({UNCOVER, x - 1, y});
    // nextMoves.push({UNCOVER, x + 1, y});
    // nextMoves.push({UNCOVER, x - 1, y - 1});
    // nextMoves.push({UNCOVER, x, y - 1});
    // nextMoves.push({UNCOVER, x + 1, y - 1});
  }

  return {LEAVE, -1, -1};
}

void MyAI::updateVecs(int number, int x, int y) {
  boardStatus->at(x).at(y) = UNCOVER;
  boardValues->at(x).at(y) = number;
}

// void MyAI::pushInQueue()

void MyAI::printVecs() {
  int numRows = this->rowDimension;
  int numCols = this->colDimension;
  cout << "SIZE: " << numRows << "X" << numCols << endl;
  for (int i = 0; i < numCols; ++i) {
    for (int j = 0; j < numRows; ++j) {
      cout << boardStatus->at(i).at(j) << ",";
    }
    cout << endl;
  }
  cout << endl;

  for (int i = 0; i < numCols; ++i) {
    for (int j = 0; j < numRows; ++j) {
      cout << boardValues->at(i).at(j) << ",";
    }
    cout << endl;
  }
}
