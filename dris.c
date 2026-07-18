#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_easy_font.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

#define MAX_FILES 2048
#define MAX_PATH_LEN 1024

// Global variables for directory scanning and file tracking
char file_list[MAX_FILES][MAX_PATH_LEN];
int file_count = 0;
int current_file_index = -1;
char current_dir[MAX_PATH_LEN] = ".";
char current_filename[MAX_PATH_LEN] = "";

void draw_text(SDL_Renderer *renderer, float x, float y, const char *text, float scale) {
    static char buffer[99999]; // Buffer for vertices
    int num_quads = stb_easy_font_print(0, 0, (char *)text, NULL, buffer, sizeof(buffer));
    
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
            v += 4; // Advance 4 floats (x,y,z, color_bytes) = 16 bytes
        }
        
        // Draw 2 triangles for the quad: 0,1,2 and 0,2,3
        int indices[6] = {0, 1, 2, 0, 2, 3};
        SDL_RenderGeometry(renderer, NULL, verts, 4, indices, 6);
    }
}

void split_path(const char *path, char *dir, char *filename) {
    const char *last_slash = strrchr(path, '/');
#ifdef _WIN32
    const char *last_backslash = strrchr(path, '\\');
    if (last_backslash > last_slash) {
        last_slash = last_backslash;
    }
#endif

    if (last_slash) {
        int dir_len = last_slash - path;
        if (dir_len == 0) {
            strcpy(dir, "/");
        } else {
            strncpy(dir, path, dir_len);
            dir[dir_len] = '\0';
        }
        strcpy(filename, last_slash + 1);
    } else {
        strcpy(dir, ".");
        strcpy(filename, path);
    }
}

void get_full_path(char *dest, const char *dir, const char *filename) {
    if (strcmp(dir, ".") == 0) {
        strcpy(dest, filename);
    } else if (strcmp(dir, "/") == 0) {
        sprintf(dest, "/%s", filename);
    } else {
#ifdef _WIN32
        sprintf(dest, "%s\\%s", dir, filename);
#else
        sprintf(dest, "%s/%s", dir, filename);
#endif
    }
}

int compare_strings(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

int is_image_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot) return 0;
    
    if (strcasecmp(dot, ".png") == 0 ||
        strcasecmp(dot, ".jpg") == 0 ||
        strcasecmp(dot, ".jpeg") == 0 ||
        strcasecmp(dot, ".bmp") == 0 ||
        strcasecmp(dot, ".gif") == 0 ||
        strcasecmp(dot, ".webp") == 0 ||
        strcasecmp(dot, ".tga") == 0 ||
        strcasecmp(dot, ".hdr") == 0) {
        return 1;
    }
    return 0;
}

void scan_directory(const char *dir_path, const char *cur_filename) {
    file_count = 0;
    current_file_index = -1;

    DIR *d = opendir(dir_path);
    if (!d) return;

    struct dirent *dir_entry;
    while ((dir_entry = readdir(d)) != NULL) {
        if (dir_entry->d_type == DT_REG || dir_entry->d_type == DT_UNKNOWN || dir_entry->d_type == DT_LNK) {
            if (is_image_extension(dir_entry->d_name)) {
                if (file_count < MAX_FILES) {
                    strncpy(file_list[file_count], dir_entry->d_name, MAX_PATH_LEN - 1);
                    file_list[file_count][MAX_PATH_LEN - 1] = '\0';
                    file_count++;
                }
            }
        }
    }
    closedir(d);

    // Sort files alphabetically
    qsort(file_list, file_count, sizeof(file_list[0]), compare_strings);

    // Find the current file's index
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_list[i], cur_filename) == 0) {
            current_file_index = i;
            break;
        }
    }

    // Fallback if not found
    if (current_file_index == -1 && file_count < MAX_FILES) {
        strncpy(file_list[file_count], cur_filename, MAX_PATH_LEN - 1);
        current_file_index = file_count;
        file_count++;
    }
}

SDL_Texture* load_texture(SDL_Renderer *renderer, const char *path, int *w, int *h) {
    int channels;
    unsigned char *data = stbi_load(path, w, h, &channels, 4);
    if (!data) {
        printf("Failed to load image: %s\n", path);
        return NULL;
    }

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
        data, *w, *h, 32, (*w) * 4, 
        rmask, gmask, bmask, amask
    );

    if (!surface) {
        stbi_image_free(data);
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    stbi_image_free(data);
    return texture;
}

void update_window_title(SDL_Window *window, const char *filename, int index, int total, float scale, int angle) {
    char title[512];
    int zoom_percent = (int)(scale * 100.0f);
    if (total > 1 && index >= 0) {
        if (angle != 0) {
            sprintf(title, "Dris - %s [%d/%d] - %d%% (Rotated %d°)", filename, index + 1, total, zoom_percent, angle);
        } else {
            sprintf(title, "Dris - %s [%d/%d] - %d%%", filename, index + 1, total, zoom_percent);
        }
    } else {
        if (angle != 0) {
            sprintf(title, "Dris - %s - %d%% (Rotated %d°)", filename, zoom_percent, angle);
        } else {
            sprintf(title, "Dris - %s - %d%%", filename, zoom_percent);
        }
    }
    SDL_SetWindowTitle(window, title);
}

void calculate_fit(int win_w, int win_h, int toolbar_h, int img_w, int img_h, int angle, float *scale, float *offset_x, float *offset_y) {
    int view_w = win_w;
    int view_h = win_h - toolbar_h;
    
    // Handle swapped aspect ratio if rotated 90 or 270 degrees
    int effective_w = img_w;
    int effective_h = img_h;
    if (angle == 90 || angle == 270) {
        effective_w = img_h;
        effective_h = img_w;
    }

    float scale_w = (float)view_w / effective_w;
    float scale_h = (float)view_h / effective_h;
    *scale = (scale_w < scale_h) ? scale_w : scale_h;
    
    // Fit should center the image
    *offset_x = (view_w - img_w * (*scale)) / 2;
    *offset_y = toolbar_h + (view_h - img_h * (*scale)) / 2;
}

void load_adjacent_image(int direction, SDL_Renderer *renderer, SDL_Texture **texture, int *img_w, int *img_h, float *scale, float *offset_x, float *offset_y, int win_w, int win_h, int toolbar_h, int *angle, SDL_Window *window) {
    if (file_count <= 1) return;

    int next_index = current_file_index + direction;
    if (next_index >= file_count) next_index = 0;
    if (next_index < 0) next_index = file_count - 1;

    char new_path[MAX_PATH_LEN * 2];
    get_full_path(new_path, current_dir, file_list[next_index]);

    int new_w, new_h;
    SDL_Texture *new_texture = load_texture(renderer, new_path, &new_w, &new_h);
    if (new_texture) {
        SDL_DestroyTexture(*texture);
        *texture = new_texture;
        *img_w = new_w;
        *img_h = new_h;
        current_file_index = next_index;
        strcpy(current_filename, file_list[next_index]);
        *angle = 0; // Reset rotation for new image

        calculate_fit(win_w, win_h, toolbar_h, *img_w, *img_h, *angle, scale, offset_x, offset_y);
        update_window_title(window, current_filename, current_file_index, file_count, *scale, *angle);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <image_path>\n", argv[0]);
        return 1;
    }

    const char *img_path = argv[1];
    split_path(img_path, current_dir, current_filename);
    scan_directory(current_dir, current_filename);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
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
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int img_w, img_h;
    char first_path[MAX_PATH_LEN * 2];
    get_full_path(first_path, current_dir, current_filename);
    SDL_Texture *texture = load_texture(renderer, first_path, &img_w, &img_h);
    if (!texture) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Image & UI State
    float scale = 1.0f;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    int is_dragging = 0;
    int last_mouse_x = 0;
    int last_mouse_y = 0;

    int toolbar_h = 40;
    int show_about = 0;
    int angle = 0;
    int is_fullscreen = 0;
    int is_toolbar_hidden = 0;

    // Initial fit and window title
    calculate_fit(win_w, win_h, toolbar_h, img_w, img_h, angle, &scale, &offset_x, &offset_y);
    update_window_title(window, current_filename, current_file_index, file_count, scale, angle);

    SDL_Event event;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (show_about) {
                        show_about = 0;
                    } else if (is_fullscreen) {
                        SDL_SetWindowFullscreen(window, 0);
                        is_fullscreen = 0;
                        toolbar_h = is_toolbar_hidden ? 0 : 40;
                        SDL_GetWindowSize(window, &win_w, &win_h);
                        calculate_fit(win_w, win_h, toolbar_h, img_w, img_h, angle, &scale, &offset_x, &offset_y);
                        update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                    } else {
                        quit = 1;
                    }
                } else if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_PAGEDOWN) {
                    if (!show_about) {
                        if (event.key.keysym.sym == SDLK_RIGHT && (event.key.keysym.mod & KMOD_CTRL)) {
                            offset_x += 40.0f;
                        } else {
                            load_adjacent_image(1, renderer, &texture, &img_w, &img_h, &scale, &offset_x, &offset_y, win_w, win_h, toolbar_h, &angle, window);
                        }
                    }
                } else if (event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_BACKSPACE || event.key.keysym.sym == SDLK_PAGEUP) {
                    if (!show_about) {
                        if (event.key.keysym.sym == SDLK_LEFT && (event.key.keysym.mod & KMOD_CTRL)) {
                            offset_x -= 40.0f;
                        } else {
                            load_adjacent_image(-1, renderer, &texture, &img_w, &img_h, &scale, &offset_x, &offset_y, win_w, win_h, toolbar_h, &angle, window);
                        }
                    }
                } else if (event.key.keysym.sym == SDLK_UP) {
                    if (!show_about) offset_y -= 40.0f;
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    if (!show_about) offset_y += 40.0f;
                } else if (event.key.keysym.sym == SDLK_r) {
                    if (!show_about) {
                        angle = (angle + 90) % 360;
                        calculate_fit(win_w, win_h, toolbar_h, img_w, img_h, angle, &scale, &offset_x, &offset_y);
                        update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                    }
                } else if (event.key.keysym.sym == SDLK_l) {
                    if (!show_about) {
                        angle = (angle + 270) % 360;
                        calculate_fit(win_w, win_h, toolbar_h, img_w, img_h, angle, &scale, &offset_x, &offset_y);
                        update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                    }
                } else if (event.key.keysym.sym == SDLK_f || event.key.keysym.sym == SDLK_F11) {
                    if (!show_about) {
                        if (is_fullscreen) {
                            SDL_SetWindowFullscreen(window, 0);
                            is_fullscreen = 0;
                            toolbar_h = is_toolbar_hidden ? 0 : 40;
                        } else {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                            is_fullscreen = 1;
                            toolbar_h = 0;
                        }
                        SDL_GetWindowSize(window, &win_w, &win_h);
                        calculate_fit(win_w, win_h, toolbar_h, img_w, img_h, angle, &scale, &offset_x, &offset_y);
                        update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                    }
                } else if (event.key.keysym.sym == SDLK_t || event.key.keysym.sym == SDLK_h) {
                    if (!show_about) {
                        is_toolbar_hidden = !is_toolbar_hidden;
                        toolbar_h = is_toolbar_hidden ? 0 : 40;
                        calculate_fit(win_w, win_h, toolbar_h, img_w, img_h, angle, &scale, &offset_x, &offset_y);
                        update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                    }
                } else if (event.key.keysym.sym == SDLK_KP_PLUS || event.key.keysym.sym == SDLK_PLUS || event.key.keysym.sym == SDLK_EQUALS) {
                    if (!show_about) {
                        scale *= 1.1f;
                        update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                    }
                } else if (event.key.keysym.sym == SDLK_KP_MINUS || event.key.keysym.sym == SDLK_MINUS) {
                    if (!show_about) {
                        scale *= 0.9f;
                        update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                    }
                } else if (event.key.keysym.sym == SDLK_0 || event.key.keysym.sym == SDLK_HOME) {
                    if (!show_about) {
                        calculate_fit(win_w, win_h, toolbar_h, img_w, img_h, angle, &scale, &offset_x, &offset_y);
                        update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                    }
                }
            } else if (event.type == SDL_MOUSEWHEEL) {
                if (!show_about) {
                    int mouse_x, mouse_y;
                    SDL_GetMouseState(&mouse_x, &mouse_y);
                    if (mouse_y > toolbar_h) {
                        float zoom_factor = (event.wheel.y > 0) ? 1.1f : 0.9f;
                        float new_scale = scale * zoom_factor;
                        if (new_scale > 0.01f && new_scale < 100.0f) {
                            offset_x = mouse_x - (mouse_x - offset_x) * (new_scale / scale);
                            offset_y = mouse_y - (mouse_y - offset_y) * (new_scale / scale);
                            scale = new_scale;
                            update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                        }
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (show_about) {
                        show_about = 0;
                    } else if (event.button.y < toolbar_h && !is_toolbar_hidden) {
                        int x = event.button.x;
                        if (x > 10 && x < 60) { // Zoom +
                            scale *= 1.1f;
                            update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                        } else if (x > 70 && x < 120) { // Zoom -
                            scale *= 0.9f;
                            update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                        } else if (x > 130 && x < 210) { // Reset
                            calculate_fit(win_w, win_h, toolbar_h, img_w, img_h, angle, &scale, &offset_x, &offset_y);
                            update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                        } else if (x > 220 && x < 300) { // About
                            show_about = 1;
                        } else if (x > 310 && x < 370) { // Rotate CW
                            angle = (angle + 90) % 360;
                            calculate_fit(win_w, win_h, toolbar_h, img_w, img_h, angle, &scale, &offset_x, &offset_y);
                            update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                        } else if (x > 380 && x < 440) { // Rotate CCW
                            angle = (angle + 270) % 360;
                            calculate_fit(win_w, win_h, toolbar_h, img_w, img_h, angle, &scale, &offset_x, &offset_y);
                            update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                        }
                    } else {
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
                    // Recalculate centering when window resizes
                    calculate_fit(win_w, win_h, toolbar_h, img_w, img_h, angle, &scale, &offset_x, &offset_y);
                    update_window_title(window, current_filename, current_file_index, file_count, scale, angle);
                }
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // Draw Image with rotation support
        SDL_Rect dest_rect;
        dest_rect.x = (int)offset_x;
        dest_rect.y = (int)offset_y;
        dest_rect.w = (int)(img_w * scale);
        dest_rect.h = (int)(img_h * scale);
        
        if (angle != 0) {
            SDL_RenderCopyEx(renderer, texture, NULL, &dest_rect, (double)angle, NULL, SDL_FLIP_NONE);
        } else {
            SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
        }

        // Draw Toolbar
        if (!is_toolbar_hidden) {
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_Rect toolbar_rect = {0, 0, win_w, toolbar_h};
            SDL_RenderFillRect(renderer, &toolbar_rect);

            // Draw Buttons
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

            // Rotate CW (310, 5, 60, 30)
            SDL_Rect btn_rot_cw = {310, 5, 60, 30};
            SDL_RenderFillRect(renderer, &btn_rot_cw);
            draw_text(renderer, 320, 12, "CW", 2);

            // Rotate CCW (380, 5, 60, 30)
            SDL_Rect btn_rot_ccw = {380, 5, 60, 30};
            SDL_RenderFillRect(renderer, &btn_rot_ccw);
            draw_text(renderer, 390, 12, "CCW", 2);
        }

        // Draw About Overlay
        if (show_about) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
            SDL_Rect overlay_rect = {0, 0, win_w, win_h};
            SDL_RenderFillRect(renderer, &overlay_rect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            SDL_Rect box_rect = {win_w/2 - 150, win_h/2 - 120, 300, 240};
            SDL_RenderFillRect(renderer, &box_rect);

            draw_text(renderer, box_rect.x + 20, box_rect.y + 30, "DRIS VIEWER", 3);
            draw_text(renderer, box_rect.x + 20, box_rect.y + 80, "Version: 1.1", 2);
            draw_text(renderer, box_rect.x + 20, box_rect.y + 110, "Fast & Simple", 2);
            draw_text(renderer, box_rect.x + 20, box_rect.y + 140, "Credits: vaibhav-rm", 2);
            draw_text(renderer, box_rect.x + 20, box_rect.y + 180, "Esc / Click to Close", 1.5);
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
