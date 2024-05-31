#ifndef MOVETABLES_H
#define MOVETABLES_H

#include <array>

#include "bitboard.h"
#include "board.h"

namespace Dagor::MoveTables {

/// @brief The attacks a pawn can make on a given square.
/// Access: `pawnAttacks[color][square]`, where white is `0` and black is `1`.
extern const BitBoard::BitBoard pawnAttacks[2][Board::size];

/// @brief The moves a knight can make on a given square.
extern const BitBoard::BitBoard knightMoves[Board::size];

/// @brief The moves a king can make on a given square. For his home square
/// this does not include castling moves.
extern const BitBoard::BitBoard kingMoves[Board::size];

/// @brief The move that a sliding piece (bishop, rook or queen) can
/// make on a given square. Access through the hash functions in
/// `bishopHashes` and `rookHashes`.
extern const BitBoard::BitBoard slidingMoves[];

/// @brief A hash function that maps a configuration of blocking
/// pieces to an index into the `slidingMoves` table, where the
/// possible moves of a rook or bishop are stored.
/// Both rooks and bishops have one separate hash function for each square.
class BlockerHash {
 public:
  /// @brief The mask singling out the blocking pieces that actually matter
  /// to the figure under consideration.
  const BitBoard::BitBoard blockerMask;
  /// @brief The magic number that yields the perfect hash function.
  const BitBoard::BitBoard magic;
  /// @brief The amount by which the hash should be shifted down.
  const unsigned downShift;
  /// @brief The offset that should be added to the hash. In `slidingMoves` all
  /// entries lie consecutively, this marks where the entries begin, that can be
  /// accessed through this hash function.
  const unsigned tableOffset;

  BlockerHash(BitBoard::BitBoard mask, BitBoard::BitBoard magic,
              unsigned downShift, unsigned tableOffset)
      : blockerMask{mask},
        magic{magic},
        downShift{downShift},
        tableOffset{tableOffset} {}

  /// @brief Computes the hash for a configuration of blocking pieces.
  /// @param blockers pieces blocking the bishop’s/rook’s movement.
  /// @return the hash.
  unsigned hash(BitBoard::BitBoard blockers) const {
    blockers &= blockerMask;
    std::uint64_t h = blockers.as_uint() * magic.as_uint();
    return static_cast<unsigned>(h >> downShift) + tableOffset;
  }

  /// @brief Looks up the possible moves for a bishop/rook with the
  /// specified blocking pieces.
  /// @param blockers pieces blocking the bishop’s/rook’s movement.
  /// @return a bitboard where all squares, to which the bishop/rook can move,
  /// are set.
  BitBoard::BitBoard lookUp(BitBoard::BitBoard blockers) const {
    return slidingMoves[hash(blockers)];
  }
};

/// @brief the hash functions to look up bishop moves, by square.
extern const BlockerHash bishopHashes[Board::size];
/// @brief the hash function to look up rook moves, by square.
extern const BlockerHash rookHashes[Board::size];
}  // namespace Dagor::MoveTables

#endif