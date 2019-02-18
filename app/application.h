#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifndef FIRMWARE
#define FIRMWARE "climate-monitor"
#endif

#ifndef VERSION
#define VERSION "2.0"
#endif

#include <bcl.h>

typedef struct
{
    uint8_t channel;
    float value;
    bc_tick_t next_pub;

} event_param_t;

typedef struct
{
    event_param_t temperature;
    event_param_t humidity;
    event_param_t illuminance;
    event_param_t pressure;

} climate_module_t;

void climate_module_event_handler(bc_module_climate_event_t event, void *event_param);
void battery_module_event_handler(bc_module_battery_event_t event, void *event_param);
void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);

#endif
