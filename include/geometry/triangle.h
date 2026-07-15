#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stddef.h>

#include "geometry/point.h"

/**
 * @brief Structure to hold data of triangles.
 */
typedef struct triangle
{
    size_t indices[3];
    Point **vertex_normals;
    Point *normal;
} Triangle;

/**
 * @brief Creates a triangle.
 *
 * @param indices The indices of each vertex relative to the mesh's vertices.
 * @return The created triangle.
 */
Triangle *create_triangle(size_t indices[3]);

/**
 * @brief Destroys (frees) a triangle.
 *
 * @param triangle The triangle to destroy.
 */
void destroy_triangle(Triangle *triangle);

/**
 * @brief Calculates the normal of a triangle.
 *
 * @param triangle The triangle to calculate the normal of.
 * @return The triangle normal.
 */
Point *get_triangle_normal(Triangle *triangle);

#endif /* TRIANGLE_H */
