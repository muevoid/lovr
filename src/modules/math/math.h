#include <stdbool.h>

#pragma once

struct RandomGenerator;

bool lovrMathInit(void);
void lovrMathDestroy(void);
struct RandomGenerator* lovrMathGetRandomGenerator(void);
float lovrMathGammaToLinear(float x);
float lovrMathLinearToGamma(float x);
#ifdef _WIN32
    #define LOVR_EXPORT __declspec(dllexport)
#else
    #define LOVR_EXPORT
#endif

LOVR_EXPORT float lovrMathNoise1(float x, int seed);
LOVR_EXPORT float lovrMathNoise2(float x, float y, int seed);
LOVR_EXPORT float lovrMathNoise3(float x, float y, float z, int seed);
LOVR_EXPORT float lovrFractalNoise(float x, float y, float z, float lacunarity, float gain, float offset, int octaves);