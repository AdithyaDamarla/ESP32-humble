## Features

- `/servo`: Control the servo for marker placement on the drawing surface.
- `/LEDs`: Manage the LEDs mounted on the robot.
- `/left_motor_ticks` and `/right_motor_ticks`: Get motor encoder values for left and right wheels.
- `/battery`: Monitor the battery level of the robot.
- `/cmd_vel`: Control the movement of the robot.

## Getting Started

Follow these steps to get started with your Lineturtle robot:
1. Go to your workspace:
   ```shell
   cd ros2_ws/src/
   
2. Build the ROS2 package and install dependencies:
   ```shell
   cd  ..
   rosdep install --from-paths src --ignore-src -r -y
   colcon build --symlink-install
   source install/local_setup.bash
3. Edit the config file /teleop_twist_joy/config/xbox.config.yaml, clear it and paste this
   ```shell
   teleop_twist_joy_node:
     ros__parameters:
       axis_linear: # right thumb stick vertical
         x: 4
       scale_linear:
         x: 0.15
       scale_linear_turbo:
         x: 0.3
       axis_angular: # right thumb stick horizontal
         yaw: 3
       scale_angular:
         yaw: 0.2
       scale_angular_turbo:
         yaw: 0.4
       enable_button: 6 # Left trigger button
       enable_turbo_button: 4 # left trigger button
4. 3D print the robot body using these [STL files](https://github.com/robofuntastic/lineturtle/tree/main/CAD%20%26%203D%20Printing).
5. Gather all the components and configure your robot's hardware and connections as described in the [guide](https://github.com/robofuntastic/lineturtle/blob/main/circuit%20%26%20components/LineTurtle%20Circuit%20Diagram.pdf).
6. Upload the [microros sketch](https://github.com/robofuntastic/lineturtle/blob/main/esp32_microros/lineturtle_esp32_microros/lineturtle_esp32_microros.ino) to ESP32.
7. Launch the lineturtle robot
   ```ros2 launch lineturtle lineturtle_launch.py```

## License
This project is licensed under the MIT License.

