#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL.h>

#define TITLE "Space Defense by Chigozie Agomo"
#define WINDOW_W 600
#define WINDOW_H 900
#define BACKGROUND_BMP "stars.bmp"
#define QUIT SDL_SCANCODE_ESCAPE
#define RESET SDL_SCANCODE_R
#define LEFT SDL_SCANCODE_LEFT
#define RIGHT SDL_SCANCODE_RIGHT
#define SHOOT SDL_SCANCODE_SPACE
#define PLAYER_BMP "rocket.bmp"
#define PLAYER_W 75
#define PLAYER_H 150
#define PLAYER_SHIFT 25
#define PLAYER_SPD 9
#define LASER_BMP "laser.bmp"
#define LASER_W 10
#define LASER_H 100
#define LASER_VEL -50
#define MISSILE_BMP "missile.bmp"
#define MISSILE_W 75
#define MISSILE_H 150
#define BASE_VEL 1
#define VEL_GRAD 0.1

typedef struct {
	SDL_Surface* surface;
	SDL_Rect rect;
	double true_y;
	bool active;
} Laser;

typedef struct {
	Laser laser;
	SDL_Surface* surface;
	SDL_Rect rect;
	double true_x;
	bool shoot;
} Player;

typedef struct {
	SDL_Surface* surface;
	SDL_Rect rect;
	double true_y;
	double vel_y;
	bool active;
} Missile;

int rand_int(int low, int high) {
	return low + (high - low + 1) * rand() / (RAND_MAX + 1);
}

double calc_vel(int score) {
	return BASE_VEL * (1 + score * VEL_GRAD);
}

void shoot_missile(Missile* missile, int score) {
	missile->rect.x = rand_int(0, WINDOW_W - MISSILE_W);
	missile->rect.y = -MISSILE_H;
	missile->true_y = missile->rect.y;
	missile->vel_y = calc_vel(score);
}

bool update_missile(Missile* missile, SDL_Surface* display) {
	missile->true_y += missile->vel_y;
	missile->rect.y = missile->true_y;
			
	SDL_BlitSurface(missile->surface, NULL, display, &missile->rect);
			
	if (missile->rect.y >= WINDOW_W - MISSILE_W) {
		return true;
	}
			
	return false;
}

void shoot_laser(Player* player) {
	player->laser.rect.x = player->rect.x + player->rect.w / 2 - player->laser.rect.w / 2;
	player->laser.rect.y = player->rect.y;
	player->laser.true_y = player->laser.rect.y;
	player->laser.active = true;
}

void update_laser(Laser* laser, Missile* missile, int* score, SDL_Surface* display) {
	laser->true_y += LASER_VEL;
	laser->rect.y = laser->true_y;
	
	if (laser->rect.y <= -laser->rect.h) {
		laser->active = false;
		return;
	}
	
	SDL_BlitSurface(laser->surface, NULL, display, &laser->rect);
	
	if (SDL_HasIntersection(&laser->rect, &missile->rect)) {
		laser->active = false;
		shoot_missile(missile, *score);
		++*score;
	}
}

bool update_player(Player* player, Missile* missile, int* score, SDL_Surface* display) {
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	
	player->true_x += (keys[RIGHT] - keys[LEFT]) * PLAYER_SPD;
	
	if (player->true_x < 0) {
		player->true_x = 0;
	}
	
	else if (player->true_x > WINDOW_W - PLAYER_W) {
		player->true_x = WINDOW_W - PLAYER_W;
	}
	
	player->rect.x = player->true_x;
	
	if (keys[SHOOT]) {
		if (!player->shoot) {
			if (!player->laser.active) {
				shoot_laser(player);
			}
			
			player->shoot = true;
		}
	}
	
	else {
		player->shoot = false;
	}
	
	if (player->laser.active) {
		update_laser(&player->laser, missile, score, display);
	}
	
	SDL_BlitSurface(player->surface, NULL, display, &player->rect);
	
	if (SDL_HasIntersection(&player->rect, &missile->rect)) {
		return true;
	}
	
	return false;
}
	

int main(int argc, char** argv) {
	srand(time(NULL));
	
	SDL_Window* window = SDL_CreateWindow(
		TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN
	);
	SDL_Surface* display = SDL_GetWindowSurface(window);
	SDL_Surface* background = SDL_LoadBMP(BACKGROUND_BMP);
	
	Player player = {
		.laser = {
			.surface = SDL_LoadBMP(LASER_BMP),
			.rect = {
				.w = LASER_W,
				.h = LASER_H
			}
		},
		.surface = SDL_LoadBMP(PLAYER_BMP),
		.rect = {
			.y = WINDOW_H - PLAYER_H - PLAYER_SHIFT,
			.w = PLAYER_W,
			.h = PLAYER_H
		}
	};
	
	Missile missile = {
		.surface = SDL_LoadBMP(MISSILE_BMP),
		.rect.w = MISSILE_W,
		.rect.h = MISSILE_H
	};
	
	bool quit = false;
	
	while (!quit) {
		bool end = false;
		int score = 0;
		
		player.laser.active = false;
		player.true_x = WINDOW_W / 2 - PLAYER_W / 2;
		player.rect.x = player.true_x;
		player.shoot = false;
		
		shoot_missile(&missile, score);
		
		while (!quit && !end) {
			SDL_BlitSurface(background, NULL, display, NULL);
			
			quit = SDL_GetKeyboardState(NULL)[QUIT];
			end = SDL_GetKeyboardState(NULL)[RESET];
			
			if (update_missile(&missile, display) || update_player(&player, &missile, &score, display)) {
				printf("Score: %d\n", score);
				
				while (!quit && !end) {
					quit = SDL_GetKeyboardState(NULL)[QUIT];
					end = SDL_GetKeyboardState(NULL)[RESET];
					SDL_PumpEvents();
				}
			}
			
			SDL_UpdateWindowSurface(window);
			SDL_PumpEvents();
		}
		
		while (SDL_GetKeyboardState(NULL)[RESET]) {
			SDL_PumpEvents();
		}
	}
}