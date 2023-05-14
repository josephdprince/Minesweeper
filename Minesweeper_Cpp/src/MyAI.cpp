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
  // cout << "Inside getAction. Current number: " << number << endl;
  // cout << "Agent checkpt 0" << endl;
  // Update last action
  int x = this->agentX;
  int y = this->agentY;
  // cout << "x: " << x << "\ty: " << y << endl;
  updateVecs(number, x, y);
  // cout << "Agent checkpt 0.1" << endl;

  int numCoveredNeighbor = -1;
  int numFlaggedNeighbor = -1;
  if (number != 0) {
    neighbors(x, y, numCoveredNeighbor, numFlaggedNeighbor);
  }
  // cout << "Agent checkpt 0.2" << endl;
  // cout << "Cover neigh = " << numCoveredNeighbor << endl;
  // cout << "Flagged neigh = " << numFlaggedNeighbor << endl;

  // ****************************************************************
  // Select what to push into the next action queue based
  // on the number reveal by last action
  // ****************************************************************
  bool allClear = addForSureAround(x, y);
  // cout << "Agent checkpt 1" << endl;
  // Revealed tile doesnt give enough info
  if (!allClear) {
    comeBackLaterSet.insert({x, y});
  } else {
    // checks the previous unclear tiles
    vector<Coordinate> nowSetTiles;

    for (auto i : comeBackLaterSet) {
      if (addForSureAround(i.x, i.y)) {
        nowSetTiles.push_back(i);
      }
    }
    for (auto i : nowSetTiles) {
      comeBackLaterSet.erase(i);
    }
  }

  // cout << "Agent checkpt 2" << endl;
  // cout << "\tgetting next move" << endl;
  // ****************************************************************
  // Check what to return next
  // ****************************************************************
  while (!nextMoves.empty()) {
    Agent::Action next = nextMoves.front();
    nextMoves.pop();

    this->agentX = next.x;
    this->agentY = next.y;

    // cout << "Uncover (" << next.x << "," << next.y << ") next" << endl;
    return next;
  }
  // cout << "Agent checkpt 3" << endl;
  // cout << "GG no more in queue" << endl;
  return {LEAVE, -1, -1};
}

/*
  Add action we know for sure to do around (x, y) to queue
*/
bool MyAI::addForSureAround(int x, int y) {
  // cout << "ForSure checkpt 1" << endl;
  bool allClear = false;
  int centerVal = getTileValue(x, y);
  int numCoveredNeighbor = -1;
  int numFlaggedNeighbor = -1;
  neighbors(x, y, numCoveredNeighbor, numFlaggedNeighbor);

  // cout << "ForSure checkpt 2" << endl;
  if (centerVal != 0 && numCoveredNeighbor != centerVal &&
      numFlaggedNeighbor != centerVal) {
    return allClear;
  }
  // cout << "ForSure checkpt 3" << endl;
  allClear = true;
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;
      // cout << "Checking (" << currX << "," << currY << ")" << endl;
      if ((i == 0 && j == 0) || currX < 0 || currX >= this->colDimension ||
          currY < 0 || currY >= this->rowDimension) {
        // cout << "\tSkipped" << endl;
        continue;
      }
      TileStatus currStat = getTileStatus(currX, currY);
      // cout << "GOT STATUS" << endl;

      // There are no bombs so every surrounding tile must be safe
      if (centerVal == 0) {
        // cout << "ForSure checkpt 3.1" << endl;
        if (currStat == COVERED) {
          // cout << "\tNum = 0, pushing (" << currX << ", " << currY << ")"
          //      << endl;
          nextMoves.push({Action_type::UNCOVER, currX, currY});
          setTileStatus(currX, currY, INQ);
        }
      }
      // Every surrounding tile must be a bomb so flag everything covered
      else if (centerVal == numCoveredNeighbor) {
        // cout << "ForSure checkpt 3.2" << endl;
        //  cout << "\tNum = coveredNeighbors" << endl;
        if (currStat == COVERED) {
          setTileStatus(currX, currY, FLAGGED);
        }
      }
      // Everything not flagged must be safe
      else if (centerVal == numFlaggedNeighbor) {
        // cout << "ForSure checkpt 3.3" << endl;
        //  cout << "\tNum = numFlaggedNeighbor" << endl;
        if (currStat == COVERED) {
          nextMoves.push({Action_type::UNCOVER, currX, currY});
          setTileStatus(currX, currY, INQ);
        }
      }
    }
  }
  // cout << "ForSure checkpt 4" << endl;
  return allClear;
}

void MyAI::updateVecs(int number, int x, int y) {
  // cout << "UpdateVec (" << x << "," << y << ") = " << number << endl;
  setTileStatus(x, y, UNCOVERED);
  setTileValue(x, y, number);
}

bool MyAI::inBounds(int x, int y) {
  return x >= 0 && x < this->colDimension && y >= 0 && y < this->rowDimension;
}

/*
  Count the number of covered neighbors around a given coordinate (x, y)
*/
void MyAI::neighbors(int x, int y, int &numCoveredNeighbors, int &numFlags) {
  numCoveredNeighbors = 0;
  numFlags = 0;

  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;

      if ((i == 0 && j == 0) || currX < 0 || currX >= this->colDimension ||
          currY < 0 || currY >= this->rowDimension) {
        continue;
      }

      if (getTileStatus(currX, currY) == COVERED) {
        numCoveredNeighbors++;
      }
      if (getTileStatus(currX, currY) == FLAGGED) {
        numCoveredNeighbors++;
        numFlags++;
      }
    }
  }
}

int MyAI::countNearCovered(int x, int y) {
  int count = 0;
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;
      TileStatus currStat = getTileStatus(currX, currY);
      if (i != 0 && j != 0 && inBounds(currX, currY) &&
          (currStat == COVERED || currStat == FLAGGED)) {
        ++count;
      }
    }
  }
  return count;
}

int MyAI::countNearFlag(int x, int y) {
  int count = 0;
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;
      if (i != 0 && j != 0 && inBounds(currX, currY) &&
          getTileStatus(currX, currY) == FLAGGED) {
        ++count;
      }
    }
  }
  return count;
}

MyAI::TileStatus MyAI::getTileStatus(int x, int y) {
  return this->boardStatus.at(y).at(x);
}

void MyAI::setTileStatus(int x, int y, TileStatus newStat) {
  this->boardStatus.at(y).at(x) = newStat;
}

int MyAI::getTileValue(int x, int y) { return this->boardValues.at(y).at(x); }
void MyAI::setTileValue(int x, int y, int newVal) {
  this->boardValues.at(y).at(x) = newVal;
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
