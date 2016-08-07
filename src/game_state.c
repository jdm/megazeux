#include "game_state.h"
#include "window.h"
#include "world.h"

void run_game_state(game_state *game)
{
  if (game->delay && get_ticks() < game->delay_begin + game->delay) {
    return;
  }

  switch (game->current_state) {
    case SHOWING_DIALOG:
      break;
    case IN_GAME:
  }
}
