struct world;
struct dialog;

enum state {
  TITLE_SCREEN,
  IN_GAME,
  SHOWING_DIALOG,
};

struct game_state {
  struct world *mzx_world;
  struct dialog *di;
  enum state last_state;
  enum state current_state;
  Uint32 delay_begin;
  Uint32 delay;
};
