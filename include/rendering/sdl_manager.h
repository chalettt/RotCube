#ifndef SDL_MANAGER_H
#define SDL_MANAGER_H

#include <SDL2/SDL.h>

/**
 * @brief Window width.
 */
#define WIDTH 1000

/**
 * @brief Window height.
 */
#define HEIGHT 1000

/**
 * @brief White color.
 */
#define WHITE 255, 255, 255

/**
 * @brief Black color.
 */
#define BLACK 0, 0, 0

/**
 * @brief Gray color.
 */
#define GRAY 128, 128, 128

/**
 * @brief Red color.
 */
#define RED 255, 0, 0

/**
 * @brief Green color.
 */
#define GREEN 0, 255, 0

/**
 * @brief Blue color.
 */
#define BLUE 0, 0, 255

/**
 * @brief Initializes SDL.
 *
 * @return The error code, 0 on success.
 */
int sdl_init(void);

/**
 * @brief Quits SDL.
 *
 * @param renderer The renderer to close.
 * @param window The window to close.
 */
void sdl_quit(SDL_Renderer *renderer, SDL_Window *window);

/**
 * @brief Creates an SDL window.
 *
 * @param w The window width.
 * @param h The window height.
 * @return The created window.
 */
SDL_Window *create_window(int w, int h);

/**
 * @brief Creates an SDL renderer.
 *
 * @param window The window to render on.
 * @return The created renderer.
 */
SDL_Renderer *create_renderer(SDL_Window *window);

/**
 * @brief Creates an SDL texture.
 *
 * @param renderer The renderer to draw on.
 * @return The created texture.
 */
SDL_Texture *create_texture(SDL_Renderer *renderer);

#endif /* ! SDL_MANAGER_H */
