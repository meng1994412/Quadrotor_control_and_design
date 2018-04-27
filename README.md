# Quadrotor Control and Design
## Project Objectives
* Assemble the electromechanical system of a [quadrotor](#week-3).
* Program interfaces between embedded computer (Raspberry Pi running Linux) and external sensors and actuators (IMU, motor controllers).
* Program and tune a time-critical control loop for stable flight
* Create software stack that interacts with low-level code to achieve high-level behavior including:
  1. Control the quadrotor with [logitech joystick](#week-4).
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
* [Logitech Joystick F310](#week-4)
* HTC vive lighthouse

## Weekly Milestones
### Week 1 & 2
* Set up IMU, SD card, wifi dongle, Raspberry Pi.
* Calibrate gyro from X, Y, and Z direction, roll and pitch angles from corresponding X, Y acceleration.
* Add complimentary filters to reduce the noises from acceleration in short term and drifts from gyro in long term.
* Add safety check and control.

### Week 3
* Build quadrotor from commodity parts.
* Check safty control.
* Set up proportional control (P control) w.r.t pitch. 
* Set up proportional + differential control (PD control) w.r.t pitch.
* Set up proportional + integral + differential control (PID control) w.r.t pitch.

Here is the electric system diagram for quadrotor:

<img src="https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/images/wiring%20diagram.png" width="500">

Here is the assembled quadrotor:

<img src="https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/images/quadrotor_1.JPG" width="500">

### Week 4
* Tune the PID w.r.t pitch.
* Build connection between joystick and quadrotor via virtual machine.
* Test pause, unpause, calibrate, kill all buttons on the joystick. 

Here is what can each button on joystick do:

<img src="https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/images/joystick%20demonstration.PNG" width="500">

### Week 5
* Tune the PID w.r.t pitch, which eventually reduce the overshoot and transient time.
* Add desire pitch control on joystick (±25 degree). [check joystick here](#week-4)
* Add Thrust control on joystick (1550 ± 100 pwm). [check joystick here](#week-4)
* Set up proportional control (P control) w.r.t roll. 
* Set up proportional + differential control (PD control) w.r.t roll.
* Set up proportional + integral + differential control (PID control) w.r.t roll.

### Week 6
* Tune the PID w.r.t roll, which eventually reduce the overshoot and transient time.
* Add desire roll control on joystick (±25 degree). [check joystick here](#week-4)
* Add desire yaw control on joystick (±45 degree). [check joystick here](#week-4)
* Ground flight test (fly from one point to another point at low level).


