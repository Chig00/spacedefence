/**
 * A mobile version of Space Defence 2.
 * The controls have been modified for mobile devices.
 * Tapping the screen will set the x-coordinate as the destination.
 * The player's ship will travel to the destination.
 * Upon raching the destination, the player will then attempt to shoot.
 */

#include <iostream>
#include <list>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include "sdlandnet.hpp"

// Constants
//{
// System Constants
//{
// The program's current version.
constexpr int VERSION[System::VERSION_LENGTH] = {1, 1, 1, 0};

// The total number of threads used for parallel computation.
constexpr int THREADS = 4;

// Player Constants
//{
constexpr double PLAYER_SPEED = 1;
constexpr double SHOT_VELOCITY = -2;
//}

// Enemy Constants
//{
constexpr double ENEMY_DELAY = 0.5;
constexpr double ENEMY_VELOCITY = 0.1;
constexpr double ENEMY_ACCELERATION = 0.00125;
//}
//}

// Video Constants
//{
// Renderer Constants
//{
// The renderer's asset folder.
constexpr const char* RENDERER_DIRECTORY = "data/";

// The extension of the renderer's assets.
constexpr const char* RENDERER_EXTENSION = ".bmp";

// The numbers of letters used by the renderer.
constexpr int RENDERER_LETTERS = 26;

// The number of letter cases used by the renderer.
constexpr int RENDERER_CASES = 2;

// The number of numbers used by the renderer.
constexpr int RENDERER_NUMBERS = 10;

// The first index of the extra characters and sources.
constexpr int RENDERER_EXTRA_INDEX = RENDERER_CASES * RENDERER_LETTERS + RENDERER_NUMBERS;

// The number of extra characters and sources.
constexpr int RENDERER_EXTRAS = 4;

// The extra characters used by the renderer.
constexpr char RENDERER_EXTRA_CHARACTERS[RENDERER_EXTRAS] = {
    '.',
    ',',
    '!',
    ':'
};

// The extra sources used by the renderer.
constexpr const char* RENDERER_EXTRA_SOURCES[RENDERER_EXTRAS] = {
    "fullstop",
    "comma",
    "exclamation",
    "colon"
};

// The total number of characters-source pairings for the renderer.
constexpr int RENDERER_COUNT = RENDERER_EXTRA_INDEX + RENDERER_EXTRAS;
//}

// Background Constants
//{
// Main Menu Background.
constexpr const char* MENU_BACKGROUND_SOURCE = "data/menubackground.bmp";

// Game Background Constants
//{
constexpr const char* GAME_BACKGROUND_SOURCE = "data/gamebackground.bmp";
constexpr double GAME_BACKGROUND_WIDTH = 1;
constexpr double GAME_BACKGROUND_HEIGHT = 0.9;
constexpr double GAME_BACKGROUND_X = 0.5;
constexpr double GAME_BACKGROUND_Y = 1 - GAME_BACKGROUND_HEIGHT / 2;
//}
//}

// Main Menu Constants
//{
// Title Constants
//{
constexpr const char* TITLE_STRING = "Space Defence\nMobile";
constexpr double TITLE_X = 0.5;
constexpr double TITLE_Y = 0.1875;
constexpr double TITLE_WIDTH = 0.07;
constexpr double TITLE_HEIGHT = 2 * TITLE_WIDTH;
constexpr double TITLE_X_SEPARATION = TITLE_WIDTH / 20;
constexpr double TITLE_Y_SEPARATION = TITLE_HEIGHT / 5;
//}

// Play Constants
//{
constexpr const char* PLAY_STRING = "Play";
constexpr double PLAY_X = 0.5;
constexpr double PLAY_Y = 0.5;
constexpr double PLAY_WIDTH = TITLE_WIDTH;
constexpr double PLAY_HEIGHT = 2 * PLAY_WIDTH;
constexpr double PLAY_SEPARATION = PLAY_WIDTH / 20;
//}

// Help Constants
//{
constexpr const char* HELP_STRING = "Help";
constexpr double HELP_X = 0.5;
constexpr double HELP_Y = 0.7;
constexpr double HELP_WIDTH = PLAY_WIDTH;
constexpr double HELP_HEIGHT = 2 * HELP_WIDTH;
constexpr double HELP_SEPARATION = HELP_WIDTH / 20;
//}

// Info Constants
//{
#define INFO_STRING (                \
    "2020 Chigozie Agomo\nVersion: " \
    + System::version(VERSION)       \
    + "\nUtilities: "                \
    + System::version()              \
)
constexpr double INFO_X = 0.23;
constexpr double INFO_Y = 0.93;
constexpr double INFO_WIDTH = 0.025;
constexpr double INFO_HEIGHT = 1.5 * INFO_WIDTH;
constexpr double INFO_X_SEPARATION = INFO_WIDTH / 20;
constexpr double INFO_Y_SEPARATION = INFO_HEIGHT / 5;
constexpr double INFO_MAX_WIDTH = 0;
constexpr Renderer::Justification INFO_JUSTIFICATION = Renderer::LEFT_JUSTIFY;
//}
//}

// Help Message Constants
//{
constexpr const char* HELP_MESSAGE_STRING =
    "The enemy has sent a barrage of rockets at Earth!\n"
    "You are the only pilot left from your fleet!\n"
    "Defend Earth for as long as possible!\n\n"
    "Tap on the screen to direct your ship.\n"
    "When you reach your destination, you will shoot!\n"
    "Only one shot can be active at a time."
;
constexpr double HELP_MESSAGE_X = 0.5;
constexpr double HELP_MESSAGE_Y = 0.5;
constexpr double HELP_MESSAGE_WIDTH = 0.04;
constexpr double HELP_MESSAGE_HEIGHT = 1.25 * HELP_MESSAGE_WIDTH;
constexpr double HELP_MESSAGE_X_SEPARATION = HELP_MESSAGE_WIDTH / 20;
constexpr double HELP_MESSAGE_Y_SEPARATION = HELP_MESSAGE_HEIGHT / 5;
constexpr double HELP_MESSAGE_MAX_WIDTH = 0.8;
//}

// Game Constants
//{
// Blank Constants
//{
constexpr int BLANK_X = 0;
constexpr int BLANK_Y = 0;
#define BLANK_WIDTH (display.width())
#define BLANK_HEIGHT ((1 - GAME_BACKGROUND_HEIGHT + 0.001) * display.height())
//}

// General Button Constants
//{
constexpr double BUTTON_HEIGHT = 1 - GAME_BACKGROUND_HEIGHT;
constexpr double BUTTON_WIDTH = 1.5 * BUTTON_HEIGHT;
constexpr double BUTTON_Y = BUTTON_HEIGHT / 2;
//}

// Quit Button Constants
//{
constexpr const char* QUIT_BUTTON_SOURCE = "data/quit.bmp";
constexpr double QUIT_BUTTON_X = 1 - BUTTON_WIDTH / 2;
//}

// Reset Button Constants
//{
constexpr const char* RESET_BUTTON_SOURCE = "data/reset.bmp";
constexpr double RESET_BUTTON_X = QUIT_BUTTON_X - BUTTON_WIDTH;
//}

// Play Button Constants
//{
constexpr const char* PLAY_BUTTON_SOURCE = "data/play.bmp";
constexpr double PLAY_BUTTON_X = RESET_BUTTON_X - BUTTON_WIDTH;
//}

// Pause Button Constants
//{
constexpr const char* PAUSE_BUTTON_SOURCE = "data/pause.bmp";
constexpr double PAUSE_BUTTON_X = PLAY_BUTTON_X;
//}
//}

// Class Constants
//{
// Player Constants
//{
constexpr const char* PLAYER_SOURCE = "data/player.bmp";
constexpr double PLAYER_WIDTH = 0.2;
constexpr double PLAYER_HEIGHT = 0.2;
constexpr double PLAYER_X = 0.5;
constexpr double PLAYER_Y = 0.975 - PLAYER_HEIGHT / 2;
//}

// Shot Constants
//{
constexpr const char* SHOT_SOURCE = "data/shot.bmp";
constexpr double SHOT_WIDTH = 0.01;
constexpr double SHOT_HEIGHT = PLAYER_HEIGHT;
//}

// Score Constants
//{
#define SCORE_STRING ("Score: " + std::to_string(score))
constexpr int SCORE_X = 0;
constexpr int SCORE_Y = 0;
constexpr double SCORE_HEIGHT = BUTTON_HEIGHT;
constexpr double SCORE_WIDTH = SCORE_HEIGHT / 2;
constexpr double SCORE_SEPARATION = SCORE_WIDTH / 20;
//}

// Enemy Constants
//{
constexpr const char* ENEMY_SOURCE = "data/enemy.bmp";
constexpr double ENEMY_WIDTH = 0.2;
constexpr double ENEMY_HEIGHT = 0.2;
constexpr double ENEMY_Y = BUTTON_HEIGHT - ENEMY_HEIGHT / 2;
constexpr double ENEMY_MIN = ENEMY_WIDTH / 2;
constexpr double ENEMY_MAX = 1 - ENEMY_MIN;
//}
//}
//}

// Audio Constants
//{
constexpr const char* AUDIO_SOURCE = "data/song.wav";
constexpr int AUDIO_LENGTH = 127;
//}
//}

// Classes
//{
/**
 * A class for a single enemy.
 * Enemies move downwards and end the game if they bypass the player.
 */
class Enemy {
    public:
        /**
         * An enemy has a reference to its sprite and is reset.
         */
        Enemy(const Sprite& sprite, double position, double velocity) noexcept:
            sprite(&sprite),
            position({position, ENEMY_Y}),
            velocity(velocity)
        {}
        
        /**
         * Blits the enemy to the display.
         */
        void blit_to(Sprite& display) const noexcept {
            display.blit(*sprite, position[0], position[1]);
        }
        
        /**
         * The enemy is displaced by the product of its velocity and time.
         */
        void update(double elapsed) noexcept {
            position[1] += velocity * elapsed;
        }
        
        /**
         * Returns the enemy's x-coordinate.
         */
        double get_x() const noexcept {
            return position[0];
        }
        
        /**
         * Returns the enemy's y-coordinate.
         */
        double get_y() const noexcept {
            return position[1];
        }
        
    private:
        const Sprite* sprite; // A reference to the enemy sprite.
        std::array<double, 2> position; // The enemy's coordinates.
        double velocity; // The enemy's velocity.
};

/**
 * A class that defines a shot.
 * A shot is fired by a player and can destroy enemies.
 */
class Shot {
    public:
        /**
         * Loads the shot's assets and reset it.
         */
        Shot(const Sprite& display) noexcept:
            sprite(SHOT_SOURCE, display, SHOT_WIDTH, SHOT_HEIGHT)
        {
            reset();
        }
        
        /**
         * Deactivates the shot and resets its y-coordinate.
         */
        void reset() noexcept {
            position[1] = PLAYER_Y;
            active = false;
        }
        
        /**
         * Blits the shot's sprite to the display, if the shot is active.
         */
        void blit_to(Sprite& display) const noexcept {
            if (active) {
                display.blit(sprite, position[0], position[1]);
            }
        }
        
        /**
         * Only takes effect if the shot is active.
         * The shot is moved according to its velocity.
         * If the shot moves offscreen, it is reset.
         */
        void update() noexcept {
            if (active) {
                // The current time.
                double now = Timer::time();
                
                // The time since the last update.
                double elapsed = now - last_move;
                
                // The last update is set to the present.
                last_move = now;
                
                // The displacement is a product of velocity and time.
                position[1] += SHOT_VELOCITY * elapsed;
                
                // If the shot moved completely offscreen, it is reset.
                if (position[1] < BUTTON_HEIGHT - SHOT_HEIGHT / 2) {
                    reset();
                }
            }
        }
        
        /**
         * If the shot is inactive, it is set to the given position and activated.
         * The time of last movement is set to the present.
         * Else, this function has no effect.
         */
        void activate(double position) noexcept {
            if (!active) {
                this->position[0] = position;
                last_move = Timer::time();
                active = true;
            }
        }
        
        /**
         * Increases last_move by the given time.
         */
        void pause_shift(double elapsed) noexcept {
            last_move += elapsed;
        }
        
        /**
         * Returns the shot's x-coordinate.
         */
        double get_x() const noexcept {
            return position[0];
        }
        
        /**
         * Returns the shot's y-coordinate.
         */
        double get_y() const noexcept {
            return position[1];
        }
        
    private:
        Sprite sprite; // The shot's sprite.
        std::array<double, 2> position; // The shot's coordinates.
        bool active; // True when the shot is being fired.
        double last_move; // The time when the shot was moved last.
};

/**
 * A package that stores enemies and the thread index.
 */
class EnemyPackage {
    public:
        /**
         * Stores the thread index and a reference to the enemies.
         */
        EnemyPackage(std::list<Enemy>& enemies, double data, int index) noexcept:
            enemies(enemies),
            data(data),
            index(index)
        {}
        
        /**
         * Returns the reference to the enemies.
         */
        std::list<Enemy>& get_enemies() noexcept {
            return enemies;
        }
        
        /**
         * Returns the data.
         */
        double get_data() const noexcept {
            return data;
        }
        
        /**
         * Returns the thread index.
         */
        int get_index() const noexcept {
            return index;
        }
        
    private:
        std::list<Enemy>& enemies; // The reference to the enemies.
        double data; // Extra optional data.
        int index; // The thread index.
};

/**
 * A container class for the enemies.
 * Manages all of the enemies and their shared resources.
 */
class Enemies {
    public:
        /**
         * Loads the enemy sprite and resets the enemy container.
         */
        Enemies(const Sprite& display) noexcept:
            sprite(ENEMY_SOURCE, display, ENEMY_WIDTH, ENEMY_HEIGHT),
            generator(Timer::current())
        {
            reset();
        }
        
        /**
         * Removes all of the enemies and resets the last move and next spawn time.
         */
        void reset() noexcept {
            enemies.clear();
            last_move = Timer::time();
            next_spawn = last_move + ENEMY_DELAY;
        }
        
        /**
         * Blits all of the enemies to the display.
         */
        void blit_to(Sprite& display) const noexcept {
            for (const Enemy& enemy: enemies) {
                enemy.blit_to(display);
            }
        }
        
        /**
         * Moves all of the enemies.
         * Spawns a new enemy periodically.
         */
        void update(int score) noexcept {
            // The current time.
            double now = Timer::time();
            
            // The time since the last update.
            double elapsed = now - last_move;
            
            // The time of the last update is set to the present.
            last_move = now;
            
            // Enemy packages are created for multithreading.
            EnemyPackage package0(enemies, elapsed, 0);
            EnemyPackage package1(enemies, elapsed, 1);
            EnemyPackage package2(enemies, elapsed, 2);
            EnemyPackage package3(enemies, elapsed, 3);
            
            Thread thread1(Enemies::thread_update, &package1);
            Thread thread2(Enemies::thread_update, &package2);
            Thread thread3(Enemies::thread_update, &package3);
            thread_update(&package0);
            thread1.wait();
            thread2.wait();
            thread3.wait();
            
            // A new enemy is spawned if enough time has passed.
            if (now >= next_spawn) {
                enemies.push_front(
                    Enemy(
                        sprite,
                        new_position(),
                        ENEMY_VELOCITY + score * ENEMY_ACCELERATION
                    )
                );
                
                // The time of the next spawn is set.
                next_spawn += ENEMY_DELAY;
            }
        }
        
        /**
         * Increases the value of last_move and next_spawn by the given value.
         */
        void pause_shift(double elapsed) noexcept {
            last_move += elapsed;
            next_spawn += elapsed;
        }
        
        /**
         * Checks if the shot made contact with an enemy.
         * If it did, the enemy is removed and true is returned.
         */
        bool contact(const Shot& shot) noexcept {
            for (
                auto i = enemies.crbegin();
                i != enemies.crend();
                ++i
            ) {
                if (
                    std::abs(shot.get_x() - i->get_x())
                    <= (SHOT_WIDTH + ENEMY_WIDTH) / 2
                    && std::abs(shot.get_y() - i->get_y())
                    <= (SHOT_HEIGHT + ENEMY_HEIGHT) / 2
                ) {
                    enemies.erase(std::next(i).base());
                    return true;
                }
            }
            
            return false;
        }
        
        /**
         * Returns true if one of the enemies has reached the end.
         * Returns true if one of the enemies has come into contact with the player.
         * Returns false otherwise.
         */
        bool victory(double position) const noexcept {
            const Enemy& enemy = enemies.back();
            
            return
                enemy.get_y() >= 1 - ENEMY_HEIGHT / 2
                || enemy.get_y() >= PLAYER_Y - (PLAYER_HEIGHT + ENEMY_HEIGHT) / 2
                && std::abs(enemy.get_x() - position) <= (PLAYER_WIDTH + ENEMY_WIDTH) / 2
            ;
        }
        
        /**
         * A static method to allow for multi-threading.
         */
        static int thread_update(void* data) noexcept {
            EnemyPackage& enemy_package = *static_cast<EnemyPackage*>(data);
            std::list<Enemy>& enemies = enemy_package.get_enemies();
            double elapsed = enemy_package.get_data();
            int index = enemy_package.get_index();
            
            // The enemies are moved.
            for (auto i = enemies.begin(); i != enemies.end(); ++i) {
                if (index) {
                    --index;
                }
                
                else {
                    i->update(elapsed);
                    index = THREADS - 1;
                }
            }
            
            return 0;
        }
        
    private:
        /**
         * Randomy generates an onscreen position for a new enemy.
         */
        double new_position() noexcept {
            return Random::get_double(generator, ENEMY_MIN, ENEMY_MAX);
        }
        
        Sprite sprite; // The sprite of all of the enemies.
        std::mt19937 generator; // The enemy RNG.
        std::list<Enemy> enemies; // The enemy store.
        double last_move; // The last time when the enemies were moved.
        double next_spawn; // The last time when an enemy was spawned.
};

/**
 * A class that defines a player.
 * A player can move and shoot a shot that destroys enemies.
 */
class Player {
    public:
        /**
         * Constructs a player.
         * The assets for the player are loaded.
         * The player is then reset.
         */
        Player(const Sprite& display) noexcept:
            shot(display),
            sprite(PLAYER_SOURCE, display, PLAYER_WIDTH, PLAYER_HEIGHT)
        {
            reset();
        }
        
        /**
         * Resets the player and its shot to their initial state.
         */
        void reset() noexcept {
            shot.reset();
            position = PLAYER_X;
            destination = position;
            last_move = Timer::time();
            score = 0;
        }
        
        /**
         * Blits the shot to the display.
         */
        void blit_shot(Sprite& display) const noexcept {
            shot.blit_to(display);
        }
        
        /**
         * Renders the player and the player's score.
         */
        void blit_to(Sprite& display, const Renderer& renderer) const noexcept {
            // The player is blit to the display first.
            display.blit(sprite, position, PLAYER_Y);
            
            // The score is blit to the display last.
            display.blit(
                renderer.render(
                    display,
                    SCORE_STRING,
                    SCORE_WIDTH,
                    SCORE_HEIGHT,
                    SCORE_SEPARATION
                ),
                SCORE_X,
                SCORE_Y
            );
        }
        
        /**
         * Updates the player and shot's positions.
         * Manages enemy shooting procedures.
         * Returns true if the game is over.
         */
        bool update(double display_width, Enemies& enemies) noexcept {
            // Shot contact is checked first.
            if (enemies.contact(shot)) {
                // If an enemy was shot, the score is incremented and the shot is reset.
                shot.reset();
                ++score;
            }
            
            // Then, enemy victory is checked.
            if (enemies.victory(position)) {
                return true;
            }
            
            // Then, input is checked for a new destination.
            Point mouse;
            
            if (mouse.click()) {
                destination = mouse.get_x() / display_width;
                
                // The destination can't lead the player offscreen.
                if (destination > 1 - PLAYER_WIDTH / 2) {
                    destination = 1 - PLAYER_WIDTH / 2;
                }
                
                else if (destination < PLAYER_WIDTH / 2) {
                    destination = PLAYER_WIDTH / 2;
                }
                
                // If the player is already at the destination, a shot is fired.
                else if (position == destination) {
                    shoot();
                }
            }
            
            // Then, the shot's position is updated.
            shot.update();
            
            // Finally, the player's position is updated and a shot may be fired.
            //     The current time.
            double now = Timer::time();
            
            //     The time since the last position update is calculated.
            double elapsed = now - last_move;
            
            //     The last position update is set to the present.
            last_move = now;
            
            //     The distance to be travelled is the product of speed and time.
            double distance = PLAYER_SPEED * elapsed;
            
            //     The player is to the left of the destination.
            if (position < destination) {
                // The player moves to the right.
                position += distance;
                
                // If the player passed the destination, the player moves back.
                // If the player just reached the destination, a shot is fired.
                if (position >= destination) {
                    position = destination;
                    shoot();
                }
            }
            
            //     The player is to the right of the destination.
            else if (position > destination) {
                // The player moves to the left.
                position -= distance;
                
                // If the player passed the destination, the player moves back.
                // If the player just reached the destination, a shot is fired.
                if (position <= destination) {
                    position = destination;
                    shoot();
                }
            }
            
            // The game is not over and false is returned.
            return false;
        }
        
        /**
         * Attempts to fire a shot.
         * If the shot is inactive, it is set to the player's position and activated.
         * Else, this function has no effect.
         */
        void shoot() noexcept {
            shot.activate(position);
        }
        
        /**
         * Increases the last_move value for the player and its shot by the given value.
         */
        void pause_shift(double elapsed) noexcept {
            shot.pause_shift(elapsed);
            last_move += elapsed;
        }
        
        /**
         * Returns the score.
         */
        int get_score() const noexcept {
            return score;
        }
        
    private:
        Sprite sprite; // The player's sprite.
        Shot shot; // The player's shot.
        double position; // The player's x-coordinate.
        double destination; // The player's destination.
        double last_move; // The time when the player last moved.
        int score; // The player's score.
};
//}

// Main Functions
//{
/**
 * Manages the main game.
 */
void game(Display& display, const Renderer& renderer) noexcept {
    // The background is initialised.
    Sprite background(
        GAME_BACKGROUND_SOURCE,
        display,
        GAME_BACKGROUND_WIDTH,
        GAME_BACKGROUND_HEIGHT
    );
    
    // The blank space is initialised.
    Rectangle blank(
        BLANK_X,
        BLANK_Y,
        BLANK_WIDTH,
        BLANK_HEIGHT
    );
    
    // The play button is intialised.
    Button play(
        Sprite(
            PLAY_BUTTON_SOURCE,
            display,
            BUTTON_WIDTH,
            BUTTON_HEIGHT
        ),
        display,
        PLAY_BUTTON_X,
        BUTTON_Y
    );
    
    // The pause button is intialised.
    Button pause(
        Sprite(
            PAUSE_BUTTON_SOURCE,
            display,
            BUTTON_WIDTH,
            BUTTON_HEIGHT
        ),
        display,
        PAUSE_BUTTON_X,
        BUTTON_Y
    );
    
    // The reset button is intialised.
    Button reset(
        Sprite(
            RESET_BUTTON_SOURCE,
            display,
            BUTTON_WIDTH,
            BUTTON_HEIGHT
        ),
        display,
        RESET_BUTTON_X,
        BUTTON_Y
    );
    
    // The quit button is intialised.
    Button quit(
        Sprite(
            QUIT_BUTTON_SOURCE,
            display,
            BUTTON_WIDTH,
            BUTTON_HEIGHT
        ),
        display,
        QUIT_BUTTON_X,
        BUTTON_Y
    );
    
    // The player is initialised.
    Player player(display);
    
    // The enemies are initialised.
    Enemies enemies(display);
    
    // True if the game is paused.
    bool paused = false;
    
    // Main game loop.
    while (true) {
        // The display is blitted to.
        display.blit(background, GAME_BACKGROUND_X, GAME_BACKGROUND_Y);
        player.blit_shot(display);
        enemies.blit_to(display);
        display.fill(blank);
        player.blit_to(display, renderer);
        pause.blit_to(display);
        reset.blit_to(display);
        quit.blit_to(display);
        
        // The display is updated.
        display.update();
        
        // If the quit button was clicked, the game ends.
        if (quit.get_rectangle().unclick()) {
            break;
        }
        
        // If the reset button was clicked, the game state is reset.
        else if (reset.get_rectangle().unclick()) {
            player.reset();
            enemies.reset();
            continue;
        }
        
        // If the pause button was clicked, the game is paused.
        else if (pause.get_rectangle().unclick()) {
            // The time of pausing is recorded.
            double now = Timer::time();
            
            // The play button is displayed.
            play.blit_to(display);
            display.update();
            
            // Defines the operation to be performed depending on the button pressed.
            enum Operation {
                PLAY,
                RESET,
                QUIT
            } operation;
            
            while (true) {
                if (play.get_rectangle().unclick()) {
                    operation = PLAY;
                    break;
                }
                
                else if (reset.get_rectangle().unclick()) {
                    operation = RESET;
                    break;
                }
                
                else if (quit.get_rectangle().unclick()) {
                    operation = QUIT;
                    break;
                }
                
                Events::update();
            }
            
            // The total time spent paused is recorded.
            double elapsed = Timer::time() - now;
            
            // The last_move times are shifted according to the pause time.
            player.pause_shift(elapsed);
            enemies.pause_shift(elapsed);
            
            if (operation == RESET) {
                player.reset();
                enemies.reset();
                continue;
            }
            
            else if (operation == QUIT) {
                break;
            }
        }
        
        // The player is updated.
        // A true return value means that the game is over.
        if (player.update(display.width(), enemies)) {
            enum Operation {
                RESET,
                QUIT
            } operation;
            
            while (true) {
                if (reset.get_rectangle().unclick()) {
                    operation = RESET;
                    break;
                }
                
                else if (quit.get_rectangle().unclick()) {
                    operation = QUIT;
                    break;
                }
                
                Events::update();
            }
            
            if (operation == RESET) {
                player.reset();
                enemies.reset();
            }
            
            else if (operation == QUIT) {
                break;
            }
        }
        
        // The enemies are updated.
        enemies.update(player.get_score());
        
        Events::update();
    }
}

/**
 * Displays the help message.
 * Returns to the main menu after the screen is tapped.
 */
void display_help(Display& display, const Renderer& renderer) noexcept {
    // The display is cleared.
    display.fill();
    
    // The help message is displayed.
    display.blit(
        renderer.lined_render(
            display,
            HELP_MESSAGE_STRING,
            HELP_MESSAGE_WIDTH,
            HELP_MESSAGE_HEIGHT,
            HELP_MESSAGE_X_SEPARATION,
            HELP_MESSAGE_Y_SEPARATION,
            HELP_MESSAGE_MAX_WIDTH
        ),
        HELP_MESSAGE_X,
        HELP_MESSAGE_Y
    );
    
    // The display is updated.
    display.update();
    
    // Any tap will return to the main menu.
    while (!Events::unclick()) {
        Events::update();
    }
}

/**
 * Initialises the utilities and loads the main menu.
 * Allows access to the main game and the help message.
 */
int main(int argc, char** argv) {
    // The system is initialised for video and audio.
    System::initialise(System::VIDEO | System::AUDIO);
    
    // Scope to ensure destruction of objects before termination.
    {
        // The display is initialised.
        Display display;
        
        // The audio is intialised and queued in another thread.
        AudioThread audio(AUDIO_SOURCE, AUDIO_LENGTH);
        
        // The characters and sources for the renderer are intialised.
        std::array<char, RENDERER_COUNT> characters;
        std::array<std::string, RENDERER_COUNT> sources;
        
        //     The directory is set.
        for (int i = 0; i < RENDERER_COUNT; ++i) {
            sources[i] = RENDERER_DIRECTORY;
        }
        
        //     The lowercase letters are set.
        for (int i = 0; i < RENDERER_LETTERS; ++i) {
            characters[i] = 'a' + i;
            sources[i] += 'a' + i;
        }
        
        //     The uppercase letters are set.
        for (int i = 0; i < RENDERER_LETTERS; ++i) {
            characters[RENDERER_LETTERS + i] = 'A' + i;
            sources[RENDERER_LETTERS + i] += 'a' + i;
        }
        
        //     The numbers are set.
        for (int i = 0; i < RENDERER_NUMBERS; ++i) {
            characters[RENDERER_CASES * RENDERER_LETTERS + i] = '0' + i;
            sources[RENDERER_CASES * RENDERER_LETTERS + i] += '0' + i;
        }
        
        //     The punctuation is set.
        for (int i = 0; i < RENDERER_EXTRAS; ++i) {
            characters[RENDERER_EXTRA_INDEX + i] = RENDERER_EXTRA_CHARACTERS[i];
            sources[RENDERER_EXTRA_INDEX + i] += RENDERER_EXTRA_SOURCES[i];
        }
        
        //     The file extension is set.
        for (int i = 0; i < RENDERER_COUNT; ++i) {
            sources[i] += RENDERER_EXTENSION;
        }
        
        // The renderer is initialised.
        const Renderer& renderer = FullRenderer<RENDERER_COUNT>(characters, sources);
        
        // The background is initialised.
        Sprite background(MENU_BACKGROUND_SOURCE);
        
        // The title is initialised.
        Sprite title(
            renderer.lined_render(
                display,
                TITLE_STRING,
                TITLE_WIDTH,
                TITLE_HEIGHT,
                TITLE_X_SEPARATION,
                TITLE_Y_SEPARATION
            )
        );
        
        // The play button is initialised.
        Button play(
            renderer.render(
                display,
                PLAY_STRING,
                PLAY_WIDTH,
                PLAY_HEIGHT,
                PLAY_SEPARATION
            ),
            display,
            PLAY_X,
            PLAY_Y
        );
        
        // The help button is intialised.
        Button help(
            renderer.render(
                display,
                HELP_STRING,
                HELP_WIDTH,
                HELP_HEIGHT,
                HELP_SEPARATION
            ),
            display,
            HELP_X,
            HELP_Y
        );
        
        // The info sprite is initialised.
        Sprite info(
            renderer.lined_render(
                display,
                INFO_STRING,
                INFO_WIDTH,
                INFO_HEIGHT,
                INFO_X_SEPARATION,
                INFO_Y_SEPARATION,
                INFO_MAX_WIDTH,
                INFO_JUSTIFICATION
            )
        );
        
        // True when the program should prepare for termination.
        bool end = false;
        
        // Loop to blit sprites to the display.
        while (!end) {
            // The display is blitted to.
            display.blit(background);
            display.blit(title, TITLE_X, TITLE_Y);
            display.blit(info, INFO_X, INFO_Y);
            play.blit_to(display);
            help.blit_to(display);
            
            // The display is updated.
            display.update();
            
            // Loop for user input.
            while (true) {
                // Termination method for non-mobile devices.
                if (Events::press(Events::ESCAPE)) {
                    end = true;
                    break;
                }
                
                // Help displays the help message.
                else if (help.get_rectangle().unclick()) {
                    display_help(display, renderer);
                    break;
                }
                
                // Play starts the game.
                else if (play.get_rectangle().unclick()) {
                    game(display, renderer);
                    break;
                }
                
                Events::update();
            }
        }
        
        // The audio thread is terminated.
        audio.stop();
    }
    
    // The system is terminated.
    System::terminate();
    
    // Successful program termination.
    return 0;
}
//}

/* CHANGELOG:
     v1.1:
       Multiple threads are used to update enemies.
       The oldest enemy is checked first for shot contact.
       Only the oldest enemy is checked for game over.
       Shot speed was decreased from 3 to 2.
       Enemy base speed was increased from 0.075 to 0.1.
     v1.0.3:
       Shot speed was increased from 1.5 to 3.
       Enemy base speed was decreased from 0.1 to 0.075.
     v1.0.2:
       Shot speed was decreased from 2.5 to 1.5.
       Enemy base speed was increased from 0.05 to 0.1.
       Enemy speed scaling was decreased from 0.0025 to 0.00125.
       Shots now disappear after they leave the playable area.
     v1.0.1:
       std::list replaced std::vector as the Enemy container.
       Missiles now spawn just off the playable screen instead of offscreen.
       Shot speed was decreased from 3 to 2.5.
     v1:
       Initial release.
 */