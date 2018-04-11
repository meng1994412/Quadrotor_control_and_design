# Quadrotor Control and Design
## Project Objectives
* Assemble the electromechanical system of a quadrotor.
* Program interfaces between embedded computer (Raspberry Pi running Linux) and external sensors and actuators (IMU, motor controllers).
* Program and tune a time-critical control loop for stable flight
* Create software stack that interacts with low-level code to achieve high-level behavior including:
  1. Control the quadrotor with logitech joystick.
  2. Autonomous flight with HTC vive lighthouse.

## Software/Language Used 
* C 
* MobaXterm
* VMware Workstation Player

## Hardware Used
* Raspberry Pi
* IMU
* SD Card
* Wifi Dongle
* Motor Controllers
* Logitech Joystick F310
* HTC vive lighthouse

## Weekly Milestones
### Week 1 & 2
* Set up IMU, SD card, wifi dongle, Raspberry Pi.
* Calibrate gyro from X, Y, and Z direction, roll and pitch angles from corresponding X, Y acceleration.
* Add complimentary filters to reduce the noises from acceleration in short term and drifts from gyro in long term.
* Add safety check and control.

### Week 3
* Build quadrotor from commodity parts.
