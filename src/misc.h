﻿/*
  nozomi, a USI shogi playing engine derived from Stockfish (chess playing engin)
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

#ifndef _MISC_H_
#define _MISC_H_

#include <chrono>
#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>

#include "types.h"

extern const
std::string engine_info(bool to_usi = false);

extern void
prefetch(void *addr);

typedef std::chrono::milliseconds::rep TimePoint;
inline TimePoint
now() 
{ 
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

template<class Entry, size_t Size>
struct HashTable 
{
  HashTable() : table(Size, Entry()) {}

  Entry *
  operator[](Key k)
  {
    return &table[static_cast<size_t>(k) & (Size - 1)];
  }

private:
  std::vector<Entry> table;
};


enum SyncCout
{
  kIoLock,
  kIoUnlock
};

std::ostream &
operator<<(std::ostream&, SyncCout);

#define sync_cout std::cout << kIoLock
#define sync_endl std::endl << kIoUnlock

#if defined(_MSC_VER)
inline int
msb(uint64_t b)
{
  unsigned long index;
  _BitScanReverse64(&index, b);
  return (int)index;
}
#else
inline int
msb(uint64_t b)
{
  return (int)(63 - __builtin_clzll(b));
}
#endif

#endif
