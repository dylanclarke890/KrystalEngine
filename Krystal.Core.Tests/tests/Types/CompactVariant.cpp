#include "Krystal.Core/Types/CompactVariant.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/FlatteningVariant.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <bit>
#include <string>

namespace krys::tests
{
  namespace
  {
    std::string &Log()
    {
      static std::string log;
      return log;
    }

    std::string TakeLog()
    {
      return std::exchange(Log(), std::string {});
    }

    struct EmptyStruct
    {
      constexpr bool operator==(const EmptyStruct &) const = default;
    };

    struct SmallEnoughStruct
    {
      float Value {0};

      constexpr bool operator==(const SmallEnoughStruct &) const = default;
    };

    struct TooBigStruct
    {
      double Value {0};

      constexpr bool operator==(const TooBigStruct &) const = default;
    };

    struct MoveOnly
    {
      uint32 Value {0};

      explicit MoveOnly(uint32 value) noexcept : Value(value)
      {
      }

      MoveOnly(const MoveOnly &) = delete;
      MoveOnly &operator=(const MoveOnly &) = delete;

      MoveOnly(MoveOnly &&other) noexcept : Value(std::exchange(other.Value, 0))
      {
      }

      MoveOnly &operator=(MoveOnly &&other) noexcept
      {
        Value = std::exchange(other.Value, 0);
        return *this;
      }

      bool operator==(const MoveOnly &) const = default;
    };

    // Logs its lifetime events; pointer sized, so it's treated as a smart pointer below.
    struct LifecycleLogger
    {
      const char *Name {"<default>"};

      LifecycleLogger() noexcept = default;

      explicit LifecycleLogger(const char *name) noexcept : Name(name)
      {
        Log() += std::string("construct(") + Name + ") ";
      }

      LifecycleLogger(const LifecycleLogger &other) noexcept : Name(other.Name)
      {
        Log() += std::string("copy-construct(") + Name + ") ";
      }

      LifecycleLogger(LifecycleLogger &&other) noexcept : Name(std::exchange(other.Name, "<default>"))
      {
        Log() += std::string("move-construct(") + Name + ") ";
      }

      LifecycleLogger &operator=(const LifecycleLogger &other) noexcept
      {
        Name = other.Name;
        Log() += std::string("copy-assign(") + Name + ") ";
        return *this;
      }

      LifecycleLogger &operator=(LifecycleLogger &&other) noexcept
      {
        Name = std::exchange(other.Name, "<default>");
        Log() += std::string("move-assign(") + Name + ") ";
        return *this;
      }

      ~LifecycleLogger() noexcept
      {
        Log() += std::string("destruct(") + Name + ") ";
      }

      bool operator==(const LifecycleLogger &) const = default;
    };
  }
}

namespace krys
{
  template <>
  constexpr bool IsSmartPtr<tests::LifecycleLogger> = true;

  template <>
  struct CompactVariantTraits<tests::TooBigStruct>
  {
    constexpr static bool HasAlternativeRepresentation = true;

    constexpr static uint64 EncodeFromArguments(double value) noexcept
    {
      return static_cast<uint64>(std::bit_cast<uint32>(static_cast<float>(value)));
    }

    constexpr static uint64 Encode(const tests::TooBigStruct &value) noexcept
    {
      return static_cast<uint64>(std::bit_cast<uint32>(static_cast<float>(value.Value)));
    }

    constexpr static uint64 Encode(tests::TooBigStruct &&value) noexcept
    {
      return static_cast<uint64>(std::bit_cast<uint32>(static_cast<float>(value.Value)));
    }

    constexpr static tests::TooBigStruct Decode(uint64 value) noexcept
    {
      return {std::bit_cast<float>(static_cast<uint32>(value))};
    }
  };
}

namespace krys::tests
{
  TEST_CASE("CompactVariant: pointers", "[CompactVariant]")
  {
    int testInt = 1;
    float testFloat = 2.0f;

    CompactVariant<int *, float *> variant = &testInt;
    CHECK(HoldsAlternative<int *>(variant));
    CHECK_FALSE(HoldsAlternative<float *>(variant));
    SwitchOn(variant, [&](int *const &value) { CHECK(*value == 1); }, [&](float *const &) { FAIL(); });

    variant = &testFloat;
    CHECK_FALSE(HoldsAlternative<int *>(variant));
    CHECK(HoldsAlternative<float *>(variant));
    SwitchOn(variant, [&](int *const &) { FAIL(); }, [&](float *const &value) { CHECK(*value == 2.0f); });

    STATIC_CHECK(sizeof(variant) == sizeof(uint64));
  }

  TEST_CASE("CompactVariant: 64-bit pointers keep their top byte", "[CompactVariant]")
  {
    int testInt = 1;
    void *testPtr = reinterpret_cast<void *>(0x26'00'12'34'56'78'9a'bcULL);

    CompactVariant<int *, void *> variant = &testInt;
    CHECK(HoldsAlternative<int *>(variant));
    CHECK_FALSE(HoldsAlternative<void *>(variant));

    variant = testPtr;
    CHECK_FALSE(HoldsAlternative<int *>(variant));
    CHECK(HoldsAlternative<void *>(variant));
    SwitchOn(
      variant, [&](int *const &) { FAIL(); },
      [&](void *const &value) { CHECK(reinterpret_cast<uintptr_t>(value) == 0x26'00'12'34'56'78'9a'bcULL); });
  }

  TEST_CASE("CompactVariant: smart pointers", "[CompactVariant]")
  {
    CompactVariant<UniquePtr<int>, UniquePtr<double>> variant {std::in_place_type<UniquePtr<int>>,
                                                               std::make_unique<int>(1)};
    CHECK(HoldsAlternative<UniquePtr<int>>(variant));
    SwitchOn(
      variant, [&](const UniquePtr<int> &value) { CHECK(*value == 1); },
      [&](const UniquePtr<double> &) { FAIL(); });

    variant = std::make_unique<double>(2.0);
    CHECK_FALSE(HoldsAlternative<UniquePtr<int>>(variant));
    CHECK(HoldsAlternative<UniquePtr<double>>(variant));
    SwitchOn(
      variant, [&](const UniquePtr<int> &) { FAIL(); },
      [&](const UniquePtr<double> &value) { CHECK(*value == 2.0); });
  }

  TEST_CASE("CompactVariant: small scalars and structs", "[CompactVariant]")
  {
    float testFloat = 2.0f;

    CompactVariant<int *, float *, float> scalar = 3.0f;
    CHECK(HoldsAlternative<float>(scalar));
    SwitchOn(
      scalar, [&](int *const &) { FAIL(); }, [&](float *const &) { FAIL(); },
      [&](const float &value) { CHECK(value == 3.0f); });
    scalar = &testFloat;
    CHECK(HoldsAlternative<float *>(scalar));
    CHECK_FALSE(HoldsAlternative<float>(scalar));

    CompactVariant<int *, float *, EmptyStruct> empty = EmptyStruct {};
    CHECK(HoldsAlternative<EmptyStruct>(empty));
    empty = &testFloat;
    CHECK_FALSE(HoldsAlternative<EmptyStruct>(empty));

    CompactVariant<int *, float *, SmallEnoughStruct> small = SmallEnoughStruct {3.0f};
    CHECK(HoldsAlternative<SmallEnoughStruct>(small));
    SwitchOn(
      small, [&](int *const &) { FAIL(); }, [&](float *const &) { FAIL(); },
      [&](const SmallEnoughStruct &value) { CHECK(value.Value == 3.0f); });
    small = &testFloat;
    CHECK(HoldsAlternative<float *>(small));
  }

  TEST_CASE("CompactVariant: alternative representation", "[CompactVariant]")
  {
    float testFloat = 2.0f;

    CompactVariant<int *, float *, TooBigStruct> variant = TooBigStruct {4.0};
    CHECK(HoldsAlternative<TooBigStruct>(variant));
    SwitchOn(
      variant, [&](int *const &) { FAIL(); }, [&](float *const &) { FAIL(); },
      [&](const TooBigStruct value) { CHECK(value.Value == 4.0); });

    variant = &testFloat;
    CHECK(HoldsAlternative<float *>(variant));

    variant = TooBigStruct {5.0};
    CompactVariant<int *, float *, TooBigStruct> movedToVariant = krys::move(variant);
    CHECK(variant.IsValuelessByMove());
    CHECK(HoldsAlternative<TooBigStruct>(movedToVariant));
    SwitchOn(
      movedToVariant, [&](int *const &) { FAIL(); }, [&](float *const &) { FAIL(); },
      [&](const TooBigStruct value) { CHECK(value.Value == 5.0); });

    variant = movedToVariant;
    CHECK(HoldsAlternative<TooBigStruct>(movedToVariant));
    CHECK(HoldsAlternative<TooBigStruct>(variant));
    SwitchOn(
      variant, [&](int *const &) { FAIL(); }, [&](float *const &) { FAIL(); },
      [&](const TooBigStruct value) { CHECK(value.Value == 5.0); });
    CHECK(variant == movedToVariant);
  }

  TEST_CASE("CompactVariant: move-only alternatives", "[CompactVariant]")
  {
    float testFloat = 2.0f;

    CompactVariant<int *, float *, MoveOnly> variant = MoveOnly {5u};
    CHECK(HoldsAlternative<MoveOnly>(variant));
    SwitchOn(
      variant, [&](int *const &) { FAIL(); }, [&](float *const &) { FAIL(); },
      [&](const MoveOnly &value) { CHECK(value.Value == 5u); });

    variant = &testFloat;
    CHECK(HoldsAlternative<float *>(variant));

    variant = MoveOnly {6u};
    CHECK(HoldsAlternative<MoveOnly>(variant));
    SwitchOn(
      variant, [&](int *const &) { FAIL(); }, [&](float *const &) { FAIL(); },
      [&](const MoveOnly &value) { CHECK(value.Value == 6u); });
  }

  TEST_CASE("CompactVariant: valueless by move", "[CompactVariant]")
  {
    int testInt = 1;

    CompactVariant<int *, float *> variant = &testInt;
    CHECK_FALSE(variant.IsValuelessByMove());

    CompactVariant<int *, float *> other = krys::move(variant);
    CHECK_FALSE(other.IsValuelessByMove());
    CHECK(variant.IsValuelessByMove());

    CompactVariant<int *, float *> copy = variant;
    CHECK(variant.IsValuelessByMove());
    CHECK(copy.IsValuelessByMove());

    CompactVariant<int *, float *> moved = krys::move(variant);
    CHECK(variant.IsValuelessByMove());
    CHECK(moved.IsValuelessByMove());
  }

  TEST_CASE("CompactVariant: lifecycle", "[CompactVariant]")
  {
    TakeLog();

    SECTION("argument assignment")
    {
      {
        CompactVariant<float, LifecycleLogger> variant = LifecycleLogger("compact");
        CHECK(TakeLog() == "construct(compact) move-construct(compact) destruct(<default>) ");
      }
      CHECK(TakeLog() == "destruct(compact) ");
    }

    SECTION("in-place type construction")
    {
      {
        CompactVariant<float, LifecycleLogger> variant {std::in_place_type<LifecycleLogger>, "compact"};
        CHECK(TakeLog() == "construct(compact) ");
      }
      CHECK(TakeLog() == "destruct(compact) ");
    }

    SECTION("in-place index construction")
    {
      {
        CompactVariant<float, LifecycleLogger> variant {std::in_place_index<1>, "compact"};
        CHECK(TakeLog() == "construct(compact) ");
      }
      CHECK(TakeLog() == "destruct(compact) ");
    }

    SECTION("argument move and copy construction")
    {
      {
        LifecycleLogger lifecycleLogger("compact");
        CompactVariant<float, LifecycleLogger> variant {krys::move(lifecycleLogger)};
        CHECK(TakeLog() == "construct(compact) move-construct(compact) ");
      }
      CHECK(TakeLog() == "destruct(compact) destruct(<default>) ");

      {
        LifecycleLogger lifecycleLogger("compact");
        CompactVariant<float, LifecycleLogger> variant {lifecycleLogger};
        CHECK(TakeLog() == "construct(compact) copy-construct(compact) ");
      }
      CHECK(TakeLog() == "destruct(compact) destruct(compact) ");
    }

    SECTION("copy and move")
    {
      {
        CompactVariant<float, LifecycleLogger> variant {std::in_place_type<LifecycleLogger>, "compact"};
        CompactVariant<float, LifecycleLogger> other {variant};
        CHECK(TakeLog() == "construct(compact) copy-construct(compact) ");
      }
      CHECK(TakeLog() == "destruct(compact) destruct(compact) ");

      {
        CompactVariant<float, LifecycleLogger> variant {std::in_place_type<LifecycleLogger>, "compact"};
        CompactVariant<float, LifecycleLogger> other {krys::move(variant)};
        CHECK(TakeLog() == "construct(compact) move-construct(compact) ");
      }
      CHECK(TakeLog() == "destruct(compact) ");
    }

    SECTION("reassignment")
    {
      {
        CompactVariant<float, LifecycleLogger> variant {std::in_place_type<LifecycleLogger>, "compact"};
        variant = 1.0f;
        CHECK(TakeLog() == "construct(compact) destruct(compact) ");
      }
      CHECK(TakeLog().empty());

      {
        CompactVariant<float, LifecycleLogger> variant {1.0f};
        variant = LifecycleLogger("compact");
        CHECK(TakeLog() == "construct(compact) move-construct(compact) destruct(<default>) ");
      }
      CHECK(TakeLog() == "destruct(compact) ");

      {
        CompactVariant<float, LifecycleLogger> variant {1.0f};
        LifecycleLogger lifecycleLogger("compact");
        variant = lifecycleLogger;
        CHECK(TakeLog() == "construct(compact) copy-construct(compact) ");
      }
      CHECK(TakeLog() == "destruct(compact) destruct(compact) ");
    }
  }

  TEST_CASE("CompactVariant: equality, emplace and swap", "[CompactVariant]")
  {
    CompactVariant<float, SmallEnoughStruct> a = 1.0f;
    CompactVariant<float, SmallEnoughStruct> b = 1.0f;
    CompactVariant<float, SmallEnoughStruct> c = SmallEnoughStruct {1.0f};
    CHECK(a == b);
    CHECK_FALSE(a == c);
    CHECK(a.index() == 0);
    CHECK(c.index() == 1);
    CHECK(c.HoldsAlternative<1>());

    a.emplace<SmallEnoughStruct>(2.0f);
    CHECK(HoldsAlternative<SmallEnoughStruct>(a));
    b.emplace<1>(3.0f);
    CHECK(b == CompactVariant<float, SmallEnoughStruct>(SmallEnoughStruct {3.0f}));

    a.swap(b);
    CHECK(a == CompactVariant<float, SmallEnoughStruct>(SmallEnoughStruct {3.0f}));
    CHECK(b == CompactVariant<float, SmallEnoughStruct>(SmallEnoughStruct {2.0f}));
  }

  TEST_CASE("FlatteningVariant flattens nested variants", "[FlatteningVariant]")
  {
    using Nested = Variant<int, float>;
    using Flat = FlatteningVariant<Nested, double>;
    STATIC_CHECK(SameType<Flat::Representation, Variant<int, float, double>>);

    Flat fromNested = Nested {2.5f};
    CHECK(fromNested.HoldsAlternative<float>());
    CHECK(
      SwitchOn(fromNested, [](int) { return 0; }, [](float) { return 1; }, [](double) { return 2; }) == 1);

    Flat fromDouble = 1.0;
    CHECK(fromDouble.HoldsAlternative<double>());
    CHECK_FALSE(fromDouble == fromNested);
    CHECK(fromDouble == Flat(1.0));

    // Adaptors flatten through each other.
    using Small = FlatteningVariant<Nested, uint16>;
    using Outer = FlatteningCompactVariant<Small, SmallEnoughStruct>;
    STATIC_CHECK(SameType<Outer::Representation, CompactVariant<int, float, uint16, SmallEnoughStruct>>);

    Outer outer = Small {Nested {1}};
    CHECK(outer.HoldsAlternative<int>());
    Outer outerStruct = SmallEnoughStruct {2.0f};
    CHECK(outerStruct.HoldsAlternative<SmallEnoughStruct>());
  }
}
