#pragma once

#include <pebble.h>
#include "common.h"

#define NUMBER_OF_IMAGES_IN_ANIMATION 10

typedef struct {
	uint8_t animation_images[10 * sizeof(uint8_t) * NB_BYTES_PER_ROW * IMAGE_HEIGHT];
	
	uint8_t from;
	uint8_t to;
} MorphingAnimation;

MorphingAnimation* getEmptyMorphingAnimation();

void computeMorphingAnimation(MorphingAnimation* animation);

MorphingAnimation* getMorphingAnimation(uint8_t from, uint8_t to);

void resetMorphingAnimations();

#define getVal(image, x, y) ((image[y * NB_BYTES_PER_ROW + x / 8] >> (x%8)) & 1)
#define setVal(image, x, y, value) ((image[y * NB_BYTES_PER_ROW + x / 8] &= ~(0x01 << (x%8))))