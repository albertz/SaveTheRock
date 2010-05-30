/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */

#ifndef __ENGINE_HEADERS_H
#define __ENGINE_HEADERS_H

#include "constants.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <list>

#include <cmath>
#include <cassert>
#include <cstdlib>
#include <ctime>
/*// temporary
#include <cstring>
#include <sstream>
#include <string>
*/

#ifdef _WIN32
#include <windows.h>
#else
#define MAX_PATH 512
#endif

#include <GL/glfw.h>
#include <GL/glext.h>

#include <audiere.h>

// game engine includes
#include "filesystem.h"
#include "helper.h"
#include "math.h"
#include "sound.h"
#include "renderer.h"
#include "core.h"
#include "physics.h"
#include "ui.h"
#include "level.h"
#include "sprite.h"

#include "../headers.h"

#endif
