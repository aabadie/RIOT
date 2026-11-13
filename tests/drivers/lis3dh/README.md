# About
This is a manual test application for the LIS3DH accelerometer driver.

# Usage
This test application will initialize the accelerometer with the following parameters:
 - Sampling Rate: 100Hz
 - Scale:         4G
 - Temperature sensor: Enabled

After initialization, the sensor reads the acceleration values every 100ms
and prints them to the STDOUT.

The sensor is on SPI by default. For a sensor on I2C, build with:

    DRIVER=lis3dh_i2c make BOARD=<board> flash term

The test uses the INT1 pin of the sensor, `LIS3DH_PARAM_INT1` must be set
for the board.
