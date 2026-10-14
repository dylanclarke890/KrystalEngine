#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"
#include "Krystal.Booey/CSS/Values/UrlValue.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"

namespace krys::boo::style
{
  // Possible states/transitions:
  //
  // 1. empty (empty string)
  //      CSS:    [.specified = "", .resolved = "" ]
  //      Style:  [.resolved = ""]
  // 2. local (starts with #)
  //      CSS:    [.specified = "#foo", .resolved = "#foo"]
  //      Style:  [.resolved = "#foo"]
  // 3. relative, in stylesheet
  //      CSS:    [.specified = "foo/bar.png", .resolved = { stylesheet-base-url, "foo/bar.png" }]
  //      Style:  [.resolved = { stylesheet-base-url, "foo/bar.png" }]
  // 4. relative, in document with base-url
  //      CSS:    [.specified = "foo/bar.png", .resolved = { "foo/bar.png", document-base-url }]
  //      Style:  [.resolved = { document-base-url, "foo/bar.png" }]
  // 5. relative, in document without base-url
  //      CSS:    [.specified = "foo/bar.png", .resolved = null-url]
  //      Style:  [.resolved = "foo/bar.png"]

  URL ToStyleWithScriptExecutionContext(const css::CSSURL &url, const dom::Document &context) noexcept
  {
    if (url.Resolved.IsNull())
    {
      return {
        .Resolved = context.CompleteURL(url.Specified),
        .Modifiers = url.Modifiers,
      };
    }

    return {
      .Resolved = url.Resolved,
      .Modifiers = url.Modifiers,
    };
  }

  css::CSSURL ToCSS<URL>::operator()(const URL &url, const RenderStyle &) noexcept
  {
    return {
      .Specified = url.Resolved.String(),
      .Resolved = url.Resolved,
      .Modifiers = url.Modifiers,
    };
  }

  URL ToStyle<css::CSSURL>::operator()(const css::CSSURL &url, const BuilderState &state) noexcept
  {
    return ToStyleWithScriptExecutionContext(url, state.GetDocument());
  }

  Ref<css::Value> CSSValueCreation<URL>::operator()(css::ValuePool &, const RenderStyle &style,
                                                    const URL &value) noexcept
  {
    return css::UrlValue::Create(ConvertToCSS(value, style));
  }

  URL CSSValueConversion<URL>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (auto *url = RequiredDowncast<css::UrlValue>(state, value))
    {
      return ConvertToStyle(url->URL(), state);
    }
    return {.Resolved = url::URL {u8""}, .Modifiers = {}};
  }

  // MARK: - Serialisation

  void Serialiser<URL>::operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                                   const RenderStyle &style, const URL &value) noexcept
  {
    css::Serialise(builder, context, ConvertToCSS(value, style));
  }
}
