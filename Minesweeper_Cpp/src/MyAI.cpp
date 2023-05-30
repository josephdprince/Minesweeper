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

vector<Coordinate> similarities(const vector<Coordinate> mainCoords,
                                const vector<Coordinate> neighCoords);

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
  bool allClear = easyRules(x, y);

  // Revealed tile doesnt give enough info
  if (!allClear) {
    comeBackLaterSet.insert({x, y});
  } else {
    // Now that we have a tile that satisifed an easy rule, check neighbors to
    // see if they are in the come back set. If so, check them again with easy
    // rules
    checkComeBack();
  }

  // Found all bombs and reveal the rest that are not flagged
  if (this->discovered_bomb == this->totalMines) {
    revealAllSquares();
  }

  // If nextMoves queue is empty, check our come back later set
  // #TODO: add the prediction logic
  if (nextMoves.empty()) {
    checkComeBack();

    // Intermediate Logic on all Come Back tiles

    // Stores the number of 1's (bombs) of the number of the index
    // i.e. 0 maps to 0 bombs, 7 maps to 3 bombs...
    vector<int> lookupTable{
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4,
        2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4,
        2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6,
        4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5,
        3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6,
        4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};
    for (auto comeBackTile : this->comeBackLaterSet) {
      int currX = comeBackTile.x;
      int currY = comeBackTile.y;

      // Get info of center tile
      int numCoveredMain, numFlagsMain;
      vector<Coordinate> coordsMain;
      neighbors(currX, currY, coordsMain, numCoveredMain, numFlagsMain);

      // Get effective value
      int effectiveValMain = getTileValue(currX, currY) - numFlagsMain;

      // Get truth table size
      int ttSize = 1 << effectiveValMain;

      // Find each surrounding tile that is uncovered
      for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
          int neighX = x + i;
          int neighY = y + j;

          // bounds check
          if ((i == 0 && j == 0) || !inBounds(neighX, neighY)) {
            continue;
          }

          // We want to perform intermediate logic on curr node and neighbor
          if (getTileStatus(neighX, neighY) == UNCOVERED) {
            // Get details of neighbor tile
            int numCoveredNeigh, numFlagsNeigh;
            vector<Coordinate> coordsNeigh;
            neighbors(neighX, neighY, coordsNeigh, numCoveredNeigh,
                      numFlagsNeigh);

            // Get effective value
            int effectiveValNeigh =
                getTileValue(neighX, neighY) - numFlagsNeigh;

            // Find bomb similarities between curr and neigh
            vector<Coordinate> sims = similarities(coordsMain, coordsNeigh);
            int shareCase;
            // Case 1
            if (sims.size() == coordsNeigh.size()) {
              shareCase = 1;
            }
            // Case 2
            else if (sims.size() == 0) {
              shareCase = 2;
            }
            // Case 3
            else {
              shareCase = 3;
            }

            // Iterate through each value in truth table to determine validity
            vector<int> validRows;
            for (int row = 0; row < ttSize; ++row) {
              // Two things to consider:
              // 1. Is total number of bombs correct?
              // 2. Is number of bombs for both uncovered tile valid

              // 1:
              int numBombs = lookupTable.at(row);
              if (numBombs != effectiveValMain) {
                continue;
              }

              // 2:
              // Need to check if neigh bomb count
              int neighBombCount = 0;

              int spot = row;
              int loc = 0;
              while (spot) {
                // Check if first spot has a bomb
                if (spot & 1) {
                  Coordinate bombLoc = coordsMain.at(loc);
                  // Check if this bombLoc is also in neighbor
                  for (Coordinate c : coordsNeigh) {
                    if (c == bombLoc) {
                      ++neighBombCount;
                      break;
                    }
                  }
                }

                ++loc;
                spot >>= 1;
              }

              if ((shareCase == 1 && neighBombCount != effectiveValNeigh) ||
                  (shareCase == 3 && neighBombCount > effectiveValNeigh)) {
                continue;
              }

              validRows.push_back(row);
            }

            // for each valid row, check to see if there are any similarities.
            // Any similarity across each row must be true.
          }
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

void MyAI::grabSurrTiles(int x, int y, vector<Coordinate> &coverTiles,
                         vector<Coordinate> &comeBackTails,
                         vector<Coordinate> &otherTiles) {
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;

      // bounds check
      if ((i == 0 && j == 0) || !inBounds(currX, currY)) {
        continue;
      }
      if (comeBackLaterSet.find({currX, currY}) != comeBackLaterSet.end()) {
        // found a neighboring tile that is also in the come back later set
        comeBackTails.push_back({currX, currY});
      } else if (getTileStatus(currX, currY) == COVERED) {
        //
        coverTiles.push_back({currX, currY});
      } else {
        otherTiles.push_back({currX, currY});
      }
    }
  }
}

void MyAI::checkComeBack() {
  // checks the previous unclear tiles
  vector<Coordinate> nowSetTiles;

  for (auto i : comeBackLaterSet) {
    if (easyRules(i.x, i.y)) {
      nowSetTiles.push_back(i);
    }
  }
  for (auto i : nowSetTiles) {
    comeBackLaterSet.erase(i);
  }
}

/*
  Reveal the rest of the covered squares if flagged tiles = total # of bombs
  (meaning we found all bombs)
*/
void MyAI::revealAllSquares() {
  // Just for safety
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
  Add action to queue that we know for sure to do around (x, y)
*/
bool MyAI::easyRules(int x, int y) {
  bool allClear = false;
  int centerVal = getTileValue(x, y);
  int numCoveredNeighbor = -1;
  int numFlaggedNeighbor = -1;
  neighbors(x, y, numCoveredNeighbor, numFlaggedNeighbor);

  // We can't figure anything out
  if (centerVal != 0 && numCoveredNeighbor != centerVal &&
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

      // There are no bombs so every surrounding tile must be safe
      if (centerVal == 0) {
        if (currStat == COVERED) {
          nextMoves.push({Action_type::UNCOVER, currX, currY});
          setTileStatus(currX, currY, INQ);
        }
      }
      // Every surrounding tile must be a bomb so flag everything covered
      else if (centerVal == numCoveredNeighbor) {
        if (currStat == COVERED) {
          setTileStatus(currX, currY, FLAGGED);
          // NOTE: This line is just for visualization purposes for
          // debugging Comment this line if not debugging
          nextMoves.push({Action_type::FLAG, currX, currY});
          ++discovered_bomb;
        }
      }
      // Everything not flagged must be safe
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
  Update the tile at (x, y) with value = number
*/
void MyAI::updateVecs(int number, int x, int y) {
  if (number == -1)
    return;
  setTileStatus(x, y, UNCOVERED);
  setTileValue(x, y, number);
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;

      // bound check
      if ((i == 0 && j == 0) || !inBounds(currX, currY)) {
        continue;
      }
      setTilePossi(currX, currY, number);
    }
  }
}

/*
  Check if a given coordinate (x, y) is within bound of the map
*/
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

      if ((i == 0 && j == 0) || !inBounds(currX, currY)) {
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

void MyAI::neighbors(int x, int y, vector<Coordinate> &coords,
                     int &numCoveredNeighbors, int &numFlags) {
  numCoveredNeighbors = 0;
  numFlags = 0;

  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int currX = x + i;
      int currY = y + j;

      if ((i == 0 && j == 0) || !inBounds(currX, currY)) {
        continue;
      }
      if (getTileStatus(currX, currY) == COVERED) {
        numCoveredNeighbors++;
        coords.push_back({currX, currY});
      }
      if (getTileStatus(currX, currY) == FLAGGED) {
        numCoveredNeighbors++;
        numFlags++;
      }
    }
  }
}

/*
  Return the number of covered tiles around (x, y) including flagged ones
*/
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

/*
  Print out the current boardStatus map to cout
*/
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

//==================================================
// Helpers
//==================================================

vector<Coordinate> similarities(const vector<Coordinate> mainCoords,
                                const vector<Coordinate> neighCoords) {
  vector<Coordinate> similarities;
  // FIXME: make more efficient
  for (Coordinate i : mainCoords) {
    for (Coordinate j : neighCoords) {
      if (i == j) {
        similarities.push_back(i);
        break;
      }
    }
  }
  return similarities;
}