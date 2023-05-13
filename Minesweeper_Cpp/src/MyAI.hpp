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
#include <iostream> // FIXME: temporary use
#include <map>
#include <queue>
#include <set>
#include <vector>

using namespace std;
// TODO: Memory management
class MyAI : public Agent {
private:
  enum TileStatus { COVERED, UNCOVERED, FLAGGED };
  queue<Action> nextMoves;
  vector<vector<TileStatus>> boardStatus;
  vector<vector<int>> boardValues;

public:
  MyAI(int _rowDimension, int _colDimension, int _totalMines, int _agentX,
       int _agentY);
  ~MyAI();
  Action getAction(int number) override;

private:
  void updateVecs(int number, int x, int y);
  bool inBounds(int x, int y);
  void neighbors(int x, int y, int &numCoveredNeighbors, int &numFlags);

  void printVecs();
};

#endif // MINE_SWEEPER_CPP_SHELL_MYAI_HPP
