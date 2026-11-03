# HTML review checklist: 8.11 Images

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `Krystal.Booey/tools/spec_checklist.py`: 19 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python Krystal.Booey/tools/spec_status.py html imagebitmap-and-animations` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python Krystal.Booey/tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [8.11 Images](https://html.spec.whatwg.org/#images-2)

Status: reviewed (2026-11-01)
> The sections below.

#### [8.11.1 The ImageData interface](https://html.spec.whatwg.org/#the-imagedata-interface)

Status: reviewed (2026-11-01)
> `html::ImageData`'s constructors (`ImageData::Create`); every pixel format has 8 bits a channel and there's no `Float16Array` (see `KNOWN-ISSUES.md`). There's no structured serialization.
<!-- definitions: concept-imagedata-bitmap-representation initialize-imagedata-source initialize-imagedata-defaultcolorspace dom-imagedata-data dom-imagedata-width dom-imagedata-height dom-imagedata-pixelformat dom-imagedatasettings-pixelformat dom-imagedata-colorspace dom-imagedatapixelformat-rgba-unorm8 dom-imagedatapixelformat-rgba-float16 -->

- [x] [ImageData.new ImageData(sw, sh, settings)](https://html.spec.whatwg.org/#dom-imagedata) — The new ImageData(sw, sh, settings) constructor steps are:
  - 1. If one or both of sw and sh are zero, then throw an "IndexSizeError" DOMException.
  - 2. Initialize this given sw, sh, and settings.
  - 3. Initialize the image data of this to transparent black.

- [x] [ImageData.new ImageData(data, sw, sh, settings)](https://html.spec.whatwg.org/#dom-imagedata-with-data) — The new ImageData(data, sw, sh, settings) constructor steps are:
  - 1. Let bytesPerPixel be 4 if settings["pixelFormat"] is "rgba-unorm8"; otherwise 8.
  - 2. Let length be the buffer source byte length of data.
  - 3. If length is not a nonzero integral multiple of bytesPerPixel, then throw an "InvalidStateError" DOMException.
  - 4. Let length be length divided by bytesPerPixel.
  - 5. If length is not an integral multiple of sw, then throw an "IndexSizeError" DOMException.
  - 6. Let height be length divided by sw.
  - 7. If sh was given and its value is not equal to height, then throw an "IndexSizeError" DOMException.
  - 8. Initialize this given sw, sh, settings, and source set to data.

- [x] [initialize an ImageData object](https://html.spec.whatwg.org/#initialize-an-imagedata-object) — To initialize an ImageData object imageData, given a positive integer number of pixels per row pixelsPerRow, a positive integer number of rows rows, an ImageDataSettings settings, an optional ImageDataArray source, and an optional PredefinedColorSpace defaultColorSpace:
  - 1. If source was given:
    - 1a. If settings["pixelFormat"] equals "rgba-unorm8" and source is not a Uint8ClampedArray, then throw an "InvalidStateError" DOMException.
    - 1b. If settings["pixelFormat"] is "rgba-float16" and source is not a Float16Array, then throw an "InvalidStateError" DOMException.
    - 1c. Initialize the data attribute of imageData to source.
  - 2. Otherwise (source was not given):
    - 2a. If settings["pixelFormat"] is "rgba-unorm8", then initialize the data attribute of imageData to a new Uint8ClampedArray object. The Uint8ClampedArray object must use a new ArrayBuffer for its storage, and must have a zero byte offset and byte length equal to the length of its storage, in bytes. The storage ArrayBuffer must have a length of 4 × rows × pixelsPerRow bytes.
    - 2b. Otherwise, if settings["pixelFormat"] is "rgba-float16", then initialize the data attribute of imageData to a new Float16Array object. The Float16Array object must use a new ArrayBuffer for its storage, and must have a zero byte offset and byte length equal to the length of its storage, in bytes. The storage ArrayBuffer must have a length of 8 × rows × pixelsPerRow bytes.
    - 2c. If the storage ArrayBuffer could not be allocated, then rethrow the RangeError thrown by JavaScript, and return.
  - 3. Initialize the width attribute of imageData to pixelsPerRow.
  - 4. Initialize the height attribute of imageData to rows.
  - 5. Initialize the pixelFormat attribute of imageData to settings["pixelFormat"].
  - 6. If settings["colorSpace"] exists, then initialize the colorSpace attribute of imageData to settings["colorSpace"].
  - 7. Otherwise, if defaultColorSpace was given, then initialize the colorSpace attribute of imageData to defaultColorSpace.
  - 8. Otherwise, initialize the colorSpace attribute of imageData to "srgb".

- [x] [ImageData objects are serializable objects. Their serialization steps, given va…](https://html.spec.whatwg.org/#dom-imagedatasettings-colorspace) — ImageData objects are serializable objects. Their serialization steps, given value and serialized, are:
  - 1. Set serialized.[[Data]] to the sub-serialization of the value of value's data attribute.
  - 2. Set serialized.[[Width]] to the value of value's width attribute.
  - 3. Set serialized.[[Height]] to the value of value's height attribute.
  - 4. Set serialized.[[ColorSpace]] to the value of value's colorSpace attribute.
  - 5. Set serialized.[[PixelFormat]] to the value of value's pixelFormat attribute.

- [x] [Their deserialization steps, given serialized, value, and targetRealm,](https://html.spec.whatwg.org/#dom-imagedatasettings-colorspace) — Their deserialization steps, given serialized, value, and targetRealm, are:
  - 1. Initialize value's data attribute to the sub-deserialization of serialized.[[Data]].
  - 2. Initialize value's width attribute to serialized.[[Width]].
  - 3. Initialize value's height attribute to serialized.[[Height]].
  - 4. Initialize value's colorSpace attribute to serialized.[[ColorSpace]].
  - 5. Initialize value's pixelFormat attribute to serialized.[[PixelFormat]].

#### [8.11.2 The ImageBitmap interface](https://html.spec.whatwg.org/#the-imagebitmap-interface)

Status: n/a (there is no `ImageBitmap` or `createImageBitmap`, see `KNOWN-ISSUES.md`)
<!-- definitions: concept-imagebitmap-bitmap-data bitmap-task-source dom-imagebitmapoptions-resizewidth dom-imagebitmapoptions-resizeheight dom-imagebitmapoptions-resizequality dom-imagebitmapoptions-imageorientation dom-imageorientation-flipy dom-imagebitmapoptions-colorspaceconversion dom-colorspaceconversion-default dom-colorspaceconversion-none dom-imagebitmapoptions-premultiplyalpha dom-premultiplyalpha-default dom-premultiplyalpha-premultiply dom-premultiplyalpha-none dom-resizequality-pixelated dom-resizequality-low dom-resizequality-medium dom-resizequality-high -->

- [x] [Their serialization steps, given value and serialized,](https://html.spec.whatwg.org/#concept-imagebitmap-good) — Their serialization steps, given value and serialized, are:
  - 1. If value's origin-clean flag is not set, then throw a "DataCloneError" DOMException.
  - 2. Set serialized.[[BitmapData]] to a copy of value's bitmap data.

- [x] [Their deserialization steps, given serialized, value, and targetRealm,](https://html.spec.whatwg.org/#concept-imagebitmap-good) — Their deserialization steps, given serialized, value, and targetRealm, are:
  - 1. Set value's bitmap data to serialized.[[BitmapData]].

- [x] [Their transfer steps, given value and dataHolder,](https://html.spec.whatwg.org/#concept-imagebitmap-good) — Their transfer steps, given value and dataHolder, are:
  - 1. If value's origin-clean flag is not set, then throw a "DataCloneError" DOMException.
  - 2. Set dataHolder.[[BitmapData]] to value's bitmap data.
  - 3. Unset value's bitmap data.

- [x] [Their transfer-receiving steps, given dataHolder and value,](https://html.spec.whatwg.org/#concept-imagebitmap-good) — Their transfer-receiving steps, given dataHolder and value, are:
  - 1. Set value's bitmap data to dataHolder.[[BitmapData]].

- [x] [WindowOrWorkerGlobalScope.createImageBitmap(image, options)](https://html.spec.whatwg.org/#dom-createimagebitmap) — The createImageBitmap(image, options) and createImageBitmap(image, sx, sy, sw, sh, options) methods, when invoked, must run these steps:
  - 1. If either sw or sh is given and is 0, then return a promise rejected with a RangeError.
  - 2. If either options's resizeWidth or options's resizeHeight is present and is 0, then return a promise rejected with an "InvalidStateError" DOMException.
  - 3. Check the usability of the image argument. If this throws an exception or returns bad, then return a promise rejected with an "InvalidStateError" DOMException.
  - 4. Let promise be a new promise.
  - 5. Let imageBitmap be a new ImageBitmap object.
  - 6. Switch on image: img SVG image video canvas Blob Run these steps in parallel: ImageData ImageBitmap VideoFrame
    - 6a. If image's media data has no natural dimensions (e.g., it's a vector graphic with no specified content size) and either options's resizeWidth or options's resizeHeight is not present, then return a promise rejected with an "InvalidStateError" DOMException.
    - 6b. If image's media data has no natural dimensions (e.g., it's a vector graphic with no specified content size), it should be rendered to a bitmap of the size specified by the resizeWidth and the resizeHeight options.
    - 6c. Set imageBitmap's bitmap data to a copy of image's media data, cropped to the source rectangle with formatting. If this is an animated image, imageBitmap's bitmap data must only be taken from the default image of the animation (the one that the format defines is to be used when animation is not supported or is disabled), or, if there is no such image, the first frame of the animation.
    - 6d. If image is not origin-clean, then set the origin-clean flag of imageBitmap's bitmap to false.
    - 6e. Queue a global task, using the bitmap task source, to resolve promise with imageBitmap.
    - 6a. If image's networkState attribute is NETWORK_EMPTY, then return a promise rejected with an "InvalidStateError" DOMException.
    - 6b. Set imageBitmap's bitmap data to a copy of the frame at the current playback position, at the media resource's natural width and natural height (i.e., after any aspect-ratio correction has been applied), cropped to the source rectangle with formatting.
    - 6c. If image is not origin-clean, then set the origin-clean flag of imageBitmap's bitmap to false.
    - 6d. Queue a global task, using the bitmap task source, to resolve promise with imageBitmap.
    - 6a. Set imageBitmap's bitmap data to a copy of image's bitmap data, cropped to the source rectangle with formatting.
    - 6b. Set the origin-clean flag of the imageBitmap's bitmap to the same value as the origin-clean flag of image's bitmap.
    - 6c. Queue a global task, using the bitmap task source, to resolve promise with imageBitmap.
    - 6a. Let imageData be the result of reading image's data. If an error occurs during reading of the object, then queue a global task, using the bitmap task source, to reject promise with an "InvalidStateError" DOMException and abort these steps.
    - 6b. Apply the image sniffing rules to determine the file format of imageData, with MIME type of image (as given by image's type attribute) giving the official type.
    - 6c. If imageData is not in a supported image file format (e.g., it's not an image at all), or if imageData is corrupted in some fatal way such that the image dimensions cannot be obtained (e.g., a vector graphic with no natural size), then queue a global task, using the bitmap task source, to reject promise with an "InvalidStateError" DOMException and abort these steps.
    - 6d. Set imageBitmap's bitmap data to imageData, cropped to the source rectangle with formatting. If this is an animated image, imageBitmap's bitmap data must only be taken from the default image of the animation (the one that the format defines is to be used when animation is not supported or is disabled), or, if there is no such image, the first frame of the animation.
    - 6e. Queue a global task, using the bitmap task source, to resolve promise with imageBitmap.
    - 6a. Let buffer be image's data attribute value's [[ViewedArrayBuffer]] internal slot.
    - 6b. If IsDetachedBuffer(buffer) is true, then return a promise rejected with an "InvalidStateError" DOMException.
    - 6c. Set imageBitmap's bitmap data to image's image data, cropped to the source rectangle with formatting.
    - 6d. Queue a global task, using the bitmap task source, to resolve promise with imageBitmap.
    - 6a. Set imageBitmap's bitmap data to a copy of image's bitmap data, cropped to the source rectangle with formatting.
    - 6b. Set the origin-clean flag of imageBitmap's bitmap to the same value as the origin-clean flag of image's bitmap.
    - 6c. Queue a global task, using the bitmap task source, to resolve promise with imageBitmap.
    - 6a. Set imageBitmap's bitmap data to a copy of image's visible pixel data, cropped to the source rectangle with formatting.
    - 6b. Queue a global task, using the bitmap task source, to resolve promise with imageBitmap.
  - 7. Return promise.

- [x] [crop bitmap data to the source rectangle with formatting](https://html.spec.whatwg.org/#cropped-to-the-source-rectangle-with-formatting) — When the steps above require that the user agent crop bitmap data to the source rectangle with formatting, the user agent must run the following steps:
  - 1. Let input be the bitmap data being transformed.
  - 2. If sx, sy, sw and sh are specified, let sourceRectangle be a rectangle whose corners are the four points (sx, sy), (sx+sw, sy), (sx+sw, sy+sh), (sx, sy+sh). Otherwise, let sourceRectangle be a rectangle whose corners are the four points (0, 0), (width of input, 0), (width of input, height of input), (0, height of input).
  - 3. Let outputWidth be determined as follows: If the resizeWidth member of options is specifiedthe value of the resizeWidth member of optionsIf the resizeWidth member of options is not specified, but the resizeHeight member is specifiedthe width of sourceRectangle, times the value of the resizeHeight member of options, divided by the height of sourceRectangle, rounded up to the nearest integerIf neit…
  - 4. Let outputHeight be determined as follows: If the resizeHeight member of options is specifiedthe value of the resizeHeight member of optionsIf the resizeHeight member of options is not specified, but the resizeWidth member is specifiedthe height of sourceRectangle, times the value of the resizeWidth member of options, divided by the width of sourceRectangle, rounded up to the nearest integerIf ne…
  - 5. Place input on an infinite transparent black grid plane, positioned so that its top left corner is at the origin of the plane, with the x-coordinate increasing to the right, and the y-coordinate increasing down, and with each pixel in the input image data occupying a cell on the plane's grid.
  - 6. Let output be the rectangle on the plane denoted by sourceRectangle.
  - 7. Scale output to the size specified by outputWidth and outputHeight. The user agent should use the value of the resizeQuality option to guide the choice of scaling algorithm.
  - 8. If the value of the imageOrientation member of options is "flipY", output must be flipped vertically, disregarding any image orientation metadata of the source (such as EXIF metadata), if any. [EXIF]
  - 9. If image is an img element or a Blob object, let val be the value of the colorSpaceConversion member of options, and then run these substeps:
    - 9a. If val is "default", the color space conversion behavior is implementation-specific, and should be chosen according to the default color space that the implementation uses for drawing images onto the canvas.
    - 9b. If val is "none", output must be decoded without performing any color space conversions. This means that the image decoding algorithm must ignore color profile metadata embedded in the source data as well as the display device color profile.
  - 10. Let val be the value of premultiplyAlpha member of options, and then run these substeps:
    - 10a. If val is "default", the alpha premultiplication behavior is implementation-specific, and should be chosen according to implementation deems optimal for drawing images onto the canvas.
    - 10b. If val is "premultiply", the output that is not premultiplied by alpha must have its color components multiplied by alpha and that is premultiplied by alpha must be left untouched.
    - 10c. If val is "none", the output that is not premultiplied by alpha must be left untouched and that is premultiplied by alpha must have its color components divided by alpha.
  - 11. Return output.

- [x] [ImageBitmap.close()](https://html.spec.whatwg.org/#dom-imagebitmap-close) — The close() method steps are:
  - 1. Set this's [[Detached]] internal slot value to true.
  - 2. Unset this's bitmap data.

- [x] [ImageBitmap.width](https://html.spec.whatwg.org/#dom-imagebitmap-width) — The width getter steps are:
  - 1. If this's [[Detached]] internal slot's value is true, then return 0.
  - 2. Return this's width, in CSS pixels.

- [x] [ImageBitmap.height](https://html.spec.whatwg.org/#dom-imagebitmap-height) — The height getter steps are:
  - 1. If this's [[Detached]] internal slot's value is true, then return 0.
  - 2. Return this's height, in CSS pixels.

### [8.12 Animation frames](https://html.spec.whatwg.org/#animation-frames)

Status: reviewed (2026-11-01)
> `LocalDOMWindow::RequestAnimationFrame`/`CancelAnimationFrame` and `dom::ScriptedAnimationController`; callbacks get the frame's timestamp in whole milliseconds (see `DIFFERENCES.md`).
<!-- definitions: list-of-animation-frame-callbacks animation-frame-callback-identifier -->

- [x] [target object](https://html.spec.whatwg.org/#concept-animationframeprovider-target-object) — Each AnimationFrameProvider object also has a target object that stores the provider's internal state. It is defined as follows:
  - • If the AnimationFrameProvider is a Window: The Window's associated Document
  - • If the AnimationFrameProvider is a DedicatedWorkerGlobalScope: The DedicatedWorkerGlobalScope

- [x] [supported](https://html.spec.whatwg.org/#concept-animationframeprovider-supported) — An AnimationFrameProvider provider is considered supported if any of the following are true:

- [x] [AnimationFrameProvider.requestAnimationFrame(callback)](https://html.spec.whatwg.org/#dom-animationframeprovider-requestanimationframe) — The requestAnimationFrame(callback) method steps are:
  - 1. If this is not supported, then throw a "NotSupportedError" DOMException.
  - 2. Let target be this's target object.
  - 3. Increment target's animation frame callback identifier by one, and let handle be the result.
  - 4. Let callbacks be target's map of animation frame callbacks.
  - 5. Set callbacks[handle] to callback.
  - 6. Return handle.

- [x] [AnimationFrameProvider.cancelAnimationFrame(handle)](https://html.spec.whatwg.org/#animationframeprovider-cancelanimationframe) — The cancelAnimationFrame(handle) method steps are:
  - 1. If this is not supported, then throw a "NotSupportedError" DOMException.
  - 2. Let callbacks be this's target object's map of animation frame callbacks.
  - 3. Remove callbacks[handle].

- [x] [run the animation frame callbacks](https://html.spec.whatwg.org/#run-the-animation-frame-callbacks) — To run the animation frame callbacks for a target object target with a timestamp now:
  - 1. Let callbacks be target's map of animation frame callbacks.
  - 2. Let callbackHandles be the result of getting the keys of callbacks.
  - 3. For each handle in callbackHandles, if handle exists in callbacks:
    - 3a. Let callback be callbacks[handle].
    - 3b. Remove callbacks[handle].
    - 3c. Invoke callback with « now » and "report".
