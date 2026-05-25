#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  enum class HTMLTagName : uint16
  {
    Unknown,
    A,
    Abbr,
    Acronym,
    Address,
    Applet,
    Area,
    Article,
    Aside,
    Attachment,
    B,
    Base,
    Basefont,
    Bdi,
    Bdo,
    Bgsound,
    Big,
    Blink,
    Blockquote,
    Body,
    Br,
    Button,
    Canvas,
    Caption,
    Center,
    Cite,
    Code,
    Col,
    Colgroup,
    Data,
    Datalist,
    Dd,
    Del,
    Details,
    Dfn,
    Dialog,
    Dir,
    Div,
    Dl,
    Dt,
    Em,
    Embed,
    Fieldset,
    Figcaption,
    Figure,
    Font,
    Footer,
    Form,
    Frame,
    Frameset,
    H1,
    H2,
    H3,
    H4,
    H5,
    H6,
    Head,
    Header,
    Hgroup,
    Hr,
    Html,
    I,
    Iframe,
    Image,
    Img,
    Input,
    Ins,
    Isindex,
    Kbd,
    Keygen,
    Label,
    Legend,
    Li,
    Link,
    Listing,
    Main,
    Map,
    Mark,
    Marquee,
    Menu,
    Meta,
    Meter,
    Model,
    Multicol,
    Nav,
    Nextid,
    Nobr,
    Noembed,
    Noframes,
    Object,
    Ol,
    Optgroup,
    Option,
    Output,
    P,
    Param,
    Picture,
    Plaintext,
    Pre,
    Progress,
    Q,
    Rb,
    Rp,
    Rt,
    Rtc,
    Ruby,
    S,
    Samp,
    Script,
    Search,
    Section,
    Select,
    Selectedcontent,
    Slot,
    Small,
    Source,
    Spacer,
    Span,
    Strike,
    Strong,
    Style,
    Sub,
    Summary,
    Sup,
    Table,
    Tbody,
    Td,
    Template,
    Textarea,
    Tfoot,
    Th,
    Thead,
    Time,
    Title,
    Tr,
    Track,
    Tt,
    U,
    Ul,
    Var,
    Video,
    Wbr,
    Xmp,
    Noscript,
    AnnotationXml,
    ColorProfile,
    FontFace,
    FontFaceFormat,
    FontFaceName,
    FontFaceSrc,
    FontFaceUri,
    MissingGlyph,
  };

  KRYS_NODISCARD constexpr Maybe<HTMLTagName> TryParseHTMLTagName(DOMStringView tagName) noexcept
  {
    if (tagName.empty())
    {
      return Null;
    }

    switch (tagName.size())
    {
      case 1:
      {
        switch (tagName[0])
        {
          case u8'A':
          {
            return HTMLTagName::A;
          }
          case u8'B':
          {
            return HTMLTagName::B;
          }
          case u8'I':
          {
            return HTMLTagName::I;
          }
          case u8'P':
          {
            return HTMLTagName::P;
          }
          case u8'Q':
          {
            return HTMLTagName::Q;
          }
          case u8'S':
          {
            return HTMLTagName::S;
          }
          case u8'U':
          {
            return HTMLTagName::U;
          }
        }
        break;
      }
      case 2:
      {
        if (tagName == u8"BR")
        {
          return HTMLTagName::Br;
        }
        if (tagName == u8"DD")
        {
          return HTMLTagName::Dd;
        }
        if (tagName == u8"DL")
        {
          return HTMLTagName::Dl;
        }
        if (tagName == u8"DT")
        {
          return HTMLTagName::Dt;
        }
        if (tagName == u8"EM")
        {
          return HTMLTagName::Em;
        }
        if (tagName == u8"HR")
        {
          return HTMLTagName::Hr;
        }
        if (tagName == u8"H1")
        {
          return HTMLTagName::H1;
        }
        if (tagName == u8"H2")
        {
          return HTMLTagName::H2;
        }
        if (tagName == u8"H3")
        {
          return HTMLTagName::H3;
        }
        if (tagName == u8"H4")
        {
          return HTMLTagName::H4;
        }
        if (tagName == u8"H5")
        {
          return HTMLTagName::H5;
        }
        if (tagName == u8"H6")
        {
          return HTMLTagName::H6;
        }
        if (tagName == u8"LI")
        {
          return HTMLTagName::Li;
        }
        if (tagName == u8"OL")
        {
          return HTMLTagName::Ol;
        }
        if (tagName == u8"UL")
        {
          return HTMLTagName::Ul;
        }
        if (tagName == u8"RB")
        {
          return HTMLTagName::Rb;
        }
        if (tagName == u8"RP")
        {
          return HTMLTagName::Rp;
        }
        if (tagName == u8"RT")
        {
          return HTMLTagName::Rt;
        }
        if (tagName == u8"TD")
        {
          return HTMLTagName::Td;
        }
        if (tagName == u8"TH")
        {
          return HTMLTagName::Th;
        }
        if (tagName == u8"TR")
        {
          return HTMLTagName::Tr;
        }
        if (tagName == u8"TT")
        {
          return HTMLTagName::Tt;
        }
        if (tagName == u8"UL")
        {
          return HTMLTagName::Ul;
        }
        break;
      }
      case 3:
      {
        if (tagName == u8"BDI")
        {
          return HTMLTagName::Bdi;
        }
        if (tagName == u8"BDO")
        {
          return HTMLTagName::Bdo;
        }
        if (tagName == u8"BIG")
        {
          return HTMLTagName::Big;
        }
        if (tagName == u8"COL")
        {
          return HTMLTagName::Col;
        }
        if (tagName == u8"DEL")
        {
          return HTMLTagName::Del;
        }
        if (tagName == u8"DFN")
        {
          return HTMLTagName::Dfn;
        }
        if (tagName == u8"DIR")
        {
          return HTMLTagName::Dir;
        }
        if (tagName == u8"DIV")
        {
          return HTMLTagName::Div;
        }
        if (tagName == u8"IMG")
        {
          return HTMLTagName::Img;
        }
        if (tagName == u8"INS")
        {
          return HTMLTagName::Ins;
        }
        if (tagName == u8"KBD")
        {
          return HTMLTagName::Kbd;
        }
        if (tagName == u8"MAP")
        {
          return HTMLTagName::Map;
        }
        if (tagName == u8"NAV")
        {
          return HTMLTagName::Nav;
        }
        if (tagName == u8"PRE")
        {
          return HTMLTagName::Pre;
        }
        if (tagName == u8"RTC")
        {
          return HTMLTagName::Rtc;
        }
        if (tagName == u8"SUB")
        {
          return HTMLTagName::Sub;
        }
        if (tagName == u8"SUP")
        {
          return HTMLTagName::Sup;
        }
        if (tagName == u8"VAR")
        {
          return HTMLTagName::Var;
        }
        if (tagName == u8"WBR")
        {
          return HTMLTagName::Wbr;
        }
        if (tagName == u8"XMP")
        {
          return HTMLTagName::Xmp;
        }
        break;
      }
      case 4:
      {
        if (tagName == u8"ABBR")
        {
          return HTMLTagName::Abbr;
        }
        if (tagName == u8"AREA")
        {
          return HTMLTagName::Area;
        }
        if (tagName == u8"BASE")
        {
          return HTMLTagName::Base;
        }
        if (tagName == u8"BODY")
        {
          return HTMLTagName::Body;
        }
        if (tagName == u8"CITE")
        {
          return HTMLTagName::Cite;
        }
        if (tagName == u8"CODE")
        {
          return HTMLTagName::Code;
        }
        if (tagName == u8"DATA")
        {
          return HTMLTagName::Data;
        }
        if (tagName == u8"FONT")
        {
          return HTMLTagName::Font;
        }
        if (tagName == u8"FORM")
        {
          return HTMLTagName::Form;
        }
        if (tagName == u8"HEAD")
        {
          return HTMLTagName::Head;
        }
        if (tagName == u8"HTML")
        {
          return HTMLTagName::Html;
        }
        if (tagName == u8"LINK")
        {
          return HTMLTagName::Link;
        }
        if (tagName == u8"MAIN")
        {
          return HTMLTagName::Main;
        }
        if (tagName == u8"MARK")
        {
          return HTMLTagName::Mark;
        }
        if (tagName == u8"MENU")
        {
          return HTMLTagName::Menu;
        }
        if (tagName == u8"META")
        {
          return HTMLTagName::Meta;
        }
        if (tagName == u8"NOBR")
        {
          return HTMLTagName::Nobr;
        }
        if (tagName == u8"RUBY")
        {
          return HTMLTagName::Ruby;
        }
        if (tagName == u8"SAMP")
        {
          return HTMLTagName::Samp;
        }
        if (tagName == u8"SLOT")
        {
          return HTMLTagName::Slot;
        }
        if (tagName == u8"SPAN")
        {
          return HTMLTagName::Span;
        }
        if (tagName == u8"TIME")
        {
          return HTMLTagName::Time;
        }
        break;
      }
      case 5:
      {
        if (tagName == u8"ASIDE")
        {
          return HTMLTagName::Aside;
        }
        if (tagName == u8"BLINK")
        {
          return HTMLTagName::Blink;
        }
        if (tagName == u8"EMBED")
        {
          return HTMLTagName::Embed;
        }
        if (tagName == u8"FRAME")
        {
          return HTMLTagName::Frame;
        }
        if (tagName == u8"IMAGE")
        {
          return HTMLTagName::Image;
        }
        if (tagName == u8"INPUT")
        {
          return HTMLTagName::Input;
        }
        if (tagName == u8"LABEL")
        {
          return HTMLTagName::Label;
        }
        if (tagName == u8"METER")
        {
          return HTMLTagName::Meter;
        }
        if (tagName == u8"MODEL")
        {
          return HTMLTagName::Model;
        }
        if (tagName == u8"PARAM")
        {
          return HTMLTagName::Param;
        }
        if (tagName == u8"SMALL")
        {
          return HTMLTagName::Small;
        }
        if (tagName == u8"STYLE")
        {
          return HTMLTagName::Style;
        }
        if (tagName == u8"TABLE")
        {
          return HTMLTagName::Table;
        }
        if (tagName == u8"TBODY")
        {
          return HTMLTagName::Tbody;
        }
        if (tagName == u8"TFOOT")
        {
          return HTMLTagName::Tfoot;
        }
        if (tagName == u8"THEAD")
        {
          return HTMLTagName::Thead;
        }
        if (tagName == u8"TITLE")
        {
          return HTMLTagName::Title;
        }
        if (tagName == u8"TRACK")
        {
          return HTMLTagName::Track;
        }
        if (tagName == u8"VIDEO")
        {
          return HTMLTagName::Video;
        }
        break;
      }
      case 6:
      {
        if (tagName == u8"APPLET")
        {
          return HTMLTagName::Applet;
        }
        if (tagName == u8"BUTTON")
        {
          return HTMLTagName::Button;
        }
        if (tagName == u8"CANVAS")
        {
          return HTMLTagName::Canvas;
        }
        if (tagName == u8"CENTER")
        {
          return HTMLTagName::Center;
        }
        if (tagName == u8"DIALOG")
        {
          return HTMLTagName::Dialog;
        }
        if (tagName == u8"FIGURE")
        {
          return HTMLTagName::Figure;
        }
        if (tagName == u8"FOOTER")
        {
          return HTMLTagName::Footer;
        }
        if (tagName == u8"HEADER")
        {
          return HTMLTagName::Header;
        }
        if (tagName == u8"HGROUP")
        {
          return HTMLTagName::Hgroup;
        }
        if (tagName == u8"IFRAME")
        {
          return HTMLTagName::Iframe;
        }
        if (tagName == u8"KEYGEN")
        {
          return HTMLTagName::Keygen;
        }
        if (tagName == u8"LEGEND")
        {
          return HTMLTagName::Legend;
        }
        if (tagName == u8"NEXTID")
        {
          return HTMLTagName::Nextid;
        }
        if (tagName == u8"OBJECT")
        {
          return HTMLTagName::Object;
        }
        if (tagName == u8"OPTION")
        {
          return HTMLTagName::Option;
        }
        if (tagName == u8"OUTPUT")
        {
          return HTMLTagName::Output;
        }
        if (tagName == u8"SCRIPT")
        {
          return HTMLTagName::Script;
        }
        if (tagName == u8"SEARCH")
        {
          return HTMLTagName::Search;
        }
        if (tagName == u8"SELECT")
        {
          return HTMLTagName::Search;
        }
        if (tagName == u8"SOURCE")
        {
          return HTMLTagName::Source;
        }
        if (tagName == u8"SPACER")
        {
          return HTMLTagName::Spacer;
        }
        if (tagName == u8"STRIKE")
        {
          return HTMLTagName::Source;
        }
        if (tagName == u8"STRONG")
        {
          return HTMLTagName::Source;
        }
        break;
      }
      case 7:
      {
        if (tagName == u8"ACRONYM")
        {
          return HTMLTagName::Acronym;
        }
        if (tagName == u8"ADDRESS")
        {
          return HTMLTagName::Address;
        }
        if (tagName == u8"ARTICLE")
        {
          return HTMLTagName::Article;
        }
        if (tagName == u8"BGSOUND")
        {
          return HTMLTagName::Bgsound;
        }
        if (tagName == u8"CAPTION")
        {
          return HTMLTagName::Caption;
        }
        if (tagName == u8"ISINDEX")
        {
          return HTMLTagName::Isindex;
        }
        if (tagName == u8"LISTING")
        {
          return HTMLTagName::Listing;
        }
        if (tagName == u8"MARQUEE")
        {
          return HTMLTagName::Marquee;
        }
        if (tagName == u8"NOEMBED")
        {
          return HTMLTagName::Noembed;
        }
        if (tagName == u8"PICTURE")
        {
          return HTMLTagName::Picture;
        }
        if (tagName == u8"SECTION")
        {
          return HTMLTagName::Section;
        }
        if (tagName == u8"SUMMARY")
        {
          return HTMLTagName::Section;
        }
        break;
      }
      case 8:
      {
        if (tagName == u8"BASEFONT")
        {
          return HTMLTagName::Basefont;
        }
        if (tagName == u8"COLGROUP")
        {
          return HTMLTagName::Colgroup;
        }
        if (tagName == u8"DATALIST")
        {
          return HTMLTagName::Datalist;
        }
        if (tagName == u8"DETAILS")
        {
          return HTMLTagName::Details;
        }
        if (tagName == u8"FIELDSET")
        {
          return HTMLTagName::Fieldset;
        }
        if (tagName == u8"FRAMESET")
        {
          return HTMLTagName::Frameset;
        }
        if (tagName == u8"MULTICOL")
        {
          return HTMLTagName::Multicol;
        }
        if (tagName == u8"NOFRAMES")
        {
          return HTMLTagName::Noframes;
        }
        if (tagName == u8"OPTGROUP")
        {
          return HTMLTagName::Optgroup;
        }
        if (tagName == u8"PROGRESS")
        {
          return HTMLTagName::Progress;
        }
        if (tagName == u8"TEMPLATE")
        {
          return HTMLTagName::Template;
        }
        if (tagName == u8"TEXTAREA")
        {
          return HTMLTagName::Textarea;
        }
        if (tagName == u8"NOSCRIPT")
        {
          return HTMLTagName::Noscript;
        }
      }
      case 9:
      {
        if (tagName == u8"PLAINTEXT")
        {
          return HTMLTagName::Plaintext;
        }
        if (tagName == u8"FONT-FACE")
        {
          return HTMLTagName::FontFace;
        }
        break;
      }
      case 10:
      {
        if (tagName == u8"ATTACHMENT")
        {
          return HTMLTagName::Attachment;
        }
        if (tagName == u8"BLOCKQUOTE")
        {
          return HTMLTagName::Blockquote;
        }
        if (tagName == u8"FIGCAPTION")
        {
          return HTMLTagName::Figcaption;
        }
        break;
      }
      case 13:
      {
        if (tagName == u8"COLOR-PROFILE")
        {
          return HTMLTagName::ColorProfile;
        }
        if (tagName == u8"FONT-FACE-SRC")
        {
          return HTMLTagName::FontFaceSrc;
        }
        if (tagName == u8"FONT-FACE-URI")
        {
          return HTMLTagName::FontFaceUri;
        }
        if (tagName == u8"MISSING-GLYPH")
        {
          return HTMLTagName::MissingGlyph;
        }
      }
      case 14:
      {
        if (tagName == u8"FONT-FACE-NAME")
        {
          return HTMLTagName::FontFaceName;
        }
        if (tagName == u8"ANNOTATION-XML")
        {
          return HTMLTagName::AnnotationXml;
        }
      }
      case 15:
      {
        if (tagName == u8"SELECTEDCONTENT")
        {
          return HTMLTagName::Selectedcontent;
        }
      }
      case 16:
      {
        if (tagName == u8"FONT-FACE-FORMAT")
        {
          return HTMLTagName::FontFaceFormat;
        }
        break;
      }
    }

    return Null;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::HTMLTagName, 152u);