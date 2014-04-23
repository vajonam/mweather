#ifndef BATTERY_LAYER_H
#define BATTERY_LAYER_H

void battery_layer_create(GRect frame, Window *window);
void handle_battery(BatteryChargeState charge_state);
void battery_enable_display();
void battery_disable_display(bool force);
void battery_timer_callback();
void battery_layer_update();
void battery_layer_destroy();

#endif