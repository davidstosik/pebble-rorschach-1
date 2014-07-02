#include <pebble.h>

#include "common.h"
#include "morphing_layer.h"
#include "animation_manager.h"

static const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
	RESOURCE_ID_IMAGE_NUM_0,
	RESOURCE_ID_IMAGE_NUM_1,
	RESOURCE_ID_IMAGE_NUM_2,
	RESOURCE_ID_IMAGE_NUM_3,
	RESOURCE_ID_IMAGE_NUM_4,
	RESOURCE_ID_IMAGE_NUM_5,
	RESOURCE_ID_IMAGE_NUM_6,
	RESOURCE_ID_IMAGE_NUM_7,
	RESOURCE_ID_IMAGE_NUM_8,
	RESOURCE_ID_IMAGE_NUM_9,
};

static void drawImageReverse(GContext* ctx, uint8_t* image){
	for(int y=0; y<IMAGE_HEIGHT; y++){
		for(int x=0; x<IMAGE_WIDTH; x++){
			if(!getVal(image, x, y))
				graphics_draw_pixel(ctx, GPoint(IMAGE_WIDTH - 1 - x, y));
		}
	}
}

static void layer_update_callback(Layer *me, GContext* ctx) {
	MorphingLayer* morphing_layer = *(MorphingLayer**)(layer_get_data(me));

	GRect layer_bounds = layer_get_bounds(me);

	if(morphing_layer->reverse){
		if(morphing_layer->isAnimating){
			drawImageReverse(ctx, morphing_layer->tmp_image->addr);
		}
		else {
			drawImageReverse(ctx, morphing_layer->dest_image->addr);
		}
	}
	else {
		if(morphing_layer->isAnimating){
			graphics_draw_bitmap_in_rect(ctx, morphing_layer->tmp_image, layer_bounds);
		}
		else {
			graphics_draw_bitmap_in_rect(ctx, morphing_layer->dest_image, layer_bounds);
		}
	}
}

Layer* morphing_layer_get_layer(MorphingLayer *morphing_layer){
	return morphing_layer->layer;
}

void animationUpdate(struct Animation *animation, const uint32_t time_normalized){
	MorphingLayer *morphing_layer = (MorphingLayer *)animation_get_context(animation);

	int percent = time_normalized * 100 / ANIMATION_NORMALIZED_MAX;
	int step = percent / 10;

	if(step != morphing_layer->currentAnimationStep && step < 10){
		morphing_layer->currentAnimationStep =  step;
		morphing_layer->tmp_image->addr = morphing_layer->morphing_animation->animation_images + step * (sizeof(uint8_t) * NB_BYTES_PER_ROW * IMAGE_HEIGHT); 
		layer_mark_dirty(morphing_layer->layer);
	}
}

static void animation_started(Animation *animation, void *data) {
	MorphingLayer *morphing_layer = (MorphingLayer *)data;
	morphing_layer->tmp_image->addr = morphing_layer->morphing_animation->animation_images;
	morphing_layer->isAnimating = true;

	layer_mark_dirty(morphing_layer->layer);
}

void animation_stopped(Animation *animation, bool finished, void *data) {
	MorphingLayer *morphing_layer = (MorphingLayer *)data;
	morphing_layer->isAnimating = false;
	
	layer_mark_dirty(morphing_layer->layer);
}

void morphing_layer_animate_to(MorphingLayer *morphing_layer, uint8_t next_Digit){
	if(!morphing_layer->isAnimating && morphing_layer->current_Digit != next_Digit){
		if(morphing_layer->dest_image){
			gbitmap_destroy(morphing_layer->dest_image);
		}
		morphing_layer->dest_image = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[next_Digit]);
		morphing_layer->morphing_animation = getMorphingAnimation(morphing_layer->current_Digit, next_Digit);
		morphing_layer->current_Digit = next_Digit;
		if(morphing_layer->morphing_animation){
			memcpy(morphing_layer->tmp_image, morphing_layer->dest_image, sizeof(GBitmap));
			animation_schedule(morphing_layer->animation);
		}
	}
}

MorphingLayer* morphing_layer_create(GRect frame){
	MorphingLayer* morphing_layer = malloc(sizeof(MorphingLayer));

	morphing_layer->layer = layer_create_with_data(frame, sizeof(MorphingLayer*));
	layer_set_update_proc(morphing_layer->layer, layer_update_callback);
	memcpy(layer_get_data(morphing_layer->layer), &morphing_layer, sizeof(MorphingLayer*));

	morphing_layer->animation = animation_create();
	morphing_layer->animImpl.update = animationUpdate;
	animation_set_handlers(morphing_layer->animation, (AnimationHandlers) {
		.started = (AnimationStartedHandler) animation_started,
		.stopped = (AnimationStoppedHandler) animation_stopped,
	}, morphing_layer);

	animation_set_duration(morphing_layer->animation, 700);
	animation_set_implementation(morphing_layer->animation, &(morphing_layer->animImpl));

	morphing_layer->tmp_image = malloc(sizeof(GBitmap));

	morphing_layer->current_Digit = 0;

	return morphing_layer;
}

void morphing_layer_destroy(MorphingLayer *morphing_layer){
	layer_destroy(morphing_layer->layer);
	if(morphing_layer->dest_image){
		gbitmap_destroy(morphing_layer->dest_image);
	}
	if(morphing_layer->animation){
		animation_destroy(morphing_layer->animation);
	}
	free(morphing_layer->tmp_image);
	free(morphing_layer);
}

void morphing_layer_set_value(MorphingLayer *morphing_layer, uint8_t value){
	morphing_layer->current_Digit = value;
	
	if(morphing_layer->dest_image){
		gbitmap_destroy(morphing_layer->dest_image);
	}
	morphing_layer->dest_image = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[morphing_layer->current_Digit]);
}

void morphing_layer_set_reverse(MorphingLayer *morphing_layer, bool reverse){
	morphing_layer->reverse = reverse;
}
