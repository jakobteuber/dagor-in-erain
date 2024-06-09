
#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
#include <iterator>
#include <ostream>

#include "types.h"

namespace Dagor::BitBoards {

static_assert(sizeof(std::uint64_t) == 8,
              "For its BitBoards, this program assumes 64 bit integers.");

/// @brief BitBoards represent some subset of the chess boards pieces,
/// such as the set of all fields occupied by whit pawns or the
/// set of all pieces to which a given piece can move on it’s next
/// turn etc.
class BitBoard {
 private:
  std::uint64_t board;

 public:
  /// @brief constructs an empty BitBoard.
  BitBoard() : board{0} {}
  /// @brief
  /// @param bitboard a uint64 as returned by the `as_uint` function.
  BitBoard(std::uint64_t bitboard) : board{bitboard} {}

  /// @brief
  /// @return a uint64 where all the 1 bits indicate the set squares
  constexpr std::uint64_t as_uint() const { return board; }

  /// @brief removes all the squares that are not also present in `other`.
  /// @param other
  /// @return this bitboard after the modification
  BitBoard &operator&=(BitBoard other) {
    board &= other.board;
    return *this;
  }

  /// @brief adds all the squares of the `other` bitboard to this one.
  /// @param other
  /// @return this bitboard after the modification
  BitBoard &operator|=(BitBoard other) {
    board |= other.board;
    return *this;
  }

  /// @brief checks whether the bitboard is empty, that is whether no squares
  /// are set.
  /// @return `true`, iff no squares are set.
  constexpr bool is_empty() const { return board == 0; }

  /// @brief Checks whether a particular square is set.
  /// @param square the square to check.
  /// @return `true`, iff the square is set.
  constexpr bool is_set(Square::t square) const {
    return board & (1ULL << square);
  }

  /// @brief Adds the given square to the bitboard.
  /// @param square the square to add.
  void set_bit(Square::t square) { board |= (1ULL << square); }

  /// Adds the given square to the bitboard, if the coordinates are
  /// valid on a chess board, that is, if `file, rank are from {0,...,7}`. If
  /// this is not the case, nothing happens. This function exists to protect
  /// against warping around the edges of the board when calculating moves etc.
  /// @param file the file (i. e. column) of the square to add.
  /// @param rank the rank (i. e. row) of the square to add.
  void set_bit_if_index_valid(Coord::t file, Coord::t rank) {
    if (Coord::inRange(file) && Coord::inRange(rank)) {
      set_bit(Square::index(file, rank));
    }
  }

  /// @brief Removes a given square from the bitboard.
  /// @param square the square to remove.
  void unset_bit(Square::t square) { board &= ~(1ULL << square); }

  /// @brief Counts the number of set squares in the bitboard.
  /// @return the number of set squares in the bitboard.
  constexpr int popcount() const { return __builtin_popcountll(board); }

  /// @brief Finds the index of the first set square in the bitboard.
  /// Do not call this function for the empty bitboard.
  /// @return the index of the first set square.
  constexpr Square::t findFirstSet() const {
    return static_cast<Square::t>(__builtin_ctzll(board));
  }
  class Iterator {
   private:
    std::uint64_t board;

   public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Square::t;
    using pointer = value_type *;
    using reference = value_type &;

    Iterator(std::uint64_t board) : board{board} {}

    value_type operator*() const { return __builtin_ctzll(board); }

    Iterator &operator++() {
      value_type index = operator*();
      // In C++, you cannot shift by the word size or greater, so we need to do
      // this explicitly. Hopefully this gets optimized away to a normal
      // x68 shift instruction.
      if (index >= 63) {
        board = 0;
      } else {
        std::uint64_t upper = 0xffffffffffffffff << (index + 1);
        board &= upper;
      }
      return *this;
    }

    Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const Iterator &other) const {
      return board == other.board;
    }

    bool operator!=(const Iterator &other) const { return !(*this == other); }
  };

  using const_iterator = Iterator;
  Iterator begin() { return {as_uint()}; }
  Iterator end() { return {0}; }
};

inline BitBoard operator&(BitBoard a, BitBoard b) { return a &= b; }
inline BitBoard operator|(BitBoard a, BitBoard b) { return a |= b; }
inline BitBoard operator~(BitBoard a) { return BitBoard(~a.as_uint()); }

inline bool operator==(BitBoard a, BitBoard b) {
  return a.as_uint() == b.as_uint();
}
inline bool operator!=(BitBoard a, BitBoard b) {
  return a.as_uint() != b.as_uint();
}

std::ostream &operator<<(std::ostream &out, const BitBoard &printer);

/// @brief Constructs a bitboard with only a single square set.
/// @param square the square to be set
/// @return the bitboard
inline BitBoard single(Square::t square) { return {1ULL << square}; }

inline BitBoard wholeFile(Coord::t file) {
  std::uint64_t a_file{0x101010101010101};
  return {a_file << file};
}

inline BitBoard wholeRank(Coord::t rank) {
  std::uint64_t base_rank{0xff};
  return {base_rank << (rank * Coord::width)};
}

inline BitBoard rightOf(Coord::t file) {
  switch (file) {
    case 0:
      return {0xfefefefefefefefe};
    case 1:
      return {0xfcfcfcfcfcfcfcfc};
    case 2:
      return {0xf8f8f8f8f8f8f8f8};
    case 3:
      return {0xf0f0f0f0f0f0f0f0};
    case 4:
      return {0xe0e0e0e0e0e0e0e0};
    case 5:
      return {0xc0c0c0c0c0c0c0c0};
    case 6:
      return {0x8080808080808080};
    case 7:
      return {0};
    default:
      return {0};
  }
}

inline BitBoard leftOf(Coord::t file) { return ~rightOf(file - 1); }

inline BitBoard above(Coord::t rank) {
  std::uint64_t all{0xffffffffffffffff};
  return {all << ((rank + 1) * Coord::width)};
}

inline BitBoard below(Coord::t rank) {
  std::uint64_t all{0xffffffffffffffff};
  return {all >> ((rank - 1) * Coord::width)};
}

/// @brief A bitboard containing all squares adjacent to one of the edges of
/// the board.
///
///     8 | @ @ @ @ @ @ @ @
///     7 | @ . . . . . . @
///     6 | @ . . . . . . @
///     5 | @ . . . . . . @
///     4 | @ . . . . . . @
///     3 | @ . . . . . . @
///     2 | @ . . . . . . @
///     1 | @ @ @ @ @ @ @ @
///         ----------------     as decimal: 18411139144890810879
///         a b c d e f g h      as hex:     0xff818181818181ff
inline const BitBoard edgesOnly{0xff818181818181ff};

inline const BitBoard all{0xffffffffffffffff};

}  // namespace Dagor::BitBoards

#endif