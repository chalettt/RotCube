#include "geometry/mesh.h"

#include <iso646.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "geometry/point.h"
#include "utils/debug.h"

Mesh *mesh = NULL;

static double *get_vertex(char *line, bool is_normal)
{
    double *vertex = malloc(3 * sizeof(double));

    size_t token_index = 0;
    char *token = strtok(line + 2 + (is_normal ? 1 : 0), " ");
    while (token && token_index < 3)
    {
        vertex[token_index] = strtod(token, NULL);
        token_index++;
        token = strtok(NULL, " ");
    }

    return vertex;
}

static size_t **get_face(char *line)
{
    size_t vertex_count = 4;
    size_t **face = malloc((vertex_count + 1) * sizeof(size_t *));

    size_t token_index = 0;
    char *line_save;
    char *token = strtok_r(line + 2, " ", &line_save);
    while (token)
    {
        if (token[0] < '0' || token[0] > '9')
            break;
        char *composants[3] = { 0 };
        if (token_index >= vertex_count)
        {
            vertex_count *= 2;
            face = realloc(face, (vertex_count + 1) * sizeof(size_t *));
        }

        face[token_index] = malloc(3 * sizeof(size_t));

        for (size_t i = 0; i < 3; i++)
        {
            composants[i] = token;
            if (i < 2)
                token = strchr(token, '/') + 1;
        }

        face[token_index][2] = strtol(composants[2], NULL, 10);
        face[token_index][0] = strtol(composants[0], NULL, 10);

        token_index++;

        token = strtok_r(NULL, " ", &line_save);
    }

    face[token_index] = 0;
    return face;
}

static size_t **triangulize(size_t **points)
{
    size_t vertex_count = 0;
    while (points[vertex_count])
        vertex_count++;
    size_t triangle_count = vertex_count - 2;
    size_t **triangles = malloc((triangle_count + 1) * sizeof(size_t *));
    triangles[triangle_count] = 0;

    for (size_t i = 0; i < triangle_count; i++)
    {
        triangles[i] = malloc(3 * sizeof(size_t));
        triangles[i][0] = points[0][0] - 1;
        triangles[i][1] = points[i + 1][0] - 1;
        triangles[i][2] = points[i + 2][0] - 1;
    }

    return triangles;
}

Mesh *load_mesh(char *path, Point *origin)
{
    FILE *obj = fopen(path, "r");
    if (!obj)
    {
        ERROR("Could not open file: %s", path);
        return NULL;
    }

    LOG("File opened, parsing commencing with file: %s\n", path);

    size_t buffer_size = BUFFER_SIZE;
    size_t ***faces = malloc((buffer_size + 1) * sizeof(size_t **));
    double **vertices = malloc((buffer_size + 1) * sizeof(double *));
    double **normals = malloc((buffer_size + 1) * sizeof(double *));

    vertices[0] = 0;

    char *line = NULL;
    size_t size = 0;

    size_t vertex_count = 0;
    size_t face_count = 0;
    size_t normal_count = 0;
    ssize_t nread;
    while ((nread = getline(&line, &size, obj)) != -1)
    {
        if (vertex_count >= buffer_size || face_count >= buffer_size
            || normal_count >= buffer_size)
        {
            buffer_size *= 2;
            faces = realloc(faces, (buffer_size + 1) * sizeof(size_t **));
            vertices = realloc(vertices, (buffer_size + 1) * sizeof(double *));
            normals = realloc(normals, (buffer_size + 1) * sizeof(double *));
        }
        if (line[0] == 'v')
        {
            if (line[1] == 'n')
            {
                normals[normal_count++] = get_vertex(line, true);
                normals[normal_count] = 0;
            }
            if (line[1] == ' ')
            {
                vertices[vertex_count++] = get_vertex(line, false);
                vertices[vertex_count] = 0;
            }
        }
        else if (line[0] == 'f')
        {
            faces[face_count++] = get_face(line);
            faces[face_count] = 0;
        }
        else
            continue;
    }

    free(line);

    fclose(obj);

    mesh = malloc(sizeof(Mesh));
    if (!mesh)
    {
        ERROR("mesh allocation failed.");
        return NULL;
    }

    mesh->origin = origin;
    mesh->vertex_count = vertex_count;

    size_t triangle_count = face_count;
    size_t triangle_index = 0;
    mesh->triangles = malloc((triangle_count + 1) * sizeof(Triangle *));
    mesh->vertices = malloc((vertex_count + 1) * sizeof(Point *));
    mesh->vertices[vertex_count] = 0;
    for (size_t i = 0; vertices[i]; i++)
    {
        double *vertex = vertices[i];
        Point *vertex_position = create_point(vertex[0], vertex[1], vertex[2]);
        add_point(vertex_position, origin);
        mesh->vertices[i] = create_vertex(vertex_position, NULL);
    }
    for (size_t i = 0; faces[i]; i++)
    {
        for (size_t j = 0; faces[i][j]; j++)
        {
            size_t v_idx = faces[i][j][0] - 1;
            size_t n_idx = faces[i][j][2] - 1;
            Vertex *vertex = mesh->vertices[v_idx];
            if (vertex->normal)
                free(vertex->normal);
            Point *normal = create_point(normals[n_idx][0], normals[n_idx][1],
                                         normals[n_idx][2]);
            vertex->normal = normal;
        }
        size_t **triangles = triangulize(faces[i]);
        for (size_t j = 0; triangles[j]; j++)
        {
            if (triangle_index >= triangle_count)
            {
                triangle_count *= 2;
                mesh->triangles = realloc(
                    mesh->triangles, (triangle_count + 1) * sizeof(Triangle *));
            }
            mesh->triangles[triangle_index++] = create_triangle(triangles[j]);
            for (size_t k = 0; k < 3; k++)
            {
                mesh->triangles[triangle_index - 1]->vertex_normals[k] =
                    dup_point(mesh->vertices[triangles[j][k]]->normal);
            }
            free(triangles[j]);
            mesh->triangles[triangle_index] = 0;
        }
        free(triangles);
    }

    mesh->triangle_count = triangle_index + 1;

    for (size_t i = 0; faces[i]; i++)
    {
        for (size_t j = 0; faces[i][j]; j++)
            free(faces[i][j]);
        free(faces[i]);
    }
    for (size_t i = 0; vertices[i]; i++)
        free(vertices[i]);

    for (size_t i = 0; normals[i]; i++)
        free(normals[i]);

    free(faces);
    free(vertices);
    free(normals);
    return mesh;
}

void rotate_mesh(Mesh *mesh, double alpha, Direction direction)
{
    Point world_origin = { 0, 0, 0 };
    for (size_t i = 0; mesh->vertices[i]; i++)
    {
        Point *point = mesh->vertices[i]->position;
        switch (direction)
        {
        case LEFT:
            rotate_point_y(point, mesh->origin, alpha);
            break;
        case RIGHT:
            rotate_point_y(point, mesh->origin, -alpha);
            break;
        case FRONT:
            rotate_point_x(point, mesh->origin, alpha);
            break;
        case BACK:
            rotate_point_x(point, mesh->origin, -alpha);
            break;
        case UP:
            rotate_point_z(point, mesh->origin, alpha);
            break;
        case DOWN:
            rotate_point_z(point, mesh->origin, -alpha);
            break;
        }
    }
    for (size_t i = 0; mesh->triangles[i]; i++)
    {
        Triangle *triangle = mesh->triangles[i];
        free(triangle->normal);
        triangle->normal = get_triangle_normal(triangle);
        for (size_t i = 0; i < 3; i++)
        {
            Point *normal = triangle->vertex_normals[i];
            switch (direction)
            {
            case LEFT:
                rotate_point_y(normal, &world_origin, alpha);
                break;
            case RIGHT:
                rotate_point_y(normal, &world_origin, -alpha);
                break;
            case FRONT:
                rotate_point_x(normal, &world_origin, alpha);
                break;
            case BACK:
                rotate_point_x(normal, &world_origin, -alpha);
                break;
            case UP:
                rotate_point_z(normal, &world_origin, alpha);
                break;
            case DOWN:
                rotate_point_z(normal, &world_origin, -alpha);
                break;
            }
        }
    }
}

void scale_mesh(Mesh *mesh, double scale)
{
    for (size_t i = 0; mesh->vertices[i]; i++)
    {
        Point *p = mesh->vertices[i]->position;
        sub_point(p, mesh->origin);
        scalar_product(mesh->vertices[i]->position, scale);
        add_point(p, mesh->origin);
    }
}

void destroy_mesh(Mesh *mesh)
{
    for (size_t i = 0; mesh->triangles[i]; i++)
        destroy_triangle(mesh->triangles[i]);

    for (size_t i = 0; mesh->vertices[i]; i++)
        destroy_vertex(mesh->vertices[i]);

    free(mesh->triangles);
    free(mesh->vertices);
    free(mesh->origin);
    free(mesh);
}
