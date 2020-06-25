local cube = createPart("res/3d/cube.obj");
local dragon = createPart("res/3d/dragon.obj");

setSize(cube, 8, 0.5, 8);

setPosition(cube, 0, -5, 0);
setPosition(dragon, 0, -5, 0);

setColor(dragon, 0.25, 0.25, 0.9);

setCameraPosition(0, 0, 15);

local x = 0;

function tick()
    x = x +  1/60;
    setRotation(cube, 0, x, 0);
    setRotation(dragon, 0, x, 0);
end