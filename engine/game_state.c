#include "game_state.h"

#include "loaders.h"
#include "player.h"
#include "structs.h"

GameState
initGameState(void)
{
  GameState state = { 0 };

  state.engineConfig = loadConfig();
  state.currentScene = 1;

  state.scene = loadScene(state.currentScene);
  state.shader = loadShader(state.currentScene);

  state.player = state.scene.spawn;
  updatePlayerDirection(&state.player);

  return state;
}

void
updateScene(GameState* gameState)
{
  freeScene(&gameState->scene);
  gameState->scene = loadScene(gameState->currentScene);
  gameState->player = gameState->scene.spawn;

  updatePlayerDirection(&gameState->player);
  gameState->shader = loadShader(gameState->currentScene);
}