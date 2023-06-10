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

ostream &operator<<(ostream &ost, const Coordinate target) {
  return ost << "(" << target.x + 1 << ", " << target.y + 1 << ")";
}

MyAI::MyAI(int _rowDimension, int _colDimension, int _totalMines, int _agentX,
           int _agentY)
    : Agent() {
  // Initialize data in base class
  this->rowDimension = _rowDimension;
  this->colDimension = _colDimension;
  this->totalMines = _totalMines;
  this->agentX = _agentX;
  this->agentY = _agentY;
  this->discovered_bomb = 0;

  // Populate both arrays with starting board information
  // TODO: This is backwars from how world.cpp does it
  for (int i = 0; i < _rowDimension; ++i) {
    vector<TileStatus> initialStatus(_colDimension, COVERED);
    vector<int> initialValues(_colDimension, -1);
    vector<int> initialPossi(_colDimension, 100);

    possiTable.push_back(initialPossi);
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
  // Update last action
  int x = this->agentX;
  int y = this->agentY;
  updateVecs(number, x, y);

  // ****************************************************************
  // Select what to push into the next action queue based
  // on the number reveal by last action
  // ****************************************************************
  if (number != -1) {
    bool allClear = easyRules(x, y);

    // Revealed tile doesnt give enough info
    if (!allClear) {
      if (global_debug) {
        cout << "**************" << endl;
        cout << "Inserting into comeBack(" << x + 1 << ", " << y + 1 << ")"
             << endl;
      }
      auto insertion = comeBackLaterSet.insert({x, y});
      if (global_debug) {
        printSet(this->comeBackLaterSet);
        cout << "Inserted or not = " << insertion.second;
        cout << "**************" << endl;
      }
    } else {
      // Now that we have a tile that satisifed an easy rule, check neighbors to
      // see if they are in the come back set. If so, check them again with easy
      // rules
      checkComeBack();
    }
  }

  // Found all bombs and reveal the rest that are not flagged
  if (this->discovered_bomb == this->totalMines) {
    revealAllSquares();
  }

  // If nextMoves queue is empty, check our come back later set
  if (nextMoves.empty()) {
    if (global_debug) {
      cout << "QUEUE is empty, ComeBack easy check" << endl;
    }
    checkComeBack();
  }

  // still empty after chekcing easy logic on comeback set
  // #TODO: add the Intermediate logic
  if (nextMoves.empty()) {
    if (global_debug) {
      cout << "QUEUE is empty, ComeBack advance check" << endl;
    }
    // Intermediate Logic on all Come Back tiles
    for (auto comeBackTile : this->comeBackLaterSet) {
      int currX = comeBackTile.x;
      int currY = comeBackTile.y;
      int currVal = getTileValue(currX, currY);
      if (global_debug) {
        cout << "\tChekcing comback (" << currX + 1 << ", " << currY + 1 << ")"
             << endl;
      }
      // identify and store all surrounding tiles
      vector<Coordinate> allCoveredTile;
      vector<Coordinate> allComeBackTile;
      vector<Coordinate> allOtherTile;
      grabSurrTiles(currX, currY, allCoveredTile, allComeBackTile,
                    allOtherTile);

      // #TODO: check possible bomb possition and see valid or not
      /*
        go through all the come back later tiles then check if the tile is
        valid. Remove any that is not valid. Find th common spot that can be a
        zero
      */
      int ttableSize = 1 << allCoveredTile.size();
      int count = 0;
      if (global_debug) {
        cout << "\t\tTruth Table size = 2^" << allCoveredTile.size() << " = "
             << ttableSize << endl;
      }
      vector<int> validCombination;
      while (count < ttableSize) {
        if (global_debug) {
          cout << "\t\t**************" << endl;
          cout << "\t\tCombination " << count << " = ";
        }
        // going through each entry in truth table look for valid values
        vector<int> bitTable(allCoveredTile.size());
        int copy = count;
        int bombcount = countNearFlag(currX, currY);
        // grab the bit representation of the current truth table row
        for (int i = 0; i < bitTable.size(); i++) {
          if (copy & 1) {
            ++bombcount;
          }
          bitTable[i] = copy & 1;
          copy >>= 1;
        }
        if (global_debug) {
          for (int i : bitTable) {
            cout << i;
          }
          for (int i = 0; i < bitTable.size(); i++) {
            cout << " (" << allCoveredTile[i].x + 1 << ", "
                 << allCoveredTile[i].y + 1 << "),";
          }
          cout << endl;
          cout << "\t\t**************" << endl;
        }
        // too many or too little bombs for this combination
        if (bombcount != currVal) {
          continue;
        }

        // check validity of the current truth table row
        set<Coordinate> changedTiles;
        set<Coordinate> visited;
        if (checkIsPossible(currX, currY, comeBackTile, bitTable, visited,
                            changedTiles)) {
          validCombination.push_back(count);
          if (global_debug) {
            cout << "\t\t\tPossible combination" << endl;
          }
        } else if (global_debug) {
          cout << "\t\t\tNot Possible combination" << endl;
        }

        // set the tiles back to covered
        for (int i = 0; i < bitTable.size(); i++) {
          Coordinate currtargettile = allCoveredTile[i];
          setTileStatus(currtargettile.x, currtargettile.y, COVERED);
        }
        ++count;
      }

      // TODO: check common variable across the possible combination
      vector<int> sumTable(allCoveredTile.size());

      // count the bits in the valid combination. If equals to
      // validCombination.size that means that bit appeared in all the
      // combination meaning it is safe to uncover
      for (int i : validCombination) {
        int copy = i;
        // grab the bit representation of the current truth table row
        for (int i = 0; i < sumTable.size(); i++) {
          sumTable[i] += copy & 1;
          copy >>= 1;
        }
      }
      for (int i = 0; i < sumTable.size(); i++) {

        if (sumTable[i] == validCombination.size()) {
          if (global_debug) {
            cout << "\t\t  INTERMEDIATE LOGIC Flag TILE: "
                    "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
                 << endl;
            cout << "\t\t  (" << allCoveredTile[i].x + 1 << ", "
                 << allCoveredTile[i].y + 1 << ")" << endl;
          }
          Coordinate validTile = allCoveredTile[i];
          nextMoves.push({Action_type::FLAG, validTile.x, validTile.y});
          setTileStatus(validTile.x, validTile.y, INQ);
        } else if (sumTable[i] == 0) {
          if (global_debug) {
            cout << "\t\t  INTERMEDIATE LOGIC Uncover TILE: "
                    "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
                 << endl;
            cout << "\t\t  (" << allCoveredTile[i].x + 1 << ", "
                 << allCoveredTile[i].y + 1 << ")" << endl;
          }
          Coordinate validTile = allCoveredTile[i];
          nextMoves.push({Action_type::UNCOVER, validTile.x, validTile.y});
          setTileStatus(validTile.x, validTile.y, INQ);
        }
      }
    }
  }
  // ****************************************************************
  // Check what to return next
  // ****************************************************************
  while (!nextMoves.empty()) {
    Agent::Action next = nextMoves.front();
    nextMoves.pop();

    this->agentX = next.x;
    this->agentY = next.y;
    return next;
  }
  // Exhausted all Action we leave
  return {LEAVE, -1, -1};
}

// TODO: git gud and implement this
bool MyAI::checkIsPossible(Coordinate curr, const Coordinate original,
                           const vector<int> &bitTable,
                           set<Coordinate> &visited, set<Coordinate> &changed) {

  vector<Coordinate> justChanged;
  if (isInSet(curr, visited)) {
    
  } else {
    if (curr == original) {
      // set the covered tile to Flagged/Covered acoording to current truth
      // table row
      vector<Coordinate> allCoveredTile;
      grabSurrCovered(curr.x, curr.y, allCoveredTile);
      for (int i = 0; i < bitTable.size(); i++) {
        Coordinate currtargettile = allCoveredTile[i];
        changed.insert(currtargettile);
        justChanged.push_back(currtargettile);
        if (bitTable[i] == 1) {
          setTileStatus(currtargettile.x, currtargettile.y, FLAGGED);
        } else {
          setTileStatus(currtargettile.x, currtargettile.y, UNCOVERED);
        }
      }

      int effectiveNum =
          getTileValue(curr.x, curr.y) - countNearFlag(curr.x, curr.y);
      visited.insert(curr);
      vector<Coordinate> needToCheck;
      bool stillPossible = true;
      for (Coordinate i : justChanged) {
        grabSurrComeBack(i.x, i.y, needToCheck);
      }
      for (Coordinate i : needToCheck) {
        if (stillPossible == false) {
          break;
        }
      }
      return stillPossible && effectiveNum == 0;
    }
  }
}

void MyAI::checkComeBack() {
  // checks the previous unclear tiles
  vector<Coordinate> toBeRemoveTiles;

  for (Coordinate i : comeBackLaterSet) {
    if (easyRules(i.x, i.y)) {
      toBeRemoveTiles.push_back(i);
    }
  }
  if (global_debug) {
    for (Coordinate i : toBeRemoveTiles) {
      cout
          << "XXXXXXXXXXXXXXXXXXXXremoving from comeback set from checkComeBack"
          << endl;
      comeBackLaterSet.erase(i);
    }
  }
}

/*
  Reveal the rest of the covered squares if flagged tiles = total # of bombs
  (meaning we found all bombs).
*/
void MyAI::revealAllSquares() {
  // Just for safety check
  if (this->totalMines != this->discovered_bomb) {
    return;
  }

  int numCol = this->colDimension;
  int numRow = this->rowDimension;
  for (int i = 0; i < numRow; ++i) {
    for (int j = 0; j < numCol; ++j) {
      if (getTileStatus(i, j) == COVERED) {
        nextMoves.push({Action_type::UNCOVER, i, j});
        setTileStatus(i, j, INQ);
      }
    }
  }
}

/*
  Add action to queue that we know for sure to do around (x, y) if:
    1. Value at (x, y) = 0 (meaning no bomb) --> reaveal all neighboring tiles
    2. Value at (x, y) = # of Covered tiles  --> flag every covered tiles
    3. Value at (x, y) = # of Flagged neighbors --> Uncover all the surrounding
      tiles
*/
bool MyAI::easyRules(int x, int y) {
  bool allClear = false;
  int centerVal = getTileValue(x, y);
  int numCoveredNeighbor = countNearCovered(x, y);
  int numFlaggedNeighbor = countNearFlag(x, y);

  // We can't figure anything out
  // centerVal ==0 || numCoveredNeighbor == centerVal || numFlaggedNeighbor ==
  // centerVal
  if (centerVal != 0 && numCoveredNeighbor + numFlaggedNeighbor != centerVal &&
      numFlaggedNeighbor != centerVal) {
    return allClear;
  }

  // We can easily figure something out, so we iterate through each
  // surrounding tile
  allClear = true;
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;

      // bounds check
      if ((i == 0 && j == 0) || !inBounds(currX, currY)) {
        continue;
      }
      TileStatus currStat = getTileStatus(currX, currY);

      // 1. There are no bombs so every surrounding tile must be safe
      if (centerVal == 0) {
        if (currStat == COVERED) {
          nextMoves.push({Action_type::UNCOVER, currX, currY});
          setTileStatus(currX, currY, INQ);
        }
      }
      // 2. Every surrounding tile must be a bomb so flag everything covered
      else if (centerVal == numCoveredNeighbor) {
        if (currStat == COVERED) {
          setTileStatus(currX, currY, FLAGGED);
          // NOTE: This is just for visualization purposes for debugging.
          // It puts the Flag action into the action queue, so that when running
          // we can see what is flagged
          if (global_debug) {
            nextMoves.push({Action_type::FLAG, currX, currY});
          }
          ++discovered_bomb;
        }
      }
      // 3. Everything not flagged must be safe
      else if (centerVal == numFlaggedNeighbor) {
        if (currStat == COVERED) {
          nextMoves.push({Action_type::UNCOVER, currX, currY});
          setTileStatus(currX, currY, INQ);
        }
      }
    }
  }
  return allClear;
}

/*
  Update the tile at (x, y) with value = number and mark as UNCOVERED
*/
void MyAI::updateVecs(int number, int x, int y) {
  if (number == -1)
    return;
  setTileStatus(x, y, UNCOVERED);
  setTileValue(x, y, number);
}

/*
==================================================
Helper Fucntions
==================================================
*/

void MyAI::grabSurrTiles(int x, int y, vector<Coordinate> &coverTiles,
                         vector<Coordinate> &comeBackTails,
                         vector<Coordinate> &otherTiles) {

  grabSurrCovered(x, y, coverTiles);
  grabSurrComeBack(x, y, comeBackTails);
  grabSurrFlagged(x, y, otherTiles);
  grabSurrUncoverd(x, y, otherTiles);
}

void MyAI::grabSurrCovered(int x, int y, vector<Coordinate> &storage) {
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;
      if ((i == 0 && j == 0) || !inBounds(currX, currY)) {
        continue;
      }

      if (getTileStatus(currX, currY) == COVERED) {
        storage.push_back({currX, currY});
      }
    }
  }
}
void MyAI::grabSurrComeBack(int x, int y, vector<Coordinate> &storage) {
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;
      if ((i == 0 && j == 0) || !inBounds(currX, currY)) {
        continue;
      }

      if (isInSet({currX, currY}, comeBackLaterSet)) {
        storage.push_back({currX, currY});
      }
    }
  }
}
void MyAI::grabSurrFlagged(int x, int y, vector<Coordinate> &storage) {
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;
      if ((i == 0 && j == 0) || !inBounds(currX, currY)) {
        continue;
      }

      if (getTileStatus(currX, currY) == FLAGGED) {
        storage.push_back({currX, currY});
      }
    }
  }
}
void MyAI::grabSurrUncoverd(int x, int y, vector<Coordinate> &storage) {
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;
      if ((i == 0 && j == 0) || !inBounds(currX, currY)) {
        continue;
      }
      if (getTileStatus(currX, currY) == UNCOVERED) {
        storage.push_back({currX, currY});
      }
    }
  }
}

template <class T> bool isInSet(T target, set<T> &targetSet) {
  bool ans = false;
  for (T i : targetSet) {
    if (i == target) {
      ans = true;
      break;
    }
  }
  return ans;
}

template <class T> bool isInVec(T target, vector<T> &targetVec) {
  bool ans = false;
  for (T i : targetVec) {
    if (i == target) {
      ans = true;
      break;
    }
  }
  return ans;
}

/*
  Check if a given coordinate (x, y) is within bound of the map
*/
bool MyAI::inBounds(int x, int y) {
  return x >= 0 && x < this->colDimension && y >= 0 && y < this->rowDimension;
}

/*
  Return the number of covered tiles around (x, y) excluding flagged ones
*/
int MyAI::countNearCovered(int x, int y) {
  int count = 0;
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;
      if (i != 0 && j != 0 && inBounds(currX, currY) &&
          getTileStatus(currX, currY) == COVERED) {
        ++count;
      }
    }
  }
  return count;
}

/*
  Return the number of flagged tiles around (x, y)
*/
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

void MyAI::printCurrMaps(string prefix = "") {
  int numRows = this->rowDimension;
  int numCols = this->colDimension;
  cout << prefix << "SIZE: " << numRows << "X" << numCols << endl;
  for (int col = numCols - 1; col >= 0; --col) {
    cout << prefix;
    printf("%-4d%c", col + 1, '|');
    for (int row = 0; row < numRows; ++row) {
      if (getTileStatus(row, col) == COVERED) {
        cout << ". ";
      } else if (getTileStatus(row, col) == FLAGGED) {
        cout << "# ";
      } else if (getTileStatus(row, col) == INQ) {
        cout << "Q ";
      } else {
        cout << getTileValue(row, col) << " ";
      }
    }
    cout << endl;
  }
  cout << prefix << "     ";
  for (int c = 0; c < numRows; ++c)
    cout << "- ";
  cout << endl;

  cout << prefix << "     ";
  for (int c = 0; c < numRows; ++c)
    cout << c + 1 << " ";
  cout << endl;
}

/*
  Print out the given set<Coordinate>
*/
void MyAI::printSet(set<Coordinate> &target) {
  cout << "Printing a Coordinate Set: ";
  for (Coordinate i : target) {
    cout << "(" << i.x + 1 << ", " << i.y + 1 << "), ";
  }
  cout << endl;
}

/*
  Print out the given vector<Coordinate>
*/
void MyAI::printVec(vector<Coordinate> &target) {
  cout << "Printing a Coordinate Vector: ";
  for (Coordinate i : target) {
    cout << "(" << i.x + 1 << ", " << i.y + 1 << "), ";
  }
  cout << endl;
}

//==================================================
// Getter & Setters
//==================================================

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

int MyAI::getTilePossi(int x, int y) { return this->possiTable.at(y).at(x); }

void MyAI::setTilePossi(int x, int y, int newVal) {
  if (getTilePossi(x, y) == 100) {
    this->possiTable.at(y).at(x) = newVal;
  } else {
    this->possiTable.at(y).at(x) += newVal;
  }
}
