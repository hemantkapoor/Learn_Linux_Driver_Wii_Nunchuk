The Wii Nunchuk communicates via I2C protocol.

The logical view of Wii Nunchuck is basic Input Device.

The Physical view is I2C device driver.

The Linux Framework used for this is a simple Input framework and since there is no interrupt mechanism with this device, 
so subsystem in Input frame work called input_polling (something like this) is used.

