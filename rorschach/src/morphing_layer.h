#pragma once

#include <pebble.h>
#include "animation_manager.h"

typedef struct {
	Layer       *layer;

	bool 		reverse;

	Animation* animation;
	AnimationImplementation animImpl;

	GBitmap* tmp_image;
	GBitmap* dest_image;

	bool isAnimating;

	uint8_t current_Digit;
	
	MorphingAnimation* morphing_animation;

	uint8_t currentAnimationStep;
} MorphingLayer;

MorphingLayer* morphing_layer_create(GRect frame);

void morphing_layer_destroy(MorphingLayer *morphing_layer);

Layer* morphing_layer_get_layer(MorphingLayer *morphing_layer);

void morphing_layer_animate_to(MorphingLayer *text_layer, uint8_t next_value);

void morphing_layer_set_value(MorphingLayer *morphing_layer, uint8_t value);

void morphing_layer_set_reverse(MorphingLayer *morphing_layer, bool reverse);

