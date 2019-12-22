#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Constants
enum {
    BUTTON_SIZE = 100,
    BORDER_SIZE = 2,
    SCREEN_SIZE = 4 * BUTTON_SIZE + 5 * BORDER_SIZE,
    ROWS = 4,
    COLS = ROWS,
    BUTTONS = 15,
    SHIFT_SIZE = 5
};
const char* IMAGE_FILE = "board.png";

// Types
typedef struct {
    int val; // 1 - 15
    SDL_Point real_pos; // Left-Top corner on image
    SDL_Point view_pos; // Left-Top corner on screen
    SDL_Point board_pos; // x - column, y - row
} Button;

// Global variables
SDL_Window* window; // Window to render to
SDL_Renderer* renderer; // Window renderer
SDL_Texture* texture; // Board texture
Button buttons[BUTTONS];
Button empty_cell;

// Function declarations
// Start up SDL and create window
bool init_SDL(void);

// Free media and shut down SDL
void close_SDL(void);

// Init game board
void init_board(void);

// Draw board
void draw_board(void);

// Process event mouse down
void on_mouse_down(SDL_Event*);

// Move button
void move_button(Button*, Button*);

// Function definitions
int main(int argc, char* argv[])
{
    // Start up SDL and create window
	if (!init_SDL()) {
		fprintf(stderr, "Failed to initialize SDL!\n");
        return EXIT_FAILURE;
	}
    
    init_board();
    
    // Main loop flag
    bool quit = false;

    // While application is running
    while (!quit) {
        // Event handler
        SDL_Event e;
        
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // User requests quit
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                on_mouse_down(&e);
            }                
        }

        draw_board();
    }
    
    // Free resources and close SDL
	close_SDL();
    
    return EXIT_SUCCESS;
}

// Start up SDL and create window
bool init_SDL(void)
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}

    // Set texture filtering to linear
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
        fprintf(stderr, "Warning: Linear texture filtering not enabled!");
    }

    // Create window
    window = SDL_CreateWindow("Game 15", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        SCREEN_SIZE, SCREEN_SIZE, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }
    
    // Create vsynced renderer for window
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    // Initialize renderer color
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x80, 0xFF);

    // Initialize PNG loading
    int img_flags = IMG_INIT_PNG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }
    
    // Load board image
	SDL_Surface* surface = IMG_Load(IMAGE_FILE);
	if (!surface) {
		fprintf(stderr, "Unable to load image %s! SDL_image Error: %s\n", IMAGE_FILE, IMG_GetError());
        return false;
	}
    
    // Set color key image
	SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 0xFF, 0xFF));

    // Create texture from surface
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        fprintf(stderr, "Unable to create texture from %s! SDL Error: %s\n", IMAGE_FILE, SDL_GetError());
        return false;
    }
    
    // Free loaded surface
	SDL_FreeSurface(surface);
    
	return true;
}

// Free media and shut down SDL
void close_SDL(void)
{
	// Free texture, renderer and window
    SDL_DestroyTexture(texture);
    texture = NULL;	
	SDL_DestroyRenderer(renderer);
    renderer = NULL;
	SDL_DestroyWindow(window);
	window = NULL;

	// Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

// Init game board
void init_board(void)
{
    // Init buttons
    int col = 0;
    int row = 0;
    for (int i = 0; i < BUTTONS; i++) {
        buttons[i].val = i + 1;
        buttons[i].real_pos.x = BORDER_SIZE + col * (BORDER_SIZE + BUTTON_SIZE);
        buttons[i].real_pos.y = BORDER_SIZE + row * (BORDER_SIZE + BUTTON_SIZE);
        buttons[i].view_pos = buttons[i].real_pos;
        buttons[i].board_pos.x = col;
        buttons[i].board_pos.y = row;
        if (++col == COLS) {
            col = 0;
            row++;
        }
    }
    
    // Init empty cell at right-bottom corner
    empty_cell.real_pos.x = empty_cell.real_pos.y = empty_cell.view_pos.x = empty_cell.view_pos.y = 
        BORDER_SIZE + 3 * (BORDER_SIZE + BUTTON_SIZE);
    empty_cell.board_pos.x = empty_cell.board_pos.y = 3;
    
    // Shuffle buttons
    srand(time(NULL));
    
    for (int i = 0; i < BUTTONS * BUTTONS; i++) {
        int index = rand() % BUTTONS; // Rand num 0 - 14
        // Move view_pos
        SDL_Point temp = empty_cell.view_pos;
        empty_cell.view_pos = buttons[index].view_pos;        
        buttons[index].view_pos = temp;
        // Move board_pos
        temp = empty_cell.board_pos;
        empty_cell.board_pos = buttons[index].board_pos;
        buttons[index].board_pos = temp;
    }
}

// Draw board
void draw_board(void)
{
    // Clear screen
    SDL_RenderClear(renderer);

    // Draw board
    for (int i = 0; i < BUTTONS; i++) {
        SDL_Rect real_rect = {buttons[i].real_pos.x, buttons[i].real_pos.y, BUTTON_SIZE, BUTTON_SIZE};
        SDL_Rect view_rect = {buttons[i].view_pos.x, buttons[i].view_pos.y, BUTTON_SIZE, BUTTON_SIZE};
        SDL_RenderCopy(renderer, texture, &real_rect, &view_rect);
    }
        
    // Update screen
    SDL_RenderPresent(renderer);
}

// Move button
void on_mouse_down(SDL_Event* e)
{
    // Get mouse position
    SDL_Point point;
    SDL_GetMouseState(&point.x, &point.y);
    
    for (int i = 0; i < BUTTONS; i++) {
        SDL_Rect rect = {buttons[i].view_pos.x, buttons[i].view_pos.y, BUTTON_SIZE, BUTTON_SIZE};
        // Wich button clicked
        if (SDL_PointInRect(&point, &rect)) {
            // If near empty cell
            if ((buttons[i].board_pos.x == empty_cell.board_pos.x && 
                abs(buttons[i].board_pos.y - empty_cell.board_pos.y) == 1) ||
                (buttons[i].board_pos.y == empty_cell.board_pos.y &&
                abs(buttons[i].board_pos.x - empty_cell.board_pos.x) == 1)) {
                
                // Move button to empty cell
                move_button(&buttons[i], &empty_cell);
            }
            break;
        }
    }
}

// Move button
void move_button(Button* from, Button* to)
{
    Button temp_button = *from;
    
    if (from->view_pos.x == to->view_pos.x) { // Move vertically
        if (from->view_pos.y < to->view_pos.y) { // Move down            
            for (from->view_pos.y += SHIFT_SIZE; 
                from->view_pos.y < to->view_pos.y; 
                from->view_pos.y += SHIFT_SIZE) { 
                draw_board();
            }
        } else if (from->view_pos.y > to->view_pos.y) { // Move up
            for (from->view_pos.y -= SHIFT_SIZE;
                from->view_pos.y > to->view_pos.y;
                from->view_pos.y -= SHIFT_SIZE) {                
                draw_board();
            }
        }
    } else if (from->view_pos.y == to->view_pos.y) { // Move horizontally
        if (from->view_pos.x < to->view_pos.x) { // Move to right
            for (from->view_pos.x += SHIFT_SIZE;
                from->view_pos.x < to->view_pos.x;
                from->view_pos.x += SHIFT_SIZE) {                
                draw_board();
            }
        } else if (from->view_pos.x > to->view_pos.x) { // Move to left
            for (from->view_pos.x -= SHIFT_SIZE;
                from->view_pos.x > to->view_pos.x;
                from->view_pos.x -= SHIFT_SIZE) {                                
                draw_board();
            }
        }
    }
    
    // Move view_pos
    from->view_pos = to->view_pos;        
    to->view_pos = temp_button.view_pos;
    
    // Move board_pos
    from->board_pos = to->board_pos;
    to->board_pos = temp_button.board_pos;
}
