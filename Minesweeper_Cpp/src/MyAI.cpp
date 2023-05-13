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
  // TODO: This is backwars from how world.cpp does it
  for (int i = 0; i < _rowDimension; ++i) {
    vector<TileStatus> initialStatus(_colDimension, COVERED);
    vector<int> initialValues(_colDimension, -1);

    boardStatus.push_back(initialStatus);
    boardValues.push_back(initialValues);
  }
};

/*
  Function is called when world is looking for the next step
  param:
    number - the number revealed on the last action spot
  return:
    Agent::Action - The next action to perform by this agent
*/
Agent::Action MyAI::getAction(int number) {
  cout << "Inside getAction. Current number: " << number << endl;
  // Update last action
  int x = this->agentX;
  int y = this->agentY;
  cout << "x: " << x << "\ty: " << y << endl;
  updateVecs(number, x, y);

  int numCoveredNeighbor = -1;
  int numFlaggedNeighbor = -1;
  if (number == 0) {
    neighbors(x, y, numCoveredNeighbor, numFlaggedNeighbor);
  }

  cout << "Cover neigh = " << numCoveredNeighbor << endl;
  cout << "Flagged neigh = " << numFlaggedNeighbor << endl;
  // Select what to push into the next action queue based
  // on the number reveal by last action
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = (x + i < 0)                     ? 0
                  : (x + i >= this->colDimension) ? this->colDimension - 1
                                                  : x + i;
      int currY = (y + j < 0)                     ? 0
                  : (y + j >= this->rowDimension) ? this->rowDimension - 1
                                                  : y + j;

      // There are no bombs so every surrounding tile must be safe
      if (number == 0) {
        if (!(i == 0 && j == 0) && boardStatus.at(currY).at(currX) == COVERED) {
          cout << "\tNum = 0, pushing (" << currX << ", " << currY << ")"
               << endl;
          nextMoves.push({Action_type::UNCOVER, currX, currY});
        }
      }
      // Every surrounding tile must be a bomb so flag everything covered
      else if (number == numCoveredNeighbor) {
        cout << "\tNum = coveredNeighbors" << endl;
        if (boardStatus.at(currY).at(currX) == COVERED) {
          boardStatus.at(currY).at(currX) = FLAGGED;
        }
      }
      // Everything not flagged must be safe
      else if (number == numFlaggedNeighbor) {
        cout << "\tNum = numFlaggedNeighbor" << endl;
        if (boardStatus.at(currY).at(currX) == COVERED) {
          nextMoves.push({Action_type::UNCOVER, currX, currY});
        }
      }
    }
  }

  cout << "\tgetting next move" << endl;
  // Check if we already stored a next move
  while (!nextMoves.empty()) {
    Agent::Action next = nextMoves.front();
    nextMoves.pop();

    if (!(inBounds(next.x, next.y) &&
          boardStatus.at(next.y).at(next.x) == COVERED)) {
      continue;
    }

    this->agentX = next.x;
    this->agentY = next.y;

    cout << "Uncover (" << next.x << "," << next.y << ") next" << endl;
    return next;
  }
  cout << "GG no more in queue" << endl;
  return {LEAVE, -1, -1};
}

void MyAI::updateVecs(int number, int x, int y) {
  cout << "UpdateVec (" << x << "," << y << ") = " << number << endl;
  boardStatus.at(y).at(x) = UNCOVERED;
  boardValues.at(y).at(x) = number;
}

bool MyAI::inBounds(int x, int y) {
  return x >= 0 && x < this->colDimension && y >= 0 && y < this->rowDimension;
}

/*
  Count the number of covered neighbors around a given coordinate (x, y)
*/
void MyAI::neighbors(int x, int y, int &numCoveredNeighbors, int &numFlags) {
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = (x + i < 0)                     ? 0
                  : (x + i >= this->colDimension) ? this->colDimension - 1
                                                  : x + i;
      int currY = (y + j < 0)                     ? 0
                  : (y + j >= this->rowDimension) ? this->rowDimension - 1
                                                  : y + j;

      if (boardStatus.at(currY).at(currX) == COVERED) {
        numCoveredNeighbors++;
      } else if (boardStatus.at(currY).at(currX) == FLAGGED) {
        numFlags++;
      }
    }
  }
}

void MyAI::printVecs() {
  int numRows = this->rowDimension;
  int numCols = this->colDimension;
  cout << "SIZE: " << numRows << "X" << numCols << endl;
  for (int row = numRows - 1; row >= 0; --row) {
    for (int col = 0; col < numCols; ++col) {
      cout << boardStatus.at(row).at(col) << ",";
    }
    cout << endl;
  }
  cout << endl;

  for (int row = numRows - 1; row >= 0; --row) {
    for (int col = 0; col < numCols; ++col) {
      cout << boardValues.at(row).at(col) << ",";
    }
    cout << endl;
  }
}
