#include "eval.h"

#include <array>

#include "types.h"

namespace Dagor::Eval {

constexpr std::array<std::int8_t, Square::size * Piece::all.size()>
    opening_table = {
        /* Pawns */
        0, 0, 0, 0, 0, 0, 0, 0,          //
        50, 50, 50, 50, 50, 50, 50, 50,  //
        10, 10, 20, 30, 30, 20, 10, 10,  //
        5, 5, 10, 25, 25, 10, 5, 5,      //
        0, 0, 0, 20, 20, 0, 0, 0,        //
        5, -5, -10, 0, 0, -10, -5, 5,    //
        5, 10, 10, -20, -20, 10, 10, 5,  //
        0, 0, 0, 0, 0, 0, 0, 0,          //

        /* Knights */
        -50, -40, -30, -30, -30, -30, -40, -50,  //
        -40, -20, 0, 0, 0, 0, -20, -40,          //
        -30, 0, 10, 15, 15, 10, 0, -30,          //
        -30, 5, 15, 20, 20, 15, 5, -30,          //
        -30, 0, 15, 20, 20, 15, 0, -30,          //
        -30, 5, 10, 15, 15, 10, 5, -30,          //
        -40, -20, 0, 5, 5, 0, -20, -40,          //
        -50, -40, -30, -30, -30, -30, -40, -50,  //

        /* Bishops */
        -20, -10, -10, -10, -10, -10, -10, -20,  //
        -10, 0, 0, 0, 0, 0, 0, -10,              //
        -10, 0, 5, 10, 10, 5, 0, -10,            //
        -10, 5, 5, 10, 10, 5, 5, -10,            //
        -10, 0, 10, 10, 10, 10, 0, -10,          //
        -10, 10, 10, 10, 10, 10, 10, -10,        //
        -10, 5, 0, 0, 0, 0, 5, -10,              //
        -20, -10, -10, -10, -10, -10, -10, -20,  //

        /* Rooks */
        0, 0, 0, 0, 0, 0, 0, 0,        //
        5, 10, 10, 10, 10, 10, 10, 5,  //
        -5, 0, 0, 0, 0, 0, 0, -5,      //
        -5, 0, 0, 0, 0, 0, 0, -5,      //
        -5, 0, 0, 0, 0, 0, 0, -5,      //
        -5, 0, 0, 0, 0, 0, 0, -5,      //
        -5, 0, 0, 0, 0, 0, 0, -5,      //
        0, 0, 0, 5, 5, 0, 0, 0,        //

        /* Queen */
        -20, -10, -10, -5, -5, -10, -10, -20,  //
        -10, 0, 0, 0, 0, 0, 0, -10,            //
        -10, 0, 5, 5, 5, 5, 0, -10,            //
        -5, 0, 5, 5, 5, 5, 0, -5,              //
        0, 0, 5, 5, 5, 5, 0, -5,               //
        -10, 5, 5, 5, 5, 5, 0, -10,            //
        -10, 0, 5, 0, 0, 0, 0, -10,            //
        -20, -10, -10, -5, -5, -10, -10, -20,  //

        /* King */
        -30, -40, -40, -50, -50, -40, -40, -30,  //
        -30, -40, -40, -50, -50, -40, -40, -30,  //
        -30, -40, -40, -50, -50, -40, -40, -30,  //
        -30, -40, -40, -50, -50, -40, -40, -30,  //
        -20, -30, -30, -40, -40, -30, -30, -20,  //
        -10, -20, -20, -20, -20, -20, -20, -10,  //
        20, 20, 0, 0, 0, 0, 20, 20,              //
        20, 30, 10, 0, 0, 10, 30, 20,            //
};

int eval(const GameState& state) {
  int result = 0;

  for (Piece::t piece : Piece::nonKing) {
    BitBoards::BitBoard ourPieces = state.forPiece(piece, state.us());
    BitBoards::BitBoard opponentPieces = state.forPiece(piece, state.them());

    /* Material */
    int diff = ourPieces.populationCount() - opponentPieces.populationCount();
    result += diff * Piece::worth[piece];

    /* Positions */
    for (Square::t square : ourPieces) {
      Square::t forWhite = Square::reverseForColor(square, state.us());
      result += opening_table[forWhite + piece * Square::size];
    }
  }

  if (state.uneventfulHalfMoves >= 50) {
    return 0;
  }

  return result;
}

}  // namespace Dagor::Eval