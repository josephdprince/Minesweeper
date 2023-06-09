// ======================================================================
// FILE:        MyAI.hpp
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

#ifndef MINE_SWEEPER_CPP_SHELL_MYAI_HPP
#define MINE_SWEEPER_CPP_SHELL_MYAI_HPP

#include "Agent.hpp"
#include <algorithm>
#include <bitset>
#include <cmath>
#include <iomanip>
#include <iostream> // FIXME: temporary use
#include <map>
#include <queue>
#include <set>
#include <vector>

using namespace std;

struct Coordinate {
  int x;
  int y;

  bool operator<(const Coordinate &rhs) const { return x < rhs.x || y < rhs.y; }
  bool operator==(const Coordinate &rhs) const {
    return this->x == rhs.x && this->y == rhs.y;
  }
};

class MyAI : public Agent {
private:
  enum TileStatus { COVERED, UNCOVERED, FLAGGED, INQ };
  queue<Action> nextMoves;
  set<Coordinate> comeBackLaterSet;
  vector<vector<TileStatus>> boardStatus;
  vector<vector<int>> boardValues;
  vector<vector<int>> possiTable;
  int discovered_bomb;
  bool global_debug = true;

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

public:
  MyAI(int _rowDimension, int _colDimension, int _totalMines, int _agentX,
       int _agentY);

  Action getAction(int number) override;

private:
  void updateVecs(int number, int x, int y);
  bool inBounds(int x, int y);
  void neighbors(int x, int y, int &numCoveredNeighbors, int &numFlags);
  void neighbors(int x, int y, vector<Coordinate> &coords,
                 int &numCoveredNeighbors, int &numFlags);
  int countNearCovered(int x, int y);
  int countNearFlag(int x, int y);
  bool easyRules(int x, int y);
  void printVecs(string prefix);
  void revealAllSquares();
  void checkComeBack();
  void grabSurrTiles(int x, int y, vector<Coordinate> &coverTiles,
                     vector<Coordinate> &comeBackTails,
                     vector<Coordinate> &otherTiles);

  template <class T> bool isInVec(T target, vector<T> &targetVec);
  void printVec(vector<Coordinate> &target);

  // Getters and Setters
  TileStatus getTileStatus(int x, int y);
  int getTileValue(int x, int y);
  void setTileStatus(int x, int y, TileStatus newStat);
  void setTileValue(int x, int y, int newVal);
  int getTilePossi(int x, int y);
  void setTilePossi(int x, int y, int newVal);
};

#endif // MINE_SWEEPER_CPP_SHELL_MYAI_HPP
