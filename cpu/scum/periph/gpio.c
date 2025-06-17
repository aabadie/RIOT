#include "cpu.h"
#include "periph/gpio.h"


int gpio_init(gpio_t pin, gpio_mode_t mode)
{
    (void)pin;
    (void)mode;

    return 0;
}

bool gpio_read(gpio_t pin)
{
    (void)pin;
    return false;
}

void gpio_set(gpio_t pin)
{
    (void)pin;
}

void gpio_clear(gpio_t pin)
{
    (void)pin;
}

void gpio_toggle(gpio_t pin)
{
    (void)pin;
}

void gpio_write(gpio_t pin, bool value)
{
    (void)pin;
    (void)value;
}
