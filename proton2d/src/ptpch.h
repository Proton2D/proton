#pragma once

#include <iostream>
#include <functional>
#include <memory>
#include <sstream>
#include <cassert>

#include <string>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>

#if PROTON_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include "Proton/Core/Base.h"
#include "Proton/Debug/Assert.h"
#include "Proton/Debug/Instrumentor.h"
