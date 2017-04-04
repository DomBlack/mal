#pragma once

#include <memory>
#include <string>
#include <functional>

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

      Left<L>  asLeft()  const { return Left<L>(left); };
      Right<R> asRight() const { return Right<R>(right); };

      ~Either() {};

      /*
       * Example Uses
       *
       * mal::Either<std::string, int> test1 = mal::Right<int>(123);
       * mal::Either<std::string, int> test2 = mal::Right<int>(100);
       *
       * mal::Either<std::string, int> result = test1.flatMap<int>(
       *     [&test2](int a) { return test2.map<int>([&a](int b) -> int { return a - b; }); }
       * );
       *
       * auto zipTest = test1.zip(test2).map<int>([](std::tuple<int, int> t) { return std::get<0>(t) - std::get<1>(t); });
       *
       * std::cout << "flatMap: " << result.right << " zipTest: " << zipTest.right << std::endl;
       */

      /// Zips this either with another
      template <typename B>
      Either<L, std::tuple<R, B>> zip(const Either<L, B> &other) const {
        if (_isRight) {
          if (other._isRight) {
            return Right<std::tuple<R, B>>(std::make_tuple(right, other.right));
          } else {
            return other.asLeft();
          }
        } else {
          return this->asLeft();
        }
      };


      /// Maps this either into the other type
      template <typename B>
      Either<L, B> map(std::function<B(const R &)> fn) const {
        if (_isRight) {
          return Right<B>(fn(right));
        } else {
          return this->asLeft();
        }
      };

      /// Flatmaps this either with the other type
      template <typename B>
      Either<L, B> flatMap(std::function<Either<L, B>(const R &)> fn) const {
        if (_isRight) {
          return fn(right);
        } else {
          return this->asLeft();
        }
      };

      /// Returns true if this either is a right
      operator bool() const { return _isRight; }
  };
}
