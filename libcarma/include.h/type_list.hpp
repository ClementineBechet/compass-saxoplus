#ifndef CARMA_TYPE_LIST_H
#define CARMA_TYPE_LIST_H

#include <utility>

// Create instantiations for templated functions by keeping their addresses
// and therefore forcing the compiler to keep their object code
// attribute((unused)) silences the clang/gcc warning
template <typename T>
void force_keep(T t) {
  static __attribute__((used)) T x = t;
}

template <typename...>
struct GenericTypeList;

template <typename Type, typename... Types>
struct GenericTypeList<Type, Types...> {
  using Head = Type;
  using Tail = GenericTypeList<Types...>;
};

template <>
struct GenericTypeList<> {};

using EmptyList = GenericTypeList<>;

template <typename Interfacer, typename TList>
struct TypeMap;

template <typename Interfacer>
struct TypeMap<Interfacer, EmptyList> {
  template <typename... Args>
  static void apply(Args &&...) {}
};

template <typename Interfacer, typename Type,
          typename... Types  //,
                             // typename TList = GenericTypeList<Type,
                             // Types...>, typename Head = typename TList::Head,
                             // typename Tail = typename TList::Tail
          >
struct TypeMap<Interfacer, GenericTypeList<Type, Types...>>
    : TypeMap<Interfacer, typename GenericTypeList<Type, Types...>::Tail> {
  using TList = GenericTypeList<Type, Types...>;
  using Head = typename TList::Head;
  using Tail = typename TList::Tail;

  using Base = TypeMap<Interfacer, Tail>;

  template <typename... Args>
  static void apply(Args &&... args) {
    Interfacer::template call<Head>(std::forward<Args>(args)...);
    Base::template apply(std::forward<Args>(args)...);
  }
};

template <typename Interfacer, typename TList, typename... Args>
void apply(Args &&... args) {
  TypeMap<Interfacer, TList>::template apply(std::forward<Args>(args)...);
}

// template< template <typename> class TemplateT, typename GenericTypeList>
// struct TypeMap;

// template< template <typename> class TemplateT, typename... Types>
// struct TypeMap<TemplateT, GenericTypeList<Types...>> : TemplateT<Types>...
// {};

#endif  // CARMA_TYPE_LIST_H