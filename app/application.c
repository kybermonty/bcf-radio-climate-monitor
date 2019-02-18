#include <application.h>

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define TEMPERATURE_TAG_PUB_VALUE_CHANGE 0.2f
#define TEMPERATURE_TAG_UPDATE_INTERVAL (30 * 1000)

#define HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL (5 * 60 * 1000)
#define HUMIDITY_TAG_PUB_VALUE_CHANGE 2.0f
#define HUMIDITY_TAG_UPDATE_INTERVAL (30 * 1000)

#define LUX_METER_TAG_PUB_NO_CHANGE_INTERVAL (5 * 60 * 1000)
#define LUX_METER_TAG_UPDATE_INTERVAL (30 * 1000)

#define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)

bc_led_t led;
climate_module_t climate_module;

void application_init(void)
{
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    bc_radio_init(BC_RADIO_MODE_NODE_SLEEPING);

    static bc_button_t button;
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize climate module
    memset(&climate_module, 0, sizeof(climate_module));
    bc_module_climate_init();
    bc_module_climate_set_event_handler(climate_module_event_handler, NULL);
    bc_module_climate_set_update_interval_thermometer(TEMPERATURE_TAG_UPDATE_INTERVAL);
    bc_module_climate_set_update_interval_hygrometer(HUMIDITY_TAG_UPDATE_INTERVAL);
    bc_module_climate_set_update_interval_lux_meter(LUX_METER_TAG_UPDATE_INTERVAL);
    bc_module_climate_measure_all_sensors();

    // Battery Module
    bc_module_battery_init();
    bc_module_battery_set_event_handler(battery_module_event_handler, NULL);
    bc_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);

    bc_radio_pairing_request(FIRMWARE, VERSION);

    bc_led_pulse(&led, 2000);
}

void climate_module_event_handler(bc_module_climate_event_t event, void *event_param)
{
    (void) event_param;

    float value;

    if (event == BC_MODULE_CLIMATE_EVENT_UPDATE_THERMOMETER)
    {
        if (bc_module_climate_get_temperature_celsius(&value))
        {
            if ((fabs(value - climate_module.temperature.value) >= TEMPERATURE_TAG_PUB_VALUE_CHANGE) || (climate_module.temperature.next_pub < bc_scheduler_get_spin_tick()))
            {
                bc_radio_pub_temperature(BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT, &value);
                climate_module.temperature.value = value;
                climate_module.temperature.next_pub = bc_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTEVAL;
            }
        }
    }
    else if (event == BC_MODULE_CLIMATE_EVENT_UPDATE_HYGROMETER)
    {
        if (bc_module_climate_get_humidity_percentage(&value))
        {
            if ((fabs(value - climate_module.humidity.value) >= HUMIDITY_TAG_PUB_VALUE_CHANGE) || (climate_module.humidity.next_pub < bc_scheduler_get_spin_tick()))
            {
                bc_radio_pub_humidity(BC_RADIO_PUB_CHANNEL_R3_I2C0_ADDRESS_DEFAULT, &value);
                climate_module.humidity.value = value;
                climate_module.humidity.next_pub = bc_scheduler_get_spin_tick() + HUMIDITY_TAG_PUB_NO_CHANGE_INTEVAL;
            }
        }
    }
    else if (event == BC_MODULE_CLIMATE_EVENT_UPDATE_LUX_METER)
    {
        if (bc_module_climate_get_illuminance_lux(&value))
        {
            if (climate_module.illuminance.next_pub < bc_scheduler_get_spin_tick())
            {
                bc_radio_pub_luminosity(BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT, &value);
                climate_module.illuminance.value = value;
                climate_module.illuminance.next_pub = bc_scheduler_get_spin_tick() + LUX_METER_TAG_PUB_NO_CHANGE_INTERVAL;
            }
        }
    }
}

void battery_module_event_handler(bc_module_battery_event_t event, void *event_param)
{
    (void) event_param;

    float voltage;

    if (event == BC_MODULE_BATTERY_EVENT_UPDATE)
    {
        if (bc_module_battery_get_voltage(&voltage))
        {
            bc_radio_pub_battery(&voltage);
        }
    }
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);

        climate_module.temperature.next_pub = 0;
        climate_module.humidity.next_pub = 0;
        climate_module.illuminance.next_pub = 0;
        bc_module_climate_measure_all_sensors();

        bc_module_battery_measure();
    }
}
