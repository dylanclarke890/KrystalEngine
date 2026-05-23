#include "Krystal.HTML.Tests/ParserTestUtils.hpp"
#include "Krystal.HTML/Parser/HTMLTokenizer.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  INTEGRATION_TEST(
    "_ExampleMarkup1",
    (IntegrationTest {
      .Input = UR"html(<!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link type="text/css" href="index.css">
            <title>Document</title>
        </head>
        <body style="background-color: maroon;">
            <div style="
              box-sizing: border-box;">Hello, Krystal UI</div>
            <script type="javascript" src="index.js"></script>
        </body
>
        </html>)html",
      .Output =
        {
          CreateDOCTYPEToken({.Name = U"html"}),
          CreateCharacterToken(U"\n        "),
          CreateStartTagToken({.Name = U"html", .Attributes = {{.Name = U"lang", .Value = U"en"}}}),
          CreateCharacterToken(U"\n        "),
          CreateStartTagToken({.Name = U"head"}),
          CreateCharacterToken(U"\n            "),
          CreateStartTagToken({.Name = U"meta", .Attributes = {{.Name = U"charset", .Value = U"UTF-8"}}}),
          CreateCharacterToken(U"\n            "),
          CreateStartTagToken({.Name = U"meta",
                               .Attributes = {{.Name = U"name", .Value = U"viewport"},
                                              {.Name = U"content",
                                               .Value = U"width=device-width, initial-scale=1.0"}}}),
          CreateCharacterToken(U"\n            "),
          CreateStartTagToken({.Name = U"link",
                               .Attributes = {{.Name = U"type", .Value = U"text/css"},
                                              {.Name = U"href", .Value = U"index.css"}}}),
          CreateCharacterToken(U"\n            "),
          CreateStartTagToken({.Name = U"title"}),
          CreateCharacterToken(U"Document"),
          CreateEndTagToken({.Name = U"title"}),
          CreateCharacterToken(U"\n        "),
          CreateEndTagToken({.Name = U"head"}),
          CreateCharacterToken(U"\n        "),
          CreateStartTagToken({.Name = U"body",
                               .Attributes = {{.Name = U"style", .Value = U"background-color: maroon;"}}}),
          CreateCharacterToken(U"\n            "),
          CreateStartTagToken({.Name = U"div",
                               .Attributes = {{.Name = U"style",
                                               .Value = U"\n              box-sizing: border-box;"}}}),
          CreateCharacterToken(U"Hello, Krystal UI"),
          CreateEndTagToken({.Name = U"div"}),
          CreateCharacterToken(U"\n            "),
          CreateStartTagToken({.Name = U"script",
                               .Attributes = {{.Name = U"type", .Value = U"javascript"},
                                              {.Name = U"src", .Value = U"index.js"}}}),
          CreateEndTagToken({.Name = U"script"}),
          CreateCharacterToken(U"\n        "),
          CreateEndTagToken({.Name = U"body"}),
          CreateCharacterToken(U"\n        "),
          CreateEndTagToken({.Name = U"html"}),
          CreateEOFToken(),
        },
      .Errors = {},
    }));
}