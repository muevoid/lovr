#include "math.h"
#include "math/randomGenerator.h"
#include "core/maf.h"
#include "core/util.h"
#define STB_PERLIN_IMPLEMENTATION
#include "lib/stb/stb_perlin.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static struct {
  bool initialized;
  RandomGenerator* generator;
} state;

bool lovrMathInit() {
  if (state.initialized) return false;
  state.generator = lovrRandomGeneratorCreate();
  Seed seed = { .b64 = (uint64_t) time(0) };
  lovrRandomGeneratorSetSeed(state.generator, seed);
  return state.initialized = true;
}

void lovrMathDestroy() {
  if (!state.initialized) return;
  lovrRelease(state.generator, lovrRandomGeneratorDestroy);
  memset(&state, 0, sizeof(state));
}

RandomGenerator* lovrMathGetRandomGenerator() {
  return state.generator;
}

float lovrMathGammaToLinear(float x) {
  if (x <= .04045f) {
    return x / 12.92f;
  } else {
    return powf((x + .055f) / 1.055f, 2.4f);
  }
}

float lovrMathLinearToGamma(float x) {
  if (x <= .0031308f) {
    return x * 12.92f;
  } else {
    return 1.055f * powf(x, 1.f / 2.4f) - .055f;
  }
}

float lovrMathNoise1(float x, int seed) {
  return stb_perlin_noise3(x, 0, 0, 0, 0, 0, seed);
}

float lovrMathNoise2(float x, float y, int seed) {
  return stb_perlin_noise3(x, y, 0, 0, 0, 0, seed);
}

float lovrMathNoise3(float x, float y, float z, int seed) {
  return stb_perlin_noise3(x, y, z, 0, 0, 0, seed);
}

float lovrFractalNoise(float x, float y, float z, float lacunarity, float gain, float offset, int octaves) {
  return stb_perlin_ridge_noise3(x, y, z, lacunarity, gain, offset, octaves);
}
