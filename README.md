# JoystickInteractionApps
Joystick Data Capture with SDL3

# Prepare
This is only tested on Windows.

I use an VCPKG installed in D:\GitHub\vcpkg with all additional needed libs.

# Tested
I created this because my JoystickPlugin for UE did not recognise the Saitek Rudder Pedals. So i try SDL3 and some patches to get this running seperatly from UE. Next step is to upgrade the plugin for SDL3. Maybe restructure.

![image](https://github.com/tsky1971/JoystickInteractionApps/assets/7058122/c9ecd29c-6578-4e45-b85f-b33464ed9733)


# Roadmap
- add OSC to send joystick updates
- add MQTT or ZMQ
- connect with an UE-plugin (maybe JoystickDeviceConnector?!) to get device data in UE via OSC or MQTT or ZMQ from JoystickSDL
- run JoystickSDL on RPI

- 
