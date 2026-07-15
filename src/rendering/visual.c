#include "rendering/visual.h"

#include <stdbool.h>
#include <stdint.h>

#include "geometry/triangle.h"
#include "geometry/vector.h"
#include "rendering/camera.h"
#include "rendering/sdl_manager.h"

static uint32_t framebuffer[WIDTH * HEIGHT];
static float zbuffer[WIDTH * HEIGHT];
static Point world_light = { 0, 0.7, -0.7 };
static Point *projected_vertices = NULL;

static void clear_buffers(void)
{
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        uint8_t value = 64;
        framebuffer[i] = (255 << 24) | (value << 16) | (value << 8) | value;
        zbuffer[i] = FLT_MAX;
    }
}

static double get_light_intensity(Point *face_normal)
{
    double intensity = dot_product(face_normal, &world_light);
    return intensity;
}

static bool is_valid(Point p)
{
    return p.z > 0.01;
}

static void project_vertices(Vertex **vertices)
{
    for (size_t i = 0; vertices[i]; i++)
        projected_vertices[i] = project(vertices[i]->position);
}

uint32_t get_color(Triangle *triangle, double w0, double w1, double w2)
{
    Point *a = triangle->vertex_normals[0];
    Point *b = triangle->vertex_normals[1];
    Point *c = triangle->vertex_normals[2];

    double intensity_a = get_light_intensity(a);
    double intensity_b = get_light_intensity(b);
    double intensity_c = get_light_intensity(c);

    float intensity = w0 * intensity_a + w1 * intensity_b + w2 * intensity_c;
    intensity = fmax(0.1, fmin(intensity, 1));
    uint8_t value = (uint8_t)(intensity * 255);
    return (255 << 24) | (value << 16) | (value << 8) | value;
}

static void rasterize_triangle(Triangle *triangle)
{
    Point a = projected_vertices[triangle->indices[0]];
    Point b = projected_vertices[triangle->indices[1]];
    Point c = projected_vertices[triangle->indices[2]];
    if (!is_valid(a) || !is_valid(b) || !is_valid(c))
        return;

    double area = determinant(&a, &b, &c);

    double min_x = fmax(0, floor(fmin(a.x, fmin(b.x, c.x))));
    double max_x = fmin(WIDTH, ceil(fmax(a.x, fmax(b.x, c.x))));
    double min_y = fmax(0, floor(fmin(a.y, fmin(b.y, c.y))));
    double max_y = fmin(HEIGHT, ceil(fmax(a.y, fmax(b.y, c.y))));

    for (size_t y = min_y; y < max_y; y++)
    {
        for (size_t x = min_x; x < max_x; x++)
        {
            Point p = { x + 0.5, y + 0.5, 0 };

            double w0 = determinant(&b, &c, &p);
            double w1 = determinant(&c, &a, &p);
            double w2 = determinant(&a, &b, &p);

            if ((w0 >= 0 && w1 >= 0 && w2 >= 0)
                || (w0 <= 0 && w1 <= 0 && w2 <= 0))
            {
                w0 /= area;
                w1 /= area;
                w2 /= area;

                float depth = w0 * a.z + w1 * b.z + w2 * c.z;

                int idx = y * WIDTH + x;
                if (idx < 0 || idx >= WIDTH * HEIGHT)
                    continue;
                if (depth < zbuffer[idx])
                {
                    zbuffer[idx] = depth;
                    framebuffer[idx] = get_color(triangle, w0, w1, w2);
                }
            }
        }
    }
}

void draw_mesh(SDL_Texture *texture, Mesh *mesh)
{
    clear_buffers();

    Triangle **triangles = mesh->triangles;
    if (!projected_vertices)
        projected_vertices = malloc(mesh->vertex_count * sizeof(Point));
    project_vertices(mesh->vertices);

    for (size_t i = 0; triangles[i]; i++)
    {
        Triangle *triangle = triangles[i];
        Point rel = *(mesh->vertices[triangle->indices[0]]->position);
        sub_point(&rel, camera->position);
        // if (dot_product(triangle->normal, &rel) < 0)
        rasterize_triangle(triangle);
    }

    SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t));
}
