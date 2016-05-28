/*
  nozomi, a USI shogi playing engine
  Copyright (C) 2016 Yuhei Ohmori

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

#ifndef _EVALUATE_H_
#define _EVALUATE_H_

#include "types.h"
#include "move.h"

class Position;
struct SearchStack;

namespace Eval 
{
enum KPPIndex
{
  kFHandPawn = 0,
  kEHandPawn = kFHandPawn + 19,
  kFHandLance = kEHandPawn + 19,
  kEHandLance = kFHandLance + 5,
  kFHandKnight = kEHandLance + 5,
  kEHandKnight = kFHandKnight + 5,
  kFHandSilver = kEHandKnight + 5,
  kEHandSilver = kFHandSilver + 5,
  kFHandGold = kEHandSilver + 5,
  kEHandGold = kFHandGold + 5,
  kFHandBishop = kEHandGold + 5,
  kEHandBishop = kFHandBishop + 3,
  kFHandRook = kEHandBishop + 3,
  kEHandRook = kFHandRook + 3,
  kFEHandEnd = kEHandRook + 3,

  kFPawn = kFEHandEnd,
  kEPawn = kFPawn + 81,
  kFLance = kEPawn + 81,
  kELance = kFLance + 81,
  kFKnight = kELance + 81,
  kEKnight = kFKnight + 81,
  kFSilver = kEKnight + 81,
  kESilver = kFSilver + 81,
  kFGold = kESilver + 81,
  kEGold = kFGold + 81,
  kFBishop = kEGold + 81,
  kEBishop = kFBishop + 81,
  kFHorse = kEBishop + 81,
  kEHorse = kFHorse + 81,
  kFRook = kEHorse + 81,
  kERook = kFRook + 81,
  kFDragon = kERook + 81,
  kEDragon = kFDragon + 81,
  kFEEnd = kEDragon + 81,
  kFENone = kFEEnd
};

constexpr KPPIndex
PieceToIndexBlackTable[kPieceMax] =
{
  kFENone,  // kEmpty
  kFPawn,   // kBlackPawn
  kFLance,  // kBlackLance
  kFKnight, // kBlackKnight
  kFSilver, // kBlackSilver
  kFBishop, // kBlackBishop
  kFRook,   // kBlackRook
  kFGold,   // kBlackGold
  kFENone,  // kBlackKing
  kFGold,   // kBlackPromotedPawn
  kFGold,   // kBlackPromotedLance
  kFGold,   // kBlackPromotedKnight
  kFGold,   // kBlackPromotedSilver
  kFHorse,  // kBlackHorse
  kFDragon, // kBlackDragon
  kFENone,  // 15
  kFENone,  // kFlagWhite
  kEPawn,   // kWhitePawn
  kELance,  // kWhiteLance
  kEKnight, // kWhiteKnight
  kESilver, // kWhiteSilver
  kEBishop, // kWhiteBishop
  kERook,   // kWhiteRook
  kEGold,   // kWhiteGold
  kFENone,  // kWhiteKing
  kEGold,   // kWhitePromotedPawn
  kEGold,   // kWhitePromotedLance
  kEGold,   // kWhitePromotedKnight
  kEGold,   // kWhitePromotedSilver
  kEHorse,  // kWhiteHorse
  kEDragon  // kWhiteDragon
};

constexpr KPPIndex
PieceToIndexWhiteTable[kPieceMax] =
{
  kFENone,  // kEmpty
  kEPawn,   // kBlackPawn
  kELance,  // kBlackLance
  kEKnight, // kBlackKnight
  kESilver, // kBlackSilver
  kEBishop, // kBlackBishop
  kERook,   // kBlackRook
  kEGold,   // kBlackGold
  kFENone,  // kBlackKing
  kEGold,   // kBlackPromotedPawn
  kEGold,   // kBlackPromotedLance
  kEGold,   // kBlackPromotedKnight
  kEGold,   // kBlackPromotedSilver
  kEHorse,  // kBlackHorse
  kEDragon, // kBlackDragon
  kFENone,  // 15
  kFENone,  // kFlagWhite
  kFPawn,   // kWhitePawn
  kFLance,  // kWhiteLance
  kFKnight, // kWhiteKnight
  kFSilver, // kWhiteSilver
  kFBishop, // kWhiteBishop
  kFRook,   // kWhiteRook
  kFGold,   // kWhiteGold
  kFENone,  // kWhiteKing
  kFGold,   // kWhitePromotedPawn
  kFGold,   // kWhitePromotedLance
  kFGold,   // kWhitePromotedKnight
  kFGold,   // kWhitePromotedSilver
  kFHorse,  // kWhiteHorse
  kFDragon  // kWhiteDragon
};

constexpr KPPIndex
PieceTypeToBlackHandIndexTable[kNumberOfColor][kPieceTypeMax] =
{
  {
    kFEHandEnd,
    kFHandPawn,
    kFHandLance,
    kFHandKnight,
    kFHandSilver,
    kFHandBishop,
    kFHandRook,
    kFHandGold,
    kFEHandEnd,
    kFHandPawn,
    kFHandLance,
    kFHandKnight,
    kFHandSilver,
    kFHandBishop,
    kFHandRook
  },
  {
    kFEHandEnd,
    kEHandPawn,
    kEHandLance,
    kEHandKnight,
    kEHandSilver,
    kEHandBishop,
    kEHandRook,
    kEHandGold,
    kFEHandEnd,
    kEHandPawn,
    kEHandLance,
    kEHandKnight,
    kEHandSilver,
    kEHandBishop,
    kEHandRook
  }
};

constexpr KPPIndex
PieceTypeToWhiteHandIndexTable[kNumberOfColor][kPieceTypeMax] =
{
  {
    kFEHandEnd,
    kEHandPawn,
    kEHandLance,
    kEHandKnight,
    kEHandSilver,
    kEHandBishop,
    kEHandRook,
    kEHandGold,
    kFEHandEnd,
    kEHandPawn,
    kEHandLance,
    kEHandKnight,
    kEHandSilver,
    kEHandBishop,
    kEHandRook
  },
  {
    kFEHandEnd,
    kFHandPawn,
    kFHandLance,
    kFHandKnight,
    kFHandSilver,
    kFHandBishop,
    kFHandRook,
    kFHandGold,
    kFEHandEnd,
    kFHandPawn,
    kFHandLance,
    kFHandKnight,
    kFHandSilver,
    kFHandBishop,
    kFHandRook
  }
};


constexpr int 
KPPHandIndex[8] =
{
  0,  // None
  0,  // Pawn
  2,  // Lance
  4,  // Knight
  6,  // Silver
  10, // Bishop
  12, // Rook
  8   //Gold
};

#ifdef Apery 
enum PieceValue
{
  kPawnValue = 90,
  kLanceValue = 315,
  kKnightValue = 405,
  kSilverValue = 495,
  kGoldValue = 540,
  kProSilverValue = 540,
  kProLanceValue = 540,
  kProKnightValue = 540,
  kProPawnValue = 540,
  kBishopValue = 855,
  kRookValue = 990,
  kHorseValue = 945,
  kDragonValue = 1395,
  kKingValue = 15000
};
#else
enum PieceValue
{
  kPawnValue      = 86,
  kLanceValue     = 235,
  kKnightValue    = 257,
  kSilverValue    = 369,
  kGoldValue      = 444,
  kProSilverValue = 489,
  kProLanceValue  = 492,
  kProKnightValue = 516,
  kProPawnValue   = 542,
  kBishopValue    = 564,
  kRookValue      = 637,
  kHorseValue     = 823,
  kDragonValue    = 946,
  kKingValue      = 15000
};
#endif

constexpr int
PieceValueTable[kPieceTypeMax] =
{
  0,
  kPawnValue,
  kLanceValue,
  kKnightValue,
  kSilverValue,
  kBishopValue,
  kRookValue,
  kGoldValue,
  kKingValue,
  kProPawnValue,
  kProLanceValue,
  kProKnightValue,
  kProSilverValue,
  kHorseValue,
  kDragonValue
};

constexpr int
PromotePieceValueTable[7] =
{
  0,
  PieceValueTable[kPromotedPawn]   - PieceValueTable[kPawn],
  PieceValueTable[kPromotedLance]  - PieceValueTable[kLance],
  PieceValueTable[kPromotedKnight] - PieceValueTable[kKnight],
  PieceValueTable[kPromotedSilver] - PieceValueTable[kSilver],
  PieceValueTable[kHorse]          - PieceValueTable[kBishop],
  PieceValueTable[kDragon]         - PieceValueTable[kRook]
};

constexpr int
ExchangePieceValueTable[kPieceTypeMax] =
{
  0,
  PieceValueTable[kPawn] * 2,
  PieceValueTable[kLance] * 2,
  PieceValueTable[kKnight] * 2,
  PieceValueTable[kSilver] * 2,
  PieceValueTable[kBishop] * 2,
  PieceValueTable[kRook] * 2,
  PieceValueTable[kGold] * 2,
  0,
  PieceValueTable[kPromotedPawn] + PieceValueTable[kPawn],
  PieceValueTable[kPromotedLance] + PieceValueTable[kLance],
  PieceValueTable[kPromotedKnight] + PieceValueTable[kKnight],
  PieceValueTable[kPromotedSilver] + PieceValueTable[kSilver],
  PieceValueTable[kHorse] + PieceValueTable[kBishop],
  PieceValueTable[kDragon] + PieceValueTable[kRook]
};

constexpr Value
kTempo = Value(80);

constexpr int
kListNum = 38;

constexpr int
kFvScale = 32;

inline Square
inverse(Square sq)
{
  return static_cast<Square>(kBoardSquare - 1 - sq);
}

extern bool
init();

extern Value 
evaluate(const Position &pos, SearchStack *ss);

#ifdef TWIG
typedef std::array<int16_t, 2> ValueKpp;
typedef std::array<int32_t, 2> ValueKkp;
typedef std::array<int32_t, 2> ValueKk;
extern ValueKpp KPP[kBoardSquare][kFEEnd][kFEEnd];
extern ValueKkp KKP[kBoardSquare][kBoardSquare][kFEEnd];
extern ValueKk KK[kBoardSquare][kBoardSquare];
#elif Apery
extern int16_t KPP[kBoardSquare][kFEEnd][kFEEnd];
extern int32_t KKP[kBoardSquare][kBoardSquare][kFEEnd];
extern int32_t KK[kBoardSquare][kBoardSquare];
#else
extern int16_t KPP[kBoardSquare][kFEEnd][kFEEnd];
extern int16_t KKP[kBoardSquare][kBoardSquare][kFEEnd];
#endif

} // namespace Eval

#ifdef TWIG
#include <array>
#include <fstream>
#include <string>
using namespace std;

template <typename Tl, typename Tr>
inline std::array<Tl, 2> operator += (std::array<Tl, 2>& lhs, const std::array<Tr, 2>& rhs) {
  lhs[0] += rhs[0];
  lhs[1] += rhs[1];
  return lhs;
}
template <typename Tl, typename Tr>
inline std::array<Tl, 2> operator -= (std::array<Tl, 2>& lhs, const std::array<Tr, 2>& rhs) {
  lhs[0] -= rhs[0];
  lhs[1] -= rhs[1];
  return lhs;
}

struct EvalSum {
#if defined USE_AVX2_EVAL
  EvalSum(const EvalSum& es) {
    _mm256_store_si256(&mm, es.mm);
  }
  EvalSum& operator = (const EvalSum& rhs) {
    _mm256_store_si256(&mm, rhs.mm);
    return *this;
  }
#elif defined USE_SSE_EVAL
  EvalSum(const EvalSum& es) {
    _mm_store_si128(&m[0], es.m[0]);
    _mm_store_si128(&m[1], es.m[1]);
  }
  EvalSum& operator = (const EvalSum& rhs) {
    _mm_store_si128(&m[0], rhs.m[0]);
    _mm_store_si128(&m[1], rhs.m[1]);
    return *this;
  }
#endif
  EvalSum() {}
  int32_t sum(const Color c) const {
    const int32_t scoreBoard = p[0][0] - p[1][0] + p[2][0];
    const int32_t scoreTurn = p[0][1] + p[1][1] + p[2][1];
    return (c == kBlack ? scoreBoard : -scoreBoard) + scoreTurn;
  }
  EvalSum& operator += (const EvalSum& rhs) {
#if defined USE_AVX2_EVAL
    mm = _mm256_add_epi32(mm, rhs.mm);
#elif defined USE_SSE_EVAL
    m[0] = _mm_add_epi32(m[0], rhs.m[0]);
    m[1] = _mm_add_epi32(m[1], rhs.m[1]);
#else
    p[0][0] += rhs.p[0][0];
    p[0][1] += rhs.p[0][1];
    p[1][0] += rhs.p[1][0];
    p[1][1] += rhs.p[1][1];
    p[2][0] += rhs.p[2][0];
    p[2][1] += rhs.p[2][1];
#endif
    return *this;
  }
  EvalSum& operator -= (const EvalSum& rhs) {
#if defined USE_AVX2_EVAL
    mm = _mm256_sub_epi32(mm, rhs.mm);
#elif defined USE_SSE_EVAL
    m[0] = _mm_sub_epi32(m[0], rhs.m[0]);
    m[1] = _mm_sub_epi32(m[1], rhs.m[1]);
#else
    p[0][0] -= rhs.p[0][0];
    p[0][1] -= rhs.p[0][1];
    p[1][0] -= rhs.p[1][0];
    p[1][1] -= rhs.p[1][1];
    p[2][0] -= rhs.p[2][0];
    p[2][1] -= rhs.p[2][1];
#endif
    return *this;
  }
  EvalSum operator + (const EvalSum& rhs) const { return EvalSum(*this) += rhs; }
  EvalSum operator - (const EvalSum& rhs) const { return EvalSum(*this) -= rhs; }

#if 0
  // ehash 用。
  void encode() {
#if defined USE_AVX2_EVAL
    // EvalSum は atomic にコピーされるので key が合っていればデータも合っている。
#else
    key ^= data[0] ^ data[1] ^ data[2];
#endif
  }
  void decode() { encode(); }
#endif
#if 1
  union {
    std::array<std::array<int32_t, 2>, 3> p;
    struct {
      uint64_t data[3];
      uint64_t key; // ehash用。
    };
#if defined USE_AVX2_EVAL
    __m256i mm;
#endif
#if defined USE_AVX2_EVAL || defined USE_SSE_EVAL
    __m128i m[2];
#endif
  };
#endif
};
#endif

#endif
