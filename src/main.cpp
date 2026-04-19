#include "game.h"

int main()
{
    GV::Game game;

    if (!game.Initialize())
    {
        return -1;
    }

    game.Run();
    game.Shutdown();

    return 0;
}