#include "noise.hpp"
#include <cstdio>
#include <iostream>

float perlinOctave(unsigned int nOctaves, float frequency, float persistence, float lacunarity, float x, float y){
    // perlin noise rescaled and added into itself to create fractal noise
    // persistence defines how much of an impact each successive layer of noise has
    // lacunarity defines how the rate which frequency changes with each successive layer
    float noise = 0.0f;
    float amplitude = 1.0f;
    float totalAmplitude = 0.0f;
    for (int i=0; i<nOctaves; i++){
        noise += perlin(x * frequency, y * frequency) * amplitude;
        totalAmplitude += amplitude;
        frequency *= lacunarity;
        amplitude *= persistence;
    }
    // divide by total amplitude to normalise
    // multiply by 1.5 to maximise noise coverage of the -1 to 1 range
    return ((noise / totalAmplitude) * 1.5f + 1.0f) * 0.5f; // rescale noise to 0-1 range
}

float perlin(float x, float y) {
    // find unit square that contains point
    const int X = (int)x & 255;
    const int Y = (int)y & 255;

    // find relative x,y coord in unit square
    x -= (int)x;
    y -= (int)y;

    // compute fade curve for x and y
    const float u = fade(x);
    const float v = fade(y);

    // calculate a hash for each corner of the unit square
    const int A = permutation[X] + Y,   B = permutation[X + 1] + Y;
    const int AA = permutation[A],      BA = permutation[B];
    const int AB = permutation[A + 1],  BB = permutation[B + 1];

    // for each corner use the hash to select a gradient and calculate dot product with x,y
    // interpolate between the results with u,v to calculate final noise value and avoid artefacts
    const float interp0 = lerp(u, grad(permutation[AA], x, y), grad(permutation[BA], x - 1, y));
    const float interp1 = lerp(u, grad(permutation[AB], x, y - 1), grad(permutation[BB], x - 1, y - 1));
    return lerp(v, interp0, interp1); // noise outputs in range -1 to 1
}