/* paul/paul_random.h -- https://github.com/takeiteasy/paul

 Copyright (C) 2025 George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

 /*!
 @header paul_random.h
 @copyright George Watson GPLv3
 @updated 2025-07-19
 @abstract Random number generation and noise functions for C/C++.
 @discussion
    Implementation is included when PAUL_RANDOM_IMPLEMENTATION or PAUL_IMPLEMENTATION is defined.
 */

#ifndef PAUL_RANDOM_HEAD
#define PAUL_RANDOM_HEAD
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <float.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#define _PRNG_RAND_SSIZE ((UINT16_C(1))<<6)

/*!
 @typedef rng_t
 @abstract Random number generator state structure.
 @field s State array for the PRNG.
 @field i Current index in the state array.
 @field c Counter for refill cycles.
 */
typedef struct rng {
    uint64_t s[_PRNG_RAND_SSIZE];
    uint_fast16_t i;
    uint_fast16_t c;
} rng_t;

/*!
 @function rng_init
 @abstract Initializes the random number generator with a seed.
 @param rng Pointer to the rng_t structure to initialize.
 @param seed The seed value for the generator.
 */
void rng_init(rng_t *rng, uint64_t seed);

/*!
 @function rnd_randi
 @abstract Generates a random 64-bit unsigned integer.
 @param rng Pointer to the initialized rng_t structure.
 @return A random uint64_t value.
 */
uint64_t rnd_randi(rng_t *rng);

/*!
 @function rnd_randf
 @abstract Generates a random float between 0.0 and 1.0.
 @param rng Pointer to the initialized rng_t structure.
 @return A random float value in [0.0, 1.0).
 */
float rnd_randf(rng_t *rng);

/*!
 @function rnd_randi_range
 @abstract Generates a random integer within a specified range.
 @param rng Pointer to the initialized rng_t structure.
 @param min The minimum value (inclusive).
 @param max The maximum value (inclusive).
 @return A random int value between min and max.
 */
int rnd_randi_range(rng_t *rng, int min, int max);

/*!
 @function rnd_randf_range
 @abstract Generates a random float within a specified range.
 @param rng Pointer to the initialized rng_t structure.
 @param min The minimum value (inclusive).
 @param max The maximum value (inclusive).
 @return A random float value between min and max.
 */
float rnd_randf_range(rng_t *rng, float min, float max);

/*!
 @function cellular_automata
 @abstract Generates a cellular automata pattern.
 @param rng Pointer to the initialized rng_t structure.
 @param width Width of the grid.
 @param height Height of the grid.
 @param fill_chance Percentage chance to fill a cell initially (1-99).
 @param smooth_iterations Number of smoothing iterations.
 @param survive Minimum neighbors to survive.
 @param starve Maximum neighbors to starve.
 @param dst Output buffer for the grid (uint8_t array).
 */
void cellular_automata(rng_t *rng,
                       unsigned int width, unsigned int height,
                       unsigned int fill_chance, unsigned int smooth_iterations,
                       unsigned int survive, unsigned int starve,
                       uint8_t* dst);

/*!
 @typedef noise_fn_t
 @abstract Function pointer type for noise functions.
 @param x X coordinate.
 @param y Y coordinate.
 @param z Z coordinate.
 @return Noise value at the given coordinates.
 */
typedef float(*noise_fn_t)(float, float, float);

/*!
 @function perlin_noise
 @abstract Computes Perlin noise at the given coordinates.
 @param x X coordinate.
 @param y Y coordinate.
 @param z Z coordinate.
 @return Perlin noise value in [-1, 1].
 */
float perlin_noise(float x, float y, float z);

/*!
 @function simplex_noise
 @abstract Computes Simplex noise at the given coordinates.
 @param x X coordinate.
 @param y Y coordinate.
 @param z Z coordinate.
 @return Simplex noise value.
 */
float simplex_noise(float x, float y, float z);

/*!
 @function worley_noise
 @abstract Computes Worley (cellular) noise at the given coordinates.
 @param x X coordinate.
 @param y Y coordinate.
 @param z Z coordinate.
 @return Worley noise value.
 */
float worley_noise(float x, float y, float z);

/*!
 @function value_noise
 @abstract Computes value noise at the given coordinates.
 @param x X coordinate.
 @param y Y coordinate.
 @param z Z coordinate.
 @return Value noise value in [-1, 1].
 */
float value_noise(float x, float y, float z);

/*!
 @function white_noise
 @abstract Computes white noise at the given coordinates.
 @param x X coordinate.
 @param y Y coordinate.
 @param z Z coordinate.
 @return White noise value in [-1, 1].
 */
float white_noise(float x, float y, float z);

/*!
 @function fbm
 @abstract Computes fractal Brownian motion noise.
 @param x X coordinate.
 @param y Y coordinate.
 @param z Z coordinate.
 @param lacunarity Frequency multiplier between octaves.
 @param gain Amplitude multiplier between octaves.
 @param octaves Number of octaves.
 @param noise_fn Base noise function.
 @return FBM noise value.
 */
float fbm(float x, float y, float z,
          float lacunarity, float gain, int octaves,
          noise_fn_t noise_fn);

/*!
 @function fbm2D
 @abstract Generates a 2D fractal Brownian motion noise map.
 @param width Width of the output map.
 @param height Height of the output map.
 @param z Z coordinate (fixed for 2D slice).
 @param offset_x X offset.
 @param offset_y Y offset.
 @param scale Scale factor.
 @param lacunarity Frequency multiplier.
 @param gain Amplitude multiplier.
 @param octaves Number of octaves.
 @param noise_fn Base noise function.
 @param dst Output buffer (uint8_t array).
 */
void fbm2D(unsigned int width, unsigned int height, float z,
           float offset_x, float offset_y,
           float scale, float lacunarity, float gain, int octaves,
           noise_fn_t noise_fn, uint8_t* dst);

/*!
 @function fbm3D
 @abstract Generates a 3D fractal Brownian motion noise volume.
 @param width Width of the output volume.
 @param height Height of the output volume.
 @param depth Depth of the output volume.
 @param offset_x X offset.
 @param offset_y Y offset.
 @param offset_z Z offset.
 @param scale Scale factor.
 @param lacunarity Frequency multiplier.
 @param gain Amplitude multiplier.
 @param octaves Number of octaves.
 @param noise_fn Base noise function.
 @param dst Output buffer (uint8_t array).
 */
void fbm3D(unsigned int width, unsigned int height, unsigned int depth,
           float offset_x, float offset_y, float offset_z,
           float scale, float lacunarity, float gain, int octaves,
           noise_fn_t noise_fn, uint8_t* dst);

/*!
 @typedef poisson_point_t
 @abstract Structure representing a 2D point for Poisson disc sampling.
 @field x X coordinate of the point.
 @field y Y coordinate of the point.
 */
typedef struct poisson_point {
    float x;
    float y;
} poisson_point_t;

/*!
 @typedef poisson_list_t
 @abstract Structure for storing a list of Poisson disc sampling points.
 @field points Array of points.
 @field count Number of points in the list.
 @field capacity Capacity of the points array.
 */
typedef struct poisson_list {
    poisson_point_t *points;
    size_t count;
    size_t capacity;
} poisson_list_t;

/*!
 @typedef poisson_check_callback_t
 @abstract Callback function type for checking if a point is valid.
 @param x X coordinate of the point.
 @param y Y coordinate of the point.
 @param user_data User-provided data.
 @return Non-zero if the point is valid, zero otherwise.
 */
typedef int (*poisson_check_callback_t)(float x, float y, void *user_data);

/*!
 @typedef poisson_callback_t
 @abstract Callback function type for processing each generated point.
 @param x X coordinate of the point.
 @param y Y coordinate of the point.
 @param user_data User-provided data.
 */
typedef void (*poisson_callback_t)(float x, float y, void *user_data);

/*!
 @function poisson_disc_sample_list
 @abstract Generates Poisson disc sampling points and returns them in a list.
 @param width Width of the sampling area.
 @param height Height of the sampling area.
 @param min_dist Minimum distance between points.
 @param max_attempts Maximum attempts to place a point around each active point.
 @param check_fn Optional callback to validate points (NULL to accept all).
 @param user_data User data passed to the check callback.
 @return Allocated list of points (must be freed with poisson_disc_free_list).
 */
poisson_list_t *poisson_disc_sample_list(float width, float height, float min_dist,
                                    int max_attempts, poisson_check_callback_t check_fn,
                                    void *user_data);

/*!
 @function poisson_disc_free_list
 @abstract Frees a Poisson disc sampling point list.
 @param list The list to free.
 */
void poisson_disc_free_list(poisson_list_t *list);

/*!
 @function poisson_disc_sample_foreach
 @abstract Generates Poisson disc sampling points and calls a callback for each.
 @param width Width of the sampling area.
 @param height Height of the sampling area.
 @param min_dist Minimum distance between points.
 @param max_attempts Maximum attempts to place a point around each active point.
 @param check_fn Optional callback to validate points (NULL to accept all).
 @param check_data User data passed to the check callback.
 @param point_fn Callback called for each generated point.
 @param point_data User data passed to the point callback.
 */
void poisson_disc_sample_foreach(float width, float height, float min_dist,
                                 int max_attempts, poisson_check_callback_t check_fn,
                                 void *check_data, poisson_callback_t point_fn,
                                 void *point_data);

#ifdef __cplusplus
}
#endif
#endif // PAUL_RANDOM_HEAD

#if defined(PAUL_RANDOM_IMPLEMENTATION) || defined(PAUL_IMPLEMENTATION)
#define _PRNG_LAG1 (UINT16_C(24))
#define _PRNG_LAG2 (UINT16_C(55))
#define _PRNG_RAND_SMASK (_PRNG_RAND_SSIZE-1)
#define _PRNG_RAND_EXHAUST_LIMIT _PRNG_LAG2
// 10x is a heuristic, it just needs to be large enough to remove correlation
#define _PRNG_RAND_REFILL_COUNT ((_PRNG_LAG2*10)-_PRNG_RAND_EXHAUST_LIMIT)

#define PRNG_RAND_MAX UINT64_MAX

#define _MAX(A, B) ((A) > (B) ? (A) : (B))
#define _CLAMP(X, LO, HI) ((X) < (LO) ? (LO) : ((X) > (HI) ? (HI) : (X)))
#define _REMAP(X, IN_MIN, IN_MAX, OUT_MIN, OUT_MAX) ((OUT_MIN) + (((X) - (IN_MIN)) * ((OUT_MAX) - (OUT_MIN)) / ((IN_MAX) - (IN_MIN))))

void rnd_seed(rng_t *rng, uint64_t seed) {
    if (!seed)
        seed = (uint64_t)time(NULL);
    uint_fast16_t i;
    // Naive seed
    rng->c = _PRNG_RAND_EXHAUST_LIMIT;
    rng->i = 0;
    rng->s[0] = seed;
    // Arbitrary magic, mostly to eliminate the effect of low-value seeds.
    // Probably could be better, but the run-up obviates any real need to.
    for(i=1; i<_PRNG_RAND_SSIZE; i++)
        rng->s[i] = i*(UINT64_C(2147483647)) + seed;
    // Run forward 10,000 numbers
    for(i=0; i<10000; i++)
        (void)rng_randi(rng);
}

uint64_t rng_randi(rng_t *rng) {
    uint_fast16_t i = 0;
    uint_fast16_t r, new_rands=0;

    if( !rng->c ) { // Randomness exhausted, run forward to refill
        new_rands += _PRNG_RAND_REFILL_COUNT+1;
        rng->c = _PRNG_RAND_EXHAUST_LIMIT-1;
    } else {
        new_rands = 1;
        rng->c--;
    }

    for( r=0; r<new_rands; r++ ) {
        i = rng->i;
        rng->s[i&_PRNG_RAND_SMASK] =
        rng->s[(i+_PRNG_RAND_SSIZE-_PRNG_LAG1)&_PRNG_RAND_SMASK] +
        rng->s[(i+_PRNG_RAND_SSIZE-_PRNG_LAG2)&_PRNG_RAND_SMASK];
        rng->i++;
    }
    return rng->s[i&_PRNG_RAND_SMASK];
}

float rng_randf(rng_t *rng) {
    return (float)rng_randi(rng) / (float)PRNG_RAND_MAX;
}

int rnd_randi_range(rng_t *rng, int min, int max) {
    return _REMAP(rng_randi(rng), 0, PRNG_RAND_MAX, min, max);
}

float rnd_randf_range(rng_t *rng, float min, float max) {
    return _REMAP(rng_randf(rng), 0.f, 1.f, min, max);
}

void cellular_automata(rng_t *rng, unsigned int width, unsigned int height, unsigned int fill_chance, unsigned int smooth_iterations, unsigned int survive, unsigned int starve, uint8_t* result) {
    memset(result, 0, width * height * sizeof(int));
    // Randomly fill the grid
    fill_chance = _CLAMP(fill_chance, 1, 99);
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            result[y * width + x] = (rng_randf(rng) * 99) + 1 < fill_chance;
    // Run cellular-automata on grid n times
    for (int i = 0; i < _MAX(smooth_iterations, 1); i++)
        for (int x = 0; x < width; x++)
            for (int y = 0; y < height; y++) {
                // Count the cell's living neighbours
                int neighbours = 0;
                for (int nx = x - 1; nx <= x + 1; nx++)
                    for (int ny = y - 1; ny <= y + 1; ny++)
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            if ((nx != x || ny != y) && result[ny * width + nx] > 0)
                                neighbours++;
                        } else
                            neighbours++;
                // Update cell based on neighbour and surive/starve values
                if (neighbours > survive)
                    result[y * width + x] = 1;
                else if (neighbours < starve)
                    result[y * width + x] = 0;
            }
}

static const float grad3[][3] = {
    { 1, 1, 0 }, { -1, 1, 0 }, { 1, -1, 0 }, { -1, -1, 0 },
    { 1, 0, 1 }, { -1, 0, 1 }, { 1, 0, -1 }, { -1, 0, -1 },
    { 0, 1, 1 }, { 0, -1, 1 }, { 0, 1, -1 }, { 0, -1, -1 }
};

static const unsigned int perm[] = {
    182, 232, 51, 15, 55, 119, 7, 107, 230, 227, 6, 34, 216, 61, 183, 36,
    40, 134, 74, 45, 157, 78, 81, 114, 145, 9, 209, 189, 147, 58, 126, 0,
    240, 169, 228, 235, 67, 198, 72, 64, 88, 98, 129, 194, 99, 71, 30, 127,
    18, 150, 155, 179, 132, 62, 116, 200, 251, 178, 32, 140, 130, 139, 250, 26,
    151, 203, 106, 123, 53, 255, 75, 254, 86, 234, 223, 19, 199, 244, 241, 1,
    172, 70, 24, 97, 196, 10, 90, 246, 252, 68, 84, 161, 236, 205, 80, 91,
    233, 225, 164, 217, 239, 220, 20, 46, 204, 35, 31, 175, 154, 17, 133, 117,
    73, 224, 125, 65, 77, 173, 3, 2, 242, 221, 120, 218, 56, 190, 166, 11,
    138, 208, 231, 50, 135, 109, 213, 187, 152, 201, 47, 168, 185, 186, 167, 165,
    102, 153, 156, 49, 202, 69, 195, 92, 21, 229, 63, 104, 197, 136, 148, 94,
    171, 93, 59, 149, 23, 144, 160, 57, 76, 141, 96, 158, 163, 219, 237, 113,
    206, 181, 112, 111, 191, 137, 207, 215, 13, 83, 238, 249, 100, 131, 118, 243,
    162, 248, 43, 66, 226, 27, 211, 95, 214, 105, 108, 101, 170, 128, 210, 87,
    38, 44, 174, 188, 176, 39, 14, 143, 159, 16, 124, 222, 33, 247, 37, 245,
    8, 4, 22, 82, 110, 180, 184, 12, 25, 5, 193, 41, 85, 177, 192, 253,
    79, 29, 115, 103, 142, 146, 52, 48, 89, 54, 121, 212, 122, 60, 28, 42,

    182, 232, 51, 15, 55, 119, 7, 107, 230, 227, 6, 34, 216, 61, 183, 36,
    40, 134, 74, 45, 157, 78, 81, 114, 145, 9, 209, 189, 147, 58, 126, 0,
    240, 169, 228, 235, 67, 198, 72, 64, 88, 98, 129, 194, 99, 71, 30, 127,
    18, 150, 155, 179, 132, 62, 116, 200, 251, 178, 32, 140, 130, 139, 250, 26,
    151, 203, 106, 123, 53, 255, 75, 254, 86, 234, 223, 19, 199, 244, 241, 1,
    172, 70, 24, 97, 196, 10, 90, 246, 252, 68, 84, 161, 236, 205, 80, 91,
    233, 225, 164, 217, 239, 220, 20, 46, 204, 35, 31, 175, 154, 17, 133, 117,
    73, 224, 125, 65, 77, 173, 3, 2, 242, 221, 120, 218, 56, 190, 166, 11,
    138, 208, 231, 50, 135, 109, 213, 187, 152, 201, 47, 168, 185, 186, 167, 165,
    102, 153, 156, 49, 202, 69, 195, 92, 21, 229, 63, 104, 197, 136, 148, 94,
    171, 93, 59, 149, 23, 144, 160, 57, 76, 141, 96, 158, 163, 219, 237, 113,
    206, 181, 112, 111, 191, 137, 207, 215, 13, 83, 238, 249, 100, 131, 118, 243,
    162, 248, 43, 66, 226, 27, 211, 95, 214, 105, 108, 101, 170, 128, 210, 87,
    38, 44, 174, 188, 176, 39, 14, 143, 159, 16, 124, 222, 33, 247, 37, 245,
    8, 4, 22, 82, 110, 180, 184, 12, 25, 5, 193, 41, 85, 177, 192, 253,
    79, 29, 115, 103, 142, 146, 52, 48, 89, 54, 121, 212, 122, 60, 28, 42
};

static float dot3(const float a[], float x, float y, float z) {
    return a[0]*x + a[1]*y + a[2]*z;
}

static float lerp(float a, float b, float t) {
    return (1 - t) * a + t * b;
}

static float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

#define FASTFLOOR(x) (((x) >= 0) ? (int)(x) : (int)(x)-1)

/* 3D Perlin noise */
float perlin(float x, float y, float z) {
    /* Find grid points */
    int gx = FASTFLOOR(x);
    int gy = FASTFLOOR(y);
    int gz = FASTFLOOR(z);
    /* Relative coords within grid cell */
    float rx = x - gx;
    float ry = y - gy;
    float rz = z - gz;
    /* Wrap cell coords */
    gx = gx & 255;
    gy = gy & 255;
    gz = gz & 255;
    /* Calculate gradient indices */
    unsigned int gi[8];
    for (int i = 0; i < 8; i++)
        gi[i] = perm[gx+((i>>2)&1)+perm[gy+((i>>1)&1)+perm[gz+(i&1)]]] % 12;
    /* Noise contribution from each corner */
    float n[8];
    for (int i = 0; i < 8; i++)
        n[i] = dot3(grad3[gi[i]], rx - ((i>>2)&1), ry - ((i>>1)&1), rz - (i&1));
    /* Fade curves */
    float u = fade(rx);
    float v = fade(ry);
    float w = fade(rz);
    /* Interpolate */
    float nx[4];
    for (int i = 0; i < 4; i++)
        nx[i] = lerp(n[i], n[4+i], u);
    float nxy[2];
    for (int i = 0; i < 2; i++)
        nxy[i] = lerp(nx[i], nx[2+i], v);
    return lerp(nxy[0], nxy[1], w);
}

/* Hash function for noise generation */
static inline uint32_t hash(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

static inline uint32_t hash3d(int x, int y, int z) {
    return hash(x + hash(y + hash(z)));
}

/* Random float from hash (0 to 1) */
static inline float hash_to_float(uint32_t h) {
    return (h & 0xFFFFFF) / 16777216.0f;
}

/* White noise - completely random */
float white_noise(float x, float y, float z) {
    int ix = FASTFLOOR(x);
    int iy = FASTFLOOR(y);
    int iz = FASTFLOOR(z);
    uint32_t h = hash3d(ix, iy, iz);
    return hash_to_float(h) * 2.0f - 1.0f;
}

/* Value noise - smoother than white noise, interpolates between random values */
float value_noise(float x, float y, float z) {
    int x0 = FASTFLOOR(x), x1 = x0 + 1;
    int y0 = FASTFLOOR(y), y1 = y0 + 1;
    int z0 = FASTFLOOR(z), z1 = z0 + 1;
    
    float fx = x - x0, fy = y - y0, fz = z - z0;
    float u = fade(fx), v = fade(fy), w = fade(fz);
    
    /* Get random values at corners */
    float n000 = hash_to_float(hash3d(x0, y0, z0)) * 2.0f - 1.0f;
    float n001 = hash_to_float(hash3d(x0, y0, z1)) * 2.0f - 1.0f;
    float n010 = hash_to_float(hash3d(x0, y1, z0)) * 2.0f - 1.0f;
    float n011 = hash_to_float(hash3d(x0, y1, z1)) * 2.0f - 1.0f;
    float n100 = hash_to_float(hash3d(x1, y0, z0)) * 2.0f - 1.0f;
    float n101 = hash_to_float(hash3d(x1, y0, z1)) * 2.0f - 1.0f;
    float n110 = hash_to_float(hash3d(x1, y1, z0)) * 2.0f - 1.0f;
    float n111 = hash_to_float(hash3d(x1, y1, z1)) * 2.0f - 1.0f;
    
    /* Interpolate */
    float nx00 = lerp(n000, n100, u);
    float nx01 = lerp(n001, n101, u);
    float nx10 = lerp(n010, n110, u);
    float nx11 = lerp(n011, n111, u);
    
    float nxy0 = lerp(nx00, nx10, v);
    float nxy1 = lerp(nx01, nx11, v);
    
    return lerp(nxy0, nxy1, w);
}

/* Simplex noise - faster alternative to Perlin with no directional artifacts */
float simplex_noise(float x, float y, float z) {
    const float F3 = 1.0f / 3.0f;
    const float G3 = 1.0f / 6.0f;
    
    /* Skew input space */
    float s = (x + y + z) * F3;
    int i = FASTFLOOR(x + s);
    int j = FASTFLOOR(y + s);
    int k = FASTFLOOR(z + s);
    
    float t = (i + j + k) * G3;
    float X0 = i - t, Y0 = j - t, Z0 = k - t;
    float x0 = x - X0, y0 = y - Y0, z0 = z - Z0;
    
    /* Determine which simplex */
    int i1, j1, k1, i2, j2, k2;
    if (x0 >= y0) {
        if (y0 >= z0) { i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; }
        else if (x0 >= z0) { i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; }
        else { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; }
    } else {
        if (y0 < z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; }
        else if (x0 < z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; }
        else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; }
    }
    
    float x1 = x0 - i1 + G3, y1 = y0 - j1 + G3, z1 = z0 - k1 + G3;
    float x2 = x0 - i2 + 2.0f*G3, y2 = y0 - j2 + 2.0f*G3, z2 = z0 - k2 + 2.0f*G3;
    float x3 = x0 - 1.0f + 3.0f*G3, y3 = y0 - 1.0f + 3.0f*G3, z3 = z0 - 1.0f + 3.0f*G3;
    
    /* Calculate contributions */
    float n0, n1, n2, n3;
    
    float t0 = 0.6f - x0*x0 - y0*y0 - z0*z0;
    if (t0 < 0) n0 = 0.0f;
    else {
        int gi0 = perm[(i + perm[(j + perm[k & 255]) & 255]) & 255] % 12;
        t0 *= t0;
        n0 = t0 * t0 * dot3(grad3[gi0], x0, y0, z0);
    }
    
    float t1 = 0.6f - x1*x1 - y1*y1 - z1*z1;
    if (t1 < 0) n1 = 0.0f;
    else {
        int gi1 = perm[(i+i1 + perm[(j+j1 + perm[(k+k1) & 255]) & 255]) & 255] % 12;
        t1 *= t1;
        n1 = t1 * t1 * dot3(grad3[gi1], x1, y1, z1);
    }
    
    float t2 = 0.6f - x2*x2 - y2*y2 - z2*z2;
    if (t2 < 0) n2 = 0.0f;
    else {
        int gi2 = perm[(i+i2 + perm[(j+j2 + perm[(k+k2) & 255]) & 255]) & 255] % 12;
        t2 *= t2;
        n2 = t2 * t2 * dot3(grad3[gi2], x2, y2, z2);
    }
    
    float t3 = 0.6f - x3*x3 - y3*y3 - z3*z3;
    if (t3 < 0) n3 = 0.0f;
    else {
        int gi3 = perm[(i+1 + perm[(j+1 + perm[(k+1) & 255]) & 255]) & 255] % 12;
        t3 *= t3;
        n3 = t3 * t3 * dot3(grad3[gi3], x3, y3, z3);
    }
    
    return 32.0f * (n0 + n1 + n2 + n3);
}

/* Worley/Cellular noise - creates cell-like patterns */
float worley_noise(float x, float y, float z) {
    int xi = FASTFLOOR(x);
    int yi = FASTFLOOR(y);
    int zi = FASTFLOOR(z);
    
    float min_dist = FLT_MAX;
    
    /* Check neighboring cells */
    for (int oz = -1; oz <= 1; oz++) {
        for (int oy = -1; oy <= 1; oy++) {
            for (int ox = -1; ox <= 1; ox++) {
                int cx = xi + ox;
                int cy = yi + oy;
                int cz = zi + oz;
                
                /* Get random point in cell */
                uint32_t h = hash3d(cx, cy, cz);
                float px = cx + hash_to_float(h);
                float py = cy + hash_to_float(hash(h));
                float pz = cz + hash_to_float(hash(hash(h)));
                
                /* Calculate distance */
                float dx = x - px;
                float dy = y - py;
                float dz = z - pz;
                float dist = sqrtf(dx*dx + dy*dy + dz*dz);
                
                if (dist < min_dist)
                    min_dist = dist;
            }
        }
    }
    
    return 1.0f - min_dist;
}

float fbm(float x, float y, float z, float lacunarity, float gain, int octaves, noise_fn_t noise_fn) {
    float freq = 1.f;
    float amp = 1.f;
    float sum = 0.f;
    float tot = 0.f;
    
    for (int i = 0; i < octaves; i++) {
        sum += noise_fn(x * freq, y * freq, z * freq) * amp;
        tot += amp;
        freq *= lacunarity;
        amp *= gain;
    }
    
    return sum / tot;
}

void fbm2D(unsigned int width, unsigned int height, float z, 
           float offsetX, float offsetY, float scale, 
           float lacunarity, float gain, int octaves,
           noise_fn_t noise_fn, uint8_t* result) {
    float min = FLT_MAX, max = FLT_MIN;
    float *grid = (float*)malloc(width * height * sizeof(float));
    
    /* Generate noise values */
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++) {
            float nx = (offsetX + x) / scale;
            float ny = (offsetY + y) / scale;
            float nz = z;

            float val = fbm(nx, ny, nz, lacunarity, gain, octaves, noise_fn);
            grid[y * width + x] = val;

            if (val < min)
                min = val;
            if (val > max)
                max = val;
        }
    
    /* Normalize to 0-255 range */
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++) {
            float normalized = _REMAP(grid[y * width + x], min, max, 0.f, 1.f);
            result[y * width + x] = (uint8_t)(normalized * 255.f);
        }
    
    free(grid);
}

void fbm3D(unsigned int width, unsigned int height, unsigned int depth,
           float offsetX, float offsetY, float offsetZ, float scale,
           float lacunarity, float gain, int octaves,
           noise_fn_t noise_fn, uint8_t* result) {
    float min = FLT_MAX, max = FLT_MIN;
    float *grid = (float*)malloc(width * height * depth * sizeof(float));
    
    /* Generate noise values */
    for (int z = 0; z < depth; z++)
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++) {
                float nx = (offsetX + x) / scale;
                float ny = (offsetY + y) / scale;
                float nz = (offsetZ + z) / scale;

                float val = fbm(nx, ny, nz, lacunarity, gain, octaves, noise_fn);
                grid[z * width * height + y * width + x] = val;

                if (val < min)
                    min = val;
                if (val > max)
                    max = val;
            }
    
    /* Normalize to 0-255 range */
    for (int z = 0; z < depth; z++)
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++) {
                int idx = z * width * height + y * width + x;
                float normalized = _REMAP(grid[idx], min, max, 0.f, 1.f);
                result[idx] = (uint8_t)(normalized * 255.f);
            }
    
    free(grid);
}

typedef struct poisson_grid {
    int *cells;
    int cols;
    int rows;
    float cell_size;
} _poisson_grid_t;

typedef struct poisson_active {
    poisson_point_t *points;
    size_t count;
    size_t capacity;
} _poisson_active_t;

static void active_list_init(_poisson_active_t *list) {
    list->capacity = 32;
    list->count = 0;
    list->points = (poisson_point_t*)malloc(sizeof(poisson_point_t) * list->capacity);
}

static void active_list_free(_poisson_active_t *list) {
    free(list->points);
}

static void active_list_add(_poisson_active_t *list, poisson_point_t p) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->points = (poisson_point_t*)realloc(list->points, sizeof(poisson_point_t) * list->capacity);
    }
    list->points[list->count++] = p;
}

static void active_list_remove(_poisson_active_t *list, size_t idx) {
    if (idx < list->count - 1)
        list->points[idx] = list->points[list->count - 1];
    list->count--;
}

static void grid_init(_poisson_grid_t *grid, float width, float height, float cell_size) {
    grid->cell_size = cell_size;
    grid->cols = (int)ceilf(width / cell_size);
    grid->rows = (int)ceilf(height / cell_size);
    grid->cells = (int*)calloc(grid->cols * grid->rows, sizeof(int));
    memset(grid->cells, -1, sizeof(int) * grid->cols * grid->rows);
}

static void grid_free(_poisson_grid_t *grid) {
    free(grid->cells);
}

static void grid_set(_poisson_grid_t *grid, float x, float y, int idx) {
    int col = (int)(x / grid->cell_size);
    int row = (int)(y / grid->cell_size);
    if (col >= 0 && col < grid->cols && row >= 0 && row < grid->rows)
        grid->cells[row * grid->cols + col] = idx;
}

static int grid_check_distance(_poisson_grid_t *grid, poisson_point_t *all_points, poisson_point_t p, float min_dist_sq) {
    int col = (int)(p.x / grid->cell_size);
    int row = (int)(p.y / grid->cell_size);
    int search = 2;
    for (int dy = -search; dy <= search; dy++)
        for (int dx = -search; dx <= search; dx++) {
            int c = col + dx;
            int r = row + dy;
            
            if (c < 0 || c >= grid->cols || r < 0 || r >= grid->rows)
                continue;
            
            int idx = grid->cells[r * grid->cols + c];
            if (idx != -1) {
                poisson_point_t other = all_points[idx];
                float dist_sq = (p.x - other.x) * (p.x - other.x) +
                                (p.y - other.y) * (p.y - other.y);
                if (dist_sq < min_dist_sq)
                    return 0;
            }
        }
    return 1;
}

// Main Poisson disc sampling algorithm (private)
static void poisson_disc_sample_internal(float width, float height,
                                          float min_dist, int max_attempts,
                                          poisson_check_callback_t check_fn,
                                          void *check_data,
                                          poisson_callback_t point_fn,
                                          void *point_data) {
    float cell_size = min_dist / sqrtf(2.0f);
    float min_dist_sq = min_dist * min_dist;
    
    _poisson_grid_t grid;
    grid_init(&grid, width, height, cell_size);
    
    _poisson_active_t active;
    active_list_init(&active);
    
    // Storage for all points
    size_t all_capacity = 1024;
    size_t all_count = 0;
    poisson_point_t *all_points = (poisson_point_t*)malloc(sizeof(poisson_point_t) * all_capacity);
    
    // Add initial point
    poisson_point_t initial = {width * 0.5f, height * 0.5f};
    
    if (!check_fn || check_fn(initial.x, initial.y, check_data)) {
        all_points[all_count] = initial;
        grid_set(&grid, initial.x, initial.y, all_count);
        active_list_add(&active, initial);
        all_count++;
    }
    
    while (active.count > 0) {
        size_t idx = rand() % active.count;
        poisson_point_t base = active.points[idx];
        int found = 0;
        
        for (int i = 0; i < max_attempts; i++) {
            float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
            float radius = min_dist + ((float)rand() / RAND_MAX) * min_dist;
            
            poisson_point_t candidate = {
                base.x + cosf(angle) * radius,
                base.y + sinf(angle) * radius
            };
            
            if (candidate.x >= 0 && candidate.x < width &&
                candidate.y >= 0 && candidate.y < height &&
                grid_check_distance(&grid, all_points, candidate, min_dist_sq)) {
                
                if (!check_fn || check_fn(candidate.x, candidate.y, check_data)) {
                    if (all_count >= all_capacity) {
                        all_capacity *= 2;
                        all_points = (poisson_point_t*)realloc(all_points, sizeof(poisson_point_t) * all_capacity);
                    }
                    
                    all_points[all_count] = candidate;
                    grid_set(&grid, candidate.x, candidate.y, all_count);
                    active_list_add(&active, candidate);
                    all_count++;
                    found = 1;
                }
            }
        }
        
        if (!found)
            active_list_remove(&active, idx);
    }
    
    // Call the callback for each point
    if (point_fn)
        for (size_t i = 0; i < all_count; i++)
            point_fn(all_points[i].x, all_points[i].y, point_data);
    
    free(all_points);
    active_list_free(&active);
    grid_free(&grid);
}

static void collect_point(float x, float y, void *data) {
    poisson_list_t *list = (poisson_list_t *)data;
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->points = (poisson_point_t*)realloc(list->points, sizeof(poisson_point_t) * list->capacity);
    }
    list->points[list->count].x = x;
    list->points[list->count].y = y;
    list->count++;
}
// Public function: returns allocated list
poisson_list_t *poisson_disc_sample_list(float width, float height, float min_dist,
                                    int max_attempts, poisson_check_callback_t check_fn,
                                    void *user_data) {
    poisson_list_t *result = (poisson_list_t*)malloc(sizeof(poisson_list_t));
    result->capacity = 1024;
    result->count = 0;
    result->points = (poisson_point_t*)malloc(sizeof(poisson_point_t) * result->capacity);
    
    poisson_disc_sample_internal(width, height, min_dist, max_attempts,
                                 check_fn, user_data, collect_point, result);
    
    return result;
}

void poisson_disc_free_list(poisson_list_t *list) {
    if (list) {
        free(list->points);
        free(list);
    }
}

// Public function: foreach with callback
void poisson_disc_sample_foreach(float width, float height, float min_dist,
                                 int max_attempts, poisson_check_callback_t check_fn,
                                 void *check_data, poisson_callback_t point_fn,
                                 void *point_data) {
    poisson_disc_sample_internal(width, height, min_dist, max_attempts,
                                 check_fn, check_data, point_fn, point_data);
}
#endif