#include "ScreenCapture.h"
#include "duilib/Core/GlobalManager.h"

#if defined (DUILIB_BUILD_FOR_SDL)

namespace ui
{

std::shared_ptr<IBitmap> ScreenCapture::CaptureBitmap(const Window* /*pWindow*/)
{
    return nullptr;
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
