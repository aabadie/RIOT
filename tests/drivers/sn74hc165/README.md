# SN74HC165 shift register test

This application reads the inputs of a 74HC165 parallel-in serial-out shift
register (or a chain of up to 4) every 20 ms and prints them when they
change, most significant input (H) first.

Set the pins with `SN74HC165_PARAM_PL`, `SN74HC165_PARAM_CLK`,
`SN74HC165_PARAM_Q7` and, when the clock enable pin is wired,
`SN74HC165_PARAM_CE`, unless the board already does (for example the
Adafruit PyBadge, whose 8 buttons are behind a 74HC165):

    make BOARD=adafruit-pybadge flash term

Pressing and releasing a button prints two lines.
