# HERO-STM32F4
This is a collection of embedded C/C++ projects targeting the STM32F on the CTRE HERO Developement Board

## Goals
### Socket CAN
Provide a firmware port of [candleLight_fw](https://github.com/HubertD/candleLight_fw).  
Socket CAN support is part of the multi-platform build effort of Phoenix (linux-desktop/RaspPi/etc).
This firmware allows using HERO in this fashion, however any Socket CAN compliant USB device will be supported.
### Embedded C/C++
To provide an "on the metal" template for embedded C/C++ developers who prefer to not use the Visual Studio C# NETMF solution.
This allows use of the STM32F hardware features that is not exposed in NETMF-C#.
### Phoenix on an embedded system
To provide an "on the metal" example that uses an embedded build of Phoenix. 
This means using the Phoenix API object model (TalonSRX objects for example) in embedded C++.

## What was done
This repository is based on the STM32CubeF4 package which can be downloaded from STMicroelectronics.
The following was added to the stock download of this package...
- Under Drivers/BSP, a new BSP source set exists for HERO.
- Under Projects, a new STM32F429VI-HERO folder is added.
- Under Projects/STM32F429VI-HERO/Applications/USB_Device, the HID_Standalone projected was updated for HERO.
- Under Projects/STM32F429VI-HERO/Applications/USB_Device, new project called CandleLight which is a port of (candleLight_fw)[https://github.com/HubertD/candleLight_fw]

## Projects
HID_Standalone
Device USB example that jitters the mouse of a PC after HERO has been connected via USB.

CandleLight
Makes the HERO behave like a Socket CAN device.  

## Issues
The projects are relatively new and untested, issues will be monitored in the [tracker](https://github.com/CrossTheRoadElec/HERO-STM32F4/issues).
