#include <pebble.h>
#include "common.h"
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




#define NUMBER_OF_ANIMATIONS 4
static MorphingAnimation animations[NUMBER_OF_ANIMATIONS];

static GPoint findNearestPixel(uint8_t* image, int xx, int yy) {
	int distance = 1;

	uint32_t mindist = -1;
	int xmin = 0;
	int ymin = 0;

	while (distance < IMAGE_HEIGHT) {
		for(int x = xx - distance; x <= xx + distance; x+=2*distance){
			uint32_t distx = (xx - x) * (xx - x);
			for (int y = yy - distance; y <= yy + distance; y++) {
				if (x >= 0 && x < IMAGE_WIDTH && y >= 0 && y < IMAGE_HEIGHT) {
					if (!getVal(image,x,y)) {
						if(y == yy)
							return GPoint(x, y);
						uint32_t dist = distx + (yy - y) * (yy - y);
						if (dist < mindist) {
							mindist = dist;
							xmin = x;
							ymin = y;
						}
					}
				}
			}
		}
		for (int y = yy - distance; y <= yy + distance; y+=2*distance) {
			uint32_t disty = (yy - y) * (yy - y);
			for (int x = xx - distance; x <= xx + distance; x++) {
				if (x >= 0 && x < IMAGE_WIDTH && y >= 0 && y < IMAGE_HEIGHT) {
					if (!getVal(image,x,y)) {
						if(x == xx)
							return GPoint(x, y);
						uint32_t dist = (xx - x) * (xx - x) + disty;
						if (dist < mindist) {
							mindist = dist;
							xmin = x;
							ymin = y;
						}
					}
				}
			}
		}
		if (mindist < 10000000) {
			return GPoint(xmin, ymin);
		}
		distance++;
	}
	return GPoint(xmin, ymin);
}

static int signum(int a){
	if( a < 0 )
		return -1;
	return 1;
}

static void animate(uint8_t* source, uint8_t* dest, uint8_t* next, int step) {
	memset(next, 0xFF, sizeof(uint8_t) * NB_BYTES_PER_ROW * IMAGE_HEIGHT);

	for (int y = 0; y < IMAGE_HEIGHT; y++) {
		for (int x = 0; x < IMAGE_WIDTH; x++) {
			if (!getVal(source,x,y) && !getVal(dest,x,y)) {
				setVal(next, x, y, 0);
			} else if (!getVal(source,x,y)) {
				GPoint p = findNearestPixel(dest, x, y);
				int diffX = (p.x - x) / (NUMBER_OF_IMAGES_IN_ANIMATION - step);
				int diffY = (p.y - y) / (NUMBER_OF_IMAGES_IN_ANIMATION - step);
				if(diffX == 0 && diffY == 0){
					if(abs(p.x - x) > abs(p.y - y)){
						diffX = signum(p.x - x);
					}
					else {
						diffY = signum(p.y - y);
					}
				}
				GPoint middle = GPoint(x + diffX, y + diffY);
				setVal(next, middle.x, middle.y, 0);
			} else if (!getVal(dest,x,y)) {
				GPoint p = findNearestPixel(source, x, y);
				int diffX = (x - p.x) / (NUMBER_OF_IMAGES_IN_ANIMATION - step);
				int diffY = (y - p.y) / (NUMBER_OF_IMAGES_IN_ANIMATION - step);
				if(diffX == 0 && diffY == 0){
					if(abs(p.x - x) > abs(p.y - y)){
						diffX = signum(x - p.x);
					}
					else {
						diffY = signum(y - p.y);
					}
				}
				GPoint middle = GPoint(p.x + diffX, p.y + diffY);
				setVal(next, middle.x, middle.y, 0);
			}
		}
	}
}

MorphingAnimation* getEmptyMorphingAnimation(){
	for(int i=0; i<NUMBER_OF_ANIMATIONS; i++){
		if(animations[i].from == 0 && animations[i].to == 0){
			return &animations[i];
		}
	}
	return NULL;
}

void computeMorphingAnimation(MorphingAnimation* animation){
	APP_LOG(APP_LOG_LEVEL_INFO, "computeMorphingAnimation %d %d", animation->from, animation->to);
	GBitmap* from_image = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[animation->from]);
	GBitmap* dest_image = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[animation->to]);

	int sizeofimage = sizeof(uint8_t) * NB_BYTES_PER_ROW * IMAGE_HEIGHT;
	memcpy(animation->animation_images, from_image->addr, sizeofimage);

	for(int i=0; i<NUMBER_OF_IMAGES_IN_ANIMATION-1; i++){
		animate(animation->animation_images + i*sizeofimage, dest_image->addr, animation->animation_images + (i+1)*sizeofimage, i);
	}

	gbitmap_destroy(from_image);
	gbitmap_destroy(dest_image);
}

MorphingAnimation* getMorphingAnimation(uint8_t from, uint8_t to){
	for(int i=0; i<NUMBER_OF_ANIMATIONS; i++){
		if(animations[i].from == from && animations[i].to == to){
			return &animations[i];
		}
	}
	return NULL;
}

void resetMorphingAnimations(){
	for(int i=0; i<NUMBER_OF_ANIMATIONS; i++){
		animations[i].from = 0;
		animations[i].to = 0;
	}
}