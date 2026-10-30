#include "Krystal.Booey/Style/Values/Primitives/CoordinatedValueList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"

// A test layer with a base property, a reference property and an enum property. Its properties are keyed by
// ids that never form coordinated lists (`color`, `z-index`, `visibility`), so the accessor specialisations
// can't clash with real layers.

namespace krys::boo::style::tests
{
#define KRYS_FOR_EACH_TEST_LAYER_REFERENCE(macro)                                                            \
  macro(tests::TestLayer, Color, int, image, Image) macro(tests::TestLayer, ZIndex, int, size, Size)

#define KRYS_FOR_EACH_TEST_LAYER_ENUM(macro)                                                                 \
  macro(tests::TestLayer, Visibility, boo::Visibility, visibility, Visibility)

#define KRYS_FOR_EACH_TEST_LAYER_PROPERTY(macro)                                                             \
  KRYS_FOR_EACH_TEST_LAYER_REFERENCE(macro)                                                                  \
  KRYS_FOR_EACH_TEST_LAYER_ENUM(macro)

  struct TestLayer
  {
  private:
    int _image {InitialImage()};
    int _size {InitialSize()};
    uint8 _visibility : 2 {static_cast<uint8>(InitialVisibility())};

    KRYS_FOR_EACH_TEST_LAYER_PROPERTY(KRYS_DECLARE_COORDINATED_VALUE_LIST_IS_SET_AND_IS_FILLED_MEMBERS)

    // Needed by macros to access members.
    KRYS_NODISCARD TestLayer &Data() noexcept
    {
      return *this;
    }

    KRYS_NODISCARD const TestLayer &Data() const noexcept
    {
      return *this;
    }

  public:
    TestLayer() noexcept = default;

    explicit TestLayer(int image) noexcept
    {
      SetImage(krys::move(image));
    }

    KRYS_NODISCARD const int &GetImage() const noexcept
    {
      return _image;
    }

    KRYS_NODISCARD const int &GetSize() const noexcept
    {
      return _size;
    }

    KRYS_NODISCARD boo::Visibility GetVisibility() const noexcept
    {
      return static_cast<boo::Visibility>(_visibility);
    }

    KRYS_NODISCARD static int InitialImage() noexcept
    {
      return 0;
    }

    KRYS_NODISCARD static int InitialSize() noexcept
    {
      return -1;
    }

    KRYS_NODISCARD constexpr static boo::Visibility InitialVisibility() noexcept
    {
      return boo::Visibility::Visible;
    }

    KRYS_FOR_EACH_TEST_LAYER_REFERENCE(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_REFERENCE)
    KRYS_FOR_EACH_TEST_LAYER_ENUM(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_ENUM)

    KRYS_NODISCARD bool operator==(const TestLayer &other) const noexcept
    {
      return _image == other._image && _size == other._size && _visibility == other._visibility;
    }

    // CoordinatedValueList interface.

    constexpr static auto ComputedValueUsesUsedValues = true;
    constexpr static auto BaseProperty = css::PropertyNameConstant<css::PropertyId::Color> {};
    constexpr static auto Properties =
      std::tuple {KRYS_FOR_EACH_TEST_LAYER_PROPERTY(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY)};

    KRYS_NODISCARD static TestLayer Clone(const TestLayer &other) noexcept
    {
      return other;
    }

    KRYS_NODISCARD bool IsInitial() const noexcept
    {
      return _image == InitialImage();
    }
  };

}

namespace krys::boo::style
{
  KRYS_FOR_EACH_TEST_LAYER_REFERENCE(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_REFERENCE)
  KRYS_FOR_EACH_TEST_LAYER_ENUM(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_ENUM)
}

namespace krys::boo::style::tests
{

#undef KRYS_FOR_EACH_TEST_LAYER_REFERENCE
#undef KRYS_FOR_EACH_TEST_LAYER_ENUM
#undef KRYS_FOR_EACH_TEST_LAYER_PROPERTY

  static_assert(CoordinatedValueListValue<TestLayer>);

  TEST_CASE("CoordinatedValueList", "[Style][Values][CoordinatedValueList]")
  {
    SECTION("Property states")
    {
      TestLayer layer;
      CHECK(layer.IsImageUnset());
      CHECK(layer.IsInitial());
      CHECK(AllPropertiesAreUnsetOrFilled(layer));

      layer.SetSize(3);
      CHECK(layer.IsSizeSet());
      CHECK(layer.GetSize() == 3);
      CHECK_FALSE(AllNonBasePropertiesAreUnsetOrFilled(layer));

      layer.FillSize(4);
      CHECK(layer.IsSizeFilled());
      CHECK(AllPropertiesAreUnsetOrFilled(layer));

      layer.ClearSize();
      CHECK(layer.IsSizeUnset());
      CHECK(layer.GetSize() == TestLayer::InitialSize());

      layer.SetVisibility(Visibility::Hidden);
      CHECK(layer.GetVisibility() == Visibility::Hidden);
      CHECK(layer.VisibilityState() == CoordinatedValueListPropertyState::Set);
    }

    SECTION("Accessors")
    {
      TestLayer layer {5};
      CoordinatedValueListPropertyAccessor<css::PropertyId::Color> accessor {layer};
      CHECK(accessor.Get() == 5);
      CHECK(accessor.IsSet());
      accessor.Fill(6);
      CHECK(accessor.IsFilled());
      CHECK(CoordinatedValueListPropertyConstAccessor<css::PropertyId::Color> {layer}.Get() == 6);
      CHECK(CoordinatedValueListPropertyAccessor<css::PropertyId::Visibility>::Initial()
            == Visibility::Visible);
    }

    SECTION("Unset properties are filled by repeating the set values, and the used length follows the base")
    {
      CoordinatedValueList<TestLayer> list {TestLayer {1}, TestLayer {2}, TestLayer {3}};
      list[0].SetSize(10);
      list[1].SetSize(20);
      list[0].SetVisibility(Visibility::Hidden);
      list.PrepareForUse();

      CHECK(list.ComputedLength() == 3);
      CHECK(list.UsedLength() == 3);
      CHECK(list[2].GetSize() == 10);
      CHECK(list[2].IsSizeFilled());
      CHECK(list[1].GetVisibility() == Visibility::Hidden);
      CHECK(list[2].GetVisibility() == Visibility::Hidden);
      CHECK(list.UsedFirst().GetImage() == 1);
      CHECK(list.UsedLast().GetImage() == 3);
    }

    SECTION("Layers after the first with nothing set are removed")
    {
      CoordinatedValueList<TestLayer> list {TestLayer {1}, TestLayer {}, TestLayer {3}};
      list.PrepareForUse();
      CHECK(list.ComputedLength() == 1);
      CHECK(list.UsedLength() == 1);
    }

    SECTION("The used length counts the leading layers with the base property set")
    {
      CoordinatedValueList<TestLayer> list {TestLayer {1}, TestLayer {}};
      list[1].SetSize(7);
      list.PrepareForUse();
      CHECK(list.ComputedLength() == 2);
      CHECK(list.UsedLength() == 1);
      CHECK(list.UsedValues().size() == 1);
      CHECK(list.ComputedValues().size() == 2);
      CHECK(&list.UsedLast() == &list.UsedFirst());
      CHECK(list.ComputedLast().GetSize() == 7);
    }

    SECTION("Initial lists")
    {
      CHECK(CoordinatedValueList<TestLayer> {TestLayer {}}.IsInitial());
      CHECK_FALSE(CoordinatedValueList<TestLayer> {TestLayer {1}}.IsInitial());

      TestLayer withSize;
      withSize.SetSize(1);
      CHECK_FALSE(CoordinatedValueList<TestLayer> {krys::move(withSize)}.IsInitial());
    }

    SECTION("Copies share their data until accessed")
    {
      CoordinatedValueList<TestLayer> list {TestLayer {1}};
      auto copy = list;
      CHECK(copy == list);
      CHECK(&copy.ComputedFirst() == &list.ComputedFirst());

      copy.Access().Append(TestLayer {2});
      CHECK(&copy.ComputedFirst() != &list.ComputedFirst());
      CHECK(list.ComputedLength() == 1);
      CHECK(copy.ComputedLength() == 2);
      CHECK_FALSE(copy == list);

      // Equal data compares equal without being shared.
      CoordinatedValueList<TestLayer> other {TestLayer {1}};
      CHECK(other == list);
    }
  }
}
