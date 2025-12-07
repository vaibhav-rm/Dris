#include <SDL2/SDL.h>
#include <stdio.h>

int main() {
    // 1. Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    // 2. Create Window
    SDL_Window *w = SDL_CreateWindow(
        "Test SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, 0 // Increased size for better visibility
    );

    if (!w) {
        printf("SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit(); // Always clean up if creation fails
        return 1;
    }

    // 3. Main Event Loop (The Missing Part!)
    int quit = 0;
    SDL_Event event;

    while (!quit) {
        // Poll for events and process them
        while (SDL_PollEvent(&event)) {
            // Check if the user clicked the 'X' button on the window
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            // Optional: Check for a key press (e.g., ESC)
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                quit = 1;
            }
        }

        // Add rendering code here if you were drawing anything...

        // Use SDL_Delay to limit CPU usage in the loop
        SDL_Delay(10); 
    }

    // 4. Clean Up and Exit
    SDL_DestroyWindow(w);
    SDL_Quit();

    return 0;
}
