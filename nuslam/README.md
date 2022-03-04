# nuslam
By Marco Morales

## OVERVIEW
The nuslam package allows the use of the Extended Kalman Filter on the Turtlebot3 to estimate its distance as well as estimate the location of obstacles/landmarks in the map.

## Nodes

The the main node in this package is the slam node.

The slam node implements the Extended Kalman Filter SLAM. It reads the obstacle data being published by the nusim node to calculate the position of the Turtlebot as well as the obstacles' locations. It then publishes a MarkerArray message of where the markers are located based on the EKF filter. 

## Launchfiles

The launchfile for this package is `slam.launch`.

This launch file can be called by running the following command.
```
roslaunch nuslam slam.launch
```

The launch file has a few arguments that be changed. They are as follows.

1. robot

To run with nusim in a simulated environment, use `robot:=nusim` which is the default.

2. cmd_src

To control the robot with your keyboard, set the argument to `cmd_src:=teleop`, otherwise use `cmd_src:=circle` to control the robot with the circle node. 

3. use_rviz

To use RVIZ, set the argument to true such as the following `use_rviz:=true`.

4. config_file

To specify which config file to use with different parameters for noise or laser scan parameters, set this argument to the name of that file such as `config_file:=default_config.yaml`.

## Results
Parameters
Obstacle noise standard deviation: 0.01
Wheel Slip noise: 0.1
Wheel_cmd noise: 0.1
Laser noise: 0.01

![SLAM](pictures/slam.png)