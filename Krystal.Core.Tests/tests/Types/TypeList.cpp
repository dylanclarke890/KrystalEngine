#include "Krystal.Core/Types/TypeList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Concepts.hpp"
#include <tuple>

namespace krys::tests
{
  namespace
  {
    template <typename T>
    struct AddPointer
    {
      using type = T *;
    };

    template <typename T>
    struct IsInt : BoolConstant<SameType<T, int>>
    {
    };
  }

  TEST_CASE("TypeList: compile-time operations", "[Types][TypeList]")
  {
    using Types = TypeList<int, char, int, double>;

    STATIC_CHECK(SameType<ApplyTypeList<Types, std::tuple>, std::tuple<int, char, int, double>>);
    STATIC_CHECK(SameType<TransformTypeList<TypeList<int, char>, AddPointer>, TypeList<int *, char *>>);
    STATIC_CHECK(SameType<AppendToTypeList<TypeList<int>, char, double>, TypeList<int, char, double>>);
    STATIC_CHECK(SameType<AppendToTypeList<TypeList<>>, TypeList<>>);
    STATIC_CHECK(SameType<RemoveTypesIf<Types, IsInt>, TypeList<char, double>>);
    STATIC_CHECK(SameType<RemoveTypesIf<TypeList<int>, IsInt>, TypeList<>>);
    STATIC_CHECK(SameType<RemoveTypesIf<TypeList<>, IsInt>, TypeList<>>);
  }
}
