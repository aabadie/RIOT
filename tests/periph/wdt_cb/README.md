# Periph WDT callback test

## About

This application tests the `periph_wdt_cb` feature: the watchdog is
configured with an early warning callback and started, without ever being
kicked.

## Expected Result

Once the configured reset time expired, the callback prints "SUCCESS" and
the MCU is rebooted by the watchdog: the application startup messages are
printed again.

Note that the callback runs shortly before the reset (the warning period is
platform dependent and can be very short), so the "SUCCESS" message can be
truncated on platforms with a warning period shorter than its transmission
time over the serial port. For this reason the automated script only expects
a prefix of the message, and on the nRF DKs (~61 us warning period) the
serial speed is raised to 460800 Bd via the application Makefile.
