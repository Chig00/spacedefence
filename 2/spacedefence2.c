#include <stdio.h> // Used for debugging via the use of standard output
#include <stdlib.h> // Used for dynamic memory allocation and random number generation
#include <stdbool.h> // Used for true/false valuation
#include <time.h> // Used for timing
#include <SDL.h> // Used for video, audio, and event handling

#define DIGITS 3

typedef struct {
	double pos[2];
	double last_move;
	bool active;
	SDL_Surface* surface;
	SDL_Rect rect;
} Shot;

typedef struct {
	int score;
	double pos[2];
	double last_move;
	SDL_Surface* surface;
	SDL_Rect rect;
	Shot shot;
	bool shooting;
} Player;

typedef struct {
	double pos[2];
	double vel;
	double last_move;
	SDL_Rect rect;
} Enemy;

typedef struct {
	Enemy* enemies;
	SDL_Surface* surface;
	int count;
	double last_spawn;
} Enemies;

// Constants
// General constants
const char* const TITLE = "Space Defense 2 by Chigozie Agomo";
const double BACKGROUND_RATIO = 0.45;
const int NUMBERS = 10;

// Surface constants
// The order of RGBA depends on the machine's endianness
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	const Uint32 RED_MASK = 0xff000000;
	const Uint32 GREEN_MASK = 0x00ff0000;
	const Uint32 BLUE_MASK = 0x0000ff00;
	const Uint32 ALPHA_MASK = 0x000000ff;
#else
	const Uint32 RED_MASK = 0x000000ff;
	const Uint32 GREEN_MASK = 0x0000ff00;
	const Uint32 BLUE_MASK = 0x00ff0000;
	const Uint32 ALPHA_MASK = 0xff000000;
#endif

// The names of the surfaces
const char* const BACKGROUND_IMAGE = "data/background.bmp";
const char* const TITLE_IMAGE = "data/title.bmp";
const char* const MODES_IMAGE = "data/modes.bmp";
const char* const BACKGROUND_IMAGE_TWO = "data/background2.bmp";
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
const char* const HELP_IMAGE = "data/help.bmp";

// The number of bits used for a surface
const int SURFACE_DEPTH = 32;
// The ratios for the sizes of text compared to the size of the display
const double TITLE_WIDTH_RATIO = 0.7;
const double TITLE_HEIGHT_RATIO = 0.25;
const double TITLE_X_RATIO = 0.5;
const double TITLE_Y_RATIO = 0.25;
const double MODES_WIDTH_RATIO = 0.25;
const double MODES_HEIGHT_RATIO = 0.3;
const double MODES_X_RATIO = 0.5;
const double MODES_Y_RATIO = 0.75;
const double SCORE_WIDTH_RATIO = 0.5;
const double SCORE_HEIGHT_RATIO = 0.1;
const double SCORE_X_RATIO = 0.85;
const double SCORE_Y_RATIO = 0.45;
const double NUMBER_WIDTH_RATIO = 0.1;
const double NUMBER_HEIGHT_RATIO = 0.08;
const double NUMBER_X_RATIO = 0.85;
const double NUMBER_Y_RATIO = 0.6;
const double HELP_WIDTH_RATIO = 0.8;
const double HELP_HEIGHT_RATIO = 0.8;
const double HELP_X_RATIO = 0.5;
const double HELP_Y_RATIO = 0.5;

// Song constants
const char* const MENU_SONG = "data/menusong.wav";
const int MENU_SONG_LENGTH = 63;
const char* const GAME_SONG = "data/gamesong.wav";
const int GAME_SONG_LENGTH = 127;

// Control constants
const int QUIT = SDL_SCANCODE_ESCAPE;
const int RESET = SDL_SCANCODE_R;
const int PAUSE = SDL_SCANCODE_P;
const int PLAYER_ONE_LEFT = SDL_SCANCODE_A;
const int PLAYER_ONE_RIGHT = SDL_SCANCODE_D;
const int PLAYER_ONE_SHOOT = SDL_SCANCODE_W;
const int PLAYER_TWO_LEFT = SDL_SCANCODE_LEFT;
const int PLAYER_TWO_RIGHT = SDL_SCANCODE_RIGHT;
const int PLAYER_TWO_SHOOT = SDL_SCANCODE_UP;

// Player constants
const double PLAYER_Y_RATIO = 0.85;
const double PLAYER_WIDTH_RATIO = 0.2;
const double PLAYER_HEIGHT_RATIO = 0.2;
const double PLAYER_SPEED = 500;
const double SHOT_WIDTH_RATIO = 0.01;
const double SHOT_HEIGHT_RATIO = 0.3;
const double SHOT_SPEED = 3000;

// Enemy constants
const double ENEMY_WIDTH_RATIO = 0.15;
const double ENEMY_HEIGHT_RATIO = 0.25;
const double ENEMY_BASE_SPEED = 50;
const double ENEMY_ACCELERATION = 2.5;
const double SPAWN_DELAY = 0.5;

// Returns the time relative to previous calls in seconds
double get_time() {
	return (double) clock() / CLOCKS_PER_SEC;
}

// Returns a random floating point number, X, in the range low <= X <= high
double random(double low, double high) {
	return low + (high - low) * rand() / RAND_MAX;
}

// Returns the velocity of the enemy
double calc_vel(int score) {
	return ENEMY_BASE_SPEED + score * ENEMY_ACCELERATION;
}

// Initialises the player
void init_player(Player* player, SDL_Surface* window_surface) {
	player->pos[0] = window_surface->w / 2;
	player->pos[1] = window_surface->h * PLAYER_Y_RATIO;
	player->score = 0;
	player->shooting = false;
	player->shot.active = false;
	player->rect.x = player->pos[0] - player->rect.w / 2;
	player->rect.y = player->pos[1] - player->rect.h / 2;
	player->last_move = get_time();
}

// Initialises the enemy container
void init_enemies(Enemies* enemies) {
	enemies->enemies = NULL;
	enemies->count = 0;
}

// Creates a new enemy
void spawn(Enemies* enemies, int score, SDL_Surface* window_surface, SDL_Surface* background) {
	// The buffer for the enemies is resized
	enemies->enemies = realloc(enemies->enemies, sizeof(Enemy) * (enemies->count + 1));
	
	// The new enemy's rect and position are set
	enemies->enemies[enemies->count].rect.w = background->w * ENEMY_WIDTH_RATIO;
	enemies->enemies[enemies->count].rect.h = background->h * ENEMY_HEIGHT_RATIO;
	enemies->enemies[enemies->count].pos[0] = random(
		(window_surface->w - background->w + enemies->enemies[enemies->count].rect.w) / 2,
		(window_surface->w + background->w - enemies->enemies[enemies->count].rect.w) / 2
	);
	enemies->enemies[enemies->count].pos[1] = 0;
	enemies->enemies[enemies->count].rect.x = enemies->enemies[enemies->count].pos[0] - enemies->enemies[enemies->count].rect.w / 2;
	enemies->enemies[enemies->count].rect.y = enemies->enemies[enemies->count].pos[1] - enemies->enemies[enemies->count].rect.h / 2;
	// The new enemy's velocity is calculated and depends on the score
	enemies->enemies[enemies->count].vel = calc_vel(score);
	// The number of enemies is incremented
	enemies->count++;
	// The last movement of the new enemey is considered to be its spawn
	enemies->enemies[enemies->count - 1].last_move = get_time();
}

// Creates and activates a player shot
void shoot(Player* player) {
	player->shot.active = true;
	player->shot.pos[0] = player->pos[0];
	player->shot.pos[1] = player->pos[1];
	player->shot.rect.x = player->shot.pos[0] - player->shot.rect.w / 2;
	player->shot.rect.y = player->shot.pos[1] - player->shot.rect.h / 2;
	player->shot.last_move = get_time();
}

// Updates the player's shot
void update_shot(Shot* shot, Enemies* enemies, SDL_Surface* window_surface, int* score) {
	// The shot is moved and blitted to the window
	shot->pos[1] -= SHOT_SPEED * (get_time() - shot->last_move);
	shot->last_move = get_time();
	shot->rect.y = shot->pos[1] - shot->rect.h / 2;
	SDL_BlitSurface(shot->surface, NULL, window_surface, &shot->rect);
	
	if (shot->pos[1] < - shot->rect.h / 2) {
		shot->active = false;
		return;
	}
	
	// If the shot collides with an enemy, the shot deactivates, the enemy is destroyed, and the player gets a point
	for (int i = 0; i < enemies->count; i++) {
		if (SDL_HasIntersection(&shot->rect, &enemies->enemies[i].rect)) {
			++*score;
			shot->active = false;
			
			// The enemy is destroyed by overwriting it with subsequent enemies and desizing the buffer
			for (int j = i; j < enemies->count - 1; j++) {
				enemies->enemies[j] = enemies->enemies[j + 1];
			}
			
			enemies->enemies = realloc(enemies->enemies, sizeof(Enemy) * --enemies->count);
			
			return;
		}
	}
}

// Updates the player and the enemies
bool update(Player* player, Enemies* enemies, SDL_Surface* window_surface, SDL_Surface* background, SDL_Window* window) {
	// If the player's shot is active, it is updated first, so that the player's model is seen on top
	if (player->shot.active) {
		update_shot(&player->shot, enemies, window_surface, &player->score);
	}
	
	// The player moves left or right if the respective button is pressed
	// The distance moves depends on the time of the last move keeping the game consistent for computers of all speeds
	player->pos[0] +=
		((SDL_GetKeyboardState(NULL)[PLAYER_ONE_RIGHT] || SDL_GetKeyboardState(NULL)[PLAYER_TWO_RIGHT])
		- (SDL_GetKeyboardState(NULL)[PLAYER_ONE_LEFT] || SDL_GetKeyboardState(NULL)[PLAYER_TWO_LEFT]))
		* PLAYER_SPEED * (get_time() - player->last_move);
	// The player's last move is set to the present
	player->last_move = get_time();
	
	// If the player tries to shoot, isn't holding the shoot button, and hasn't alreay got a shot active, they shoot
	if (SDL_GetKeyboardState(NULL)[PLAYER_ONE_SHOOT] || SDL_GetKeyboardState(NULL)[PLAYER_TWO_SHOOT]) {
		if (!player->shooting) {
			if (!player->shot.active) {
				shoot(player);
			}
			
			// This is set true to prevent the player from simply holding down the shoot button to shoot
			// They must shoot each shot independently
			player->shooting = true;
		}
	}
	
	// If the player is no longer holding the shoot button down, the game recognises this
	else if (player->shooting) {
		player->shooting = false;
	}
	
	// If the player has moved offscreen, they are moved back onscreen
	if (player->pos[0] < (window_surface->w - background->w + player->rect.w) / 2) {
		player->pos[0] = (window_surface->w - background->w + player->rect.w) / 2;
	}
	
	else if (player->pos[0] > (window_surface->w + background->w - player->rect.w) / 2) {
		player->pos[0] = (window_surface->w + background->w - player->rect.w) / 2;
	}
	
	// The player's rect is updated to match its new position
	player->rect.x = player->pos[0] - player->rect.w / 2;
	
	// The player's surface is blitted to the screen
	SDL_BlitSurface(player->surface, NULL, window_surface, &player->rect);
	
	// The enemies are updated
	for (int i = 0; i < enemies->count; i++) {
		// The enemy is moved downward
		enemies->enemies[i].pos[1] += enemies->enemies[i].vel * (get_time() - enemies->enemies[i].last_move);
		enemies->enemies[i].last_move = get_time();
		// The enemy's rect is updated
		enemies->enemies[i].rect.y = enemies->enemies[i].pos[1] - enemies->enemies[i].rect.h / 2;
		
		// The enemy is blitted to the screen
		SDL_BlitSurface(enemies->surface, NULL, window_surface, &enemies->enemies[i].rect);
		
		// If the enemy comes into contact with the player the game ends
		// If the enemy comes offscreen the game also ends
		if (SDL_HasIntersection(&player->rect, &enemies->enemies[i].rect) || enemies->enemies[i].rect.y >= window_surface->h) {
			// The window is updated to show the player that they had collided
			SDL_UpdateWindowSurface(window);
			return true;
		}
		
	}
	
	return false;
}

// Function for the one_player mode
void one_player(SDL_Window* window, SDL_Surface* window_surface) {
	// The surface for the background is made
	SDL_Surface* background = SDL_CreateRGBSurface(
		0, window_surface->w * BACKGROUND_RATIO, window_surface->h, SURFACE_DEPTH,
		RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
	);
	
	SDL_Surface* raw_background = SDL_LoadBMP(BACKGROUND_IMAGE_TWO);
	SDL_BlitScaled(raw_background, NULL, background, NULL);
	SDL_FreeSurface(raw_background);
	
	// The background's rect is created
	SDL_Rect background_rect = {
		.x = (window_surface->w - background->w) / 2,
		.y = 0,
		.w = background->w,
		.h = background->h
	};
	
	// The player and enemy structures are made with their surfaces
	Player player = {
		.rect = {
			.w = background->w * PLAYER_WIDTH_RATIO,
			.h = background->h * PLAYER_HEIGHT_RATIO
		},
		.surface = SDL_CreateRGBSurface(
			0, background->w * PLAYER_WIDTH_RATIO, background->h * PLAYER_HEIGHT_RATIO, SURFACE_DEPTH,
			RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
		),
		.shot = {
			.rect = {
				.w = background->w * SHOT_WIDTH_RATIO,
				.h = background->h * SHOT_HEIGHT_RATIO
			},
			.surface = SDL_CreateRGBSurface(
				0, background-> w * SHOT_WIDTH_RATIO, background->h * SHOT_HEIGHT_RATIO, SURFACE_DEPTH,
				RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
			)
		}
	};
	
	SDL_Surface* raw_player_surface = SDL_LoadBMP(PLAYER_IMAGE);
	SDL_Surface* raw_shot_surface = SDL_LoadBMP(SHOT_IMAGE);
	SDL_BlitScaled(raw_player_surface, NULL, player.surface, NULL);
	SDL_BlitScaled(raw_shot_surface, NULL, player.shot.surface, NULL);
	SDL_FreeSurface(raw_player_surface);
	SDL_FreeSurface(raw_shot_surface);
	
	Enemies enemies = {
		.surface = SDL_CreateRGBSurface(
			0, background->w * ENEMY_WIDTH_RATIO, background->h * ENEMY_HEIGHT_RATIO, SURFACE_DEPTH,
			RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
		)
	};
	
	SDL_Surface* raw_enemy_surface = SDL_LoadBMP(ENEMY_IMAGE);
	SDL_BlitScaled(raw_enemy_surface, NULL, enemies.surface, NULL);
	SDL_FreeSurface(raw_enemy_surface);
	
	SDL_Surface* score = SDL_CreateRGBSurface(
		0, background->w * SCORE_WIDTH_RATIO, background->h * SCORE_HEIGHT_RATIO, SURFACE_DEPTH,
		RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
	);
	
	SDL_Surface* raw_score_surface = SDL_LoadBMP(SCORE_IMAGE);
	SDL_BlitScaled(raw_score_surface, NULL, score, NULL);
	SDL_FreeSurface(raw_score_surface);
	
	SDL_Rect score_rect = {
		.x = SCORE_X_RATIO * window_surface->w - score->w / 2,
		.y = SCORE_Y_RATIO * window_surface->h - score->h / 2,
		.w = score->w,
		.h = score->h
	};
	
	SDL_Surface* numbers[NUMBERS];
	
	for (int i = 0; i < NUMBERS; i++) {
		numbers[i] = SDL_CreateRGBSurface(
			0, background->w * NUMBER_WIDTH_RATIO, background->h * NUMBER_HEIGHT_RATIO, SURFACE_DEPTH,
			RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
		);
		
		SDL_Surface* raw_number_surface = SDL_LoadBMP(NUMBER_IMAGES[i]);
		SDL_BlitScaled(raw_number_surface, NULL, numbers[i], NULL);
		SDL_FreeSurface(raw_number_surface);
	}
	
	SDL_Rect number_rects[] = {
		{
			.x = NUMBER_X_RATIO * window_surface->w - 3 * numbers[0]->w / 2,
			.y = NUMBER_Y_RATIO * window_surface->h - numbers[0]->h / 2,
			.w = numbers[0]->w,
			.h = numbers[0]->h
		},
		{
			.x = NUMBER_X_RATIO * window_surface->w - numbers[0]->w / 2,
			.y = NUMBER_Y_RATIO * window_surface->h - numbers[0]->h / 2,
			.w = numbers[0]->w,
			.h = numbers[0]->h
		},
		{
			.x = NUMBER_X_RATIO * window_surface->w + numbers[0]->w / 2,
			.y = NUMBER_Y_RATIO * window_surface->h - numbers[0]->h / 2,
			.w = numbers[0]->w,
			.h = numbers[0]->h
		}
	};
	
	// This variable is valued true when the mode quits
	bool quit = false;
	
	// The game song is played
	SDL_AudioSpec audio_spec;
	Uint8* audio_buffer;
	Uint32 audio_length;
	SDL_LoadWAV(GAME_SONG, &audio_spec, &audio_buffer, &audio_length);
	SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, false, &audio_spec, NULL, 0);
	SDL_PauseAudioDevice(audio_device, false);
	SDL_QueueAudio(audio_device, audio_buffer, audio_length);
	double last_queue = get_time();
	
	while (!quit) {
		// The player and enemies are initialised for a new round
		init_player(&player, window_surface);
		init_enemies(&enemies);
		
		while (true) {
			// The background is filled in
			SDL_FillRect(window_surface, NULL, SDL_MapRGB(window_surface->format, 0, 0, 0));
			SDL_BlitSurface(background, NULL, window_surface, &background_rect);
			SDL_BlitSurface(score, NULL, window_surface, &score_rect);
			
			int score_copy[] = {
				player.score / 100,
				player.score / 10 % 10,
				player.score % 10
			};
			
			for (int i = 0; i < DIGITS; i++) {
				SDL_BlitSurface(numbers[score_copy[i]], NULL, window_surface, &number_rects[i]);
			}
			
			// The music is requeued when the song comes to an end
			if (get_time() >= last_queue + GAME_SONG_LENGTH) {
				SDL_QueueAudio(audio_device, audio_buffer, audio_length);
				last_queue = get_time();
			}
			
			// A new enemy is spawned when enough time has passed
			if (get_time() >= enemies.last_spawn + SPAWN_DELAY) {
				spawn(&enemies, player.score, window_surface, background);
				enemies.last_spawn = get_time();
			}
			
			// The game is updated
			if (update(&player, &enemies, window_surface, background, window)) {
				break;
			}
			
			// If the player presses escape or r, the game quits or resets respectively
			if ((quit = SDL_GetKeyboardState(NULL)[QUIT]) || SDL_GetKeyboardState(NULL)[RESET]) {
				break;
			}
			
			// The player can pause the game with p
			if (SDL_GetKeyboardState(NULL)[PAUSE]) {
				double pause_start = get_time();
				
				while (SDL_GetKeyboardState(NULL)[PAUSE]) {
					SDL_PumpEvents();
				}
				
				bool end = false;
				
				while (!SDL_GetKeyboardState(NULL)[PAUSE]) {
					if (get_time() >= last_queue + GAME_SONG_LENGTH) {
						SDL_QueueAudio(audio_device, audio_buffer, audio_length);
						last_queue = get_time();
					}
					
					// The player can quit or reset while the game is paused
					if ((quit = SDL_GetKeyboardState(NULL)[QUIT]) || SDL_GetKeyboardState(NULL)[RESET]) {
						end = true;
						break;
					}
					
					SDL_PumpEvents();
				}
				
				if (end) {
					break;
				}
				
				while (SDL_GetKeyboardState(NULL)[PAUSE]) {
					SDL_PumpEvents();
				}
				
				player.last_move += get_time() - pause_start;
				player.shot.last_move += get_time() - pause_start;
				enemies.last_spawn += get_time() - pause_start;
				
				for (int i = 0; i < enemies.count; i++) {
					enemies.enemies[i].last_move += get_time() - pause_start;
				}
			}
			
			// The window surface and events are updated
			SDL_UpdateWindowSurface(window);
			SDL_PumpEvents();
		}
		
		// The enemy buffer is freed after the round ends
		free(enemies.enemies);
		
		// If the round ended normally, the game waits for the player to press r to play again
		while (!(quit || SDL_GetKeyboardState(NULL)[RESET])) {
			if (get_time() >= last_queue + GAME_SONG_LENGTH) {
				SDL_QueueAudio(audio_device, audio_buffer, audio_length);
				last_queue = get_time();
			}
			
			quit = SDL_GetKeyboardState(NULL)[QUIT];
			SDL_PumpEvents();
		}
		
		// The new round doesn't start until the r button is released
		while (!quit && SDL_GetKeyboardState(NULL)[RESET]) {
			SDL_PumpEvents();
		}
	}
	
	// The surfaces loaded for the round are freed
	SDL_FreeSurface(background);
	SDL_FreeSurface(player.surface);
	SDL_FreeSurface(player.shot.surface);
	SDL_FreeSurface(enemies.surface);
	SDL_FreeSurface(score);
	
	for (int i = 0; i < NUMBERS; i++) {
		SDL_FreeSurface(numbers[i]);
	}
	
	// The audio for the game is stopped for the main menu music to play
	SDL_CloseAudioDevice(audio_device);
	SDL_FreeWAV(audio_buffer);
	
	// The main menu isn't returned to until the player lets go of the escape button
	while (SDL_GetKeyboardState(NULL)[QUIT]) {
		SDL_PumpEvents();
	}
}

// Function for the help display
void help(SDL_Window* window, SDL_Surface* window_surface) {
	SDL_Surface* help = SDL_CreateRGBSurface(
		0, HELP_WIDTH_RATIO * window_surface->w, HELP_HEIGHT_RATIO * window_surface->h, SURFACE_DEPTH,
		RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
	);
	
	SDL_Surface* raw_help_surface = SDL_LoadBMP(HELP_IMAGE);
	SDL_BlitScaled(raw_help_surface, NULL, help, NULL);
	SDL_FreeSurface(raw_help_surface);
	
	SDL_Rect help_rect = {
		.x = HELP_X_RATIO * window_surface->w - help->w / 2,
		.y = HELP_Y_RATIO * window_surface->h - help->h / 2,
		.w = help->w,
		.h = help->h
	};
	
	SDL_BlitSurface(help, NULL, window_surface, &help_rect);
	SDL_UpdateWindowSurface(window);
	
	while (!SDL_GetKeyboardState(NULL)[QUIT]) {
		SDL_PumpEvents();
	}
	
	while (SDL_GetKeyboardState(NULL)[QUIT]) {
		SDL_PumpEvents();
	}
	
	SDL_FreeSurface(help);
}

int main(int argc, char* argv[]) {
	// The random number generator is seeded and the first value is discarded for extra randomness
	srand(time(NULL));
	rand();
	
	// SDL is initialised
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	
	// The size of the display is found
	SDL_DisplayMode display_mode;
	SDL_GetDesktopDisplayMode(0, &display_mode);
	// The fullscreen window is made using the size of the display
	SDL_Window* window = SDL_CreateWindow(
		TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		display_mode.w, display_mode.h, SDL_WINDOW_FULLSCREEN
	);
	// The window's surface is obtained
	SDL_Surface* window_surface = SDL_GetWindowSurface(window);
	
	// The surfaces for the background, title, and modes are created
	SDL_Surface* background = SDL_CreateRGBSurface(
		0, window_surface->w, window_surface->h, SURFACE_DEPTH,
		RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
	);
	SDL_Surface* title = SDL_CreateRGBSurface(
		0, window_surface->w * TITLE_WIDTH_RATIO, window_surface->h * TITLE_HEIGHT_RATIO, SURFACE_DEPTH,
		RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
	);
	SDL_Surface* modes = SDL_CreateRGBSurface(
		0, window_surface->w * MODES_WIDTH_RATIO, window_surface->h * MODES_HEIGHT_RATIO, SURFACE_DEPTH,
		RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
	);
	
	// The unstretched surfaces are loaded
	SDL_Surface* raw_background = SDL_LoadBMP(BACKGROUND_IMAGE);
	SDL_Surface* raw_title = SDL_LoadBMP(TITLE_IMAGE);
	SDL_Surface* raw_modes = SDL_LoadBMP(MODES_IMAGE);
	
	// The unstretched surfaces are stretched to fit the surfaces to be used
	SDL_BlitScaled(raw_background, NULL, background, NULL);
	SDL_BlitScaled(raw_title, NULL, title, NULL);
	SDL_BlitScaled(raw_modes, NULL, modes, NULL);
	
	// The unstretched surfaces are discarded
	SDL_FreeSurface(raw_background);
	SDL_FreeSurface(raw_title);
	SDL_FreeSurface(raw_modes);
	
	// The background, title, and modes' surfaces are blitted to the window surface and the window surface is updated
	SDL_BlitSurface(background, NULL, window_surface, NULL);
	
	SDL_Rect title_rect = {
		.x = window_surface->w * TITLE_X_RATIO - title->w / 2,
		.y = window_surface->h * TITLE_Y_RATIO - title->h / 2,
		.w = title->w,
		.h = title->h
	};
	SDL_BlitSurface(title, NULL, window_surface, &title_rect);
	
	SDL_Rect modes_rect = {
		.x = window_surface->w * MODES_X_RATIO - modes->w / 2,
		.y = window_surface->h * MODES_Y_RATIO - modes->h / 2,
		.w = modes->w,
		.h = modes->h
	};
	SDL_BlitSurface(modes, NULL, window_surface, &modes_rect);
	
	SDL_UpdateWindowSurface(window);
	
	// The main menu's music is played
	SDL_AudioSpec audio_spec;
	Uint8* audio_buffer;
	Uint32 audio_length;
	SDL_LoadWAV(MENU_SONG, &audio_spec, &audio_buffer, &audio_length);
	SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, false, &audio_spec, NULL, 0);
	SDL_PauseAudioDevice(audio_device, false);
	SDL_QueueAudio(audio_device, audio_buffer, audio_length);
	
	// The last_queue variable is set to the first queue
	double last_queue = get_time();
	// The rects for the modes are created
	SDL_Rect mode_rects[] = {
		{
			.x = modes_rect.x,
			.y = modes_rect.y,
			.w = modes_rect.w,
			.h = modes_rect.h / 3
		},
		{
			.x = modes_rect.x,
			.y = modes_rect.y + modes_rect.h / 3,
			.w = modes_rect.w,
			.h = modes_rect.h / 3
		},
		{
			.x = modes_rect.x,
			.y = modes_rect.y + 2 * modes_rect.h / 3,
			.w = modes_rect.w,
			.h = modes_rect.h / 3
		}
	};
	// The mouse is created for recording its position
	SDL_Point mouse;
	
	// The agme quits when the escape key is pressed on the main menu
	while (!SDL_GetKeyboardState(NULL)[QUIT]) {
		// The main menu's music is requeued at the end of the playing of a single loop
		if (get_time() >= last_queue + MENU_SONG_LENGTH) {
			SDL_QueueAudio(audio_device, audio_buffer, audio_length);
			last_queue = get_time();
		}
		
		if (SDL_GetMouseState(&mouse.x, &mouse.y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			if (SDL_PointInRect(&mouse, &mode_rects[0])) {
				SDL_ClearQueuedAudio(audio_device);
				one_player(window, window_surface);
				SDL_QueueAudio(audio_device, audio_buffer, audio_length);
				last_queue = get_time();
			}
			
			else if (SDL_PointInRect(&mouse, &mode_rects[2])) {
				SDL_QueueAudio(audio_device, audio_buffer, audio_length);
				last_queue = get_time();
				help(window, window_surface);
			}
			
			SDL_BlitSurface(background, NULL, window_surface, NULL);
			SDL_BlitSurface(title, NULL, window_surface, &title_rect);
			SDL_BlitSurface(modes, NULL, window_surface, &modes_rect);
			
			SDL_UpdateWindowSurface(window);
		}
		
		SDL_PumpEvents();
	}
}