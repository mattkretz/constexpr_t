/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include <constexpr_t.hpp>
#include <array>

static_assert(std::cc<1> == 1);
static_assert(std::cc<1.f> == 1.f);

static_assert(std::constexpr_value<std::constexpr_t<1>>);

template <auto _Xp>
  struct Derived
  : std::constexpr_t<_Xp>
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
  inline constexpr std::constexpr_t<chars>
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
    static_assert(std::same_as<C, std::constexpr_t<Expected>>);
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

  constexpr auto
  operator->() const
  { return this; }

  constexpr int
  foo() const
  { return 5; }
};

void
test()
{
  // all of the following find the hidden friends in constexpr_t via ADL:
  check<2>(std::cc<1> + std::cc<1>);
  check<3>(std::cc<1> + std::cc<2>);
  check<4uz>(std::cc<1> + std::extent<int[3]>());
  check<5uz>(std::extent<int[3]>() + std::cc<2>);
  check<6uz>(Derived<3>() + std::extent<int[3]>());
  check<7uz>(std::extent<int[5]>() + Derived<2>());
  check<3>(std::cc<1> + Derived<2>());
  check<9>(Derived<1>() + std::cc<8>);
  check<10>(Derived<2>() + Derived<8>());
  check<16>(Derived<8>() + Derived<8>());
  check<Test{}>(std::cc<Test{}>);
  check<5>(std::cc<Test{2}>(std::cc<1>, std::cc<2>));
  check<9>(std::cc<Test{}>[std::cc<1>, std::cc<2>, std::cc<5>]);
  check<1>(std::cc<Test{}>[]);
  check<"foo"_sc.value>("foo"_sc);
  check<'f'>(("foo"_sc)[std::cc<0>]);
  check<"foobar"_sc.value>("foo"_sc + "bar"_sc);
  check<2>(std::cc<NeedsAdl(1)> + std::cc<1>);
  check<2>(std::cc<1> + std::cc<NeedsAdl(1)>);

  // error: 'std::strong_ordering' is not a valid type for a template non-type parameter because it
  // is not structural
  //check<std::strong_ordering::less>("fob"_sc <=> "foo"_sc);

  check<-1>(std::cc<1> - std::cc<2>);
  check<4.>(std::cc<2.> * std::cc<2.>);
  check<4.f>(std::cc<8.f> / std::cc<2.f>);
  check<2u>(std::cc<8u> % std::cc<3u>);
  check<1u>(std::cc<9u> & std::cc<3u>);
  check<11u>(std::cc<9u> | std::cc<3u>);
  check<10u>(std::cc<9u> ^ std::cc<3u>);

  constexpr Aaaargh a;
  auto cca = std::cc<a>;
  check<1>(++cca);
  check<2>(cca++);
  check<3>(--cca);
  check<4>(cca--);
  check<9>(cca->*(std::cc<4u>));
  check<3>(cca  = std::cc<3u>);
  check<4>(cca += std::cc<3u>);
  check<2>(cca -= std::cc<3u>);
  check<int>(cca->foo());
  check<int>(cca.value.foo());

  // NOT constexpr_t:
  check<int>(std::cc<1> + 0);
  check<int>(1 + std::cc<1>);
  check<char>(("foo"_sc)[0]); // this is consistent with the two lines above
  check<std::size_t>(std::extent<int[3]>() + std::extent<int[5]>());

  // the following only work via ADL (requires type template parameter to constexpr_t)
  check<int>(std::cc<NeedsAdl(1)> + 1);
  check<int>(1 + std::cc<NeedsAdl(1)>);
  check<strlit<char, 7>>("foo"_sc + "bar");
  check<strlit<char, 7>>("foo" + "bar"_sc);
}
