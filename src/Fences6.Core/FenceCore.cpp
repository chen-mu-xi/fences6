#include "pch.h"
#include "FenceCore.h"

// Include native components
#include "FileWatcher.h"
#include "DesktopIntegration.h"

// Static initialization
namespace Fences6
{
    bool FenceCore::s_initialized = false;
    Object^ FenceCore::s_lock = nullptr;
}
