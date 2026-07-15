#ifndef MESH_H
#define MESH_H

#include <stddef.h>

#include "geometry/triangle.h"
#include "geometry/vertex.h"

/**
 * @brief Default buffer size for retrieving data from wavefront files.
 */
#define BUFFER_SIZE 4096

/**
 * @brief Structure to hold data relative to the loaded mesh.
 */
typedef struct mesh
{
    Vertex **vertices;
    Triangle **triangles;
    Point *origin;
    size_t vertex_count;
    size_t triangle_count;
} Mesh;

extern Mesh *mesh;

/**
 * @brief Loads a mesh.
 *
 * @param path The path to the wavefront file.
 * @param origin The origin of the mesh.
 * @return The loaded mesh.
 */
Mesh *load_mesh(char *path, Point *origin);

/**
 * @brief Destroys (frees) a mesh.
 *
 * @param mesh The mesh to destroy.
 */
void destroy_mesh(Mesh *mesh);

/**
 * @brief Rotates a mesh.
 *
 * @param mesh The mesh to rotate.
 * @param alpha The angle to rotate the mesh by.
 */
void rotate_mesh(Mesh *mesh, double alpha, Direction direction);

/**
 * @brief Rotates a mesh.
 *
 * @param mesh The mesh to scale.
 * @param alpha The scalar to scale the mesh by.
 */
void scale_mesh(Mesh *mesh, double scale);

#endif /* mesh_H*/
