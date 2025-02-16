GPS - Driver

Description

This repository contains a Linux kernel module that interfaces with a GPS device via a serial connection. The module reads and processes GPS data, extracting key information such as UTC time, latitude, longitude, and date. It enables users to retrieve real-time GPS information through the sysfs interface.

Features

Reads GPS data from a serial port (/dev/ttyS0)

Parses $GPRMC NMEA sentences to extract location and time data

Exposes GPS information via the sysfs interface

Files

gps_driver.c - The main kernel module source code

Makefile - Build instructions for compiling the kernel module

Requirements

Linux kernel headers installed on your system

A GPS device providing NMEA sentence data over /dev/ttyS0

Compilation and Installation

To build the module, run:

make

To insert the module into the kernel:

sudo insmod gps_driver.ko

To remove the module:

sudo rmmod gps_driver

To clean the build files:

make clean

Usage

Once the module is loaded, GPS data can be accessed via sysfs:

cat /sys/kernel/gps/gps_time

This will output parsed GPS data including UTC time, latitude, longitude, and date.

License

This project is licensed under the GPL v2 license.

Author

Bhuvan
