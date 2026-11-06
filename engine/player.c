#include "player.h"
#include "helper.h"
#include <math.h>
// NOTICE!
// --- Move this function into file handling C file later on ---
void updatePlayerDirection(Player *player)
{
    player->dx = cosf(degToRad(player->ang));
    player->dy = -sinf(degToRad(player->ang));
}