#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTransformValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSMatrixComponent.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSPerspective.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSRotate.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSScale.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSSkew.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSSkewX.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSSkewY.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTranslate.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using Factory = CSSNumericFactory;

    template <typename T>
    KRYS_NODISCARD Ref<T> Value(dom::ExceptionOr<Ref<T>> &&result)
    {
      REQUIRE(result.HasValue());
      return result.ReleaseValue();
    }

    template <typename T>
    KRYS_NODISCARD dom::ExceptionCode Error(const dom::ExceptionOr<T> &result)
    {
      REQUIRE(result.HasException());
      return result.GetException().Code();
    }

    KRYS_NODISCARD std::string Text(const CSSTransformComponent &component)
    {
      return ToNarrow(component.ToString());
    }

    KRYS_NODISCARD Ref<DOMMatrix> Matrix(const CSSTransformComponent &component)
    {
      auto matrix = component.ToMatrix();
      REQUIRE(matrix.HasValue());
      return matrix.ReleaseValue();
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSTranslate", "[CSS][TypedOM][css-typed-om:csstranslate]")
  {
    auto translate = Value(CSSTranslate::Create(Factory::Px(10), Factory::Percent(50)));
    CHECK(translate->Is2D());
    CHECK(Text(*translate) == "translate(10px, 50%)");
    // A percentage has no matrix.
    CHECK(Error(translate->ToMatrix()) == dom::ExceptionCode::TypeError);

    auto translate3d = Value(CSSTranslate::Create(Factory::Px(1), Factory::Px(2), Factory::Px(3)));
    CHECK_FALSE(translate3d->Is2D());
    CHECK(Text(*translate3d) == "translate3d(1px, 2px, 3px)");
    CHECK(Matrix(*translate3d)->M43() == 3);

    CHECK(Error(CSSTranslate::Create(Factory::Px(1), Factory::Deg(1))) == dom::ExceptionCode::TypeError);
    CHECK(Error(CSSTranslate::Create(Factory::Px(1), Factory::Px(1), Factory::Percent(1)))
          == dom::ExceptionCode::TypeError);
    CHECK(Error(translate->SetX(Factory::Deg(1))) == dom::ExceptionCode::TypeError);
    CHECK(Error(translate->SetZ(Factory::Percent(1))) == dom::ExceptionCode::TypeError);
    CHECK_FALSE(translate->SetY(Factory::Px(4)).HasException());
    CHECK(Matrix(*translate)->F() == 4);
  }

  TEST_CASE_METHOD(ValueFixture, "CSSRotate", "[CSS][TypedOM][css-typed-om:cssrotate]")
  {
    auto rotate = Value(CSSRotate::Create(Factory::Deg(90)));
    CHECK(rotate->Is2D());
    CHECK(Text(*rotate) == "rotate(90deg)");
    CHECK(Matrix(*rotate)->B() == 1);

    auto rotate3d = Value(CSSRotate::Create(1.0, 0.0, 0.0, Factory::Deg(90)));
    CHECK_FALSE(rotate3d->Is2D());
    CHECK(Text(*rotate3d) == "rotate3d(1, 0, 0, 90deg)");

    CHECK(Error(CSSRotate::Create(Factory::Px(1))) == dom::ExceptionCode::TypeError);
    CHECK(Error(rotate->SetX(RefPtr<CSSNumericValue>(Factory::Px(1)))) == dom::ExceptionCode::TypeError);
    CHECK(Error(rotate->SetAngle(Factory::Px(1))) == dom::ExceptionCode::TypeError);
    // Setting is2D changes the serialisation.
    rotate->SetIs2D(false);
    CHECK(Text(*rotate) == "rotate3d(0, 0, 1, 90deg)");
  }

  TEST_CASE_METHOD(ValueFixture, "CSSScale", "[CSS][TypedOM][css-typed-om:cssscale]")
  {
    auto scale = Value(CSSScale::Create(2.0, 3.0));
    CHECK(Text(*scale) == "scale(2, 3)");
    auto matrix = Matrix(*scale);
    CHECK(matrix->A() == 2);
    CHECK(matrix->D() == 3);
    CHECK(Text(*Value(CSSScale::Create(1.0, 2.0, CSSNumberish {3.0}))) == "scale3d(1, 2, 3)");
    CHECK(Error(CSSScale::Create(RefPtr<CSSNumericValue>(Factory::Px(1)), 1.0))
          == dom::ExceptionCode::TypeError);
    CHECK(Error(scale->SetZ(RefPtr<CSSNumericValue>(Factory::Px(1)))) == dom::ExceptionCode::TypeError);
  }

  TEST_CASE_METHOD(ValueFixture, "CSSSkew, CSSSkewX and CSSSkewY", "[CSS][TypedOM][css-typed-om:cssskew]")
  {
    auto skew = Value(CSSSkew::Create(Factory::Deg(10), Factory::Deg(0)));
    // A zero ay is left out.
    CHECK(Text(*skew) == "skew(10deg)");
    CHECK_FALSE(skew->SetAy(Factory::Deg(5)).HasException());
    CHECK(Text(*skew) == "skew(10deg, 5deg)");
    // A skew is always 2D.
    skew->SetIs2D(false);
    CHECK(skew->Is2D());
    CHECK(Error(CSSSkew::Create(Factory::Px(1), Factory::Deg(0))) == dom::ExceptionCode::TypeError);

    auto skewX = Value(CSSSkewX::Create(Factory::Deg(45)));
    CHECK(Text(*skewX) == "skewX(45deg)");
    CHECK(Matrix(*skewX)->C() > 0.99);
    CHECK(Text(*Value(CSSSkewY::Create(Factory::Deg(45)))) == "skewY(45deg)");
    CHECK(Error(CSSSkewY::Create(Factory::Px(1))) == dom::ExceptionCode::TypeError);
  }

  TEST_CASE_METHOD(ValueFixture, "CSSPerspective", "[CSS][TypedOM][css-typed-om:cssperspective]")
  {
    auto perspective = Value(CSSPerspective::Create(RefPtr<CSSNumericValue>(Factory::Px(10))));
    CHECK_FALSE(perspective->Is2D());
    CHECK(Text(*perspective) == "perspective(10px)");
    CHECK(Matrix(*perspective)->M34() == -0.1);

    // A negative length is serialised in a calc().
    CHECK(Text(*Value(CSSPerspective::Create(RefPtr<CSSNumericValue>(Factory::Px(-5)))))
          == "perspective(calc(-5px))");

    // `none` matches in any case, and keeps the case it was given.
    auto none = Value(CSSPerspective::Create(CSSOMString(u8"NONE")));
    CHECK(Text(*none) == "perspective(NONE)");
    CHECK(Matrix(*none)->IsIdentity());

    CHECK(Error(CSSPerspective::Create(CSSOMString(u8"auto"))) == dom::ExceptionCode::TypeError);
    CHECK(Error(CSSPerspective::Create(RefPtr<CSSNumericValue>(Factory::Deg(1))))
          == dom::ExceptionCode::TypeError);
  }

  TEST_CASE_METHOD(ValueFixture, "CSSMatrixComponent", "[CSS][TypedOM][css-typed-om:cssmatrixcomponent]")
  {
    auto matrix =
      DOMMatrixReadOnly::Create(TransformationMatrix(1, 2, 3, 4, 5, 6), DOMMatrixReadOnly::Is2D::Yes);
    auto component = CSSMatrixComponent::Create(*matrix);
    CHECK(component->Is2D());
    CHECK(Text(*component) == "matrix(1, 2, 3, 4, 5, 6)");
    CHECK(Matrix(*component)->E() == 5);

    auto component3d = CSSMatrixComponent::Create(*matrix, {.Is2D = false});
    CHECK(Text(*component3d) == "matrix3d(1, 2, 0, 0, 3, 4, 0, 0, 0, 0, 1, 0, 5, 6, 0, 1)");
  }

  TEST_CASE_METHOD(ValueFixture, "CSSTransformValue", "[CSS][TypedOM][css-typed-om:csstransformvalue]")
  {
    CHECK(Error(CSSTransformValue::Create(List<Ref<CSSTransformComponent>> {}))
          == dom::ExceptionCode::TypeError);

    auto value = Value(CSSTransformValue::Create(
      {Value(CSSTranslate::Create(Factory::Px(10), Factory::Px(20))), Value(CSSScale::Create(2.0, 2.0))}));
    CHECK(value->Length() == 2);
    CHECK(value->Is2D());
    CHECK(ToNarrow(value->ToString()) == "translate(10px, 20px) scale(2, 2)");

    auto matrix = value->ToMatrix();
    REQUIRE(matrix.HasValue());
    CHECK(matrix.Value()->A() == 2);
    CHECK(matrix.Value()->E() == 10);
    CHECK(matrix.Value()->F() == 20);

    CHECK_FALSE(value->Item(2));
    CHECK(value->SetItem(2, Value(CSSRotate::Create(1.0, 0.0, 0.0, Factory::Deg(10)))).HasValue());
    CHECK_FALSE(value->Is2D());
    CHECK(Error(value->SetItem(4, Value(CSSSkewX::Create(Factory::Deg(1)))))
          == dom::ExceptionCode::RangeError);

    // It converts back to a transform list.
    auto cssValue = value->ToCSSValue();
    REQUIRE(cssValue);
    CHECK(CssText(*cssValue) == "translate(10px, 20px) scale(2, 2) rotate3d(1, 0, 0, 10deg)");

    SECTION("reified")
    {
      auto document = CreateRef<dom::Document>();
      auto parsed =
        CSSStyleValue::Parse(*document, u8"transform",
                             u8"translateX(1px) scaleY(2) rotateZ(3deg) skew(4deg) perspective(5px) "
                             u8"matrix(1, 0, 0, 1, 6, 7)");
      REQUIRE(parsed.HasValue());
      auto &transform = Downcast<CSSTransformValue>(*parsed.Value());
      REQUIRE(transform.Length() == 6);
      CHECK(Is<CSSTranslate>(*transform.Item(0)));
      CHECK(Is<CSSScale>(*transform.Item(1)));
      CHECK(Is<CSSRotate>(*transform.Item(2)));
      CHECK(Is<CSSSkew>(*transform.Item(3)));
      CHECK(Is<CSSPerspective>(*transform.Item(4)));
      CHECK(Is<CSSMatrixComponent>(*transform.Item(5)));
      CHECK(ToNarrow(transform.ToString())
            == "translate(1px, 0px) scale(1, 2) rotate3d(0, 0, 1, 3deg) skew(4deg) perspective(5px) "
               "matrix(1, 0, 0, 1, 6, 7)");
    }
  }
}
