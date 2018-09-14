# Quadrotor Control and Design
## Project Objectives
* Assemble the electromechanical system of a quadrotor ([joystick control](#week-3), [autonomous control](#week-8)).
* Program interfaces between embedded computer ([Raspberry Pi](https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/images/raspberry_pi.PNG) running Linux) and external sensors and actuators (IMU, motor controllers).
* Program and tune a time-critical control loop for stable flight.
* Create software stack that interacts with low-level code to achieve high-level behavior including:
  1. Control the quadrotor with [logitech joystick](#week-4) ([demo](#demo-for-joystick-control-of-quadrotor)).
  2. Autonomous flight with [HTC vive lighthouse](#week-8) ([demos](#demos-for-autonomous-control-of-quadrotor)).

## Software/Language Used 
* C/C++
* [MobaXterm](https://mobaxterm.mobatek.net/)
* [VMware Workstation Player](https://www.vmware.com/products/workstation-player.html)

## Hardware Used
* [Raspberry Pi](https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/images/raspberry_pi.PNG)
* IMU
* SD Card
* Wifi Dongle
* Motor Controllers
* [Logitech Joystick F310](#week-4)
* [HTC vive lighthouse](#week-8)

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

Here is the electric system schematic diagram for quadrotor:

<img src="https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/images/wiring_diagram.png" width="500">

Here is the assembled quadrotor:

<img src="https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/images/quadrotor_1.JPG" width="500">

### Week 4
* Tune the PID w.r.t pitch.
* Build connection between joystick and quadrotor via virtual machine.
* Test pause, unpause, calibrate, kill all buttons on the joystick. 

Here is what can each button on joystick do:

<img src="https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/images/joystick_demonstration.PNG" width="500">

### Week 5
* Tune the PID w.r.t pitch, which eventually reduce the overshoot and transient time.
* Add desire pitch control on joystick (±25 degree). [check joystick here](#week-4)
* Add Thrust control on joystick (1250 ± 100 pwm). 
* Set up proportional control (P control) w.r.t roll. 
* Set up proportional + differential control (PD control) w.r.t roll.
* Set up proportional + integral + differential control (PID control) w.r.t roll.

### Week 6
* Tune the PID w.r.t roll, which eventually reduce the overshoot and transient time.
* Add desire roll control on joystick (±25 degree). [check joystick here](#week-4)
* Add desire yaw control on joystick (±45 degree). 
* Ground flight test (fly from one point to another point at low level).

### Week 7
* Change the desire pitch & roll control on joytick to ±10 degree.
* 2-feet flight test (fly from one point to another point at 2-feet).

#### Demo for joystick control of quadrotor
![joystick control](https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/demo/JoystickControlDemo.gif)

### Week 8
Start from week 8, we switch from joytick control flight to autonomous flight by using [HTC vive lighthouse](#week-8) and its corresponding sensors.
* Assemble vive sensors to the chassis of quadrotor.
* Test sensor by reading the data.
* Add safety control during automonous flight. 
* Add autonomous control for yaw, which have pitch axis (front of the robot) to always face in -y direction (proportional control).

Here is vive lighthouse we use:

<img src="https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/images/vive_lighthouse.jpg" width="500">

Here is assembled quadrotor with vive sensors:

<img src="https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/images/quadrotor_2.JPG" width="500">

### Week 9
* Add autonomous control for X, Y direction (proportional + differential control).
* Add filters for autonomous control for X, Y, and yaw to reduce the noises from vive light house.
* Mix the autonomous control and joystick control for X, Y, and yaw.
* Test autonomous control so far by Using joystick thrust to keep quadrotor about 2 feet off ground.

### Week 10
* Add autonomous thrust control (Z direction, height off ground) (calibrate Z acceleration, average Z acceleration, fuse Z velocity).

#### Demos for autonomous control of quadrotor
First:

![autonomous control](https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/demo/AutonomousControlDemo1.gif)

Second:

![autonomous control](https://github.com/meng1994412/Quadrotor_control_and_design/blob/master/demo/AutonomousControlDemo2.gif)
