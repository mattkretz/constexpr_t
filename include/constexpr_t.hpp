/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2023 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 *                  Matthias Kretz <m.kretz@gsi.de>
 */

#include <concepts>
#include <type_traits>
#include <utility>

namespace std
{
  template <auto _Xp>
    struct constexpr_t;

  // *** constexpr_value<T, U = void> ***
  // If U is given, `constexpr_value<int> auto` is analogous to `int`
  // If U is void (default), the type of the value is unconstrained
  template <typename _Tp, typename _Up = void>
    concept constexpr_value = (same_as<_Up, void> || convertible_to<_Tp, _Up>)
				and not std::is_member_pointer_v<decltype(&_Tp::value)>
				and requires { typename constexpr_t<_Tp::value>; };

  namespace __detail
  {
    // exposition-only
    // Note: `not __any_constexpr_t` is not equivalent to `not derived_from<T,
    // constexpr_t<T::value>>` because of SFINAE (i.e. SFINAE would be reversed)
    template <typename _Tp>
      concept __any_constexpr_t = derived_from<_Tp, constexpr_t<_Tp::value>>;

    // exposition-only
    // Concept to require only a single binary operator declaration:
    // 1. LHS is derived from This, then RHS is either also derived from This (and
    //    there's only a single candidate) or RHS is not a constexpr_t (LHS
    //    provides the only operator candidate).
    // 2. LHS is not a constexpr_t, then RHS is derived from This (RHS provides the
    //    only operator candidate).
    template <typename _Tp, typename _This>
      concept __lhs_constexpr_t
	= constexpr_value<_Tp> and (derived_from<_Tp, _This> or not __any_constexpr_t<_Tp>);
  }

  // no type parameter, use `constexpr_value<type> auto` instead if you need to
  // constrain the type
  template <auto _Xp>
    struct constexpr_t
    {
      using value_type = std::remove_const_t<decltype(_Xp)>;

      using type = constexpr_t;

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
      // wrapping the result in constexpr_t

      // The overload of -> is inconsistent because it cannot wrap its return value in a
      // constexpr_t. The compiler/standard requires a pointer type. However, -> can work if it
      // unwraps. The utility is questionable though.
#if 0
      template <auto _Yp = _Xp>
	constexpr constexpr_t<_Yp.operator->()>
	operator->() const
	{ return {}; }
#else
      template <auto _Yp = _Xp>
	constexpr decltype(_Yp.operator->())
	operator->() const
	{ return _Yp.operator->(); }
#endif

      template <auto _Yp = _Xp>
	constexpr constexpr_t<+_Yp>
	operator+() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_t<-_Yp>
	operator-() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_t<~_Yp>
	operator~() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_t<!_Yp>
	operator!() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_t<&_Yp>
	operator&() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_t<*_Yp>
	operator*() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_t<++_Yp>
	operator++() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_t<_Yp++>
	operator++(int) const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_t<--_Yp>
	operator--() const
	{ return {}; }

      template <auto _Yp = _Xp>
	constexpr constexpr_t<_Yp-->
	operator--(int) const
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<_Ap::value + _Bp::value>
	operator+(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<_Ap::value - _Bp::value>
	operator-(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<_Ap::value * _Bp::value>
	operator*(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<_Ap::value / _Bp::value>
	operator/(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<_Ap::value % _Bp::value>
	operator%(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<_Ap::value & _Bp::value>
	operator&(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<_Ap::value | _Bp::value>
	operator|(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<_Ap::value ^ _Bp::value>
	operator^(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<_Ap::value && _Bp::value>
	operator&&(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<_Ap::value || _Bp::value>
	operator||(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value , _Bp::value)>
	operator,(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value << _Bp::value)>
	operator<<(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value >> _Bp::value)>
	operator>>(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value == _Bp::value)>
	operator==(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value != _Bp::value)>
	operator!=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value < _Bp::value)>
	operator<(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value <= _Bp::value)>
	operator<=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value > _Bp::value)>
	operator>(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value >= _Bp::value)>
	operator>=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value <=> _Bp::value)>
	operator<=>(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value ->* _Bp::value)>
	operator->*(_Ap, _Bp)
	{ return {}; }

      template <constexpr_value _Ap>
	constexpr constexpr_t<_Xp = _Ap::value>
	operator=(_Ap) const
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value += _Bp::value)>
	operator+=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value -= _Bp::value)>
	operator-=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value *= _Bp::value)>
	operator*=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value /= _Bp::value)>
	operator/=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value %= _Bp::value)>
	operator%=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value &= _Bp::value)>
	operator&=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value |= _Bp::value)>
	operator|=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value ^= _Bp::value)>
	operator^=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value <<= _Bp::value)>
	operator<<=(_Ap, _Bp)
	{ return {}; }

      template <__detail::__lhs_constexpr_t<constexpr_t> _Ap, constexpr_value _Bp>
	friend constexpr constexpr_t<(_Ap::value >>= _Bp::value)>
	operator>>=(_Ap, _Bp)
	{ return {}; }

#ifdef __cpp_static_call_operator
#define _static static
#define _const
#else
#define _static
#define _const const
#endif

      // overload operator() for constexpr_value and non-constexpr_value
      template <constexpr_value... _Args>
	_static constexpr constexpr_t<value(_Args::value...)>
	operator()(_Args...) _const
	{ return {}; }

      template <typename... _Args>
	requires (not constexpr_value<std::remove_cvref_t<_Args>> || ...)
	_static constexpr decltype(value(std::declval<_Args>()...))
	operator()(_Args&&... __args) _const
	{ return value(std::forward<_Args>(__args)...); }

      // overload operator[] for constexpr_value and non-constexpr_value
      template <constexpr_value... _Args>
	_static constexpr constexpr_t<value[_Args::value...]>
	operator[](_Args...) _const
	{ return {}; }

      template <typename... _Args>
	requires (not constexpr_value<std::remove_cvref_t<_Args>> || ...)
	_static constexpr decltype(value[std::declval<_Args>()...])
	operator[](_Args&&... __args) _const
	{ return value[std::forward<_Args>(__args)...]; }

#undef _static
#undef _const
    };

  // constexpr_t constant (cc)
  template <auto _Xp>
    inline constexpr constexpr_t<_Xp> cc{};

} // namespace std

// vim: noet tw=100 ts=8 sw=2 cc=101
