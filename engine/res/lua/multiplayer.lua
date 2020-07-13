function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end

local playerParts = {};

for i, player in ipairs(Game.GetPlayers()) do
	if player.Name ~= Game.Local.LocalPlayer.Name then
		if playerParts[player.Name] == nil then
			createPlayer(player);
		end
	end
end

local bWalkTime = os.clock();
local prevClock = os.clock();

function playerUpdate()
	local deltaSeconds = os.clock() - prevClock;
	for name, p in pairs(playerParts) do
		playerParts[name].head.Position = Game.GetPlayers()[name].head.Position;
		playerParts[name].head.Orientation = Game.GetPlayers()[name].head.Orientation;

		local xori = math.sin(math.rad(playerParts[name].head.Orientation.y + 90));
		local zori = math.cos(math.rad(playerParts[name].head.Orientation.y + 90));

		if not Game.GetPlayerWalking(name) then
			playerParts[name].torso.Orientation = playerParts[name].head.Orientation;
			playerParts[name].leftArm.Orientation = Vector3.create(0, playerParts[name].head.Orientation.y + 90, playerParts[name].leftArm.Orientation.z - playerParts[name].leftArm.Orientation.z *	 	deltaSeconds * 15);
			playerParts[name].rightArm.Orientation = Vector3.create(0, playerParts[name].head.Orientation.y + 90, playerParts[name].rightArm.Orientation.z - playerParts[name].rightArm.Orientation.z * 	deltaSeconds * 15);
			playerParts[name].leftLeg.Orientation = Vector3.create(0, playerParts[name].head.Orientation.y + 90, playerParts[name].leftLeg.Orientation.z - playerParts[name].leftLeg.Orientation.z * 		deltaSeconds * 15);
			playerParts[name].rightLeg.Orientation = Vector3.create(0, playerParts[name].head.Orientation.y + 90, playerParts[name].rightLeg.Orientation.z - playerParts[name].rightLeg.Orientation.z * 	deltaSeconds * 15);

			local zo = math.cos(math.rad(playerParts[name].head.Orientation.y));

			playerParts[name].torso.Position = Vector3.create(playerParts[name].head.Position.x, 2.5, playerParts[name].head.Position.z);
			playerParts[name].leftArm.Position = Vector3.create(-1.125 * xori + playerParts[name].head.Position.x, 2.5 - math.cos(math.rad(playerParts[name].leftArm.Orientation.z )) * 0.40 + 0.40, -1.125 * zori - zo * math.sin(math.rad(playerParts[name].leftArm.Orientation.z )) * 0.45 + playerParts[name].head.Position.z);
			playerParts[name].rightArm.Position = Vector3.create(1.125 * xori + playerParts[name].head.Position.x, 2.5 - math.cos(math.rad(playerParts[name].rightArm.Orientation.z)) * 0.40 + 0.40, 1.1250 * zori + zo * math.sin(math.rad(playerParts[name].rightArm.Orientation.z)) * 0.45 + playerParts[name].head.Position.z);
			playerParts[name].leftLeg.Position = Vector3.create(-0.375 * xori + playerParts[name].head.Position.x, 0.8 - math.cos(math.rad(playerParts[name].leftLeg.Orientation.z )) * 0.35 + 0.45, -0.375 * zori + zo * math.sin(math.rad(playerParts[name].leftLeg.Orientation.z )) * 0.40 + playerParts[name].head.Position.z);
			playerParts[name].rightLeg.Position = Vector3.create(0.375 * xori + playerParts[name].head.Position.x, 0.8 - math.cos(math.rad(playerParts[name].rightLeg.Orientation.z)) * 0.35 + 0.45, 0.3750 * zori - zo * math.sin(math.rad(playerParts[name].rightLeg.Orientation.z)) * 0.40 + playerParts[name].head.Position.z);
		else
			local limbTimer = math.sin(7.5 * (os.clock()));
			local limbOrientation = math.deg(math.tan(limbTimer)) * 0.75;

			playerParts[name].torso.Orientation = playerParts[name].head.Orientation;
			playerParts[name].leftArm.Orientation = Vector3.create(0, playerParts[name].head.Orientation.y + 90, limbOrientation);
			playerParts[name].rightArm.Orientation = Vector3.create(0, playerParts[name].head.Orientation.y + 90, -limbOrientation);
			playerParts[name].leftLeg.Orientation = Vector3.create(0, playerParts[name].head.Orientation.y + 90, -limbOrientation);
			playerParts[name].rightLeg.Orientation = Vector3.create(0, playerParts[name].head.Orientation.y + 90, limbOrientation);
			
			local xpos = math.sin(math.rad(playerParts[name].head.Orientation.y + 90));
			local zpos = math.cos(math.rad(playerParts[name].head.Orientation.y + 90));
			local lxzpos = math.sin(limbTimer);
			local lypos = math.cos(limbTimer);
	
			local xo = math.sin(math.rad(playerParts[name].head.Orientation.y));
			local zo = math.cos(math.rad(playerParts[name].head.Orientation.y));

			playerParts[name].torso.Position = Vector3.create(playerParts[name].head.Position.x, 2.5, playerParts[name].head.Position.z);
			playerParts[name].leftArm.Position = Vector3.create(-1.125 * xori - xo * lxzpos * 0.45 + playerParts[name].head.Position.x, 2.5 - lypos * 0.40 + 0.40, -1.125 * zori - zo * lxzpos * 0.45 + playerParts[name].head.Position.z);
			playerParts[name].rightArm.Position = Vector3.create(1.125 * xori + xo * lxzpos * 0.45 + playerParts[name].head.Position.x, 2.5 - lypos * 0.40 + 0.40, 1.1250 * zori + zo * lxzpos * 0.45 + playerParts[name].head.Position.z);
			playerParts[name].leftLeg.Position = Vector3.create(-0.375 * xori + xo * lxzpos * 0.40 + playerParts[name].head.Position.x, 0.8 - lypos * 0.35 + 0.45, -0.375 * zori + zo * lxzpos * 0.40 + playerParts[name].head.Position.z);
			playerParts[name].rightLeg.Position = Vector3.create(0.375 * xori - xo * lxzpos * 0.40 + playerParts[name].head.Position.x, 0.8 - lypos * 0.35 + 0.45, 0.3750 * zori - zo * lxzpos * 0.40 + playerParts[name].head.Position.z);
		end
	end
	prevClock = os.clock();
end

function createPlayer(player)
	playerParts[player.Name] = {
		head = Object.create("res/3d/head.obj"),
		torso = Object.create(),
		leftArm = Object.create(),
		rightArm = Object.create(),
		leftLeg = Object.create(),
		rightLeg = Object.create()
	};

	playerParts[player.Name].head.Color = ColorRGB.create(255, 215, 100);
	playerParts[player.Name].torso.Color = ColorRGB.create(100, 175, 255);
	playerParts[player.Name].leftArm.Color = ColorRGB.create(255, 215, 100);
	playerParts[player.Name].rightArm.Color = ColorRGB.create(255, 215, 100);
	playerParts[player.Name].leftLeg.Color = ColorRGB.create(150, 225, 120);
	playerParts[player.Name].rightLeg.Color = ColorRGB.create(150, 225, 120);

	playerParts[player.Name].head.Position = player.head.Position;
	playerParts[player.Name].head.Orientation = player.head.Orientation;

	playerParts[player.Name].torso.Position = Vector3.create(playerParts[player.Name].head.Position.x, 2.5, playerParts[player.Name].head.Position.z);
	playerParts[player.Name].leftArm.Position = Vector3.create(-1.125 * math.sin(math.rad(playerParts[player.Name].head.Orientation.y + 90)) + playerParts[player.Name].head.Position.x, 2.5, -1.125 * math.cos(math.rad(playerParts[player.Name].head.Orientation.y + 90)) + playerParts[player.Name].head.Position.z);
	playerParts[player.Name].rightArm.Position = Vector3.create(1.125 * math.sin(math.rad(playerParts[player.Name].head.Orientation.y + 90)) + playerParts[player.Name].head.Position.x, 2.5, 1.125 * math.cos(math.rad(playerParts[player.Name].head.Orientation.y + 90)) + playerParts[player.Name].head.Position.z);
	playerParts[player.Name].leftLeg.Position = Vector3.create(-0.375 * math.sin(math.rad(playerParts[player.Name].head.Orientation.y + 90)) + playerParts[player.Name].head.Position.x, 0.8, -0.375 * math.cos(math.rad(playerParts[player.Name].head.Orientation.y + 90)) + playerParts[player.Name].head.Position.z);
	playerParts[player.Name].rightLeg.Position = Vector3.create(0.375 * math.sin(math.rad(playerParts[player.Name].head.Orientation.y + 90)) + playerParts[player.Name].head.Position.x, 0.8, 0.375 * math.cos(math.rad(playerParts[player.Name].head.Orientation.y + 90)) + playerParts[player.Name].head.Position.z);
		
	playerParts[player.Name].torso.Orientation = playerParts[player.Name].head.Orientation;
	playerParts[player.Name].leftArm.Orientation = playerParts[player.Name].head.Orientation;
	playerParts[player.Name].rightArm.Orientation = playerParts[player.Name].head.Orientation;
	playerParts[player.Name].leftLeg.Orientation = playerParts[player.Name].head.Orientation;
	playerParts[player.Name].rightLeg.Orientation = playerParts[player.Name].head.Orientation;

	
	playerParts[player.Name].torso.Size = Vector3.create(0.75, 0.9, 0.375);
	playerParts[player.Name].leftArm.Size = Vector3.create(0.375, 0.9, 0.375);
	playerParts[player.Name].rightArm.Size = Vector3.create(0.375, 0.9, 0.375);
	playerParts[player.Name].leftLeg.Size = Vector3.create(0.375, 0.8, 0.375);
	playerParts[player.Name].rightLeg.Size = Vector3.create(0.375, 0.8, 0.375);
end

Events.PlayerJoin.addListener(createPlayer);
Events.Tick.addListener(playerUpdate);

return {};