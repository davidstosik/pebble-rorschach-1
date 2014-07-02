#include <pebble.h>

#include "common.h"
#include "morphing_layer.h"
#include "animation_manager.h"

#include "autoconfig.h"

static Window *window;

static MorphingLayer *morphing_layer[8];
static InverterLayer* inverter_layer;

static AppTimer *computetimer;

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  for(int i=0; i<2; i++){
    for(int j=0; j<4; j++){
      morphing_layer[i * 4 + j] = morphing_layer_create((GRect) { .origin = { 8 + j * IMAGE_WIDTH, i * IMAGE_HEIGHT + 14 }, .size = { IMAGE_WIDTH, IMAGE_HEIGHT } });
      layer_add_child(window_layer, morphing_layer_get_layer(morphing_layer[i * 4 + j]));
    }
  }

  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  morphing_layer_set_value(morphing_layer[0], tick_time->tm_hour%10);
  morphing_layer_set_reverse(morphing_layer[0], true);
  morphing_layer_set_value(morphing_layer[1], tick_time->tm_hour/10);
  morphing_layer_set_reverse(morphing_layer[1], true);
  morphing_layer_set_value(morphing_layer[2], tick_time->tm_hour/10);
  morphing_layer_set_value(morphing_layer[3], tick_time->tm_hour%10);

  morphing_layer_set_value(morphing_layer[4], tick_time->tm_min/10);
  morphing_layer_set_value(morphing_layer[5], tick_time->tm_min%10);
  morphing_layer_set_value(morphing_layer[6], tick_time->tm_min%10);
  morphing_layer_set_reverse(morphing_layer[6], true);
  morphing_layer_set_value(morphing_layer[7], tick_time->tm_min/10);
  morphing_layer_set_reverse(morphing_layer[7], true);
  
  inverter_layer = inverter_layer_create  (bounds);
  layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer));
}

static void window_unload(Window *window) {
  for(int i=0; i<8; i++){
    morphing_layer_destroy(morphing_layer[i]);
  }
  inverter_layer_destroy(inverter_layer);
}

static void timer_callback(void *context) {
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  APP_LOG(APP_LOG_LEVEL_INFO, "timer_callback");

  resetMorphingAnimations();

  MorphingAnimation* morphing_animation = NULL;

  int next_min = (tick_time->tm_min + 1) % 60;

  if(next_min/10 != tick_time->tm_min/10){
    // APP_LOG(APP_LOG_LEVEL_INFO, "timer_callback min dec will change");
    morphing_animation = getMorphingAnimation(tick_time->tm_min/10, next_min/10);
    if(morphing_animation == NULL){
      morphing_animation = getEmptyMorphingAnimation();
      if(morphing_animation){
        morphing_animation->from = tick_time->tm_min/10;
        morphing_animation->to = next_min/10;
        computeMorphingAnimation(morphing_animation);
      }
    }
  }

  if(next_min%10 != tick_time->tm_min%10){
    // APP_LOG(APP_LOG_LEVEL_INFO, "timer_callback min unit will change");
    morphing_animation = getMorphingAnimation(tick_time->tm_min%10, next_min%10);
    if(morphing_animation == NULL){
      // APP_LOG(APP_LOG_LEVEL_INFO, "timer_callback min unit will change, animation not found");
      morphing_animation = getEmptyMorphingAnimation();
      if(morphing_animation){
        // APP_LOG(APP_LOG_LEVEL_INFO, "timer_callback min unit will change, empty animation found");
        morphing_animation->from = tick_time->tm_min%10;
        morphing_animation->to = next_min%10;
        computeMorphingAnimation(morphing_animation);
      }
    }
  }
  
  if(next_min == 0){
    int next_hour = (tick_time->tm_hour + 1) % 24;

    if(next_hour/10 != tick_time->tm_hour/10){
      // APP_LOG(APP_LOG_LEVEL_INFO, "timer_callback hour dec will change");
      morphing_animation = getMorphingAnimation(tick_time->tm_hour/10, next_hour/10);
      if(morphing_animation == NULL){
        morphing_animation = getEmptyMorphingAnimation();
        if(morphing_animation){
          morphing_animation->from = tick_time->tm_hour/10;
          morphing_animation->to = next_hour/10;
          computeMorphingAnimation(morphing_animation);
        }
      }
    }
  
    if(next_min%10 != tick_time->tm_hour%10){
      // APP_LOG(APP_LOG_LEVEL_INFO, "timer_callback hour unit will change");
      morphing_animation = getMorphingAnimation(tick_time->tm_hour%10, next_hour%10);
      if(morphing_animation == NULL){
        morphing_animation = getEmptyMorphingAnimation();
        if(morphing_animation){
          morphing_animation->from = tick_time->tm_hour%10;
          morphing_animation->to = next_hour%10;
          computeMorphingAnimation(morphing_animation);
        }
      }
    }
  }

  computetimer = NULL;
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  morphing_layer_animate_to(morphing_layer[0], tick_time->tm_hour%10);
  morphing_layer_animate_to(morphing_layer[1], tick_time->tm_hour/10);
  morphing_layer_animate_to(morphing_layer[2], tick_time->tm_hour/10);
  morphing_layer_animate_to(morphing_layer[3], tick_time->tm_hour%10);

  morphing_layer_animate_to(morphing_layer[4], tick_time->tm_min/10);
  morphing_layer_animate_to(morphing_layer[5], tick_time->tm_min%10);
  morphing_layer_animate_to(morphing_layer[6], tick_time->tm_min%10);
  morphing_layer_animate_to(morphing_layer[7], tick_time->tm_min/10);

  computetimer = app_timer_register(2500, timer_callback, NULL);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  autoconfig_in_received_handler(iter, context);

  layer_set_hidden(inverter_layer_get_layer(inverter_layer), !getInverted());
}

static void init(void) {
  autoconfig_init();

  computetimer = NULL;
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  window_stack_push(window, animated);

  app_message_register_inbox_received(in_received_handler);
}

static void deinit(void) {
  window_destroy(window);
  autoconfig_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
