#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"

namespace krys::boo
{
  FrameLoadRequestBase::FrameLoadRequestBase() noexcept = default;
  FrameLoadRequestBase::FrameLoadRequestBase(const FrameLoadRequestBase &) noexcept = default;
  FrameLoadRequestBase &FrameLoadRequestBase::operator=(const FrameLoadRequestBase &) noexcept = default;
  FrameLoadRequestBase::FrameLoadRequestBase(FrameLoadRequestBase &&) noexcept = default;
  FrameLoadRequestBase &FrameLoadRequestBase::operator=(FrameLoadRequestBase &&) noexcept = default;
  FrameLoadRequestBase::~FrameLoadRequestBase() noexcept = default;

  void FrameLoadRequestBase::SetSourceElement(dom::Element *sourceElement) noexcept
  {
    _sourceElement = ShareRefPtr(sourceElement);
  }

  FrameLoadRequest::FrameLoadRequest(Ref<dom::Document> &&requester, ResourceRequest &&resourceRequest,
                                     dom::DOMStringAtom frameName,
                                     dom::DOMStringAtom downloadAttribute) noexcept
      : _requester(krys::move(requester)), _resourceRequest(krys::move(resourceRequest)),
        _frameName(frameName)
  {
    SetDownloadAttribute(downloadAttribute);
  }

  FrameLoadRequest::~FrameLoadRequest() noexcept = default;
  FrameLoadRequest::FrameLoadRequest(const FrameLoadRequest &) noexcept = default;
  FrameLoadRequest &FrameLoadRequest::operator=(const FrameLoadRequest &) noexcept = default;
  FrameLoadRequest::FrameLoadRequest(FrameLoadRequest &&) noexcept = default;
  FrameLoadRequest &FrameLoadRequest::operator=(FrameLoadRequest &&) noexcept = default;
}
