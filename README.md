Interface board firmware
------------------------
This repository contains the source code for the firmware running on the
3devo interface board. This board is part of the 3devo Filament Extruder
and since version 1.3 includes an attiny841 microcontroller to handle
powering up the display and reading the hopper sensor.

To compile this firmware, the
[ATTinyCore](https://github.com/SpenceKonde/ATTinyCore/blob/master/Installation.md)
core and [Arduino-3devo](https://github.com/3devo/Arduino-3devo) board
description files need to be installed into the `hardware` subdirectory
of your Arduino sketchbook (these two are also included as submodules in
this repository, so if you configure the Arduino IDE Sketchbook path to
point to this repository, they should also be loaded as needed). Then
select "3devo Interfaceboard" in the IDE and compile the firmware.

A compiled version of this firmware is normally included in the Filament
Extruder's main firmware, and uploaded on every startup through a
[bootloader](https://github.com/3devo/AtTinyBootloader) running on the
microcontroller.

License
-------
This firmware contains a TWI implementation taken from
[bootloader-attiny](https://github.com/modulolabs/bootloader-attiny)
written by Erin Tomson. All code in this repository  is licensed under
the GPL, version 3. See the LICENSE file for the full license.
