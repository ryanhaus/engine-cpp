local head = Object.create("res/3d/head.obj");
local torso = Object.create();
local leftArm = Object.create();
local rightArm = Object.create();
local leftLeg = Object.create();
local rightLeg = Object.create();
local baseplate = Object.create();

Game.Camera.Position = Vector3.create(0, 2.5, 10);

head.Position = Vector3.create(0, 3.85, 0);
head.Color = ColorRGB.create(255, 215, 100);

torso.Position = Vector3.create(0, 2.5, 0);
torso.Size = Vector3.create(0.75, 0.9, 0.375);
torso.Color = ColorRGB.create(100, 175, 255);

leftArm.Position = Vector3.create(-1.125, 2.5, 0);
leftArm.Size = Vector3.create(0.375, 0.9, 0.375);
leftArm.Color = ColorRGB.create(255, 215, 100);

rightArm.Position = Vector3.create(1.125, 2.5, 0);
rightArm.Size = Vector3.create(0.375, 0.9, 0.375);
rightArm.Color = ColorRGB.create(255, 215, 100);

leftLeg.Position = Vector3.create(-0.375, 0.8, 0);
leftLeg.Size = Vector3.create(0.375, 0.8, 0.375);
leftLeg.Color = ColorRGB.create(150, 225, 120);

rightLeg.Position = Vector3.create(0.375, 0.8, 0);
rightLeg.Size = Vector3.create(0.375, 0.8, 0.375);
rightLeg.Color = ColorRGB.create(150, 225, 120);

baseplate.Position = Vector3.create(0, -1, 0);
baseplate.Size = Vector3.create(32, 1, 32);
baseplate.Color = ColorRGB.create(225, 225, 225);

local halfPi = math.asin(1);
local prevClock = os.clock();
local prevCursor = Input.MousePosition;
local throwMouse;

local cameraDistance = 10;
local lastFrameRtClick = Input.isDown(Input.MouseCode.Right);

local speed = 8;

Events.Tick.addListener(function()
	local cursorPosition = Input.MousePosition;
	local deltaSeconds = os.clock() - prevClock;
	local deltaCursor = {x = cursorPosition.x - prevCursor.x, y = cursorPosition.y - prevCursor.y};

	if Input.isDown(Input.MouseCode.Right) and not lastFrameRtClick then
		Input.CurrentCursorState = Input.CursorState.Hidden;
		Input.updateScreenInput();
		Input.CurrentCursorState = Input.CursorState.Disabled;
		Input.updateScreenInput();
		throwMouse = Input.MousePosition;
	end

	if not Input.isDown(Input.MouseCode.Right) and lastFrameRtClick then
		Input.updateMousePosition(throwMouse.x, throwMouse.y);
		Input.CurrentCursorState = Input.CursorState.Normal;
		Input.updateScreenInput();
	end

	if Input.isDown(Input.MouseCode.Right) then
		Game.Camera.Orientation.x = Game.Camera.Orientation.x + deltaCursor.y / 350;
		Game.Camera.Orientation.y = Game.Camera.Orientation.y + deltaCursor.x / 325;
	end

	if Game.Camera.Orientation.x > halfPi * 0.995 then
		Game.Camera.Orientation.x = halfPi * 0.995;
	end

	if Game.Camera.Orientation.x < halfPi * -0.995 then
		Game.Camera.Orientation.x = halfPi * -0.995;
	end

	if Input.isDown(Input.KeyCode.I) then
        cameraDistance = math.min(math.max(2, cameraDistance - 32 * deltaSeconds), 32);
    end

    if Input.isDown(Input.KeyCode.O) then
        cameraDistance = math.min(math.max(2, cameraDistance + 32 * deltaSeconds), 32);
    end

	if Input.isDown(Input.KeyCode.RightArrow) then
        Game.Camera.Orientation.y = Game.Camera.Orientation.y + deltaSeconds * 1.75;
    end

    if Input.isDown(Input.KeyCode.LeftArrow) then
        Game.Camera.Orientation.y = Game.Camera.Orientation.y - deltaSeconds * 1.75;
    end

	local targetYRotation = 0;	
    local avgN = 0;	

	Game.Camera.Orientation.y = Game.Camera.Orientation.y + halfPi;

    local forwards = Input.isDown(Input.KeyCode.W) or Input.isDown(Input.KeyCode.Up);
    local backwards = Input.isDown(Input.KeyCode.S) or Input.isDown(Input.KeyCode.Down);

    if forwards and not backwards then	
        targetYRotation = targetYRotation + Game.Camera.Orientation.y;	
        avgN = avgN + 1;	
    end	

    if backwards and Input.isDown(Input.KeyCode.A) then	
        targetYRotation = targetYRotation + Game.Camera.Orientation.y + halfPi * 2.5;	
        avgN = avgN + 1;	
    else	
        if Input.isDown(Input.KeyCode.A) and not Input.isDown(Input.KeyCode.D) then	
            targetYRotation = targetYRotation + Game.Camera.Orientation.y - halfPi;	
            avgN = avgN + 1;	
        end	

        if backwards and not forwards then	
            targetYRotation = targetYRotation + Game.Camera.Orientation.y + halfPi * 2;	
            avgN = avgN + 1;	
        end	
    end	

    if Input.isDown(Input.KeyCode.D) and not Input.isDown(Input.KeyCode.A) then	
        targetYRotation = targetYRotation + Game.Camera.Orientation.y + halfPi;	
        avgN = avgN + 1;	
    end	

	targetYRotation = targetYRotation - halfPi * avgN;

    if avgN > 0 then	
        head.Orientation.y = -90 / halfPi * ((head.Orientation.y * halfPi / -90 - targetYRotation / avgN) * -8 * deltaSeconds + head.Orientation.y * halfPi / -90);
		head.Position.x = head.Position.x + math.sin(targetYRotation / avgN) * deltaSeconds * speed;	
        head.Position.z = head.Position.z - math.cos(targetYRotation / avgN) * deltaSeconds * speed;	
    end

	Game.Local.LocalPlayer.Head = head;

	torso.Position = Vector3.create(head.Position.x, 2.5, head.Position.z);
	leftArm.Position = Vector3.create(-1.125 * math.sin(math.rad(head.Orientation.y + 90)) + head.Position.x, 2.5, -1.125 * math.cos(math.rad(head.Orientation.y + 90)) + head.Position.z);
	rightArm.Position = Vector3.create(1.125 * math.sin(math.rad(head.Orientation.y + 90)) + head.Position.x, 2.5, 1.125 * math.cos(math.rad(head.Orientation.y + 90)) + head.Position.z);
	leftLeg.Position = Vector3.create(-0.375 * math.sin(math.rad(head.Orientation.y + 90)) + head.Position.x, 0.8, -0.375 * math.cos(math.rad(head.Orientation.y + 90)) + head.Position.z);
	rightLeg.Position = Vector3.create(0.375 * math.sin(math.rad(head.Orientation.y + 90)) + head.Position.x, 0.8, 0.375 * math.cos(math.rad(head.Orientation.y + 90)) + head.Position.z);

	torso.Orientation = head.Orientation;
	leftArm.Orientation = head.Orientation;
	rightArm.Orientation = head.Orientation;
	leftLeg.Orientation = head.Orientation;
	rightLeg.Orientation = head.Orientation;

	Game.Camera.Position = Vector3.create(
        math.cos(Game.Camera.Orientation.y) * math.cos(Game.Camera.Orientation.x) * cameraDistance + head.Position.x,
        math.sin(Game.Camera.Orientation.x) * cameraDistance + head.Position.y,
        math.cos(Game.Camera.Orientation.x) * math.sin(Game.Camera.Orientation.y) * cameraDistance + head.Position.z
	);
	
	Game.Camera.Orientation.y = Game.Camera.Orientation.y - halfPi;

	lastFrameRtClick = Input.isDown(Input.MouseCode.Right);
	prevCursor = cursorPosition;
	prevClock = os.clock();
end);

return {};