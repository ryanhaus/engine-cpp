local glfw = require("res.lua.extras.glfwDef");

local cube = createPart("res/3d/cube.obj");
local dragon = createPart("res/3d/dragon.obj");

setSize(cube, 8, 0.5, 8);
setPosition(cube, 0, -5, 0);
setPosition(dragon, 0, -5, 0);
setColor(dragon, 0.25, 0.25, 0.9);
setCameraPosition(0, 0, 15);

local cameraPosition = {x = 0, y = 0, z = 15};
local cameraRotation = {x = 0, y = 0, z = 0};

local lastClockTime = os.clock();
local previousCursor = getMousePosition();
local throwMouse = getMousePosition();
local lastFrameRtClick = false;

local cameraDivisor = 750;
local speed = -16;
local halfPi = math.asin(1);

function tick()
    local deltaSeconds = os.clock() - lastClockTime;

    setRotation(cube, 0, os.clock(), 0);
    setRotation(dragon, 0, os.clock(), 0);

    local mousePosition = getMousePosition();
    local deltaMousePosition = {x = mousePosition.x - previousCursor.x, y= mousePosition.y - previousCursor.y};
    previousCursor = mousePosition;

    if getMouseButtonDown(glfw["MOUSE_BUTTON_RIGHT"]) and not lastFrameRtClick then
        setCursorState(glfw["CURSOR_HIDDEN"]);
        setCursorState(glfw["CURSOR_DISABLED"]);
        throwMouse = {x = mousePosition.x, y = mousePosition.y};
    end

    if not getMouseButtonDown(glfw["MOUSE_BUTTON_RIGHT"]) and lastFrameRtClick then
        setCursorState(glfw["CURSOR_NORMAL"]);
        setMousePosition(throwMouse.x, throwMouse.y);
    end

    if getMouseButtonDown(glfw["MOUSE_BUTTON_RIGHT"]) then
        cameraRotation.y = cameraRotation.y + deltaMousePosition.x / cameraDivisor;
        cameraRotation.x = cameraRotation.x + deltaMousePosition.y / cameraDivisor;
    end

    lastFrameRtClick = getMouseButtonDown(glfw["MOUSE_BUTTON_RIGHT"]);

    if cameraRotation.x > halfPi then
        cameraRotation.x = halfPi;
    end

    if cameraRotation.x < -halfPi then
        cameraRotation.x = -halfPi;
    end

    if(getKeyDown(glfw["KEY_W"])) then
        cameraPosition.x = cameraPosition.x + math.cos(cameraRotation.y + halfPi) * deltaSeconds * speed;
        cameraPosition.z = cameraPosition.z + math.sin(cameraRotation.y + halfPi) * deltaSeconds * speed;
    end

    if(getKeyDown(glfw["KEY_A"])) then
        cameraPosition.x = cameraPosition.x + math.cos(cameraRotation.y) * deltaSeconds * speed;
        cameraPosition.z = cameraPosition.z + math.sin(cameraRotation.y) * deltaSeconds * speed;
    end
    
    if(getKeyDown(glfw["KEY_S"])) then
        cameraPosition.x = cameraPosition.x - math.cos(cameraRotation.y + halfPi) * deltaSeconds * speed;
        cameraPosition.z = cameraPosition.z - math.sin(cameraRotation.y + halfPi) * deltaSeconds * speed;
    end

    if(getKeyDown(glfw["KEY_D"])) then
        cameraPosition.x = cameraPosition.x - math.cos(cameraRotation.y) * deltaSeconds * speed;
        cameraPosition.z = cameraPosition.z - math.sin(cameraRotation.y) * deltaSeconds * speed;
    end

    setCameraPosition(cameraPosition.x, cameraPosition.y, cameraPosition.z);
    setCameraRotation(cameraRotation.x, cameraRotation.y, cameraRotation.z);

    lastClockTime = os.clock();
end