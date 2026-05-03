// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this makes them subject to being re-compiled.

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX

// Windows headers
#include <Windows.h>
#include <WinUser.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <propsys.h>
#include <propkey.h>

// STL headers
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <functional>
#include <mutex>
#include <atomic>

// CLR headers
#include <vcclr.h>
#include <msclr/marshal.h>
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Runtime::InteropServices;
using namespace System::Xml;
using namespace System::Xml::Serialization;

// Disable warnings
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4244) // conversion, possible loss of data
#pragma warning(disable: 4251) // class needs to have DLL interface
#pragma warning(disable: 4365) // signed/unsigned mismatch
#pragma warning(disable: 4514) // unreferenced inline function has been removed
#pragma warning(disable: 4571) // catch(...) blocks compiled with /EHs
#pragma warning(disable: 4625) // derived class copy constructor
#pragma warning(disable: 4626) // derived class assignment operator
#pragma warning(disable: 4668) // not defined as preprocessor macro
#pragma warning(disable: 4710) // function not inlined
#pragma warning(disable: 4820) // padding added
#pragma warning(disable: 4946) // reinterpret_cast used between related classes
