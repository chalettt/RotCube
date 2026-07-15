#include "geometry/point.h"

/**
 * @brief Structure to hold data of vertices.
 */
typedef struct vertex
{
    Point *position;
    Point *normal;
    Point *tc;
} Vertex;

/**
 * @brief Creates a vertex.
 *
 * @param position The position of the vertex in space.
 * @param normal The normal of the vertex.
 * @return The created vertex.
 */
Vertex *create_vertex(Point *position, Point *normal);

/**
 * @brief Destroys (frees) a vertex.
 *
 * @param vertex The vertex to destroy.
 */
void destroy_vertex(Vertex *vertex);
