#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"

namespace krys::boo::html
{
  /// @brief Who a list button tells it was clicked: its text field's input type.
  class DataListButtonOwner
  {
  public:
    virtual ~DataListButtonOwner() noexcept = default;

    virtual void DataListButtonElementWasClicked() noexcept = 0;
  };

  /// @brief The button at the end of a text field with a datalist (`::-webkit-list-button`), which shows its
  /// suggestions. It isn't displayed while the field has no datalist.
  class DataListButtonElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DataListButtonElement);

  private:
    DataListButtonOwner *_owner {nullptr};

  public:
    DataListButtonElement(dom::Document &document, DataListButtonOwner &owner) noexcept;

    KRYS_NODISCARD static Ref<DataListButtonElement> Create(dom::Document &document,
                                                            DataListButtonOwner &owner) noexcept;

    /// @brief The owner's going away.
    void RemoveOwner() noexcept
    {
      _owner = nullptr;
    }

    void DefaultEventHandler(dom::Event &event) noexcept override;

    /// @brief Disabled with its field.
    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept override;

  private:
    KRYS_NODISCARD Maybe<style::UnadjustedStyle>
      ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                         const RenderStyle *shadowHostStyle) noexcept override;
  };
}
