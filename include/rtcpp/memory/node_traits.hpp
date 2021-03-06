#pragma once

#include <array>
#include <vector>
#include <utility>
#include <memory>
#include <exception>
#include <type_traits>

namespace rt {

template <typename ...> using void_type = void;

#define RTCPP_HAS_NESTED_TYPE(TYPE)\
  template <typename T, typename = void_type<>>\
  struct has_##TYPE : std::false_type {};\
  \
  template <typename T>\
  struct has_##TYPE<T, void_type<typename T::TYPE>>\
  : std::true_type {};

RTCPP_HAS_NESTED_TYPE(pointer)
RTCPP_HAS_NESTED_TYPE(link_type)
RTCPP_HAS_NESTED_TYPE(value_type)
RTCPP_HAS_NESTED_TYPE(void_pointer)
RTCPP_HAS_NESTED_TYPE(const_void_pointer)

//__________________________________________________________________
template <class A, bool B = has_void_pointer<A>::value>
struct void_pointer_type {
  using void_pointer = typename A::void_pointer;
};

template <class A>
struct void_pointer_type<A, false> {
  using pointer = typename A::pointer;
  using void_pointer =
    typename std::pointer_traits<pointer>::template rebind<void>;
};

//__________________________________________________________________
template <class A, bool B = has_const_void_pointer<A>::value>
struct const_void_pointer_type {
  using const_void_pointer = typename A::const_void_pointer;
};

template <class A>
struct const_void_pointer_type<A, false> {
  using pointer = typename A::pointer;
  using const_void_pointer =
    typename std::pointer_traits<pointer>::template rebind<const void>;
};

//__________________________________________________________________

template <typename T>
struct is_node_type {
  static const bool value = has_value_type<T>::value &&
    has_link_type<T>::value;
};

// Does not require node link_type to be the same.
template < typename N1
         , typename N2
         , bool IsNode = is_node_type<N2>::value &&
           is_node_type<N1>::value>
struct is_same_node_type {
  static const bool has_same_value_type =
    std::is_same< typename N1::value_type
                , typename N2::value_type>::value;
  using value_type = typename N1::value_type;
  using same_node_type1 = typename N1::template
    rebind<value_type, void*>::other;
  using same_node_type2 = typename N2::template
    rebind<value_type, void*>::other;
  static const bool value = has_same_value_type &&
    std::is_same<same_node_type1, same_node_type2>::value;
};

template <typename N1, typename N2>
struct is_same_node_type<N1, N2, false> {
  static const bool value = false;
};

template<typename Alloc>
struct allocate_node_helper
{
  template<typename Alloc2,
    typename = decltype(std::declval<Alloc2*>()->allocate_node())>
  static std::true_type test(int);

  template<typename>
  static std::false_type test(...);

  using type = decltype(test<Alloc>(0));
};

template<typename Alloc>
using has_allocate_node = typename allocate_node_helper<Alloc>::type;

template <typename T>
struct is_node {
  static const bool value = !std::is_pointer<T>::value;
};

}

