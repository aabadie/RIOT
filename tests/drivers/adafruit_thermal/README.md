# Adafruit thermal printer

Test application of the `adafruit_thermal` driver, for the Mini Thermal
Receipt Printer of Adafruit (product 597) and its siblings.

## Wiring

- the RX pin of the printer (data in, see the label of its connector) to
  the TX pin of the
  UART, `UART_DEV(1)` by default
- the ground of the printer to the ground of the board
- optionally the DTR pin of the printer to a GPIO, for the flow control
- the printer power to a 5 to 9 V supply of at least 1.5 A: not the 3.3 V
  of the board

The printer logic accepts 3.3 V. Its TX pin is not used.

Hold the button of the printer while powering it to print its test page:
it gives the baud rate, 19200 by default here.

## Build

    make BOARD=<board> flash term

Other UART, baud rate or DTR pin:

    CFLAGS="-DADAFRUIT_THERMAL_PARAM_UART=UART_DEV\(0\) \
            -DADAFRUIT_THERMAL_PARAM_BAUDRATE=9600 \
            -DADAFRUIT_THERMAL_PARAM_DTR_PIN=GPIO_PIN\(0,3\)" \
        make BOARD=<board> flash term

## Commands

    > print Hello RIOT
    > mode bold wide
    > justify center
    > print Big
    > reset
    > pattern
    > feed 3

`pattern` prints a checkerboard of 16 dot squares on the whole width of
the paper: the squares show whether the bytes of an image reach the
printer in time. With missing or shifted squares, connect DTR, or raise
`ADAFRUIT_THERMAL_PARAM_DOT_PRINT_US`.

`testpage` prints the test page of the printer, `sleep` and `wake` test
its low power mode.
