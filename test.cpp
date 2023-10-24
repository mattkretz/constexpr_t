/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include <constexpr_wrapper.hpp>
#include <array>

#if defined __clang_major__ and __clang_major__ <= 16
  // Clang 16 ICEs saying "error: cannot compile this l-value expression yet"
#define BAD_COMPILER 1
#endif

static_assert(std::cw<1> == 1);
#if __cpp_nontype_template_args >= 201911L
static_assert(std::cw<1.f> == 1.f);
#endif

static_assert(std::constexpr_value<std::constexpr_wrapper<1>>);

template <auto _Xp>
  struct Derived
  : std::constexpr_wrapper<_Xp>
  {};

struct Test
{
  int value = 1;

  constexpr int
  operator()(int a, int b) const
  { return a + b + value; }

  constexpr int
  operator[](auto... args) const
  { return (value + ... + args); }

  constexpr bool
  operator==(const Test&) const = default;
};

template <typename Char, std::size_t N>
  struct strlit
  {
    static inline constexpr std::integral_constant<std::size_t, N - 1> size{};

    Char value[size];

    template <std::size_t... Is>
      constexpr
      strlit(const Char* c, std::index_sequence<Is...>)
      : value{ c[Is]... }
      {}

    constexpr
    strlit(const Char (&c)[N])
    : strlit(c, std::make_index_sequence<size>())
    {}

    constexpr Char
    operator[](std::size_t i) const
    { return value[i]; }

    constexpr bool
    operator==(const strlit&) const = default;

    constexpr auto
    operator<=>(const strlit&) const = default;

    template <std::size_t M>
      friend constexpr auto
      operator+(strlit l, strlit<Char, M> r)
      {
        return strlit<Char, N + M - 1>(
                 [&]<std::size_t... Is>(std::index_sequence<Is...>)
                   -> std::array<Char, N + M - 2> {
                     return {(Is < N - 1 ? l.value[Is] : r.value[Is - N + 1])...};
                   }(std::make_index_sequence<N + M - 2>()).data(),
                 std::make_index_sequence<N + M - 2>());
      }

    template <std::size_t M>
      friend constexpr auto
      operator+(strlit l, const Char (&r)[M])
      { return l + strlit(r); }

    template <std::size_t M>
      friend constexpr auto
      operator+(const Char (&l)[M], strlit r)
      { return strlit(l) + r; }
  };

// string constant
template <strlit chars>
  inline constexpr std::constexpr_wrapper<chars>
  operator"" _sc()
  { return {}; }

struct NeedsAdl
{
  int value = 0;

  constexpr
  NeedsAdl(int x)
  : value(x)
  {}

  friend constexpr int
  operator+(NeedsAdl a, NeedsAdl b)
  { return a.value + b.value; }
};

template <auto Expected, std::constexpr_value C>
  void
  check(C x)
  {
    static_assert(std::same_as<C, std::constexpr_wrapper<Expected>>);
    static_assert(C::value == Expected);
    static_assert(x == Expected);
    static_assert(x.value == Expected);
  }

template <typename Expected, typename C>
  void
  check(C)
  { static_assert(std::same_as<C, Expected>); }

struct Aaaargh
{
  constexpr int
  operator=(int x) const
  { return x; }

  constexpr int
  operator+=(int x) const
  { return x + 1; }

  constexpr int
  operator-=(int x) const
  { return x - 1; }

  constexpr int
  operator++() const
  { return 1; }

  constexpr int
  operator++(int) const
  { return 2; }

  constexpr int
  operator--() const
  { return 3; }

  constexpr int
  operator--(int) const
  { return 4; }

  constexpr int
  operator->*(int x) const
  { return x + 5; }

  constexpr int
  foo() const
  { return 5; }
};

template <typename T, int N>
struct numarray
{
  using V = T[N];
  V value;

  static constexpr std::integral_constant<std::size_t, N> size{};

  constexpr const T&
  operator[](std::size_t i) const
  { return value[i]; }

  friend constexpr numarray
  operator+(numarray a, numarray b)
  {
    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      return numarray{a.value[Is] + b.value[Is]...};
    }(std::make_index_sequence<N>());
  }

  friend constexpr bool
  operator==(const numarray&, const numarray&) = default;
};

void
test()
{
  // all of the following find the hidden friends in constexpr_wrapper via ADL:
  check<2>(std::cw<1> + std::cw<1>);
  check<3>(std::cw<1> + std::cw<2>);
  check<4uz>(std::cw<1> + std::extent<int[3]>());
  check<5uz>(std::extent<int[3]>() + std::cw<2>);
  check<6uz>(Derived<3>() + std::extent<int[3]>());
  check<7uz>(std::extent<int[5]>() + Derived<2>());
  check<3>(std::cw<1> + Derived<2>());
  check<9>(Derived<1>() + std::cw<8>);
  check<10>(Derived<2>() + Derived<8>());
  check<16>(Derived<8>() + Derived<8>());
  check<Test{}>(std::cw<Test{}>);
  check<5>(std::cw<Test{2}>(std::cw<1>, std::cw<2>));
#if not BAD_COMPILER
#if __cpp_multidimensional_subscript
  check<9>(std::cw<Test{}>[std::cw<1>, std::cw<2>, std::cw<5>]);
  check<1>(std::cw<Test{}>[]);
#endif
  check<'f'>(("foo"_sc)[std::cw<0>]);
#endif
  check<"foo"_sc.value>("foo"_sc);
  check<"foobar"_sc.value>("foo"_sc + "bar"_sc);
  check<2>(std::cw<NeedsAdl(1)> + std::cw<1>);
  check<2>(std::cw<1> + std::cw<NeedsAdl(1)>);

  // error: 'std::strong_ordering' is not a valid type for a template non-type parameter because it
  // is not structural
  //check<std::strong_ordering::less>("fob"_sc <=> "foo"_sc);

  check<-1>(std::cw<1> - std::cw<2>);
#if __cpp_nontype_template_args >= 201911L
  check<4.>(std::cw<2.> * std::cw<2.>);
  check<4.f>(std::cw<8.f> / std::cw<2.f>);
#endif
  check<2u>(std::cw<8u> % std::cw<3u>);
  check<1u>(std::cw<9u> & std::cw<3u>);
  check<11u>(std::cw<9u> | std::cw<3u>);
  check<10u>(std::cw<9u> ^ std::cw<3u>);

  constexpr Aaaargh a;
  auto cca = std::cw<a>;
  check<1>(++cca);
  check<2>(cca++);
  check<3>(--cca);
  check<4>(cca--);
  check<9>(cca->*(std::cw<4u>));
  check<3>(cca  = std::cw<3u>);
  check<4>(cca += std::cw<3u>);
  check<2>(cca -= std::cw<3u>);
  check<int>(cca->foo());
  check<int>(cca.value.foo());

  constexpr numarray<int, 4> v = {1, 2, 3, 4};
  constexpr numarray<int, 4> v0 = {};
  check<v>(std::cw<v> + std::cw<v0>);
  check<numarray<int, 4>>(std::cw<v> + v0);
#if not BAD_COMPILER
  check<2>(std::cw<v>[std::cw<1>]);
  check<int>(std::cw<v>[1]);
#endif

  // NOT constexpr_wrapper:
  check<int>(std::cw<1> + 0);
  check<int>(1 + std::cw<1>);
#if not BAD_COMPILER
  check<char>(("foo"_sc)[0]); // this is consistent with the two lines above
#endif
  check<std::size_t>(std::extent<int[3]>() + std::extent<int[5]>());

  // the following only work via ADL (requires type template parameter to constexpr_wrapper)
  check<int>(std::cw<NeedsAdl(1)> + 1);
  check<int>(1 + std::cw<NeedsAdl(1)>);
  check<strlit<char, 7>>("foo"_sc + "bar");
  check<strlit<char, 7>>("foo" + "bar"_sc);

#if __cpp_lib_constexpr_charconv >= 202207L && __cplusplus > 202002L
  using namespace std::literals;
  check<3>(1cw + 2cw);
  check<(signed char)(1)>(1cw);
  check<(signed char)(127)>(127cw);
  check<short(128)>(128cw);
  check<60'000>(60'000cw);
  check<2'000'000'000>(2'000'000'000cw);
  check<-2'000'000'000>(-2'000'000'000cw);
  check<4'000'000'000L>(4'000'000'000cw);
  check<4'000'000'000L>(4'000'000'000cw);
  check<4'000'000'000L>(4'000'000'000CW);
  check<9223372036854775807L  >(9223372036854775807cw);
  check<9223372036854775808ULL>(9223372036854775808cw);
  check<0xFFFF>(0xFFFFcw);
  check<0xffff>(0XffffCW);
  check<(signed char)0b1101>(0b1101CW);
#endif
}
