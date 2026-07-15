#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <bits/time.h>
#include <time.h>

#include "geometry/mesh.h"
#include "geometry/point.h"
#include "geometry/vector.h"
#include "rendering/camera.h"
#include "rendering/sdl_manager.h"
#include "rendering/visual.h"
#include "utils/debug.h"

void draw_normal(SDL_Renderer *r, Point *pos, Point *normal)
{
    SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
    Point projection_p = project(pos);
    Point *end = create_point(pos->x, pos->y, pos->z);
    add_point(end, normal);
    Point projection_n = project(end);
    free(end);

    if (projection_n.z < 0.01 || projection_p.z < 0.01)
        return;

    double dot = dot_product(normal, camera->look_ahead);
    if (dot < 0)
        return;

    SDL_RenderDrawLine(r, projection_p.x, projection_p.y, projection_n.x,
                       projection_n.y);
}

Point *handle_args(int argc, char **argv, double *scale)
{
    Point *origin = malloc(sizeof(Point));
    for (int i = 2; i < argc; i++)
    {
        if (!strcmp(argv[i], "-x"))
            origin->x = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-y"))
            origin->y = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-z"))
            origin->z = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-s"))
            *scale = strtof(argv[++i], NULL);
    }
    return origin;
}

int main(int argc, char **argv)
{
    if (sdl_init())
        return 1;

    // Creating the window and renderer.
    SDL_Window *window = create_window(WIDTH, HEIGHT);
    SDL_Renderer *renderer = create_renderer(window);
    SDL_Texture *texture = create_texture(renderer);

    unsigned running = 1;
    SDL_Event event;

    double scale = 1;
    Point *origin = handle_args(argc, argv, &scale);

    char *obj_path = argv[1];

    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    mesh = load_mesh(obj_path, origin);
    if (!mesh)
    {
        free(origin);
        sdl_quit(renderer, window);
        return 1;
    }

    clock_gettime(CLOCK_REALTIME, &end);
    LOG("Wavefront loading took %f seconds\n",
        (end.tv_sec - start.tv_sec)
            + (end.tv_nsec - start.tv_nsec) / 1000000000.0);

    if (scale != 1)
        scale_mesh(mesh, scale);

    size_t triangle_count = 0;
    while (mesh->triangles[triangle_count])
        triangle_count++;
    init_camera(0, 0, 0);

    // Rotation speed.
    double alpha = 0.025;
    // Movement speed.
    double delta = 2;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    int mouse_button = 0;

    // Window loop
    while (running)
    {
        double mouse_delta_x = 0.0;
        double mouse_delta_y = 0.0;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            // If the window gets terminated.
            case SDL_QUIT:
                // Exit the loop.
                running = 0;
                break;
            case SDL_MOUSEMOTION: {
                mouse_delta_x = event.motion.xrel;
                mouse_delta_y = event.motion.yrel;
                if (mouse_button)
                    break;
                if (mouse_delta_x != 0)
                    rotate_camera_y(alpha, mouse_delta_x / 10);
                if (mouse_delta_y != 0)
                    rotate_camera_x(alpha, -mouse_delta_y / 10);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
                mouse_button = 1;
                SDL_SetRelativeMouseMode(SDL_TRUE);
                break;
            case SDL_MOUSEBUTTONUP:
                mouse_button = 0;
                break;
            default:
                break;
            }
        }

        const Uint8 *state = SDL_GetKeyboardState(NULL);

        if (state[SDL_SCANCODE_ESCAPE])
            SDL_SetRelativeMouseMode(SDL_FALSE);

        if (state[SDL_SCANCODE_A])
            move_camera(LEFT, delta);

        if (state[SDL_SCANCODE_D])
            move_camera(RIGHT, delta);

        if (state[SDL_SCANCODE_W])
            move_camera(FRONT, delta);

        if (state[SDL_SCANCODE_S])
            move_camera(BACK, delta);

        if (state[SDL_SCANCODE_SPACE])
            move_camera(UP, delta);

        if (state[SDL_SCANCODE_LCTRL])
            move_camera(DOWN, delta);

        if (state[SDL_SCANCODE_LSHIFT])
            delta = 5;
        else
            delta = 2;

        if (mouse_button)
        {
            rotate_mesh(mesh, mouse_delta_x * alpha, RIGHT);
            rotate_mesh(mesh, -mouse_delta_y * alpha, FRONT);
        }

        draw_mesh(texture, mesh);
        SDL_RenderCopy(renderer, texture, NULL, NULL);

#ifdef DEBUG
        for (size_t i = 0; mesh->triangles[i]; i++)
        {
            Triangle *triangle = mesh->triangles[i];
            for (size_t j = 0; j < 3; j++)
            {
                Point *pos = mesh->vertices[triangle->indices[j]]->position;
                draw_normal(renderer, pos, triangle->vertex_normals[j]);
            }
        }
#endif
        SDL_RenderPresent(renderer);

        // 60 frames a second.
        SDL_Delay(1000 / 60);
    }

    // Properly quits sdl.
    sdl_quit(renderer, window);

    // Frees everything
    destroy_mesh(mesh);

    return 0;
}
