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
  vector<vector<int>> numCountTable;

  int discovered_bomb;
  bool global_debug = false;

public:
  MyAI(int _rowDimension, int _colDimension, int _totalMines, int _agentX,
       int _agentY);

  Action getAction(int number) override;

private:
  void updateVecs(int number, int x, int y);
  bool easyRules(int x, int y);
  void revealAllSquares();
  void checkComeBack();
  bool checkIsPossible(Coordinate curr, const Coordinate original,
                       const vector<int> &bitTable, set<Coordinate> &visited,
                       set<Coordinate> &changed);

  // Helper
  bool inBounds(int x, int y);
  int countNearCovered(int x, int y);
  int countNearFlag(int x, int y);
  void grabSurrTiles(int x, int y, vector<Coordinate> &coverTiles,
                     vector<Coordinate> &comeBackTails,
                     vector<Coordinate> &otherTiles);
  void grabSurrCovered(int x, int y, vector<Coordinate> &storage);
  void grabSurrComeBack(int x, int y, vector<Coordinate> &storage);
  void grabSurrFlagged(int x, int y, vector<Coordinate> &storage);
  void grabSurrUncoverd(int x, int y, vector<Coordinate> &storage);

  template <class T> bool isInSet(T target, set<T> &targetSet);
  template <class T> bool isInVec(T target, vector<T> &targetVec);
  void printSet(set<Coordinate> &target);
  void printVec(vector<Coordinate> &target);
  void printCurrMaps(string prefix);
  // Getters and Setters
  TileStatus getTileStatus(int x, int y);
  int getTileValue(int x, int y);
  void setTileStatus(int x, int y, TileStatus newStat);
  void setTileValue(int x, int y, int newVal);
  double calTilePossi(int x, int y);
  int getTilePossi(int x, int y);
  void addToTilePossi(int x, int y);

  int getTileCount(int x, int y);
  void addToTileCount(int x, int y);
};

#endif // MINE_SWEEPER_CPP_SHELL_MYAI_HPP
