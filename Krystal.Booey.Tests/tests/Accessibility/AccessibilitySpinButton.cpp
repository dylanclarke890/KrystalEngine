#include "Krystal.Booey/Accessibility/AccessibilitySpinButton.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AccessibilitySpinButtonPart.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AccessibilitySpinButton: a number input's spin button",
            "[Accessibility][AccessibilitySpinButton]")
  {
    AccessibilityFixture page(u8"<input id=number type=number value=5><input id=text>");

    // A text field has no children; a number input has only its spin button.
    CHECK(page.Object(u8"text").Children().empty());
    auto &input = page.Object(u8"number");
    REQUIRE(input.Children().size() == 1);
    auto &spinButton = *input.Children()[0];
    REQUIRE(Is<AccessibilitySpinButton>(spinButton));
    CHECK(spinButton.Role() == AccessibilityRole::SpinButton);
    CHECK(spinButton.ParentObject() == &input);
    CHECK_FALSE(spinButton.ElementRect().IsEmpty());

    auto &spin = Downcast<AccessibilitySpinButton>(spinButton);
    auto *increment = spin.IncrementButton();
    auto *decrement = spin.DecrementButton();
    REQUIRE(increment);
    REQUIRE(decrement);
    CHECK(increment->Role() == AccessibilityRole::SpinButtonPart);
    CHECK(increment->IsIncrementor());
    CHECK_FALSE(decrement->IsIncrementor());
    CHECK(spinButton.Children().size() == 2);
    // The increment button is the top half, the decrement button the bottom.
    CHECK(increment->ElementRect().Y() < decrement->ElementRect().Y());

    auto &element = Downcast<html::HTMLInputElement>(page.ById(u8"number"));
    CHECK(increment->Press());
    CHECK(element.Value() == u8"6");
    CHECK(decrement->Press());
    CHECK(decrement->Press());
    CHECK(element.Value() == u8"4");
  }

  TEST_CASE("AccessibilitySpinButton: the parts go with the spin button",
            "[Accessibility][AccessibilitySpinButton]")
  {
    AccessibilityFixture page(u8"<input id=number type=number>");
    auto &input = page.Object(u8"number");
    auto spinButton = ShareRef(*input.Children()[0]);
    auto part = ShareRef(*spinButton->Children()[0]);
    input.ClearChildren();
    CHECK(spinButton->IsDetached());
    CHECK(part->IsDetached());
    // New ones are made with the input's children.
    REQUIRE(input.Children().size() == 1);
    CHECK(input.Children()[0].get() != spinButton.get());
  }
}
