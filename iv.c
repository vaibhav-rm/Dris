#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_easy_font.h"
#include <SDL2/SDL.h>
#include <stdio.h>

void draw_text(SDL_Renderer *renderer, float x, float y, const char *text, float scale) {
    static char buffer[99999]; // Buffer for vertices
    int num_quads = stb_easy_font_print(0, 0, (char *)text, NULL, buffer, sizeof(buffer));

    // Convert quads to SDL_Vertex
    // Each quad has 4 vertices. stb_easy_font returns interleaved x,y,z (float)
    // We need 6 vertices per quad for triangles (2 triangles per quad) or use indices.
    // Simpler: Draw triangles directly.
    
    // Actually SDL_RenderGeometryRaw is better if available, but let's stick to standard loops for portability if needed.
    // stb_easy_font produces: x,y,z, x,y,z, x,y,z, x,y,z (4 verts per quad)
    
    float *v = (float *)buffer;
    SDL_Vertex verts[4]; // Reused per quad
    for (int i = 0; i < num_quads; i++) {
        for (int j = 0; j < 4; j++) {
            verts[j].position.x = (v[0] * scale) + x;
            verts[j].position.y = (v[1] * scale) + y;
            verts[j].color.r = 255;
            verts[j].color.g = 255;
            verts[j].color.b = 255;
            verts[j].color.a = 255;
            verts[j].tex_coord.x = 0;
            verts[j].tex_coord.y = 0;
            v += 3; // Advance 3 floats (x,y,z)
        }
        
        // Draw 2 triangles for the quad: 0,1,2 and 0,2,3
        int indices[6] = {0, 1, 2, 0, 2, 3};
        SDL_RenderGeometry(renderer, NULL, verts, 4, indices, 6);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <image_path>\n", argv[0]);
        return 1;
    }

    const char *img_path = argv[1];
    int img_w, img_h, channels;
    unsigned char *data = stbi_load(img_path, &img_w, &img_h, &channels, 4);
    if (!data) {
        printf("Failed to load image: %s\n", img_path);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        stbi_image_free(data);
        return 1;
    }

    int win_w = 800;
    int win_h = 600;

    SDL_Window *window = SDL_CreateWindow(
        "Dris Image Viewer", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        win_w, win_h, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        stbi_image_free(data);
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        stbi_image_free(data);
        SDL_Quit();
        return 1;
    }

    // Create Texture
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        Uint32 rmask = 0xff000000;
        Uint32 gmask = 0x00ff0000;
        Uint32 bmask = 0x0000ff00;
        Uint32 amask = 0x000000ff;
    #else
        Uint32 rmask = 0x000000ff;
        Uint32 gmask = 0x0000ff00;
        Uint32 bmask = 0x00ff0000;
        Uint32 amask = 0xff000000;
    #endif

    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
        data, img_w, img_h, 32, img_w * 4, 
        rmask, gmask, bmask, amask
    );

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    stbi_image_free(data);

    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Image State
    float scale = 1.0f;
    float offset_x = 0.0f;
    float offset_y = 40.0f; // Start below toolbar
    int is_dragging = 0;
    int last_mouse_x = 0;
    int last_mouse_y = 0;

    // UI State
    int toolbar_h = 40;
    int show_about = 0;

    // Viewport calculation
    int view_w = win_w;
    int view_h = win_h - toolbar_h;
    
    // Initial Fit
    float scale_w = (float)view_w / img_w;
    float scale_h = (float)view_h / img_h;
    scale = (scale_w < scale_h) ? scale_w : scale_h;
    offset_x = (view_w - img_w * scale) / 2;
    offset_y = toolbar_h + (view_h - img_h * scale) / 2;

    SDL_Event event;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (show_about) show_about = 0;
                    else quit = 1;
                }
            } else if (event.type == SDL_MOUSEWHEEL) {
                if (!show_about) {
                    int mouse_x, mouse_y;
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    if (mouse_y > toolbar_h) { // Only zoom if in image area
                        float zoom_factor = (event.wheel.y > 0) ? 1.1f : 0.9f;
                        float new_scale = scale * zoom_factor;
                        offset_x = mouse_x - (mouse_x - offset_x) * (new_scale / scale);
                        offset_y = mouse_y - (mouse_y - offset_y) * (new_scale / scale);
                        scale = new_scale;
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (show_about) {
                        show_about = 0; // Click anywhere to close about
                    } else if (event.button.y < toolbar_h) {
                        // Toolbar Clicks
                        int x = event.button.x;
                        if (x > 10 && x < 60) { // Zoom +
                            scale *= 1.2f;
                        } else if (x > 70 && x < 120) { // Zoom -
                            scale *= 0.8f;
                        } else if (x > 130 && x < 210) { // Reset
                             view_w = win_w;
                             view_h = win_h - toolbar_h;
                             scale_w = (float)view_w / img_w;
                             scale_h = (float)view_h / img_h;
                             scale = (scale_w < scale_h) ? scale_w : scale_h;
                             offset_x = (view_w - img_w * scale) / 2;
                             offset_y = toolbar_h + (view_h - img_h * scale) / 2;
                        } else if (x > 220 && x < 300) { // About
                            show_about = 1;
                        }
                    } else {
                        // Image Pan Start
                        is_dragging = 1;
                        last_mouse_x = event.button.x;
                        last_mouse_y = event.button.y;
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    is_dragging = 0;
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                if (is_dragging && !show_about) {
                    offset_x += (event.motion.x - last_mouse_x);
                    offset_y += (event.motion.y - last_mouse_y);
                    last_mouse_x = event.motion.x;
                    last_mouse_y = event.motion.y;
                }
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    win_w = event.window.data1;
                    win_h = event.window.data2;
                }
            }
        }
        
        // Render
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // Draw Image
        SDL_Rect dest_rect;
        dest_rect.x = (int)offset_x;
        dest_rect.y = (int)offset_y;
        dest_rect.w = (int)(img_w * scale);
        dest_rect.h = (int)(img_h * scale);
        SDL_RenderCopy(renderer, texture, NULL, &dest_rect);

        // Draw Toolbar
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_Rect toolbar_rect = {0, 0, win_w, toolbar_h};
        SDL_RenderFillRect(renderer, &toolbar_rect);

        // Draw Buttons (Simple Rects for now, or just text)
        // Zoom + (10, 5, 50, 30)
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_Rect btn_plus = {10, 5, 50, 30};
        SDL_RenderFillRect(renderer, &btn_plus);
        draw_text(renderer, 25, 12, "+", 2);

        // Zoom - (70, 5, 50, 30)
        SDL_Rect btn_minus = {70, 5, 50, 30};
        SDL_RenderFillRect(renderer, &btn_minus);
        draw_text(renderer, 85, 12, "-", 2);

        // Reset (130, 5, 80, 30)
        SDL_Rect btn_reset = {130, 5, 80, 30};
        SDL_RenderFillRect(renderer, &btn_reset);
        draw_text(renderer, 140, 12, "Reset", 2);

        // About (220, 5, 80, 30)
        SDL_Rect btn_about = {220, 5, 80, 30};
        SDL_RenderFillRect(renderer, &btn_about);
        draw_text(renderer, 230, 12, "About", 2);

        // Draw About Overlay
        if (show_about) {
            // Semi-transparent bg
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_Rect overlay_rect = {0, 0, win_w, win_h};
            SDL_RenderFillRect(renderer, &overlay_rect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

            // Dialog Box
            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            SDL_Rect box_rect = {win_w/2 - 150, win_h/2 - 100, 300, 200};
            SDL_RenderFillRect(renderer, &box_rect);

            // Text
            draw_text(renderer, box_rect.x + 20, box_rect.y + 40, "DRIS VIEWER", 3);
            draw_text(renderer, box_rect.x + 20, box_rect.y + 100, "Version: 1.0", 2);
            draw_text(renderer, box_rect.x + 20, box_rect.y + 130, "Fast & Simple", 2);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
