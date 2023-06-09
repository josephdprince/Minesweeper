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
      comeBackLaterSet.insert({x, y});
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
    checkComeBack();
  }

  // If the next MOves queue is still empty after easy rules, we will run the
  // intermediate logic on the come back set
  if (nextMoves.empty()) {
    // Intermediate Logic on all Come Back tiles

    for (Coordinate comeBackTile : this->comeBackLaterSet) {
      int currX = comeBackTile.x;
      int currY = comeBackTile.y;

      // Get info of curr come back tile
      int numCoveredMain, numFlagsMain;
      vector<Coordinate> coordsMain;
      neighbors(currX, currY, coordsMain, numCoveredMain, numFlagsMain);
      // Get effective value
      int effectiveValMain = getTileValue(currX, currY) - numFlagsMain;
      if (coordsMain.size() == 0) {
        continue;
      }
      // Get truth table size
      int ttSize = 1 << coordsMain.size();
      vector<int> validRows;

      if (global_debug == true) {
        cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
        printVecs(""); // NOTE REMEMEBER TO REMOVE THIS
        cout << "checking comeback tiles: " << comeBackTile << endl;
        cout << "coordsMain size = " << coordsMain.size() << ": ";
        for (Coordinate i : coordsMain) {
          cout << i << " ";
        }
        cout << endl;
        cout << "Num covered: " << numCoveredMain << endl;
        cout << "effective val: " << effectiveValMain << endl;
        cout << "num flags: " << numFlagsMain << endl;
        cout << "tile value: " << getTileValue(currX, currY) << endl;
        cout << "TruthTable Size = " << ttSize << endl;
        cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
      }
      // Find each surrounding tile that is uncovered and insert it into
      // into validRows
      // Tile @ (x, y) is reference as curr. We go through the tiles around
      // curr.

      for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
          int neighX = x + i;
          int neighY = y + j;

          // bounds check and continue if neighbor tile is covered
          if ((i == 0 && j == 0) || !inBounds(neighX, neighY) ||
              getTileStatus(neighX, neighY) != UNCOVERED) {
            continue;
          }

          // We want to perform intermediate logic on curr node and neighbor
          // Get details of neighbor tile
          int numCoveredNeigh, numFlagsNeigh;
          vector<Coordinate> coordsNeigh;
          neighbors(neighX, neighY, coordsNeigh, numCoveredNeigh,
                    numFlagsNeigh);
          // Get effective value of Neighbor tile
          int effectiveValNeigh = getTileValue(neighX, neighY) - numFlagsNeigh;

          // Find shared Covered tiles between curr and neigh. Base on the #
          // of covered tiles shares, different logics are performed
          vector<Coordinate> sims = similarities(coordsMain, coordsNeigh);
          int shareCase;
          if (sims.size() == coordsNeigh.size()) {
            // Case 1 all COVERED tiles of the neighbor tiles is a subset of
            // the curr tile
            shareCase = 1;
          } else if (sims.size() == 0) {
            // Case 2 No overlaps of covered tiles between neighbor tiles and
            // curr tile
            shareCase = 2;
          } else {
            // Case 3 some of the covered near neighbor are not in the curr
            // one too
            shareCase = 3;
          }
          // try {
          // Iterate through each value in truth table to determine validity
          for (int row = 0; row < ttSize; ++row) {
            // Two things to consider for checking validity:
            // 1. Is total number of bombs correct
            // 2. Is number of bombs for both uncovered tile valid

            // 1: grabbing from  the pre generated lookup table to check the
            // number of bombs should exsist for this current row in the truth
            // table
            int numBombs;
            numBombs = lookupTable.at(row);
            if (numBombs != effectiveValMain) {
              continue;
            }
            // 2:
            // Need to check if neigh bomb count
            if (shareCase != 2) {
              int neighBombCount = 0;

              int spot = row;
              int loc = 0;
              while (spot) {
                // Check if first spot has a bomb
                if (spot & 1) {
                  Coordinate bombLoc;
                  // try {
                  bombLoc = coordsMain.at(loc);
                  // } catch (const std::exception &exc) {
                  //   cout << "The error is happening here2" << endl;
                  //   cout << "\t" << exc.what() << endl;
                  // }
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
            }
            if (!isInVec(row, validRows)) {
              validRows.push_back(row);
            }
          }
          // } catch (const std::exception &exc) {
          //   cout << "Failed to grab valid rows " << exc.what() << endl;
          // }
        }
      }

      // Now the validRows is filled with valid possible entries. For each entry
      // , check to see if there are any similarities to other entries. Any
      // similarity across each row must be true. For example if position 1 of
      // the coveredNearCurr is a bomb across all rows then it is true that it
      // will be a bomb
      // try {
      vector<int> sumTable(coordsMain.size());
      if (global_debug) {
        cout << "coordsMain.size() = " << coordsMain.size() << endl;
        cout << "validRows.size() = " << validRows.size() << endl;
      }
      for (int row : validRows) {
        int copy = row;
        // grab the bit representation of the current entry in validRows
        for (int i = 0; i < sumTable.size(); i++) {
          sumTable[i] += copy & 1;
          copy >>= 1;
        }
      }
      if (global_debug) {
        cout << "sumtable = ";
        for (int i : sumTable) {
          cout << i << " ";
        }
        cout << endl;
        cout << "validRows = ";
        for (int i : validRows) {
          cout << i << " ";
        }
        cout << endl;
      }
      bool added = false;
      for (int i = 0; i < sumTable.size(); i++) {
        added = added || sumTable[i] == validRows.size() || sumTable[i] == 0;
        if (sumTable[i] == validRows.size()) {
          Coordinate validTile = coordsMain[i];
          nextMoves.push({Action_type::FLAG, validTile.x, validTile.y});
          setTileStatus(validTile.x, validTile.y, INQ);
          if (global_debug)
            cout << "\t FLAG FROM INTER :" << validTile << endl;
        } else if (sumTable[i] == 0) {
          Coordinate validTile = coordsMain[i];
          nextMoves.push({Action_type::UNCOVER, validTile.x, validTile.y});
          setTileStatus(validTile.x, validTile.y, INQ);
          if (global_debug)
            cout << "\t UNCOVERING FROM INTER :" << validTile << endl;
        }
      }
      if (global_debug && added) {
        cout << "\tPrint Vecs" << endl;
        printVecs("\t"); // NOTE REMEMEBER TO REMOVE THIS
      }
      // } catch (...) {
      //   cout << "UH OH STICKY" << endl;
      // }
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

/*
  Come back to the tiles in comebacklater set and see if easy rule can be
  applied to any of them
*/
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
  Add action to queue that we know for sure to do around (x, y) if:
    1. Value at (x, y) = 0 (meaning no bomb) --> reaveal all neighboring tiles
    2. Value at (x, y) = # of Covered tiles  --> flag every covered tiles
    3. Value at (x, y) = # of Flagged neighbors --> Uncover all the surrounding
      tiles
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
          // NOTE: This is just for visualization purposes for debugging.
          // It puts the Flag action into the action queue, so that when running
          // we can see what is flagged
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
  Update the tile at (x, y) with value = number and mark as UNCOVERED
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

/*
  Count the number of covered neighbors around a given coordinate (x, y).
  Store the neighbors that are COVERED into peram coords
*/
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
  Return the number of covered tiles around (x, y) INCLUDING flagged ones
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
void MyAI::printVecs(string prefix) {
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

//==================================================
// Helpers
//==================================================

/*
  Find the Coordinates that appear in both mainCoords and neighCoords
*/
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

template <class T> bool MyAI::isInVec(T target, vector<T> &targetVec) {
  bool ans = false;
  for (T i : targetVec) {
    if (i == target) {
      ans = true;
      break;
    }
  }
  return ans;
}