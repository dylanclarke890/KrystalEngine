#include "Krystal.Booey/MathML/Internals/MathMLElementFactory.hpp"
#include "Krystal.Booey/MathML/MathMLFractionElement.hpp"
#include "Krystal.Booey/MathML/MathMLMathElement.hpp"
#include "Krystal.Booey/MathML/MathMLMencloseElement.hpp"
#include "Krystal.Booey/MathML/MathMLOperatorElement.hpp"
#include "Krystal.Booey/MathML/MathMLPaddedElement.hpp"
#include "Krystal.Booey/MathML/MathMLRootElement.hpp"
#include "Krystal.Booey/MathML/MathMLScriptsElement.hpp"
#include "Krystal.Booey/MathML/MathMLSpaceElement.hpp"
#include "Krystal.Booey/MathML/MathMLTokenElement.hpp"
#include "Krystal.Booey/MathML/MathMLUnderOverElement.hpp"

namespace krys::boo::mathml
{
  RefPtr<MathMLElement> MathMLElementFactory::TryCreate(dom::Document &document,
                                                        html::TagName elementName) noexcept
  {
    using html::TagName;
    using ScriptType = MathMLScriptsElement::ScriptType;

    switch (elementName)
    {
      case TagName::annotation:
      case TagName::annotation_xml:
      case TagName::mprescripts:
      case TagName::mtable:
      case TagName::mtd:
      case TagName::mtr:
      case TagName::none:
      {
        return CreateRefPtr<MathMLPresentationElement>(document);
      }
      case TagName::maction:
      case TagName::merror:
      case TagName::mphantom:
      case TagName::mrow:
      case TagName::mstyle:
      case TagName::semantics:
      {
        return CreateRefPtr<MathMLRowElement>(document);
      }
      case TagName::math:
      {
        return CreateRefPtr<MathMLMathElement>(document);
      }
      case TagName::menclose:
      {
        return CreateRefPtr<MathMLMencloseElement>(document);
      }
      case TagName::mfrac:
      {
        return CreateRefPtr<MathMLFractionElement>(document);
      }
      case TagName::mi:
      case TagName::mn:
      case TagName::ms:
      case TagName::mtext:
      {
        return CreateRefPtr<MathMLTokenElement>(document);
      }
      case TagName::mo:
      {
        return CreateRefPtr<MathMLOperatorElement>(document);
      }
      case TagName::mpadded:
      {
        return CreateRefPtr<MathMLPaddedElement>(document);
      }
      case TagName::mroot:
      {
        return CreateRefPtr<MathMLRootElement>(document, RootType::RootWithIndex);
      }
      case TagName::msqrt:
      {
        return CreateRefPtr<MathMLRootElement>(document, RootType::SquareRoot);
      }
      case TagName::mspace:
      {
        return CreateRefPtr<MathMLSpaceElement>(document);
      }
      case TagName::msub:
      {
        return CreateRefPtr<MathMLScriptsElement>(document, ScriptType::Sub);
      }
      case TagName::msup:
      {
        return CreateRefPtr<MathMLScriptsElement>(document, ScriptType::Super);
      }
      case TagName::msubsup:
      {
        return CreateRefPtr<MathMLScriptsElement>(document, ScriptType::SubSup);
      }
      case TagName::mmultiscripts:
      {
        return CreateRefPtr<MathMLScriptsElement>(document, ScriptType::Multiscripts);
      }
      case TagName::munder:
      {
        return CreateRefPtr<MathMLUnderOverElement>(document, ScriptType::Under);
      }
      case TagName::mover:
      {
        return CreateRefPtr<MathMLUnderOverElement>(document, ScriptType::Over);
      }
      case TagName::munderover:
      {
        return CreateRefPtr<MathMLUnderOverElement>(document, ScriptType::UnderOver);
      }
      default:
      {
        break;
      }
    }

    return nullptr;
  }
}
