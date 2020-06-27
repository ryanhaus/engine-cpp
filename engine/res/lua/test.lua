local glfw = require("res.lua.extras.glfwDef");

local cube = createPart("res/3d/cube.obj");
local dragon = createPart("res/3d/dragon.obj");

setSize(cube, 8, 0.5, 8);
setPosition(cube, 0, -5, 0);
setPosition(dragon, 0, -5, 0);
setColor(dragon, 0.25, 0.25, 0.9);

local cameraPosition = {x = 0, y = 0, z = 15};
local cameraRotation = {x = 0, y = 0, z = 0};

local lastClockTime = os.clock();
local previousCursor = getMousePosition();
local throwMouse = getMousePosition();
local lastFrameRtClick = false;

local cameraDivisor = 512;
local speed = -16;
local halfPi = math.asin(1);

local cameraFocusPart = createPart("res/3d/cube.obj");
local cameraFocusPosition = {x = 0, y = 0, z = 0};
local cameraFocusRotation = {x = 0, y = 0, z = 0};
local cameraFocusDistance = 8;

function tick()
    local deltaSeconds = os.clock() - lastClockTime;

    local mousePosition = getMousePosition();
    local deltaMousePosition = {x = mousePosition.x - previousCursor.x, y= mousePosition.y - previousCursor.y};
    previousCursor = mousePosition;

    if (getMouseButtonDown(glfw["MOUSE_BUTTON_RIGHT"]) or getMouseButtonDown(glfw["MOUSE_BUTTON_MIDDLE"])) and not lastFrameRtClick then
        setCursorState(glfw["CURSOR_HIDDEN"]);
        setCursorState(glfw["CURSOR_DISABLED"]);
        throwMouse = {x = mousePosition.x, y = mousePosition.y};
    end

    if not getMouseButtonDown(glfw["MOUSE_BUTTON_RIGHT"]) and not getMouseButtonDown(glfw["MOUSE_BUTTON_MIDDLE"]) and lastFrameRtClick then
        setCursorState(glfw["CURSOR_NORMAL"]);
        setMousePosition(throwMouse.x, throwMouse.y);
    end

    if getMouseButtonDown(glfw["MOUSE_BUTTON_RIGHT"]) or getMouseButtonDown(glfw["MOUSE_BUTTON_MIDDLE"]) then
        cameraRotation.y = cameraRotation.y + deltaMousePosition.x / cameraDivisor;
        cameraRotation.x = cameraRotation.x + deltaMousePosition.y / cameraDivisor;
    end

    lastFrameRtClick = getMouseButtonDown(glfw["MOUSE_BUTTON_RIGHT"]) or getMouseButtonDown(glfw["MOUSE_BUTTON_MIDDLE"]);

    if cameraRotation.x > halfPi * 0.9 then
        cameraRotation.x = halfPi * 0.9;
    end

    if cameraRotation.x < -halfPi * 0.9 then
        cameraRotation.x = -halfPi * 0.9;
    end
    
    setCameraRotation(cameraRotation.x, cameraRotation.y - halfPi, 0);

    if getKeyDown(glfw["KEY_I"]) then
        cameraFocusDistance = math.min(math.max(2, cameraFocusDistance - 32 * deltaSeconds), 32);
    end

    if getKeyDown(glfw["KEY_O"]) then
        cameraFocusDistance = math.min(math.max(2, cameraFocusDistance + 32 * deltaSeconds), 32);
    end

    local frameCamPos = {
        x = math.cos(cameraRotation.y) * math.cos(cameraRotation.x) * cameraFocusDistance,
        y = math.sin(cameraRotation.x) * cameraFocusDistance,
        z = math.cos(cameraRotation.x) * math.sin(cameraRotation.y) * cameraFocusDistance
    }

    if getKeyDown(glfw["KEY_RIGHT"]) then
        cameraRotation.y = cameraRotation.y + deltaSeconds * 1.75;
    end

    if getKeyDown(glfw["KEY_LEFT"]) then
        cameraRotation.y = cameraRotation.y - deltaSeconds * 1.75;
    end

    local targetYRotation = 0;
    local avgN = 0;

    local forwards = getKeyDown(glfw["KEY_W"]) or getKeyDown(glfw["KEY_UP"]);
    local backwards = getKeyDown(glfw["KEY_S"]) or getKeyDown(glfw["KEY_DOWN"]);

    if forwards and not backwards then
        targetYRotation = targetYRotation + cameraRotation.y;
        avgN = avgN + 1;
    end
    
    if backwards and getKeyDown(glfw["KEY_A"]) then
        targetYRotation = targetYRotation + cameraRotation.y + halfPi * 2.5;
        avgN = avgN + 1;
    else
        if getKeyDown(glfw["KEY_A"]) and not getKeyDown(glfw["KEY_D"]) then
            targetYRotation = targetYRotation + cameraRotation.y - halfPi;
            avgN = avgN + 1;
        end

        if backwards and not forwards then
            targetYRotation = targetYRotation + cameraRotation.y + halfPi * 2;
            avgN = avgN + 1;
        end
    end

    if getKeyDown(glfw["KEY_D"]) and not getKeyDown(glfw["KEY_A"]) then
        targetYRotation = targetYRotation + cameraRotation.y + halfPi;
        avgN = avgN + 1;
    end

    if avgN > 0 then
        cameraFocusRotation.y = (cameraFocusRotation.y - targetYRotation / avgN) * -0.125 + cameraFocusRotation.y;
        cameraFocusPosition.x = cameraFocusPosition.x + math.cos(targetYRotation / avgN) * deltaSeconds * speed;
        cameraFocusPosition.z = cameraFocusPosition.z + math.sin(targetYRotation / avgN) * deltaSeconds * speed;
    end

    setPosition(cameraFocusPart, cameraFocusPosition.x, cameraFocusPosition.y, cameraFocusPosition.z);
    setRotation(cameraFocusPart, cameraFocusRotation.x, -cameraFocusRotation.y, cameraFocusRotation.z);

    setCameraPosition(frameCamPos.x + cameraFocusPosition.x, frameCamPos.y + cameraFocusPosition.y, frameCamPos.z + cameraFocusPosition.z);

    lastClockTime = os.clock();
end

function scrollWheelCallback(x, y)
    cameraFocusDistance = math.min(math.max(2, cameraFocusDistance - y * 2), 32);
end