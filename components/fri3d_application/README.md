# Fri3d Application component

This component defines a basic framework for an application. It imposes the usage of amongst others

* LVGL
* An `indev` driver
* Specific settings for hardware

Furthermore, it defines the concept of "apps" that can be launched and all that is needed for this. 

If you don't want to use for example LVGL you are in the wrong place and will need to write your own implementation.

## Structure

The application consists of the following major parts

### Application setup

Initializes base functionality

* Setup LVGL
* Setup indev
* Create the different managers
* Launch default app

### Hardware Manager

Centralized access points for hardware interaction

### Window Manager

Takes care of showing screens and navigating between them.

### App Manager

Register and manage apps.

### Theme Manager

Manages the look & feel of the badge.

### Apps

Everything else in the application is pluggable by writing an app. Each app has a main entrypoint and will register at
least one screen with the WindowManager which is shown when launched. After that the app is responsible for all user
interactions.

An app should properly release resources when exited / paused. For example if it is showing flashing LEDs, it should
stop doing so as soon as it loses focus. There is no resource manager that will take care of this for you. Remember,
with great power comes great responsibility.

## Miscellaneous

### Default app

The application has a main app that gets launched once all initialization is complete. For the standard badge firmware
this is the `fri3d_launcher` component.

The default app will also be jumped to whenever the Menu/Select button is pressed, so you could write your own launcher!
