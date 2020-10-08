#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL.h>

#define NUMBERS 10
#define DIGITS 3

typedef struct {
	double position[2];
	bool active;
	double last_move;
} Shot;

typedef struct {
	Shot shot;
	double position;
	bool shooting;
	bool alive;
	int score;
	double last_move;
} Player;

typedef struct {
	double position[2];
	double velocity;
	double last_move;
} Enemy;

typedef struct {
	Enemy* enemies;
	int count;
} EnemyContainer;

typedef struct {
	SDL_Surface* background_surface;
	SDL_Rect* background_rects;
	SDL_Surface* player_surface;
	SDL_Surface* shot_surface;
	SDL_Surface* enemy_surface;
	SDL_Surface* score_surface;
	SDL_Rect* score_rects;
	SDL_Surface* number_surfaces[NUMBERS];
	SDL_Rect* number_rects;
	Player* players;
	EnemyContainer* enemy_containers;
	double last_spawn;
	int count;
} GameSet;

typedef struct {
	GameSet game_set;
	const Uint8* keyboard;
	SDL_Surface* window_surface;
	int index;
} ThreadPackage;

const char* const TITLE = "Space Defence 3 by Chigozie Agomo";

const char* const BACKGROUND_IMAGE = "data/background.bmp";
const char* const TITLE_IMAGE = "data/title.bmp";
const char* const MODES_IMAGE = "data/modes.bmp";
const char* const HELP_IMAGE = "data/help.bmp";
const char* const BACKGROUND_IMAGE_2 = "data/background2.bmp";
const char* const PLAYER_IMAGE = "data/player.bmp";
const char* const SHOT_IMAGE = "data/shot.bmp";
const char* const ENEMY_IMAGE = "data/enemy.bmp";
const char* const SCORE_IMAGE = "data/score.bmp";
const char* const NUMBER_IMAGES[] = {
	"data/0.bmp",
	"data/1.bmp",
	"data/2.bmp",
	"data/3.bmp",
	"data/4.bmp",
	"data/5.bmp",
	"data/6.bmp",
	"data/7.bmp",
	"data/8.bmp",
	"data/9.bmp"
};

const char* const MENU_SONG = "data/menusong.wav";
const char* const GAME_SONG = "data/gamesong.wav";

const int BACKGROUND_COLOUR[] = {0x0, 0x0, 0x0};

const int SURFACE_DEPTH = 32;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	const Uint32 SURFACE_MASKS[] = {
		0xff000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff
	};
#else
	const Uint32 SURFACE_MASKS[] = {
		0x000000ff,
		0x0000ff00,
		0x00ff0000,
		0xff000000
	};
#endif

const double TITLE_WIDTH_RATIO = 0.75;
const double TITLE_HEIGHT_RATIO = 0.3;
const double TITLE_X_RATIO = 0.5;
const double TITLE_Y_RATIO = 0.2;
const double MODES_WIDTH_RATIO = 0.25;
const double MODES_HEIGHT_RATIO = 0.4;
const double MODES_X_RATIO = 0.5;
const double MODES_Y_RATIO = 0.75;
const double HELP_WIDTH_RATIO = 0.9;
const double HELP_HEIGHT_RATIO = 0.9;
const double HELP_X_RATIO = 0.5;
const double HELP_Y_RATIO = 0.5;
const double BACKGROUND_WIDTH_RATIO = 0.4;
const double PLAYER_WIDTH_RATIO = 0.07;
const double PLAYER_HEIGHT_RATIO = 0.2;
const double PLAYER_Y_RATIO = 0.85;
const double SHOT_WIDTH_RATIO = 0.0035;
const double SHOT_HEIGHT_RATIO = 0.3;
const double ENEMY_WIDTH_RATIO = 0.06;
const double ENEMY_HEIGHT_RATIO = 0.25;
const double SCORE_WIDTH_RATIO = 0.15;
const double SCORE_HEIGHT_RATIO = 0.1;
const double SCORE_X_RATIO = 0.5;
const double SCORE_Y_RATIO = 0.1875;
const double SCORE_Y_SHIFT = 0.5;
const double NUMBER_WIDTH_RATIO = 0.03;
const double NUMBER_HEIGHT_RATIO = 0.08;
const double NUMBER_X_RATIO = 0.5;
const double NUMBER_Y_RATIO = 0.3125;

const int MENU_SONG_LENGTH = 63;
const int GAME_SONG_LENGTH = 127;

const int QUIT = SDL_SCANCODE_ESCAPE;
const int RESET = SDL_SCANCODE_R;
const int PAUSE = SDL_SCANCODE_P;
const int CLICK = SDL_BUTTON(SDL_BUTTON_LEFT);

const int PLAYER_ONE_LEFT = SDL_SCANCODE_A;
const int PLAYER_ONE_RIGHT = SDL_SCANCODE_D;
const int PLAYER_ONE_SHOOT = SDL_SCANCODE_W;

const int PLAYER_TWO_LEFT = SDL_SCANCODE_LEFT;
const int PLAYER_TWO_RIGHT = SDL_SCANCODE_RIGHT;
const int PLAYER_TWO_SHOOT = SDL_SCANCODE_UP;

const double PLAYER_SPEED = 500;
const double SHOT_VELOCITY = -3000;

const double BASE_ENEMY_VELOCITY = 50;
const double ENEMY_ACCELERATION = 2.5;
const double SPAWN_DELAY = 0.5;

double get_time() {
	return (double) clock() / CLOCKS_PER_SEC;
}

double get_random(double low, double high) {
	return low + (high - low) * rand() / RAND_MAX;
}

double get_velocity(int score) {
	return BASE_ENEMY_VELOCITY + score * ENEMY_ACCELERATION;
}

void initialise(GameSet* game_set, SDL_Surface* window_surface) {
	for (int i = 0; i < game_set->count; i++) {
		game_set->players[i].position = game_set->background_surface->w / 2 + i * (window_surface->w - game_set->background_surface->w);
		game_set->players[i].shooting = false;
		game_set->players[i].alive = true;
		game_set->players[i].score = 0;
		game_set->players[i].shot.active = false;
		game_set->players[i].last_move = get_time();
		game_set->players[i].shot.last_move = get_time();
		
		game_set->enemy_containers[i].enemies = NULL;
		game_set->enemy_containers[i].count = 0;
	}
	
	game_set->last_spawn = get_time();
}

void spawn(GameSet game_set, SDL_Surface* window_surface) {
	double displacement = get_random(game_set.enemy_surface->w / 2, game_set.background_surface->w - game_set.enemy_surface->w / 2);
	
	for (int i = 0; i < game_set.count; i++) {
		if (game_set.players[i].alive) {
			game_set.enemy_containers[i].enemies = realloc(
				game_set.enemy_containers[i].enemies,
				sizeof(Enemy) * (game_set.enemy_containers[i].count + 1)
			);
			
			game_set.enemy_containers[i].enemies[game_set.enemy_containers[i].count].position[0] =
				displacement + i * (window_surface->w - game_set.background_surface->w);
			game_set.enemy_containers[i].enemies[game_set.enemy_containers[i].count].position[1] = -game_set.enemy_surface->h / 2;
			game_set.enemy_containers[i].enemies[game_set.enemy_containers[i].count].velocity = get_velocity(game_set.players[i].score);
			
			game_set.enemy_containers[i].count++;
			
			game_set.enemy_containers[i].enemies[game_set.enemy_containers[i].count - 1].last_move = get_time();
		}
	}
}

void update_shot(GameSet game_set, int index, SDL_Surface* window_surface) {
	if (game_set.players[index].shot.active) {
		game_set.players[index].shot.position[1] += SHOT_VELOCITY * (get_time() - game_set.players[index].shot.last_move);
		game_set.players[index].shot.last_move = get_time();
		
		SDL_Rect shot_rect = {
			.x = game_set.players[index].shot.position[0] - game_set.shot_surface->w / 2,
			.y = game_set.players[index].shot.position[1] - game_set.shot_surface->h / 2,
			.w = game_set.shot_surface->w,
			.h = game_set.shot_surface->h
		};
		
		SDL_BlitSurface(game_set.shot_surface, NULL, window_surface, &shot_rect);
		
		for (int i = 0; i < game_set.enemy_containers[index].count; i++) {
			SDL_Rect enemy_rect = {
				.x = game_set.enemy_containers[index].enemies[i].position[0] - game_set.enemy_surface->w / 2,
				.y = game_set.enemy_containers[index].enemies[i].position[1] - game_set.enemy_surface->h / 2,
				.w = game_set.enemy_surface->w,
				.h = game_set.enemy_surface->h
			};
			
			if (SDL_HasIntersection(&shot_rect, &enemy_rect)) {
				game_set.players[index].score++;
				game_set.players[index].shot.active = false;
				
				for (int j = i; j < game_set.enemy_containers[index].count - 1; j++) {
					game_set.enemy_containers[index].enemies[j] = game_set.enemy_containers[index].enemies[j + 1];
				}
				
				game_set.enemy_containers[index].enemies = realloc(
					game_set.enemy_containers[index].enemies,
					sizeof(Enemy) * --game_set.enemy_containers[index].count
				);
				
				return;
			}
		}
		
		if (game_set.players[index].shot.position[1] < -game_set.shot_surface->w / 2) {
			game_set.players[index].shot.active = false;
		}
	}
}

void shoot(Player* player, SDL_Surface* window_surface) {
	player->shot.position[0] = player->position;
	player->shot.position[1] = PLAYER_Y_RATIO * window_surface->h;
	player->shot.active = true;
	player->shot.last_move = get_time();
}

int update_thread(void* thread_package) {
	GameSet game_set = ((ThreadPackage*) thread_package)->game_set;
	const Uint8* keyboard = ((ThreadPackage*) thread_package)->keyboard;
	SDL_Surface* window_surface = ((ThreadPackage*) thread_package)->window_surface;
	int i = ((ThreadPackage*) thread_package)->index;
	
	if (game_set.players[i].alive) {
		update_shot(game_set, i, window_surface);
		
		switch (game_set.count) {
			case 1:
				game_set.players[i].position +=
					((keyboard[PLAYER_ONE_RIGHT] || keyboard[PLAYER_TWO_RIGHT]) - (keyboard[PLAYER_ONE_LEFT] || keyboard[PLAYER_TWO_LEFT]))
					* PLAYER_SPEED * (get_time() - game_set.players[i].last_move);
				break;
					
			case 2:
				switch (i) {
					case 0:
						game_set.players[i].position +=
							(keyboard[PLAYER_ONE_RIGHT] - keyboard[PLAYER_ONE_LEFT])
							* PLAYER_SPEED * (get_time() - game_set.players[i].last_move);
						break;
				
					case 1:
						game_set.players[i].position +=
							(keyboard[PLAYER_TWO_RIGHT] - keyboard[PLAYER_TWO_LEFT])
							* PLAYER_SPEED * (get_time() - game_set.players[i].last_move);
						break;
				}
				
				break;
		}
		
		game_set.players[i].last_move = get_time();
		
		bool shooting;
		
		switch (game_set.count) {
			case 1:
				shooting = keyboard[PLAYER_ONE_SHOOT] || keyboard[PLAYER_TWO_SHOOT];
				break;
				
			case 2:
				switch (i) {
					case 0:
						shooting = keyboard[PLAYER_ONE_SHOOT];
						break;
		
					case 1:
						shooting = keyboard[PLAYER_TWO_SHOOT];
						break;
				}
				
				break;
		}
		
		if (shooting) {
			if (!game_set.players[i].shooting) {
				if (!game_set.players[i].shot.active) {
					shoot(&game_set.players[i], window_surface);
				}
				
				game_set.players[i].shooting = true;
			}
		}
		
		else if (game_set.players[i].shooting) {
			game_set.players[i].shooting = false;
		}
		
		if (game_set.players[i].position < game_set.player_surface->w / 2 + i * (window_surface->w - game_set.background_surface->w)) {
			game_set.players[i].position = game_set.player_surface->w / 2 + i * (window_surface->w - game_set.background_surface->w);
		}
		
		else if (
			game_set.players[i].position
			> game_set.background_surface->w - game_set.player_surface->w / 2 + i * (window_surface->w - game_set.background_surface->w)
		) {
			game_set.players[i].position = 
				game_set.background_surface->w - game_set.player_surface->w / 2 + i * (window_surface->w - game_set.background_surface->w);
		}
		
		SDL_Rect player_rect = {
			.x = game_set.players[i].position - game_set.player_surface->w / 2,
			.y = PLAYER_Y_RATIO * window_surface->h - game_set.player_surface->h / 2,
			.w = game_set.player_surface->w,
			.h = game_set.player_surface->h
		};
		
		SDL_BlitSurface(game_set.player_surface, NULL, window_surface, &player_rect);
		
		for (int j = 0; j < game_set.enemy_containers[i].count; j++) {
			game_set.enemy_containers[i].enemies[j].position[1] +=
				game_set.enemy_containers[i].enemies[j].velocity
				* (get_time() - game_set.enemy_containers[i].enemies[j].last_move);
			game_set.enemy_containers[i].enemies[j].last_move = get_time();
			
			SDL_Rect enemy_rect = {
				.x = game_set.enemy_containers[i].enemies[j].position[0] - game_set.enemy_surface->w / 2,
				.y = game_set.enemy_containers[i].enemies[j].position[1] - game_set.enemy_surface->h / 2,
				.w = game_set.enemy_surface->w,
				.h = game_set.enemy_surface->h
			};
			
			SDL_BlitSurface(game_set.enemy_surface, NULL, window_surface, &enemy_rect);
			
			if (
				SDL_HasIntersection(&player_rect, &enemy_rect)
				|| game_set.enemy_containers[i].enemies[j].position[1] >= window_surface->h - game_set.enemy_surface->h / 2
			) {
				game_set.players[i].alive = false;
			}
		}
	}
	
	return 0;
}

bool update2(GameSet game_set, const Uint8* keyboard, SDL_Surface* window_surface) {
	SDL_Thread** threads = malloc(sizeof(SDL_Thread*) * game_set.count);
	ThreadPackage* thread_packages = malloc(sizeof(ThreadPackage) * game_set.count);
	
	for (int i = 0; i < game_set.count; i++) {
		thread_packages[i].game_set = game_set;
		thread_packages[i].keyboard = keyboard;
		thread_packages[i].window_surface = window_surface;
		thread_packages[i].index = i;
		threads[i] = SDL_CreateThread(update_thread, NULL, &thread_packages[i]);
	}
	
	bool alive = false;
	
	for (int i = 0; i < game_set.count; i++) {
		SDL_WaitThread(threads[i], NULL);
		
		if (game_set.players[i].alive) {
			alive = true;
		}
	}
	
	free(threads);
	free(thread_packages);
	
	return !alive;
}
bool update(GameSet game_set, const Uint8* keyboard, SDL_Surface* window_surface) {
	bool alive = false;
	
	for (int i = 0; i < game_set.count; i++) {
		if (game_set.players[i].alive) {
			alive = true;
			
			update_shot(game_set, i, window_surface);
			
			switch (game_set.count) {
				case 1:
					game_set.players[i].position +=
						((keyboard[PLAYER_ONE_RIGHT] || keyboard[PLAYER_TWO_RIGHT]) - (keyboard[PLAYER_ONE_LEFT] || keyboard[PLAYER_TWO_LEFT]))
						* PLAYER_SPEED * (get_time() - game_set.players[i].last_move);
					break;
						
				case 2:
					switch (i) {
						case 0:
							game_set.players[i].position +=
								(keyboard[PLAYER_ONE_RIGHT] - keyboard[PLAYER_ONE_LEFT])
								* PLAYER_SPEED * (get_time() - game_set.players[i].last_move);
							break;
					
						case 1:
							game_set.players[i].position +=
								(keyboard[PLAYER_TWO_RIGHT] - keyboard[PLAYER_TWO_LEFT])
								* PLAYER_SPEED * (get_time() - game_set.players[i].last_move);
							break;
					}
					
					break;
			}
			
			game_set.players[i].last_move = get_time();
			
			bool shooting;
			
			switch (game_set.count) {
				case 1:
					shooting = keyboard[PLAYER_ONE_SHOOT] || keyboard[PLAYER_TWO_SHOOT];
					break;
					
				case 2:
					switch (i) {
						case 0:
							shooting = keyboard[PLAYER_ONE_SHOOT];
							break;
			
						case 1:
							shooting = keyboard[PLAYER_TWO_SHOOT];
							break;
					}
					
					break;
			}
			
			if (shooting) {
				if (!game_set.players[i].shooting) {
					if (!game_set.players[i].shot.active) {
						shoot(&game_set.players[i], window_surface);
					}
					
					game_set.players[i].shooting = true;
				}
			}
			
			else if (game_set.players[i].shooting) {
				game_set.players[i].shooting = false;
			}
			
			if (game_set.players[i].position < game_set.player_surface->w / 2 + i * (window_surface->w - game_set.background_surface->w)) {
				game_set.players[i].position = game_set.player_surface->w / 2 + i * (window_surface->w - game_set.background_surface->w);
			}
			
			else if (
				game_set.players[i].position
				> game_set.background_surface->w - game_set.player_surface->w / 2 + i * (window_surface->w - game_set.background_surface->w)
			) {
				game_set.players[i].position = 
					game_set.background_surface->w - game_set.player_surface->w / 2 + i * (window_surface->w - game_set.background_surface->w);
			}
			
			SDL_Rect player_rect = {
				.x = game_set.players[i].position - game_set.player_surface->w / 2,
				.y = PLAYER_Y_RATIO * window_surface->h - game_set.player_surface->h / 2,
				.w = game_set.player_surface->w,
				.h = game_set.player_surface->h
			};
			
			SDL_BlitSurface(game_set.player_surface, NULL, window_surface, &player_rect);
			
			for (int j = 0; j < game_set.enemy_containers[i].count; j++) {
				game_set.enemy_containers[i].enemies[j].position[1] +=
					game_set.enemy_containers[i].enemies[j].velocity
					* (get_time() - game_set.enemy_containers[i].enemies[j].last_move);
				game_set.enemy_containers[i].enemies[j].last_move = get_time();
				
				SDL_Rect enemy_rect = {
					.x = game_set.enemy_containers[i].enemies[j].position[0] - game_set.enemy_surface->w / 2,
					.y = game_set.enemy_containers[i].enemies[j].position[1] - game_set.enemy_surface->h / 2,
					.w = game_set.enemy_surface->w,
					.h = game_set.enemy_surface->h
				};
				
				SDL_BlitSurface(game_set.enemy_surface, NULL, window_surface, &enemy_rect);
				
				if (
					SDL_HasIntersection(&player_rect, &enemy_rect)
					|| game_set.enemy_containers[i].enemies[j].position[1] >= window_surface->h - game_set.enemy_surface->h / 2
				) {
					game_set.players[i].alive = false;
				}
			}
		}
	}
	
	return !alive;
}

void game(SDL_Window* window, SDL_Surface* window_surface, int players) {
	GameSet game_set = {
		.background_surface = SDL_CreateRGBSurface(
			0, BACKGROUND_WIDTH_RATIO * window_surface->w, window_surface->h, SURFACE_DEPTH,
			SURFACE_MASKS[0], SURFACE_MASKS[1], SURFACE_MASKS[2], SURFACE_MASKS[3]
		),
		.player_surface = SDL_CreateRGBSurface(
			0, PLAYER_WIDTH_RATIO * window_surface->w, PLAYER_HEIGHT_RATIO * window_surface->h, SURFACE_DEPTH,
			SURFACE_MASKS[0], SURFACE_MASKS[1], SURFACE_MASKS[2], SURFACE_MASKS[3]
		),
		.shot_surface = SDL_CreateRGBSurface(
			0, SHOT_WIDTH_RATIO * window_surface->w, SHOT_HEIGHT_RATIO * window_surface->h, SURFACE_DEPTH,
			SURFACE_MASKS[0], SURFACE_MASKS[1], SURFACE_MASKS[2], SURFACE_MASKS[3]
		),
		.enemy_surface = SDL_CreateRGBSurface(
			0, ENEMY_WIDTH_RATIO * window_surface->w, ENEMY_HEIGHT_RATIO * window_surface->h, SURFACE_DEPTH,
			SURFACE_MASKS[0], SURFACE_MASKS[1], SURFACE_MASKS[2], SURFACE_MASKS[3]
		),
		.score_surface = SDL_CreateRGBSurface(
			0, SCORE_WIDTH_RATIO * window_surface->w, SCORE_HEIGHT_RATIO * window_surface->h, SURFACE_DEPTH,
			SURFACE_MASKS[0], SURFACE_MASKS[1], SURFACE_MASKS[2], SURFACE_MASKS[3]
		),
		.count = players
	};
	
	SDL_Surface* raw_background_surface = SDL_LoadBMP(BACKGROUND_IMAGE_2);
	SDL_Surface* raw_player_surface = SDL_LoadBMP(PLAYER_IMAGE);
	SDL_Surface* raw_shot_surface = SDL_LoadBMP(SHOT_IMAGE);
	SDL_Surface* raw_enemy_surface = SDL_LoadBMP(ENEMY_IMAGE);
	SDL_Surface* raw_score_surface = SDL_LoadBMP(SCORE_IMAGE);
	
	SDL_BlitScaled(raw_background_surface, NULL, game_set.background_surface, NULL);
	SDL_BlitScaled(raw_player_surface, NULL, game_set.player_surface, NULL);
	SDL_BlitScaled(raw_shot_surface, NULL, game_set.shot_surface, NULL);
	SDL_BlitScaled(raw_enemy_surface, NULL, game_set.enemy_surface, NULL);
	SDL_BlitScaled(raw_score_surface, NULL, game_set.score_surface, NULL);
	
	SDL_FreeSurface(raw_background_surface);
	SDL_FreeSurface(raw_player_surface);
	SDL_FreeSurface(raw_shot_surface);
	SDL_FreeSurface(raw_enemy_surface);
	SDL_FreeSurface(raw_score_surface);
	
	for (int i = 0; i < NUMBERS; i++) {
		game_set.number_surfaces[i] = SDL_CreateRGBSurface(
			0, NUMBER_WIDTH_RATIO * window_surface->w, NUMBER_HEIGHT_RATIO * window_surface->h, SURFACE_DEPTH,
			SURFACE_MASKS[0], SURFACE_MASKS[1], SURFACE_MASKS[2], SURFACE_MASKS[3]
		);
		SDL_Surface* raw_number_surface = SDL_LoadBMP(NUMBER_IMAGES[i]);
		SDL_BlitScaled(raw_number_surface, NULL, game_set.number_surfaces[i], NULL);
		SDL_FreeSurface(raw_number_surface);
	}
	
	game_set.background_rects = malloc(sizeof(SDL_Rect) * game_set.count);
	game_set.score_rects = malloc(sizeof(SDL_Rect) * game_set.count);
	game_set.number_rects = malloc(sizeof(SDL_Rect) * DIGITS * game_set.count);
	game_set.players = malloc(sizeof(Player) * game_set.count);
	game_set.enemy_containers = malloc(sizeof(EnemyContainer) * game_set.count);
	
	for (int i = 0; i < game_set.count; i++) {
		game_set.background_rects[i].x = i * (window_surface->w - game_set.background_surface->w);
		game_set.background_rects[i].y = 0;
		
		game_set.score_rects[i].x = SCORE_X_RATIO * window_surface->w - game_set.score_surface->w / 2;
		game_set.score_rects[i].y =
			SCORE_Y_RATIO * window_surface->h - game_set.score_surface->h / 2
			+ i * (SCORE_Y_SHIFT * window_surface->h);
			
		for (int j = i * DIGITS; j < (i + 1) * DIGITS; j++) {
			game_set.number_rects[j].x =
				NUMBER_X_RATIO * window_surface->w - 3 * game_set.number_surfaces[0]->w / 2
				+ j % DIGITS * (game_set.number_surfaces[0]->w);
			game_set.number_rects[j].y =
				NUMBER_Y_RATIO * window_surface->h - game_set.number_surfaces[0]->h / 2
				+ i * (SCORE_Y_SHIFT * window_surface->h);
		}
	}
	
	bool quit = false;
	
	SDL_AudioSpec audio_spec;
	Uint8* audio_buffer;
	Uint32 audio_length;
	SDL_LoadWAV(GAME_SONG, &audio_spec, &audio_buffer, &audio_length);
	SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, false, &audio_spec, NULL, 0);
	SDL_PauseAudioDevice(audio_device, false);
	SDL_QueueAudio(audio_device, audio_buffer, audio_length);
	double last_queue = get_time();
	
	while (!quit) {
		initialise(&game_set, window_surface);
		
		while (true) {
			SDL_FillRect(
				window_surface,
				NULL,
				SDL_MapRGB(
					window_surface->format,
					BACKGROUND_COLOUR[0],
					BACKGROUND_COLOUR[1],
					BACKGROUND_COLOUR[2]
				)
			);
			
			for (int i = 0; i < game_set.count; i++) {
				SDL_BlitSurface(game_set.background_surface, NULL, window_surface, &game_set.background_rects[i]);
				SDL_BlitSurface(game_set.score_surface, NULL, window_surface, &game_set.score_rects[i]);
				
				int score_copy[] = {
					game_set.players[i].score / 100,
					game_set.players[i].score / 10 % 10,
					game_set.players[i].score % 10
				};
				
				for (int j = 0; j < DIGITS; j++) {
					SDL_BlitSurface(game_set.number_surfaces[score_copy[j]], NULL, window_surface, &game_set.number_rects[i * DIGITS + j]);
				}
			}
			
			if (get_time() > last_queue + GAME_SONG_LENGTH) {
				SDL_QueueAudio(audio_device, audio_buffer, audio_length);
				last_queue = get_time();
			}
			
			if (get_time() > game_set.last_spawn + SPAWN_DELAY) {
				spawn(game_set, window_surface);
				game_set.last_spawn = get_time();
			}
			
			const Uint8* keyboard = SDL_GetKeyboardState(NULL);
			
			if (update2(game_set, keyboard, window_surface)) {
				break;
			}
			
			if ((quit = keyboard[QUIT]) || keyboard[RESET]) {
				break;
			}
			
			if (keyboard[PAUSE]) {
				double pause_start = get_time();
				while (SDL_GetKeyboardState(NULL)[PAUSE]) {
					SDL_PumpEvents();
				}
				
				bool end = false;
				
				while (!(keyboard = SDL_GetKeyboardState(NULL))[PAUSE]) {
					if ((quit = keyboard[QUIT]) || keyboard[RESET]) {
						end = true;
						break;
					}
					
					if (get_time() > last_queue + GAME_SONG_LENGTH) {
						SDL_QueueAudio(audio_device, audio_buffer, audio_length);
						last_queue = get_time();
					}
					
					SDL_PumpEvents();
				}
				
				if (end) {
					break;
				}
				
				while (SDL_GetKeyboardState(NULL)[PAUSE]) {
					SDL_PumpEvents();
				}
				
				game_set.last_spawn += get_time() - pause_start;
				
				for (int i = 0; i < game_set.count; i++) {
					game_set.players[i].last_move += get_time() - pause_start;
					game_set.players[i].shot.last_move += get_time() - pause_start;
					
					for (int j = 0; j < game_set.enemy_containers[i].count; j++) {
						game_set.enemy_containers[i].enemies[j].last_move += get_time() - pause_start;
					}
				}
			}
			
			SDL_UpdateWindowSurface(window);
			SDL_PumpEvents();
		}
		
		for (int i = 0; i < game_set.count; i++) {
			free(game_set.enemy_containers[i].enemies);
		}
		
		while (!quit && !SDL_GetKeyboardState(NULL)[RESET]) {
			if (get_time() > last_queue + GAME_SONG_LENGTH) {
				SDL_QueueAudio(audio_device, audio_buffer, audio_length);
				last_queue = get_time();
			}
			
			quit = SDL_GetKeyboardState(NULL)[QUIT];
			SDL_PumpEvents();
		}
		
		while (!quit && SDL_GetKeyboardState(NULL)[RESET]) {
			SDL_PumpEvents();
		}
	}
	
	SDL_CloseAudioDevice(audio_device);
	SDL_FreeWAV(audio_buffer);
	
	free(game_set.enemy_containers);
	free(game_set.players);
	free(game_set.number_rects);
	free(game_set.score_rects);
	free(game_set.background_rects);
	
	for (int i = 0; i < NUMBERS; i++) {
		SDL_FreeSurface(game_set.number_surfaces[i]);
	}
	
	SDL_FreeSurface(game_set.score_surface);
	SDL_FreeSurface(game_set.enemy_surface);
	SDL_FreeSurface(game_set.shot_surface);
	SDL_FreeSurface(game_set.player_surface);
	SDL_FreeSurface(game_set.background_surface);
}

void help(SDL_Window* window, SDL_Surface* window_surface,
          SDL_AudioDeviceID audio_device, Uint8* audio_buffer, Uint32 audio_length, double* last_queue) {
	SDL_Surface* raw_help_screen = SDL_LoadBMP(HELP_IMAGE);
	SDL_Surface* help_screen = SDL_CreateRGBSurface(
		0, HELP_WIDTH_RATIO * window_surface->w, HELP_HEIGHT_RATIO * window_surface->h, SURFACE_DEPTH,
		SURFACE_MASKS[0], SURFACE_MASKS[1], SURFACE_MASKS[2], SURFACE_MASKS[3]
	);
	
	SDL_BlitScaled(raw_help_screen, NULL, help_screen, NULL);
	SDL_FreeSurface(raw_help_screen);
	
	SDL_Rect rect = {
		.x = HELP_X_RATIO * window_surface->w - help_screen->w / 2,
		.y = HELP_Y_RATIO * window_surface->h - help_screen->h / 2
	};
	
	SDL_BlitSurface(help_screen, NULL, window_surface, &rect);
	SDL_UpdateWindowSurface(window);
	
	while (!SDL_GetKeyboardState(NULL)[QUIT]) {
		if (get_time() > *last_queue + MENU_SONG_LENGTH) {
			SDL_QueueAudio(audio_device, audio_buffer, audio_length);
			*last_queue = get_time();
		}
		
		SDL_PumpEvents();
	}
	
	while (SDL_GetKeyboardState(NULL)[QUIT]) {
		SDL_PumpEvents();
	}
	
	SDL_FreeSurface(help_screen);
}

int main(int argc, char* argv[]) {
	srand(time(NULL));
	rand();
	
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	
	SDL_DisplayMode display_mode;
	SDL_GetDesktopDisplayMode(0, &display_mode);
	
	SDL_Window* window = SDL_CreateWindow(
		TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		display_mode.w, display_mode.h, SDL_WINDOW_FULLSCREEN
	);
	SDL_Surface* window_surface = SDL_GetWindowSurface(window);
	
	SDL_Surface* raw_background = SDL_LoadBMP(BACKGROUND_IMAGE);
	SDL_Surface* raw_title = SDL_LoadBMP(TITLE_IMAGE);
	SDL_Surface* raw_modes = SDL_LoadBMP(MODES_IMAGE);
	
	SDL_Surface* background = SDL_CreateRGBSurface(
		0, window_surface->w, window_surface->h, SURFACE_DEPTH,
		SURFACE_MASKS[0], SURFACE_MASKS[1], SURFACE_MASKS[2], SURFACE_MASKS[3]
	);
	SDL_Surface* title = SDL_CreateRGBSurface(
		0, TITLE_WIDTH_RATIO * window_surface->w, TITLE_HEIGHT_RATIO * window_surface->h, SURFACE_DEPTH,
		SURFACE_MASKS[0], SURFACE_MASKS[1], SURFACE_MASKS[2], SURFACE_MASKS[3]
	);
	SDL_Surface* modes = SDL_CreateRGBSurface(
		0, MODES_WIDTH_RATIO * window_surface->w, MODES_HEIGHT_RATIO * window_surface->h, SURFACE_DEPTH,
		SURFACE_MASKS[0], SURFACE_MASKS[1], SURFACE_MASKS[2], SURFACE_MASKS[3]
	);
	
	SDL_BlitScaled(raw_background, NULL, background, NULL);
	SDL_BlitScaled(raw_title, NULL, title, NULL);
	SDL_BlitScaled(raw_modes, NULL, modes, NULL);
	
	SDL_FreeSurface(raw_background);
	SDL_FreeSurface(raw_title);
	SDL_FreeSurface(raw_modes);
	
	SDL_Rect rect = {.x = 0, .y = 0};
	SDL_BlitSurface(background, NULL, window_surface, &rect);
	
	rect.x = TITLE_X_RATIO * window_surface->w - title->w / 2;
	rect.y = TITLE_Y_RATIO * window_surface->h - title->h / 2;
	SDL_BlitSurface(title, NULL, window_surface, &rect);
	
	rect.x = MODES_X_RATIO * window_surface->w - modes->w / 2;
	rect.y = MODES_Y_RATIO * window_surface->h - modes->h / 2;
	SDL_BlitSurface(modes, NULL, window_surface, &rect);
	
	SDL_UpdateWindowSurface(window);
	
	SDL_Rect mode_rects[] = {
		{
			.x = rect.x,
			.y = rect.y,
			.w = modes->w,
			.h = modes->h / 3
		},
		{
			.x = rect.x,
			.y = rect.y + modes->h / 3,
			.w = modes->w,
			.h = modes->h / 3
		},
		{
			.x = rect.x,
			.y = rect.y + 2 * modes->h / 3,
			.w = modes->w,
			.h = modes->h / 3
		}
	};
	
	SDL_Point mouse;
	
	SDL_AudioSpec audio_spec;
	Uint8* audio_buffer;
	Uint32 audio_length;
	SDL_LoadWAV(MENU_SONG, &audio_spec, &audio_buffer, &audio_length);
	SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, false, &audio_spec, NULL, 0);
	SDL_PauseAudioDevice(audio_device, false);
	SDL_QueueAudio(audio_device, audio_buffer, audio_length);
	double last_queue = get_time();
	
	while (!SDL_GetKeyboardState(NULL)[QUIT]) {
		if (SDL_GetMouseState(&mouse.x, &mouse.y) & CLICK) {
			if (SDL_PointInRect(&mouse, &mode_rects[0])) {
				SDL_ClearQueuedAudio(audio_device);
				game(window, window_surface, 1);
				SDL_QueueAudio(audio_device, audio_buffer, audio_length);
				last_queue = get_time();
			}
			
			else if (SDL_PointInRect(&mouse, &mode_rects[1])) {
				SDL_ClearQueuedAudio(audio_device);
				game(window, window_surface, 2);
				SDL_QueueAudio(audio_device, audio_buffer, audio_length);
				last_queue = get_time();
			}
			
			else if (SDL_PointInRect(&mouse, &mode_rects[2])) {
				help(window, window_surface, audio_device, audio_buffer, audio_length, &last_queue);
			}
			
			rect.x = 0;
			rect.y = 0;
			SDL_BlitSurface(background, NULL, window_surface, &rect);
			
			rect.x = TITLE_X_RATIO * window_surface->w  - title->w / 2;
			rect.y = TITLE_Y_RATIO * window_surface->h - title->h / 2;
			SDL_BlitSurface(title, NULL, window_surface, &rect);
			
			rect.x = MODES_X_RATIO * window_surface->w - modes->w / 2;
			rect.y = MODES_Y_RATIO * window_surface->h - modes->h / 2;
			SDL_BlitSurface(modes, NULL, window_surface, &rect);
			
			SDL_UpdateWindowSurface(window);
		}
		
		if (get_time() > last_queue + MENU_SONG_LENGTH) {
			SDL_QueueAudio(audio_device, audio_buffer, audio_length);
			last_queue = get_time();
		}
		
		SDL_PumpEvents();
	}
	
	SDL_CloseAudioDevice(audio_device);
	SDL_FreeWAV(audio_buffer);
	
	SDL_FreeSurface(modes);
	SDL_FreeSurface(title);
	SDL_FreeSurface(background);
	
	SDL_DestroyWindow(window);
	
	SDL_Quit();
}