#pragma once

#include <array>
#include <vector>
#include <limits>

#include "align.hpp"

namespace rt {

template <class, class, std::size_t>
class node_link;

template <class T, class L, std::size_t N>
class node_storage;

template <class T, class L, std::size_t N>
class node_ptr {
  public:
  using storage_type = node_storage<T, L, N>;
  private:
  static constexpr std::size_t R = sizeof (T) / sizeof (L);
  storage_type* storage {nullptr};
  L idx {};

  public:
  template <class, class, std::size_t>
  friend class node_link;

  L get_idx() const {return idx;}
  using link_type = L;
  using element_type = T;
  template <class U>
  using rebind = node_ptr<U, L, N>;
  T& operator*()
  {
    auto b = storage->get_base_ptr(idx);
    return *reinterpret_cast<T*>(&b[(idx % N) * R]);
  }
  const T& operator*() const
  {
    auto b = storage->get_base_ptr(idx);
    return *reinterpret_cast<const T*>(&b[(idx % N) * R]);
  }
  T* operator->()
  {
    auto b = storage->get_base_ptr(idx);
    return reinterpret_cast<T*>(&b[(idx % N) * R]);
  }
  const T* operator->() const
  {
    auto b = storage->get_base_ptr(idx);
    return reinterpret_cast<const T*>(&b[(idx % N) * R]);
  }
  node_ptr() = default;
  node_ptr& operator=(const node_link<T, L, N>& rhs)
  {
    idx = rhs.idx;
    return *this;
  }
  node_ptr& operator=(const node_ptr<T, L, N>& rhs)
  {
    idx = rhs.idx;
    storage = rhs.storage;
    return *this;
  }
  node_ptr(storage_type* pp, L i) : storage(pp), idx(i) {}
};

//____________________________________________________
template <class T, class L, std::size_t N>
bool operator==( const node_ptr<T, L, N>& p1
               , const node_ptr<T, L, N>& p2)
{return p1.get_idx() == p2.get_idx();}

template <class T, class L, std::size_t N>
bool operator!=( const node_ptr<T, L, N>& p1
               , const node_ptr<T, L, N>& p2)
{return !(p1 == p2);}

template <class T, class L, std::size_t N>
class node_link {
  private:
  template <class, class, std::size_t>
  friend class node_ptr;
  private:
  L idx;
  public:
  L get_idx() const {return idx;}
  using link_type = L;
  using element_type = T;
  template <class U>
  using rebind = node_ptr<U, L, N>;
  node_link& operator=(const node_ptr<T, L, N>& rhs)
  {
    idx = rhs.idx;
    return *this;
  }
};

//____________________________________________________
template <class T, class L, std::size_t N>
bool operator==( const node_ptr<T, L, N>& p1
               , const node_link<T, L, N>& p2)
{return p1.get_idx() == p2.get_idx();}

template <class T, class L, std::size_t N>
bool operator!=( const node_ptr<T, L, N>& p1
               , const node_link<T, L, N>& p2)
{return !(p1 == p2);}

template <class T, class L, std::size_t N>
bool operator==( const node_link<T, L, N>& p1
               , const node_ptr<T, L, N>& p2)
{return p1.get_idx() == p2.get_idx();}

template <class T, class L, std::size_t N>
bool operator!=( const node_link<T, L, N>& p1
               , const node_ptr<T, L, N>& p2)
{return !(p1 == p2);}

//____________________________________________________
template <class L, std::size_t N>
class node_ptr_void {
  public:
  using link_type = L;
  using element_type = void;
  template <class U>
  using rebind = node_link<U, L, N>;
};

template <class T, class L, std::size_t N>
class node_storage {
  public:
  using pointer = node_ptr<T, L, N>;
  private:

  static_assert((!std::is_signed<L>::value),
  "node_storage: Incompatible type.");

  static_assert((N - 1 <= std::numeric_limits<L>::max()),
  "node_storage: Incompatible size.");

  static_assert((is_power_of_two<N>::value),
  "node_storage: N must be a power of 2.");

  static constexpr auto SL = sizeof (L);
  static constexpr auto ST = sizeof (T);
  static constexpr auto R = (SL < ST) ? ST / SL : 1;
  static constexpr auto S = N * R; // Size of allocated array.

  L free {0};
  std::vector<L*> bufs;

  // Non copyable
  node_storage& operator=(const node_storage&) = delete;
  node_storage(const node_storage&) = delete;
  public:
  node_storage() {}

  static_assert((S >= 2), "node_storage: Invalid N.");

  std::size_t get_n_blocs() const {return bufs.size();}
  L* get_base_ptr(L idx)
  {
    return bufs[idx / N];
  }

  ~node_storage()
  {
    for_each(std::begin(bufs), std::end(bufs), [](auto p) {
      delete [] p; });
  }

  L add_bloc() // returns the index of the next free node.
  {
      const auto s = bufs.size();
      bufs.push_back(new L[S]);
      const auto offset = s * N;
      for (std::size_t i = 1; i < N; ++i)
        bufs.back()[i * R] = offset + i - 1;

      bufs.back()[0] = 0;
      return bufs.size() * N - 1;
  }

  pointer pop()
  {
    auto i = free;

    if (!i) {
      free = add_bloc();
      i = free;
    }

    auto b = get_base_ptr(i);
    free = b[(i % N) * R];
    return pointer(this, i);
  }

  void push(pointer idx) noexcept
  {
    const auto i = idx.get_idx();
    auto b = get_base_ptr(i);
    b[(i % N) * R] = free;
    free = i;
  }

  bool operator==(const node_storage& rhs) const noexcept
  {return bufs == rhs.bufs;}
};

}

