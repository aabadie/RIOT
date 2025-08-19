#include "cpu.h"
#include "periph/gpio.h"


int gpio_init(gpio_t pin, gpio_mode_t mode)
{
    (void)mode;

    // Pull low the pin by default
    gpio_clear(pin);
    return 0;
}

bool gpio_read(gpio_t pin)
{
    return (bool)(SCUM_GPIO_INPUT & (1 << pin));
}

void gpio_set(gpio_t pin)
{
    SCUM_GPIO_INPUT |= (1 << pin);
}

void gpio_clear(gpio_t pin)
{
    SCUM_GPIO_INPUT &= ~(1 << pin);
}

void gpio_toggle(gpio_t pin)
{
    SCUM_GPIO_INPUT ^= (1 << pin);
}

void gpio_write(gpio_t pin, bool value)
{
    if (value) {
        gpio_set(pin);
    } else {
        gpio_clear(pin);
    }
}
