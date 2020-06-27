#pragma once
#include <include/lauxlib.h>
#include <include/lua.h>
#include <include/lualib.h>

int render_makePart(lua_State* Lua);

int movePart(lua_State* Lua);
int resizePart(lua_State* Lua);
int rotatePart(lua_State* Lua);

int recolorPart(lua_State* Lua);

int moveCamera(lua_State* Lua);
int rotateCamera(lua_State* Lua);

int getKeyDown(lua_State* Lua);
int getMouseButtonDown(lua_State* Lua);
int getMousePosition(lua_State* Lua);
int setMousePosition(lua_State* Lua);
int setCursorState(lua_State* Lua);