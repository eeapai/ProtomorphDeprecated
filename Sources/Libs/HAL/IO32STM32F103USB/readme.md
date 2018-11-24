# IO32 implementation on STM32F103
IO32 is a simple byte based protocol for pin wiggling and reading [see IO32 interface definition](../HAL_IO32.h).

This is one of the IO32 interface implemntations. It uses USB to communicate IO32 bytes from "application" to STM32 microcontroller.
Other implemntations are available, e.g. on [RapberyPi](../IO32RaspberryPiHost) for wiggling its IOs via ethernet.

Build settings are configured as Eclipse CDT Automake project in Eclipse subfolder.

Sources are combined in Eclipse project virtual folders from here, [HAL](..) and [Comm](../Comm).
