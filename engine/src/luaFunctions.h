#pragma once
#include <include/lauxlib.h>
#include <include/lua.h>
#include <include/lualib.h>

std::map<const char*, std::vector<int>> luaEventFunctions;

lua_State* Lua;

static void dumpstack(lua_State *L) {
	int top = lua_gettop(L);
	for (int i = 1; i <= top; i++) {
		printf("%d\t%s\t", i, luaL_typename(L, i));
		switch (lua_type(L, i)) {
		case LUA_TNUMBER:
			printf("%g\n", lua_tonumber(L, i));
			break;
		case LUA_TSTRING:
			printf("%s\n", lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
			printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
			break;
		case LUA_TNIL:
			printf("%s\n", "nil");
			break;
		default:
			printf("%p\n", lua_topointer(L, i));
			break;
		}
	}
}

int newObject(lua_State* Lua)
{
	if (lua_gettop(Lua) > 1) return 0;
	const char* path = "res/3d/cube.obj";
	if (lua_gettop(Lua) == 1)
		if(lua_isstring(Lua, 1))
			path = lua_tostring(Lua, 1);

	lua_newtable(Lua);
	lua_pushstring(Lua, "ObjectName");
	lua_setfield(Lua, -2, "Name");
	lua_pushstring(Lua, "ObjectType");
	lua_setfield(Lua, -2, "Type");

	lua_newtable(Lua);
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "x");
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "y");
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "z");

	lua_setfield(Lua, -2, "Position");

	lua_newtable(Lua);
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "x");
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "y");
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "z");

	lua_setfield(Lua, -2, "Orientation");

	lua_newtable(Lua);
	lua_pushnumber(Lua, 1);
	lua_setfield(Lua, -2, "x");
	lua_pushnumber(Lua, 1);
	lua_setfield(Lua, -2, "y");
	lua_pushnumber(Lua, 1);
	lua_setfield(Lua, -2, "z");

	lua_setfield(Lua, -2, "Size");

	lua_newtable(Lua);
	lua_pushnumber(Lua, 1);
	lua_setfield(Lua, -2, "r");
	lua_pushnumber(Lua, 1);
	lua_setfield(Lua, -2, "g");
	lua_pushnumber(Lua, 1);
	lua_setfield(Lua, -2, "b");

	lua_setfield(Lua, -2, "Color");

	int r = luaL_ref(Lua, LUA_REGISTRYINDEX);
	render[r] = createModelFromFile(path, OBJ);
	lua_rawgeti(Lua, LUA_REGISTRYINDEX, r);

	return 1;
}

int newVec3(lua_State* Lua)
{
	if (lua_gettop(Lua) != 3 || !lua_isnumber(Lua, 1) || !lua_isnumber(Lua, 2) || !lua_isnumber(Lua, 3)) return 0;
	lua_newtable(Lua);
	lua_pushnumber(Lua, lua_tonumber(Lua, 1));
	lua_setfield(Lua, -2, "x");
	lua_pushnumber(Lua, lua_tonumber(Lua, 2));
	lua_setfield(Lua, -2, "y");
	lua_pushnumber(Lua, lua_tonumber(Lua, 3));
	lua_setfield(Lua, -2, "z");
	return 1;
}

int newColorRGB(lua_State* Lua)
{
	if (lua_gettop(Lua) != 3 || !lua_isnumber(Lua, 1) || !lua_isnumber(Lua, 2) || !lua_isnumber(Lua, 3)) return 0;
	lua_newtable(Lua);
	lua_pushnumber(Lua, lua_tonumber(Lua, 1) / 255);
	lua_setfield(Lua, -2, "r");
	lua_pushnumber(Lua, lua_tonumber(Lua, 2) / 255);
	lua_setfield(Lua, -2, "g");
	lua_pushnumber(Lua, lua_tonumber(Lua, 3) / 255);
	lua_setfield(Lua, -2, "b");
	return 1;
}

int newTickListener(lua_State* Lua)
{
	if (lua_isfunction(Lua, 1) == 1)
		luaEventFunctions["Tick"].push_back(luaL_ref(Lua, LUA_REGISTRYINDEX));
	return 0;
}

int updateInput(lua_State* Lua)
{
	lua_getglobal(Lua, "Input");
	lua_getfield(Lua, -1, "CurrentCursorState");
	glfwSetInputMode(window, GLFW_CURSOR, lua_tointeger(Lua, -1));
	return 0;
}

void tickCallback()
{
	updateInput(Lua);
	for (int functionRef : luaEventFunctions["Tick"])
	{
		lua_rawgeti(Lua, LUA_REGISTRYINDEX, functionRef);
		lua_call(Lua, 0, 0);
		lua_settop(Lua, 0);
	}
}

int newMouseMoveListener(lua_State* Lua)
{
	if(lua_isfunction(Lua, 1) == 1)
		luaEventFunctions["MouseMove"].push_back(luaL_ref(Lua, LUA_REGISTRYINDEX));
	return 0;
}

void mouseMoveCallback(GLFWwindow* window, double x, double y)
{
	lua_getglobal(Lua, "Input");
	lua_newtable(Lua);
	lua_pushnumber(Lua, x);
	lua_setfield(Lua, -2, "x");
	lua_pushnumber(Lua, y);
	lua_setfield(Lua, -2, "y");
	lua_setfield(Lua, -2, "MousePosition");

	for (int functionRef : luaEventFunctions["MouseMove"])
	{
		lua_rawgeti(Lua, LUA_REGISTRYINDEX, functionRef);
		lua_newtable(Lua);
		lua_pushnumber(Lua, x);
		lua_setfield(Lua, -2, "x");
		lua_pushnumber(Lua, y);
		lua_setfield(Lua, -2, "y");
		lua_call(Lua, 1, 0);
		lua_settop(Lua, 0);
	}
}

int inputCheck(lua_State* Lua)
{
	int keyCode = lua_tointeger(Lua, 1);
	if (keyCode >= 2500 && keyCode <= 2507)
	{
		keyCode -= 2500;
		lua_pushboolean(Lua, glfwGetMouseButton(window, keyCode));
	}
	else
		lua_pushboolean(Lua, glfwGetKey(window, keyCode));
	return 1;
}

int updateMousePosition(lua_State* Lua)
{
	glfwSetCursorPos(window, lua_tonumber(Lua, 1), lua_tonumber(Lua, 2));
	return 0;
}

void registerLua(lua_State* Lua)
{
	lua_newtable(Lua);
	lua_pushcfunction(Lua, newObject);
	lua_setfield(Lua, -2, "create");
	lua_gettop(Lua);
	lua_setglobal(Lua, "Object");

	lua_newtable(Lua);
	lua_pushcfunction(Lua, newVec3);
	lua_setfield(Lua, -2, "create");
	lua_gettop(Lua);
	lua_setglobal(Lua, "Vector3");

	lua_newtable(Lua);
	lua_pushcfunction(Lua, newColorRGB);
	lua_setfield(Lua, -2, "create");
	lua_gettop(Lua);
	lua_setglobal(Lua, "ColorRGB");

	lua_newtable(Lua);

	lua_newtable(Lua);
	lua_pushcfunction(Lua, newTickListener);
	lua_setfield(Lua, -2, "addListener");
	lua_setfield(Lua, -2, "Tick");

	lua_newtable(Lua);

	lua_newtable(Lua);
	lua_pushcfunction(Lua, newMouseMoveListener);
	lua_setfield(Lua, -2, "addListener");
	lua_setfield(Lua, -2, "MouseMove");

	lua_setfield(Lua, -2, "Input");

	lua_gettop(Lua);
	lua_setglobal(Lua, "Events");

	lua_newtable(Lua);

	lua_newtable(Lua);
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	lua_pushnumber(Lua, x);
	lua_setfield(Lua, -2, "x");
	lua_pushnumber(Lua, y);
	lua_setfield(Lua, -2, "y");
	lua_setfield(Lua, -2, "MousePosition");

	lua_newtable(Lua);
	lua_pushnumber(Lua, 32);
	lua_setfield(Lua, -2, "Space");
	lua_pushnumber(Lua, 39);
	lua_setfield(Lua, -2, "Apostrophe");
	lua_pushnumber(Lua, 44);
	lua_setfield(Lua, -2, "Comma");
	lua_pushnumber(Lua, 45);
	lua_setfield(Lua, -2, "Minus");
	lua_pushnumber(Lua, 45);
	lua_setfield(Lua, -2, "Dash");
	lua_pushnumber(Lua, 46);
	lua_setfield(Lua, -2, "Period");
	lua_pushnumber(Lua, 46);
	lua_setfield(Lua, -2, "Dot");
	lua_pushnumber(Lua, 47);
	lua_setfield(Lua, -2, "Slash");
	lua_pushnumber(Lua, 47);
	lua_setfield(Lua, -2, "ForwardSlash");
	lua_pushnumber(Lua, 48);
	lua_setfield(Lua, -2, "0");
	lua_pushnumber(Lua, 49);
	lua_setfield(Lua, -2, "1");
	lua_pushnumber(Lua, 50);
	lua_setfield(Lua, -2, "2");
	lua_pushnumber(Lua, 51);
	lua_setfield(Lua, -2, "3");
	lua_pushnumber(Lua, 52);
	lua_setfield(Lua, -2, "4");
	lua_pushnumber(Lua, 53);
	lua_setfield(Lua, -2, "5");
	lua_pushnumber(Lua, 54);
	lua_setfield(Lua, -2, "6");
	lua_pushnumber(Lua, 55);
	lua_setfield(Lua, -2, "7");
	lua_pushnumber(Lua, 56);
	lua_setfield(Lua, -2, "8");
	lua_pushnumber(Lua, 57);
	lua_setfield(Lua, -2, "9");
	lua_pushnumber(Lua, 59);
	lua_setfield(Lua, -2, "Semicolon");
	lua_pushnumber(Lua, 61);
	lua_setfield(Lua, -2, "Equal");
	lua_pushnumber(Lua, 61);
	lua_setfield(Lua, -2, "Equals");
	lua_pushnumber(Lua, 65);
	lua_setfield(Lua, -2, "A");
	lua_pushnumber(Lua, 66);
	lua_setfield(Lua, -2, "B");
	lua_pushnumber(Lua, 67);
	lua_setfield(Lua, -2, "C");
	lua_pushnumber(Lua, 68);
	lua_setfield(Lua, -2, "D");
	lua_pushnumber(Lua, 69);
	lua_setfield(Lua, -2, "E");
	lua_pushnumber(Lua, 70);
	lua_setfield(Lua, -2, "F");
	lua_pushnumber(Lua, 71);
	lua_setfield(Lua, -2, "G");
	lua_pushnumber(Lua, 72);
	lua_setfield(Lua, -2, "H");
	lua_pushnumber(Lua, 73);
	lua_setfield(Lua, -2, "I");
	lua_pushnumber(Lua, 74);
	lua_setfield(Lua, -2, "J");
	lua_pushnumber(Lua, 75);
	lua_setfield(Lua, -2, "K");
	lua_pushnumber(Lua, 76);
	lua_setfield(Lua, -2, "L");
	lua_pushnumber(Lua, 77);
	lua_setfield(Lua, -2, "M");
	lua_pushnumber(Lua, 78);
	lua_setfield(Lua, -2, "N");
	lua_pushnumber(Lua, 79);
	lua_setfield(Lua, -2, "O");
	lua_pushnumber(Lua, 80);
	lua_setfield(Lua, -2, "P");
	lua_pushnumber(Lua, 81);
	lua_setfield(Lua, -2, "Q");
	lua_pushnumber(Lua, 82);
	lua_setfield(Lua, -2, "R");
	lua_pushnumber(Lua, 83);
	lua_setfield(Lua, -2, "S");
	lua_pushnumber(Lua, 84);
	lua_setfield(Lua, -2, "T");
	lua_pushnumber(Lua, 85);
	lua_setfield(Lua, -2, "U");
	lua_pushnumber(Lua, 86);
	lua_setfield(Lua, -2, "V");
	lua_pushnumber(Lua, 87);
	lua_setfield(Lua, -2, "W");
	lua_pushnumber(Lua, 88);
	lua_setfield(Lua, -2, "X");
	lua_pushnumber(Lua, 89);
	lua_setfield(Lua, -2, "Y");
	lua_pushnumber(Lua, 90);
	lua_setfield(Lua, -2, "Z");
	lua_pushnumber(Lua, 91);
	lua_setfield(Lua, -2, "LeftBracket");
	lua_pushnumber(Lua, 92);
	lua_setfield(Lua, -2, "Backslash");
	lua_pushnumber(Lua, 93);
	lua_setfield(Lua, -2, "RightBracket");
	lua_pushnumber(Lua, 96);
	lua_setfield(Lua, -2, "GraveAccent");
	lua_pushnumber(Lua, 256);
	lua_setfield(Lua, -2, "Escape");
	lua_pushnumber(Lua, 256);
	lua_setfield(Lua, -2, "Esc");
	lua_pushnumber(Lua, 257);
	lua_setfield(Lua, -2, "Enter");
	lua_pushnumber(Lua, 257);
	lua_setfield(Lua, -2, "Return");
	lua_pushnumber(Lua, 258);
	lua_setfield(Lua, -2, "Tab");
	lua_pushnumber(Lua, 259);
	lua_setfield(Lua, -2, "Backspace");
	lua_pushnumber(Lua, 260);
	lua_setfield(Lua, -2, "Insert");
	lua_pushnumber(Lua, 261);
	lua_setfield(Lua, -2, "Delete");
	lua_pushnumber(Lua, 261);
	lua_setfield(Lua, -2, "Del");
	lua_pushnumber(Lua, 262);
	lua_setfield(Lua, -2, "RightArrow");
	lua_pushnumber(Lua, 262);
	lua_setfield(Lua, -2, "Right");
	lua_pushnumber(Lua, 263);
	lua_setfield(Lua, -2, "LeftArrow");
	lua_pushnumber(Lua, 263);
	lua_setfield(Lua, -2, "Left");
	lua_pushnumber(Lua, 264);
	lua_setfield(Lua, -2, "DownArrow");
	lua_pushnumber(Lua, 264);
	lua_setfield(Lua, -2, "Down");
	lua_pushnumber(Lua, 265);
	lua_setfield(Lua, -2, "UpArrow");
	lua_pushnumber(Lua, 265);
	lua_setfield(Lua, -2, "Up");
	lua_pushnumber(Lua, 266);
	lua_setfield(Lua, -2, "PageUp");
	lua_pushnumber(Lua, 267);
	lua_setfield(Lua, -2, "PageDown");
	lua_pushnumber(Lua, 268);
	lua_setfield(Lua, -2, "Home");
	lua_pushnumber(Lua, 269);
	lua_setfield(Lua, -2, "End");
	lua_pushnumber(Lua, 280);
	lua_setfield(Lua, -2, "CapsLock");
	lua_pushnumber(Lua, 280);
	lua_setfield(Lua, -2, "Caps");
	lua_pushnumber(Lua, 281);
	lua_setfield(Lua, -2, "ScrollLock");
	lua_pushnumber(Lua, 282);
	lua_setfield(Lua, -2, "NumLock");
	lua_pushnumber(Lua, 282);
	lua_setfield(Lua, -2, "NumberLock");
	lua_pushnumber(Lua, 283);
	lua_setfield(Lua, -2, "PrintScreen");
	lua_pushnumber(Lua, 283);
	lua_setfield(Lua, -2, "PrtScr");
	lua_pushnumber(Lua, 284);
	lua_setfield(Lua, -2, "Pause");
	lua_pushnumber(Lua, 290);
	lua_setfield(Lua, -2, "F1");
	lua_pushnumber(Lua, 291);
	lua_setfield(Lua, -2, "F2");
	lua_pushnumber(Lua, 292);
	lua_setfield(Lua, -2, "F3");
	lua_pushnumber(Lua, 293);
	lua_setfield(Lua, -2, "F4");
	lua_pushnumber(Lua, 294);
	lua_setfield(Lua, -2, "F5");
	lua_pushnumber(Lua, 295);
	lua_setfield(Lua, -2, "F6");
	lua_pushnumber(Lua, 296);
	lua_setfield(Lua, -2, "F7");
	lua_pushnumber(Lua, 297);
	lua_setfield(Lua, -2, "F8");
	lua_pushnumber(Lua, 298);
	lua_setfield(Lua, -2, "F9");
	lua_pushnumber(Lua, 299);
	lua_setfield(Lua, -2, "F10");
	lua_pushnumber(Lua, 300);
	lua_setfield(Lua, -2, "F11");
	lua_pushnumber(Lua, 301);
	lua_setfield(Lua, -2, "F12");
	lua_pushnumber(Lua, 302);
	lua_setfield(Lua, -2, "F13");
	lua_pushnumber(Lua, 303);
	lua_setfield(Lua, -2, "F14");
	lua_pushnumber(Lua, 304);
	lua_setfield(Lua, -2, "F15");
	lua_pushnumber(Lua, 305);
	lua_setfield(Lua, -2, "F16");
	lua_pushnumber(Lua, 306);
	lua_setfield(Lua, -2, "F17");
	lua_pushnumber(Lua, 307);
	lua_setfield(Lua, -2, "F18");
	lua_pushnumber(Lua, 308);
	lua_setfield(Lua, -2, "F19");
	lua_pushnumber(Lua, 309);
	lua_setfield(Lua, -2, "F20");
	lua_pushnumber(Lua, 310);
	lua_setfield(Lua, -2, "F21");
	lua_pushnumber(Lua, 311);
	lua_setfield(Lua, -2, "F22");
	lua_pushnumber(Lua, 312);
	lua_setfield(Lua, -2, "F23");
	lua_pushnumber(Lua, 313);
	lua_setfield(Lua, -2, "F24");
	lua_pushnumber(Lua, 314);
	lua_setfield(Lua, -2, "F25");
	lua_pushnumber(Lua, 320);
	lua_setfield(Lua, -2, "Numpad0");
	lua_pushnumber(Lua, 320);
	lua_setfield(Lua, -2, "Keypad0");
	lua_pushnumber(Lua, 321);
	lua_setfield(Lua, -2, "Numpad1");
	lua_pushnumber(Lua, 321);
	lua_setfield(Lua, -2, "Keypad1");
	lua_pushnumber(Lua, 322);
	lua_setfield(Lua, -2, "Numpad2");
	lua_pushnumber(Lua, 322);
	lua_setfield(Lua, -2, "Keypad2");
	lua_pushnumber(Lua, 323);
	lua_setfield(Lua, -2, "Numpad3");
	lua_pushnumber(Lua, 323);
	lua_setfield(Lua, -2, "Keypad3");
	lua_pushnumber(Lua, 324);
	lua_setfield(Lua, -2, "Numpad4");
	lua_pushnumber(Lua, 324);
	lua_setfield(Lua, -2, "Keypad4");
	lua_pushnumber(Lua, 325);
	lua_setfield(Lua, -2, "Numpad5");
	lua_pushnumber(Lua, 325);
	lua_setfield(Lua, -2, "Keypad5");
	lua_pushnumber(Lua, 326);
	lua_setfield(Lua, -2, "Numpad6");
	lua_pushnumber(Lua, 326);
	lua_setfield(Lua, -2, "Keypad6");
	lua_pushnumber(Lua, 327);
	lua_setfield(Lua, -2, "Numpad7");
	lua_pushnumber(Lua, 327);
	lua_setfield(Lua, -2, "Keypad7");
	lua_pushnumber(Lua, 328);
	lua_setfield(Lua, -2, "Numpad8");
	lua_pushnumber(Lua, 328);
	lua_setfield(Lua, -2, "Keypad8");
	lua_pushnumber(Lua, 329);
	lua_setfield(Lua, -2, "Numpad9");
	lua_pushnumber(Lua, 329);
	lua_setfield(Lua, -2, "Keypad9");
	lua_pushnumber(Lua, 330);
	lua_setfield(Lua, -2, "NumpadDecimal");
	lua_pushnumber(Lua, 330);
	lua_setfield(Lua, -2, "KeypadDecimal");
	lua_pushnumber(Lua, 331);
	lua_setfield(Lua, -2, "NumpadDivide");
	lua_pushnumber(Lua, 331);
	lua_setfield(Lua, -2, "KeypadDivide");
	lua_pushnumber(Lua, 332);
	lua_setfield(Lua, -2, "NumpadMultiply");
	lua_pushnumber(Lua, 332);
	lua_setfield(Lua, -2, "KeypadMultiply");
	lua_pushnumber(Lua, 333);
	lua_setfield(Lua, -2, "NumpadSubtract");
	lua_pushnumber(Lua, 333);
	lua_setfield(Lua, -2, "KeypadSubtract");
	lua_pushnumber(Lua, 334);
	lua_setfield(Lua, -2, "NumpadAdd");
	lua_pushnumber(Lua, 334);
	lua_setfield(Lua, -2, "KeypadAdd");
	lua_pushnumber(Lua, 335);
	lua_setfield(Lua, -2, "NumpadEnter");
	lua_pushnumber(Lua, 335);
	lua_setfield(Lua, -2, "KeypadEnter");
	lua_pushnumber(Lua, 336);
	lua_setfield(Lua, -2, "NumpadEqual");
	lua_pushnumber(Lua, 336);
	lua_setfield(Lua, -2, "KeypadEqual");
	lua_pushnumber(Lua, 340);
	lua_setfield(Lua, -2, "LeftShift");
	lua_pushnumber(Lua, 340);
	lua_setfield(Lua, -2, "Shift");
	lua_pushnumber(Lua, 341);
	lua_setfield(Lua, -2, "LeftControl");
	lua_pushnumber(Lua, 341);
	lua_setfield(Lua, -2, "Control");
	lua_pushnumber(Lua, 342);
	lua_setfield(Lua, -2, "LeftControl");
	lua_pushnumber(Lua, 344);
	lua_setfield(Lua, -2, "RightShift");
	lua_pushnumber(Lua, 345);
	lua_setfield(Lua, -2, "RightControl");
	lua_pushnumber(Lua, 346);
	lua_setfield(Lua, -2, "RightAlt");
	lua_pushnumber(Lua, 348);
	lua_setfield(Lua, -2, "Menu");
	lua_setfield(Lua, -2, "KeyCode");

	lua_newtable(Lua);
	lua_pushnumber(Lua, 2500);
	lua_setfield(Lua, -2, "Button1");
	lua_pushnumber(Lua, 2501);
	lua_setfield(Lua, -2, "Button2");
	lua_pushnumber(Lua, 2502);
	lua_setfield(Lua, -2, "Button3");
	lua_pushnumber(Lua, 2503);
	lua_setfield(Lua, -2, "Button4");
	lua_pushnumber(Lua, 2504);
	lua_setfield(Lua, -2, "Button5");
	lua_pushnumber(Lua, 2505);
	lua_setfield(Lua, -2, "Button6");
	lua_pushnumber(Lua, 2506);
	lua_setfield(Lua, -2, "Button7");
	lua_pushnumber(Lua, 2507);
	lua_setfield(Lua, -2, "Button8");
	lua_pushnumber(Lua, 2500);
	lua_setfield(Lua, -2, "Left");
	lua_pushnumber(Lua, 2501);
	lua_setfield(Lua, -2, "Right");
	lua_pushnumber(Lua, 2502);
	lua_setfield(Lua, -2, "Middle");
	lua_setfield(Lua, -2, "MouseCode");

	lua_newtable(Lua);
	lua_pushnumber(Lua, 212993);
	lua_setfield(Lua, -2, "Normal");
	lua_pushnumber(Lua, 212994);
	lua_setfield(Lua, -2, "Hidden");
	lua_pushnumber(Lua, 212995);
	lua_setfield(Lua, -2, "Disabled");
	lua_setfield(Lua, -2, "CursorState");

	lua_pushnumber(Lua, 212993);
	lua_setfield(Lua, -2, "CurrentCursorState");

	lua_pushcfunction(Lua, inputCheck);
	lua_setfield(Lua, -2, "isDown");

	lua_pushcfunction(Lua, updateMousePosition);
	lua_setfield(Lua, -2, "updateMousePosition");

	lua_pushcfunction(Lua, updateInput);
	lua_setfield(Lua, -2, "updateScreenInput");

	lua_setglobal(Lua, "Input");

	lua_newtable(Lua);
	lua_newtable(Lua);

	lua_newtable(Lua);
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "x");
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "y");
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "z");


	lua_setfield(Lua, -2, "Position");

	lua_newtable(Lua);
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "x");
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "y");
	lua_pushnumber(Lua, 0);
	lua_setfield(Lua, -2, "z");

	lua_setfield(Lua, -2, "Orientation");

	lua_setfield(Lua, -2, "Camera");
	lua_setglobal(Lua, "Game");
}