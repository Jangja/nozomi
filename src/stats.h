﻿/*
  nozomi, a USI shogi playing engine
  Copyright (C) 2016 Yuhei Ohmori

  This code is based on Stockfish (Chess playing engin).
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2016 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad

  nozomi is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  nozomi is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _STATS_H_
#define _STATS_H_

#include <cstring>
#include "types.h"
#include "move.h"

template<typename T, bool CM = false>
struct Stats 
{
  static const Value
  kMax = Value(1 << 28);

  const T *
  operator[](Piece pc) const
  {
    return table[pc];
  }

  T *
  operator[](Piece p)
  {
    return table[p]; 
  }
  
  void 
  clear()
  { 
    std::memset(table, 0, sizeof(table));
  }

  void 
  update(Piece p, Square to, Move m) 
  {
    table[p][to] = m;
  }

  void
  update(Piece p, Square to, Value v)
  {
    if (abs(int(v)) >= 324)
      return;

    table[p][to] -= table[p][to] * abs(int(v)) / (CM ? 936 : 324);
    table[p][to] += int(v) * 32;
  }

private:
  T table[kPieceMax][kBoardSquare]; // kPieceMax = 31
};

typedef Stats<Move> MovesStats;
typedef Stats<Value, false> HistoryStats;
typedef Stats<Value, true> CounterMoveStats;
typedef Stats<CounterMoveStats> CounterMoveHistoryStats;

struct FromToStats
{
  Value
  get(Color c, Move m) const
  {
    return table[c][move_from(m)][move_to(m)];
  }

  void
  clear()
  {
    std::memset(table, 0, sizeof(table));
  }

  void
  update(Color c, Move m, Value v)
  {
    if (abs(int(v)) >= 324)
      return;

    Square f = move_from(m);
    Square t = move_to(m);
    assert(f < kNumberOfBoardHand);
    table[c][f][t] -= table[c][f][t] * abs(int(v)) / 324;
    table[c][f][t] += int(v) * 32;
  }

private:
  Value table[kNumberOfColor][kNumberOfBoardHand][kBoardSquare];
};

#endif
