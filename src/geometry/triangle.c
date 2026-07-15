#include <stdlib.h>

#include "geometry/mesh.h"
#include "geometry/point.h"
#include "geometry/vector.h"

Point *get_triangle_normal(Triangle *triangle)
{
    Point a = *(mesh->vertices[triangle->indices[1]]->position);
    Point b = *(mesh->vertices[triangle->indices[2]]->position);
    sub_point(&a, mesh->vertices[triangle->indices[0]]->position);
    sub_point(&b, mesh->vertices[triangle->indices[0]]->position);

    Point normal = vector_cross(&a, &b);
    normal = vector_normalize(&normal);
    return create_point(normal.x, normal.y, normal.z);
}

Triangle *create_triangle(size_t indices[3])
{
    Triangle *triangle = malloc(sizeof(Triangle));

    if (!triangle)
        return NULL;

    for (size_t i = 0; i < 3; i++)
        triangle->indices[i] = indices[i];
    triangle->normal = get_triangle_normal(triangle);
    triangle->vertex_normals = calloc(3, sizeof(Point *));

    return triangle;
}

void destroy_triangle(Triangle *triangle)
{
    if (!triangle)
        return;

    for (size_t i = 0; i < 3; i++)
        free(triangle->vertex_normals[i]);
    free(triangle->vertex_normals);
    free(triangle->normal);
    free(triangle);
}
