#pragma once

#include <memory>
#include <string>

namespace mal {

  template<typename L>
  class Left {
    private:
      const L _value;

    public:
      Left(const L value): _value(value) {};
      const L& value() const { return _value; }
  };

  template<typename R>
  class Right {
    private:
      const R _value;

    public:
      Right(const R value): _value(value) {};
      const R& value() const { return _value; }
  };

  template<typename L, typename R>
  class Either {
    static_assert(!std::is_pointer<L>::value, "Left type must not be a raw pointer");
    static_assert(!std::is_pointer<R>::value, "Right type must not be a raw pointer");

    private:
      const bool _isRight;

    public:
      union {
        /// The left value
        L left;

        /// The right value
        R right;
      };

      /// Constructs this either from a left
      Either(const Left<L> left): _isRight(false), left(left.value()) {};

      /// Constructs this either from a right
      Either(const Right<R> right): _isRight(true), right(right.value()) {};

      Either(const Either &other): _isRight(other._isRight) {
        if (_isRight) {
          right = other.right;
        } else {
          left = other.left;
        }
      };

      ~Either() {};

      /// Returns true if this either is a right
      operator bool() const { return _isRight; }
  };
}
