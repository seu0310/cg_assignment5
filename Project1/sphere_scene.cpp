//
//  sphere_scene.c
//  Rasterizer
//
//

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include <cstdio>
#include <cmath>
#include <algorithm>

struct Vec3 {
    float x, y, z;
};

struct Vec2 {
    float x, y;
};

struct Color {
    unsigned char r, g, b;
};

int     gNumVertices    = 0;    // Number of 3D vertices.
int     gNumTriangles   = 0;    // Number of triangles.
int*    gIndexBuffer    = NULL; // Vertex indices for the triangles.
Vec3* gVertexBuffer   = NULL; // 3D Vertex postion (x, y, z)

const int SCREEN_WIDTH = 512;
const int SCREEN_HEIGHT = 512;
Color framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH]; // 출력 버퍼
float depthbuffer[SCREEN_HEIGHT][SCREEN_WIDTH]; // Z-buffer


float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}


void create_scene()
{
    int width = 32;
    int height = 16;

    float theta, phi;
    int t;

    gNumVertices = (height - 2) * width + 2;
    gNumTriangles = (height - 2) * (width - 1) * 2;

    // TODO: Allocate an array for gNumVertices vertices.

    gIndexBuffer = new int[3 * gNumTriangles];
    gVertexBuffer = new Vec3[gNumVertices];

    t = 0;
    for (int j = 1; j < height - 1; ++j)
    {
        for (int i = 0; i < width; ++i)
        {
            theta = (float)j / (height - 1) * M_PI;
            phi = (float)i / (width - 1) * M_PI * 2;

            float   x = sinf(theta) * cosf(phi);
            float   y = cosf(theta);
            float   z = -sinf(theta) * sinf(phi);

            // TODO: Set vertex t in the vertex array to {x, y, z}.

            gVertexBuffer[t] = { x, y, z };
            t++;
        }
    }

    // TODO: Set vertex t in the vertex array to {0, 1, 0}.

    gVertexBuffer[t] = { 0, 1, 0 };
    t++;

    // TODO: Set vertex t in the vertex array to {0, -1, 0}.

    gVertexBuffer[t] = { 0, -1, 0 };
    t++;

    t = 0;
    for (int j = 0; j < height - 3; ++j)
    {
        for (int i = 0; i < width - 1; ++i)
        {
            gIndexBuffer[t++] = j * width + i;
            gIndexBuffer[t++] = (j + 1) * width + (i + 1);
            gIndexBuffer[t++] = j * width + (i + 1);
            gIndexBuffer[t++] = j * width + i;
            gIndexBuffer[t++] = (j + 1) * width + i;
            gIndexBuffer[t++] = (j + 1) * width + (i + 1);
        }
    }
    for (int i = 0; i < width - 1; ++i)
    {
        gIndexBuffer[t++] = (height - 2) * width;
        gIndexBuffer[t++] = i;
        gIndexBuffer[t++] = i + 1;
        gIndexBuffer[t++] = (height - 2) * width + 1;
        gIndexBuffer[t++] = (height - 3) * width + (i + 1);
        gIndexBuffer[t++] = (height - 3) * width + i;
    }

    // The index buffer has now been generated. Here's how to use to determine
    // the vertices of a triangle. Suppose you want to determine the vertices
    // of triangle i, with 0 <= i < gNumTriangles. Define:
    //
    // k0 = gIndexBuffer[3*i + 0]
    // k1 = gIndexBuffer[3*i + 1]
    // k2 = gIndexBuffer[3*i + 2]
    //
    // Now, the vertices of triangle i are at positions k0, k1, and k2 (in that
    // order) in the vertex array (which you should allocate yourself at line
    // 27).
    //
    // Note that this assumes 0-based indexing of arrays (as used in C/C++,
    // Java, etc.) If your language uses 1-based indexing, you will have to
    // add 1 to k0, k1, and k2.
}

Vec3 modeling_transform(Vec3 v)
{
    v.x *= 2;
    v.y *= 2;
    v.z -= 7;

    return v;
}

Vec3 camera_transform(Vec3 v) {
    Vec3 e = { 0.0f, 0.0f, 0.0f };
    Vec3 u = { 1.0f, 0.0f, 0.0f };
    Vec3 v_dir = { 0.0f, 1.0f, 0.0f };
    Vec3 w = { 0.0f, 0.0f, 1.0f };

    Vec3 eye_to_point = { v.x - e.x, v.y - e.y, v.z - e.z };

    float x = dot(eye_to_point, u);
    float y = dot(eye_to_point, v_dir);
    float z = -dot(eye_to_point, w);

    return { x, y, z };
}

Vec3 perspective_projection(Vec3 v)
{
    float l = -0.1f, r = 0.1f;
    float b = -0.1f, t = 0.1f;
    float n = -0.1f, f = -1000.0f;

    float A = (2 * n) / (r - l);
    float B = (2 * n) / (t - b);
    float C = (r + l) / (r - l);
    float D = (t + b) / (t - b);
    float E = (f + n) / (n - f);
    float F = (2 * f * n) / (n - f);

    float x_proj = A * v.x + C * v.z;
    float y_proj = B * v.y + D * v.z;
    float z_proj = E * v.z + F;
    float w_proj = -v.z;

    return {
        x_proj / w_proj,
        y_proj / w_proj,
        z_proj / w_proj
    };
}

Vec2 viewport_transform(Vec3 v)
{
    float x_prime = (v.x + 1.0f) * 0.5f * SCREEN_WIDTH;
    float y_prime = (1.0f - (v.y + 1.0f) * 0.5f) * SCREEN_HEIGHT;

    return { x_prime, y_prime };
}

Vec2 world_to_screen(Vec3 v)
{
    v = modeling_transform(v);
    v = camera_transform(v);
    v = perspective_projection(v);
    return viewport_transform(v);
}

bool inside_triangle(Vec2 p, Vec2 a, Vec2 b, Vec2 c, float& alpha, float& beta, float& gamma)
{
    float denomBeta = (a.y - c.y) * b.x + (c.x - a.x) * b.y + a.x * c.y - c.x * a.y;
    float denomGamma = (a.y - b.y) * c.x + (b.x - a.x) * c.y + a.x * b.y - b.x * a.y;

    if (denomBeta == 0 || denomGamma == 0)
        return false;

    beta = ((a.y - c.y) * p.x + (c.x - a.x) * p.y + a.x * c.y - c.x * a.y) / denomBeta;
    gamma = ((a.y - b.y) * p.x + (b.x - a.x) * p.y + a.x * b.y - b.x * a.y) / denomGamma;
    alpha = 1.0f - beta - gamma;

    return (alpha >= 0.0f && beta >= 0.0f && gamma >= 0.0f &&
        alpha <= 1.0f && beta <= 1.0f && gamma <= 1.0f);
}

void draw_triangle(Vec3 v0, Vec3 v1, Vec3 v2, Color color)
{
    Vec2 a = world_to_screen(v0);
    Vec2 b = world_to_screen(v1);
    Vec2 c = world_to_screen(v2);

    int minX = std::max(0, (int)floorf(std::min({ a.x, b.x, c.x })));
    int maxX = std::min(SCREEN_WIDTH - 1, (int)ceilf(std::max({ a.x, b.x, c.x })));
    int minY = std::max(0, (int)floorf(std::min({ a.y, b.y, c.y })));
    int maxY = std::min(SCREEN_HEIGHT - 1, (int)ceilf(std::max({ a.y, b.y, c.y })));

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            float alpha, beta, gamma;

            if (inside_triangle({ (float)x, (float)y }, a, b, c, alpha, beta, gamma))
            {
                float z_ndc = alpha * v0.z + beta * v1.z + gamma * v2.z;
                float z_screen = z_ndc * 0.5f + 0.5f;

                if (z_screen < depthbuffer[y][x])
                {
                    framebuffer[y][x] = color;
                    depthbuffer[y][x] = z_screen;
                }
            }
        }
    }
}

void render_scene()
{
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            framebuffer[y][x] = { 0, 0, 0 };
            depthbuffer[y][x] = 1e9;
        }
    }

    for (int i = 0; i < gNumTriangles; ++i)
    {
        int k0 = gIndexBuffer[3 * i + 0];
        int k1 = gIndexBuffer[3 * i + 1];
        int k2 = gIndexBuffer[3 * i + 2];

        draw_triangle(
            gVertexBuffer[k0],
            gVertexBuffer[k1],
            gVertexBuffer[k2],
            { 255, 255, 255 }
        );
    }
}

void save_to_bmp(const char* filename)
{
    FILE* f;
    int filesize = 54 + 3 * SCREEN_WIDTH * SCREEN_HEIGHT;
    unsigned char bmpfileheader[14] = {
        'B','M', filesize, filesize >> 8, filesize >> 16, filesize >> 24,
        0,0, 0,0, 54,0,0,0
    };
    unsigned char bmpinfoheader[40] = {
        40,0,0,0,
        SCREEN_WIDTH, SCREEN_WIDTH >> 8, SCREEN_WIDTH >> 16, SCREEN_WIDTH >> 24,
        SCREEN_HEIGHT, SCREEN_HEIGHT >> 8, SCREEN_HEIGHT >> 16, SCREEN_HEIGHT >> 24,
        1,0,24,0
    };
    unsigned char pad[3] = { 0,0,0 };

    f = fopen(filename, "wb");
    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);

    for (int y = SCREEN_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            unsigned char color[3] = {
                framebuffer[y][x].b,
                framebuffer[y][x].g,
                framebuffer[y][x].r
            };
            fwrite(color, 1, 3, f);
        }
        fwrite(pad, 1, (4 - (SCREEN_WIDTH * 3) % 4) % 4, f);
    }

    fclose(f);
}

void delete_memory()
{
    delete gIndexBuffer;
    delete gVertexBuffer;
}


int main()
{
    create_scene();
    render_scene();
    save_to_bmp("result.bmp");
    delete_memory();

    return 0;
}