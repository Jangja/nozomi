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

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstring>

#include "evaluate.h"
#include "position.h"
#include "search.h"
#include "misc.h"

namespace Eval
{
#define KKP_BIN "KKP_synthesized.bin"
#define KPP_BIN "KPP_synthesized.bin"
#define KK_BIN "KK_synthesized.bin"
ValueKpp KPP[kBoardSquare][kFEEnd][kFEEnd];
ValueKkp KKP[kBoardSquare][kBoardSquare][kFEEnd];
ValueKk KK[kBoardSquare][kBoardSquare];

Value
calc_full(const Position &pos, SearchStack *ss)
{
  int *list_black = pos.black_kpp_list();
  int *list_white = pos.white_kpp_list();

  const Square sq_bk     = conv_sq(pos.square_king(kBlack));
  const Square sq_wk     = conv_sq(pos.square_king(kWhite));
  const auto* ppkppb = KPP[sq_bk];
  const auto* ppkppw = KPP[inverse(sq_wk)];

  EvalSum sum;
  sum.p[2] = KK[sq_bk][sq_wk];
#if defined USE_AVX2_EVAL || defined USE_SSE_EVAL
  sum.m[0] = _mm_setzero_si128();
  for (int i = 0; i < kListNum; ++i) {
    const int k0 = list_black[i];
    const int k1 = list_white[i];
    const auto* pkppb = ppkppb[k0];
    const auto* pkppw = ppkppw[k1];
    for (int j = 0; j < i; ++j) {
      const int l0 = list_black[j];
      const int l1 = list_white[j];
      __m128i tmp;
      tmp = _mm_set_epi32(0, 0, *reinterpret_cast<const int*>(&pkppw[l1][0]), *reinterpret_cast<const int*>(&pkppb[l0][0]));
      tmp = _mm_cvtepi16_epi32(tmp);
      sum.m[0] = _mm_add_epi32(sum.m[0], tmp);
    }
    sum.p[2] += KKP[sq_bk][sq_wk][k0];
  }
#else
  sum.p[0][0] = 0;
  sum.p[0][1] = 0;
  sum.p[1][0] = 0;
  sum.p[1][1] = 0;

  for (int i = 0; i < kListNum; ++i)
  {
    const int k0 = list_black[i];
    const int k1 = list_white[i];
    const auto* pkppb = ppkppb[k0];
    const auto* pkppw = ppkppw[k1];
    for (int j = 0; j < i; ++j)
    {
      const int l0 = list_black[j];
      const int l1 = list_white[j];
      sum.p[0] += pkppb[l0];
      sum.p[1] += pkppw[l1];
    }
    sum.p[2] += KKP[sq_bk][sq_wk][k0];
  }
#endif

  ss->material = static_cast<Value>(pos.material() * kFvScale);
  sum.p[2][0] += ss->material;
  ss->staticEvalRaw = sum;

  return static_cast<Value>(sum.sum(pos.side_to_move()) / kFvScale);
}

Value
calc_no_capture_difference(const Position &pos, SearchStack *ss)
{
  Square    black_king        = conv_sq(pos.square_king(kBlack));
  Square    white_king        = conv_sq(pos.square_king(kWhite));
  Square    inv_white_king    = inverse(white_king);

  const int *prev_list_black   = pos.prev_black_kpp_list();
  const int *prev_list_white   = pos.prev_white_kpp_list();
  const int *list_black        = pos.black_kpp_list();
  const int *list_white        = pos.white_kpp_list();
  const int index_move = pos.list_index_move();

  assert(pos.list_index_move() < 38);

  EvalSum diff;
  diff.p[0][0] = 0;
  diff.p[0][1] = 0;
  diff.p[1][0] = 0;
  diff.p[1][1] = 0;
  diff.p[2][0] = 0;
  diff.p[2][1] = 0;
  const auto *black_prev_kpp_table = KPP[black_king][prev_list_black[index_move]];
  const auto *black_kpp_table      = KPP[black_king][list_black[index_move]];
  const auto *white_prev_kpp_table = KPP[inv_white_king][prev_list_white[index_move]];
  const auto *white_kpp_table      = KPP[inv_white_king][list_white[index_move]];
  for (int i = 0; i < kListNum; ++i)
  {
    // 前回のを引く
    diff.p[0] -= black_prev_kpp_table[prev_list_black[i]];
    // 今回のを足す
    diff.p[0] += black_kpp_table[list_black[i]];

    // 前回のを引く
    diff.p[1] -= white_prev_kpp_table[prev_list_white[i]];
    // 今回のを足す
    diff.p[1] += white_kpp_table[list_white[i]];
  }
  // 前回のを引く
  diff.p[2] -= KKP[black_king][white_king][prev_list_black[index_move]];
  // 今回のを足す
  diff.p[2] += KKP[black_king][white_king][list_black[index_move]];

  ss->material = static_cast<Value>(pos.material() * kFvScale);
  diff.p[2][0] += ss->material - (ss - 1)->material;
  diff += (ss - 1)->staticEvalRaw;
  ss->staticEvalRaw = diff;

  return static_cast<Value>(diff.sum(pos.side_to_move()) / kFvScale);
}

Value
calc_difference_capture(const Position &pos, SearchStack *ss)
{
  Square    black_king        = conv_sq(pos.square_king(kBlack));
  Square    white_king        = conv_sq(pos.square_king(kWhite));
  Square    inv_white_king    = inverse(white_king);

  const int *prev_list_black   = pos.prev_black_kpp_list();
  const int *prev_list_white   = pos.prev_white_kpp_list();
  const int *list_black        = pos.black_kpp_list();
  const int *list_white        = pos.white_kpp_list();
  const int index_capture = pos.list_index_capture();
  const int index_move = pos.list_index_move();

  assert(pos.list_index_capture() < 38);
  assert(pos.list_index_move() < 38);

  EvalSum diff;
  diff.p[0][0] = 0;
  diff.p[0][1] = 0;
  diff.p[1][0] = 0;
  diff.p[1][1] = 0;
  diff.p[2][0] = 0;
  diff.p[2][1] = 0;
  const auto *black_prev_kpp_table     = KPP[black_king][prev_list_black[index_move]];
  const auto *black_prev_cap_kpp_table = KPP[black_king][prev_list_black[index_capture]];
  const auto *black_kpp_table          = KPP[black_king][list_black[index_move]];
  const auto *black_cap_kpp_table      = KPP[black_king][list_black[index_capture]];
  const auto *white_prev_kpp_table     = KPP[inv_white_king][prev_list_white[index_move]];
  const auto *white_prev_cap_kpp_table = KPP[inv_white_king][prev_list_white[index_capture]];
  const auto *white_kpp_table          = KPP[inv_white_king][list_white[index_move]];
  const auto *white_cap_kpp_table      = KPP[inv_white_king][list_white[index_capture]];

  for (int i = 0; i < kListNum; ++i)
  {
    // 前回のを引く
    diff.p[0] -= black_prev_kpp_table[prev_list_black[i]];
    // とった分も引く
    diff.p[0] -= black_prev_cap_kpp_table[prev_list_black[i]];
    // 今回のを足す
    diff.p[0] += black_kpp_table[list_black[i]];
    diff.p[0] += black_cap_kpp_table[list_black[i]];

    // 前回のを引く
    diff.p[1] -= white_prev_kpp_table[prev_list_white[i]];
    // とった分も引く
    diff.p[1] -= white_prev_cap_kpp_table[prev_list_white[i]];

    // 今回のを足す
    diff.p[1] += white_kpp_table[list_white[i]];
    diff.p[1] += white_cap_kpp_table[list_white[i]];
  }
  // 前回ので引きすぎたのを足す
  diff.p[0] += black_prev_kpp_table[prev_list_black[index_capture]];
  // 今回ので足しすぎたのを引く
  diff.p[0] -= black_kpp_table[list_black[index_capture]];

  // 前回ので引きすぎたのを足す
  diff.p[1] += white_prev_kpp_table[prev_list_white[index_capture]];
  // 今回ので足しすぎたのを引く
  diff.p[1] -= white_kpp_table[list_white[index_capture]];

  diff.p[2] -= KKP[black_king][white_king][prev_list_black[index_move]];
  diff.p[2] -= KKP[black_king][white_king][prev_list_black[index_capture]];
  diff.p[2] += KKP[black_king][white_king][list_black[index_move]];
  diff.p[2] += KKP[black_king][white_king][list_black[index_capture]];

  ss->material = static_cast<Value>(pos.material() * kFvScale);
  diff.p[2][0] += ss->material - (ss - 1)->material;
  diff += (ss - 1)->staticEvalRaw;
  ss->staticEvalRaw = diff;

  return static_cast<Value>(diff.sum(pos.side_to_move()) / kFvScale);
}

template<Color kColor>
Value
calc_difference_king_move_no_capture(const Position &pos, SearchStack *ss)
{
  const int *list_black = pos.black_kpp_list();
  const int *list_white = pos.white_kpp_list();
  const Square sq_black_king = conv_sq(pos.square_king(kBlack));
  const Square sq_white_king = conv_sq(pos.square_king(kWhite));
  Square inv_sq_white_king   = inverse(sq_white_king);

  EvalSum sum;
  sum.p[2] = KK[sq_black_king][sq_white_king];
  sum.p[0][0] = 0;
  sum.p[0][1] = 0;
  sum.p[1][0] = 0;
  sum.p[1][1] = 0;
  const auto *kkp_table = KKP[sq_black_king][sq_white_king];

  if (kColor == kBlack)
  {
    const auto *black_kpp_table = KPP[sq_black_king];
    for (int i = 0; i < kListNum; ++i)
    {
      const int k0 = list_black[i];
      const auto *black_pp_table = black_kpp_table[k0];
      for (int j = 0; j < i; ++j)
      {
        const int l0 = list_black[j];
        sum.p[0] += black_pp_table[l0];
      }
      sum.p[2] += KKP[sq_black_king][sq_white_king][k0];
    }
    sum.p[1] = (ss - 1)->staticEvalRaw.p[1];
  }
  else
  {
    const auto *white_kpp_table = KPP[inv_sq_white_king];
    for (int i = 0; i < kListNum; ++i)
    {
      const int k0 = list_black[i];
      const int k1 = list_white[i];
      const auto *white_pp_table = white_kpp_table[k1];
      for (int j = 0; j < i; ++j)
      {
        const int l1 = list_white[j];
        sum.p[1] += white_pp_table[l1];
      }
      sum.p[2] += KKP[sq_black_king][sq_white_king][k0];
    }
    sum.p[0] = (ss - 1)->staticEvalRaw.p[0];
  }

  ss->material = static_cast<Value>(pos.material() * kFvScale);
  sum.p[2][0] += ss->material;
  ss->staticEvalRaw = sum;

  return static_cast<Value>(sum.sum(pos.side_to_move()) / kFvScale);
}

Value
calc_difference(const Position &pos, Move last_move, SearchStack *ss)
{
  const Square    from = move_from(last_move);
  const PieceType type = move_piece_type(last_move);

  if (type == kKing)
  {
    if (pos.side_to_move() == kBlack)
      return calc_difference_king_move_no_capture<kWhite>(pos, ss);
    else
      return calc_difference_king_move_no_capture<kBlack>(pos, ss);
  }
  else
  {
    if (from >= kBoardSquare)
    {
      return calc_no_capture_difference(pos, ss);
    }
    else
    {
      const PieceType capture = move_capture(last_move);

      if (capture == kPieceNone)
        return calc_no_capture_difference(pos, ss);
      else
        return calc_difference_capture(pos, ss);
    }
  }
}


Value
evaluate(const Position &pos, SearchStack *ss)
{
  Value score;

  Move last_move = (ss - 1)->current_move;
  if ((ss - 1)->evaluated && !(move_piece_type(last_move) == kKing && move_is_capture(last_move)) && is_ok(last_move))
  {
    score = calc_difference(pos, last_move, ss);

    assert(calc_full(pos, ss) == score);
    ss->evaluated = true;

    assert(score > -kValueInfinite && score < kValueInfinite);
  }
  else
  {
    score = calc_full(pos, ss);
    ss->evaluated = true;

    assert(score > -kValueInfinite && score < kValueInfinite);
  }

  return score + kTempo;
}
  
bool
init() 
{
  do {
    // KK
    std::ifstream ifsKK(KK_BIN, std::ios::binary);
    if (ifsKK) ifsKK.read(reinterpret_cast<char*>(KK), sizeof(KK));
    else goto Error;

    // KKP
    std::ifstream ifsKKP(KKP_BIN, std::ios::binary);
    if (ifsKKP) ifsKKP.read(reinterpret_cast<char*>(KKP), sizeof(KKP));
    else goto Error;

    // KPP
    std::ifstream ifsKPP(KPP_BIN, std::ios::binary);
    if (ifsKPP) ifsKPP.read(reinterpret_cast<char*>(KPP), sizeof(KPP));
    else goto Error;

  } while (0);

  return true;

Error:;
  std::cout << "\ninfo string open evaluation file failed.\n";
  //    cout << "\nERROR open evaluation file failed.\n";
  // 評価関数ファイルの読み込みに失敗した場合、思考を開始しないように抑制したほうがいいと思う。

}
} // namespace Eval
