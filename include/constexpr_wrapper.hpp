/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#ifndef VIR_CONSTEXPR_WRAPPER_HPP_
#define VIR_CONSTEXPR_WRAPPER_HPP_

#include <algorithm>
#include <array>
#include <charconv>
#include <concepts>
#include <type_traits>
#include <utility>

namespace std
{
  template <auto _Xp, typename = std::remove_cvref_t<decltype(_Xp)>>
    struct constexpr_wrapper;

  // *** constexpr_value<T, U = void> ***
  // If U is given, T must be convertible to U (`constexpr_value<int> auto` is analogous to `int`).
  // If U is void (default), the type of the value is unconstrained.
  template <typename _Tp, typename _Up = void>
    concept constexpr_value = (same_as<_Up, void> || convertible_to<_Tp, _Up>)
#if defined __GNUC__ and __GNUC__ < 13
				and not std::is_member_pointer_v<decltype(&_Tp::value)>
#endif
				and requires { typename constexpr_wrapper<_Tp::value>; };

  namespace __detail
  {
    // exposition-only
    // Note: `not __any_constexpr_wrapper` is not equivalent to `not derived_from<T,
    // constexpr_wrapper<T::value>>` because of SFINAE (i.e. SFINAE would be reversed)
    template <typename _Tp>
      concept __any_constexpr_wrapper = derived_from<_Tp, constexpr_wrapper<_Tp::value>>;

    // exposition-only
    // Concept that enables declaring a single binary operator overload per operation:
    // 1. LHS is derived from This, then RHS is either also derived from This (and there's only a
    //    single candidate) or RHS is not a constexpr_wrapper (LHS provides the only operator
    //    candidate).
    // 2. LHS is not a constexpr_wrapper, then RHS is derived from This (RHS provides the only
    //    operator candidate).
    template <typename _Tp, typename _This>
      concept __lhs_constexpr_wrapper
	= constexpr_value<_Tp> and (derived_from<_Tp, _This> or not __any_constexpr_wrapper<_Tp>);
  }

  // Prefer to use `constexpr_value<type> auto` instead of `template <typename T> void
  // f(constexpr_wrapper<T, type> ...` to constrain the type of the constant.
  template <auto _Xp, typename _Tp>
    struct constexpr_wrapper
    {
      using value_type = _Tp;

      using type = constexpr_wrapper;

      static constexpr value_type value{ _Xp };

      constexpr
      operator value_type() const
      { return _Xp; }

      // overloads the following:
      //
      // unary:
      // + - ~ ! & * ++ -- ->
      //
      // binary:
      // + - * / % & | ^ && || , << >> == != < <= > >= ->* = += -= *= /= %= &= |= ^= <<= >>=
      //
      // [] and () are overloaded for constexpr_value or not constexpr_value, the latter not
      // wrapping the result in constexpr_wrapper

      // The overload of -> is inconsistent because it cannot wrap its return value in a
      // constexpr_wrapper. The compiler/standard requires a pointer type. However, -> can work if
      // it unwraps. The utility is questionable though, which it why may be interesting to
      // "dereferencing" the wrapper to its value if _Tp doesn't implement operator->.
      constexpr const auto*
      operator->() const
      {
	if constexpr (requires{_Xp.operator->();})
	  return _Xp.operator->();
	else
	  return std::addressof(_Xp);
      }

      template <auto _Yp = _Xp>
	constexpr constexpr_wrapper<+_Yp>
	operator+() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_wrapper<-_Yp>
	operator-() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_wrapper<~_Yp>
	operator~() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_wrapper<!_Yp>
	operator!() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_wrapper<&_Yp>
	operator&() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_wrapper<*_Yp>
	operator*() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_wrapper<++_Yp>
	operator++() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_wrapper<_Yp++>
	operator++(int) const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_wrapper<--_Yp>
	operator--() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_wrapper<_Yp-->
	operator--(int) const
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<_Ap::value + _Bp::value>
	operator+(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<_Ap::value - _Bp::value>
	operator-(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<_Ap::value * _Bp::value>
	operator*(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<_Ap::value / _Bp::value>
	operator/(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<_Ap::value % _Bp::value>
	operator%(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<_Ap::value & _Bp::value>
	operator&(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<_Ap::value | _Bp::value>
	operator|(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<_Ap::value ^ _Bp::value>
	operator^(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<_Ap::value && _Bp::value>
	operator&&(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<_Ap::value || _Bp::value>
	operator||(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value , _Bp::value)>
	operator,(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value << _Bp::value)>
	operator<<(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value >> _Bp::value)>
	operator>>(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value == _Bp::value)>
	operator==(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value != _Bp::value)>
	operator!=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value < _Bp::value)>
	operator<(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value <= _Bp::value)>
	operator<=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value > _Bp::value)>
	operator>(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value >= _Bp::value)>
	operator>=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value <=> _Bp::value)>
	operator<=>(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value ->* _Bp::value)>
	operator->*(_Ap, _Bp)
	{ return {}; }

      template <constexpr_value _Ap>
	constexpr constexpr_wrapper<(_Xp = _Ap::value)>
	operator=(_Ap) const
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value += _Bp::value)>
	operator+=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value -= _Bp::value)>
	operator-=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value *= _Bp::value)>
	operator*=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value /= _Bp::value)>
	operator/=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value %= _Bp::value)>
	operator%=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value &= _Bp::value)>
	operator&=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value |= _Bp::value)>
	operator|=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value ^= _Bp::value)>
	operator^=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value <<= _Bp::value)>
	operator<<=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_wrapper<constexpr_wrapper> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_wrapper<(_Ap::value >>= _Bp::value)>
	operator>>=(_Ap, _Bp)
	{ return {}; }

#if defined __cpp_static_call_operator && __cplusplus > 202002L
#define _STATIC static
#define _CONST
#else
#define _STATIC
#define _CONST const
#endif

      // overload operator() for constexpr_value and non-constexpr_value
      template <constexpr_value... _Args>
        requires requires { typename constexpr_wrapper<value(_Args::value...)>; }
	_STATIC constexpr auto
	operator()(_Args...) _CONST
        { return constexpr_wrapper<value(_Args::value...)>{}; }

      template <typename... _Args>
	requires (not constexpr_value<std::remove_cvref_t<_Args>> || ...)
	_STATIC constexpr decltype(value(std::declval<_Args>()...))
	operator()(_Args&&... __args) _CONST
	{ return value(std::forward<_Args>(__args)...); }

      _STATIC constexpr _Tp
      operator()() _CONST
      requires (not requires { value(); })
        { return value; }

      // overload operator[] for constexpr_value and non-constexpr_value
#if __cpp_multidimensional_subscript
      template <constexpr_value... _Args>
	_STATIC constexpr constexpr_wrapper<value[_Args::value...]>
	operator[](_Args...) _CONST
	{ return {}; }

      template <typename... _Args>
	requires (not constexpr_value<std::remove_cvref_t<_Args>> || ...)
	_STATIC constexpr decltype(value[std::declval<_Args>()...])
	operator[](_Args&&... __args) _CONST
	{ return value[std::forward<_Args>(__args)...]; }
#else
      template <constexpr_value _Arg>
	_STATIC constexpr constexpr_wrapper<value[_Arg::value]>
        operator[](_Arg) _CONST
        { return {}; }

      template <typename _Arg>
        requires (not constexpr_value<std::remove_cvref_t<_Arg>>)
	_STATIC constexpr decltype(value[std::declval<_Arg>()])
        operator[](_Arg&& _arg) _CONST
        { return value[std::forward<_Arg>(_arg)]; }
#endif

#undef _STATIC
#undef _CONST
    };

  // constexpr_wrapper constant (cw)
  template <auto _Xp>
    inline constexpr constexpr_wrapper<_Xp> cw{};

#if __cpp_lib_constexpr_charconv >= 202207L && __cplusplus > 202002L
  namespace __detail
  {
    enum class _CwStatus
    {
      __okay,
      __try_next_type,
      __error
    };

    template <typename _Tp>
      struct _CwResult
      {
	_Tp _M_value;
	_CwStatus _M_status;
      };

    template <const auto& __arr, int __base,
	      typename _Tp, typename... _Next>
      consteval auto
      __do_parse()
      {
	constexpr _CwResult __r1 = []() {
	  _Tp __x = {};
	  constexpr int __offset = __base == 10 ? 0 : __base == 8 ? 1 : 2;
	  constexpr auto __end = __arr.end() - (__base == 16 ? 1 : 0);
	  const auto __result = std::from_chars(__arr.begin() + __offset, __end, __x, __base);
	  if (__result.ec == std::errc::result_out_of_range)
	    return _CwResult<_Tp>{__x, _CwStatus::__try_next_type};
	  else if (__result.ec != std::errc {} or __result.ptr != __end)
	    return _CwResult<_Tp>{__x, _CwStatus::__error};
	  else
	    return _CwResult<_Tp>{__x, _CwStatus::__okay};
	}();
	if constexpr (__r1._M_status == _CwStatus::__try_next_type and sizeof...(_Next))
	  return __do_parse<__arr, __base, _Next...>();
	else
	  return __r1;
      }

    template <char... _Chars>
      consteval auto
      __cw_prepare_array()
      {
	constexpr auto __not_digit_sep = [](char __c) { return __c != '\''; };
	constexpr char __arr0[sizeof...(_Chars)] = {_Chars...};
	constexpr auto __size
	  = std::count_if(std::begin(__arr0), std::end(__arr0), __not_digit_sep);
	std::array<char, __size> __tmp = {};
	std::copy_if(std::begin(__arr0), std::end(__arr0), __tmp.begin(), __not_digit_sep);
	return __tmp;
      }

    template <char... _Chars>
      consteval auto
      __cw_parse()
      {
	static constexpr std::array __arr = __cw_prepare_array<_Chars...>();
	constexpr int __base = __arr[0] == '0' and 2 < __arr.size()
				 ? __arr[1] == 'x' or __arr[1] == 'X' ? 16
								      : __arr[1] == 'b' ? 2 : 8
				 : 10;
	constexpr _CwResult __r
	  = __do_parse<__arr, __base, signed char, signed short, signed int, signed long,
		       signed long long, unsigned long long>();
	static_assert(__r._M_status == _CwStatus::__okay,
		      "Parsing constexpr_wrapper literal failed.");
	return __r._M_value;
      }
  }

  namespace literals
  {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
    template <char... _Chars>
      constexpr auto operator"" cw()
      { return std::cw<std::__detail::__cw_parse<_Chars...>()>; }

    template <char... _Chars>
      constexpr auto operator"" CW()
      { return std::cw<std::__detail::__cw_parse<_Chars...>()>; }

    template <char... _Chars>
      requires (sizeof...(_Chars) > 2
		  and std::array {_Chars...}[0] == '0'
		  and (std::array {_Chars...}[1] == 'x' or std::array {_Chars...}[1] == 'X')
		  and std::array {_Chars...}.back() == 'c'
	       )
      constexpr auto operator"" w()
      { return std::cw<std::__detail::__cw_parse<_Chars...>()>; }

    template <char... _Chars>
      requires (sizeof...(_Chars) > 2
		  and std::array {_Chars...}[0] == '0'
		  and (std::array {_Chars...}[1] == 'x' or std::array {_Chars...}[1] == 'X')
		  and std::array {_Chars...}.back() == 'C'
	       )
      constexpr auto operator"" W()
      { return std::cw<std::__detail::__cw_parse<_Chars...>()>; }
#pragma GCC diagnostic pop
  }
#endif

} // namespace std

#endif  // VIR_CONSTEXPR_WRAPPER_HPP_
// vim: noet tw=100 ts=8 sw=2 cc=101
