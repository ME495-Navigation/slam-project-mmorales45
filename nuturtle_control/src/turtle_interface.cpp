/// \file
/// \brief Script that controls the turtlebot based on cmd_vel commands
///
/// PARAMETERS:
///     wheel_radius (double): The radius of the turtlebot's wheel
///     track_width (double): The distance between the wheels of the turtlebot
///     collision_radius (double): radius of the colision size 
///     motor_cmd_to_radsec (double): converts between ticks to rad/sec
///     encoder_ticks_to_rad (double): converts between ticks to radians
///     motor_cmd_max (std::vector<double>): limits for motor_cmd commands
/// PUBLISHERS:
///     wheel_cmd_pub (nuturtlebot_msgs/WheelCommands): Publishes wheel_cmd commands 
///     joint_states_pub (sensor_msgs/JointState): Publishes the positions and velocities of the robot's wheels
/// SUBSCRIBERS:
///     cmd_sub (geometry_msgs/Twist): Receive cmd_vel values to make the robot move
///     sensor_data_sub (nuturtlebot_msgs/SensorData): Get the currents encoder values for the robot's wheels

#include <ros/ros.h>
#include <std_msgs/UInt64.h>
#include <iostream>
#include <ros/console.h>
#include <std_srvs/Empty.h>
#include <sensor_msgs/JointState.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Twist.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <vector>
#include <cstdlib>


#include <nuturtlebot_msgs/WheelCommands.h>
#include <nuturtlebot_msgs/SensorData.h>
#include <turtlelib/rigid2d.hpp>
#include <turtlelib/diff_drive.hpp>

/// \brief Move the robot based on cmd_vel commands and then calculate the wheel data based on sensor data

class turtle_interface
{
    public:
        turtle_interface() {
            diffDrive = turtlelib::DiffDrive();
            rate = 500;
            //import parameters
            nh.getParam("/wheel_radius",wheel_radius);
            nh.getParam("/track_width",track_width);
            nh.getParam("/collision_radius",collision_radius);
            if(!nh.getParam("/motor_cmd_to_radsec",motor_cmd_to_radsec)){
                ROS_INFO_STREAM("Please make sure the parameters are correct! for motor_cmd_to_radsec");
                ros::shutdown();
            }
            else{
                nh.getParam("/motor_cmd_to_radsec",motor_cmd_to_radsec);
            }
            if(!nh.getParam("/encoder_ticks_to_rad",encoder_ticks_to_rad)){
                ROS_INFO_STREAM("Please make sure the parameters are correct! for encoder_ticks_to_rad");
                ros::shutdown();
            }
            else{
                nh.getParam("/encoder_ticks_to_rad",encoder_ticks_to_rad);
            }
            if(!nh.getParam("/motor_cmd_max",motor_cmd_max)){
                ROS_INFO_STREAM("Please make sure the parameters are correct! for motor_cmd_max");
                ros::shutdown();
            }
            else{
                nh.getParam("/motor_cmd_max",motor_cmd_max);
            }

            motor_cmd_max_lower = motor_cmd_max[0];
            motor_cmd_max_upper = motor_cmd_max[1];

            cmd_sub = nh.subscribe("cmd_vel", 10, &turtle_interface::cmd_callback, this);
            sensor_data_sub = nh.subscribe("sensor_data",10,&turtle_interface::sensor_data_callback,this);
            
            wheel_cmd_pub = nh.advertise<nuturtlebot_msgs::WheelCommands>("wheel_cmd",10);
            joint_states_pub = nh.advertise<sensor_msgs::JointState>("/joint_states",10);
            //create joint states for red robot
            jointStates.header.stamp = ros::Time::now();
            jointStates.position.push_back(0);
            jointStates.position.push_back(0);
            jointStates.velocity.push_back(0);
            jointStates.velocity.push_back(0);
            
            jointStates.name.push_back("red-wheel_left_joint");
            jointStates.name.push_back("red-wheel_right_joint");
            //create joint states for blue robot if wheels should move
            jointStates_blue.header.stamp = ros::Time::now();
            jointStates_blue.position.push_back(0);
            jointStates_blue.position.push_back(0);
            jointStates_blue.velocity.push_back(0);
            jointStates_blue.velocity.push_back(0);
            
            jointStates_blue.name.push_back("blue-wheel_left_joint");
            jointStates_blue.name.push_back("blue-wheel_right_joint");
            timer = nh.createTimer(ros::Duration(1/rate), &turtle_interface::main_loop, this);
        }

        /// \brief Receive twist values from cmd_vel to create wheel velocities
        ///
        /// \param data - A twist consisting of both linear and angular x,y,z components
        void cmd_callback(const geometry_msgs::Twist & data)
        {
            input_twist.theta_dot = data.angular.z;
            input_twist.x_dot = data.linear.x;
            wheel_vels = diffDrive.inverse_Kinematics(input_twist);       
        }

        /// \brief Receive the encoder information from the turtlebot3's wheels to calculate the wheel's velocities
        ///
        /// \param sd - encoder values for both the left and right wheels
        void sensor_data_callback(const nuturtlebot_msgs::SensorData & sd) 
        {
            //convert sensor data to velocities and angles
            wheel_angles.left_angle = sd.left_encoder * encoder_ticks_to_rad;
            wheel_angles.right_angle = sd.right_encoder * encoder_ticks_to_rad;
            wheel_velocitys.left_vel = (sd.left_encoder-old_left)* motor_cmd_to_radsec;
            wheel_velocitys.right_vel = (sd.right_encoder-old_right) * motor_cmd_to_radsec;
            old_left = sd.left_encoder;
            old_right = sd.right_encoder;
        }

        /// \brief A timer that continously publishes the wheel_commands and joint states
        ///
        void main_loop(const ros::TimerEvent &)
        {
            wheel_commands.left_velocity = wheel_vels.left_vel/motor_cmd_to_radsec;
            wheel_commands.right_velocity = wheel_vels.right_vel/motor_cmd_to_radsec;
            wheel_Commands = wheel_commands;
            //limit wheels commands
            if(wheel_Commands.left_velocity > motor_cmd_max_upper){
                wheel_Commands.left_velocity = motor_cmd_max_upper;
            }
            if(wheel_Commands.right_velocity > motor_cmd_max_upper){
                wheel_Commands.right_velocity = motor_cmd_max_upper;
            }
            if(wheel_Commands.left_velocity < motor_cmd_max_lower){
                wheel_Commands.left_velocity = motor_cmd_max_lower;
            }
            if(wheel_Commands.right_velocity < motor_cmd_max_lower){
                wheel_Commands.right_velocity = motor_cmd_max_lower;
            }

            //publish joint states and wheel commands
            jointStates.position[0] = (wheel_angles.left_angle);
            jointStates.position[1] = (wheel_angles.right_angle);
            jointStates.velocity[0] = (wheel_vels.left_vel);
            jointStates.velocity[1] = (wheel_vels.right_vel);
            jointStates.header.stamp = ros::Time::now();

            jointStates_blue.position[0] = (wheel_angles.left_angle);
            jointStates_blue.position[1] = (wheel_angles.right_angle);
            jointStates_blue.velocity[0] = (wheel_vels.left_vel);
            jointStates_blue.velocity[1] = (wheel_vels.right_vel);
            jointStates_blue.header.stamp = ros::Time::now();
            wheel_cmd_pub.publish(wheel_Commands);
            joint_states_pub.publish(jointStates);
        }
          
    private:
    //create private variables
    ros::NodeHandle nh;
    double wheel_radius;
    double track_width;
    double collision_radius;
    double motor_cmd_to_radsec;
    double encoder_ticks_to_rad;
    std::vector<double> motor_cmd_max;

    ros::Subscriber cmd_sub;
    ros::Subscriber sensor_data_sub;
    ros::Publisher wheel_cmd_pub;
    ros::Publisher joint_states_pub;

    turtlelib::speed wheel_velocity;
    turtlelib::DiffDrive diffDrive;
    std::vector<double> wheel_angle_vector;
    std::vector<double> wheel_velocity_vector;
    
    turtlelib::Twist2D input_twist;
    nuturtlebot_msgs::WheelCommands wheel_commands;
    nuturtlebot_msgs::WheelCommands wheel_Commands;
    turtlelib::speed wheel_vels;

    turtlelib::phi_angles wheel_angles;
    turtlelib::speed wheel_velocitys;
    sensor_msgs::JointState jointStates;
    sensor_msgs::JointState jointStates_blue;
    ros::Timer timer;

    int old_left;
    int old_right;

    double rate;
    double motor_cmd_max_lower;
    double motor_cmd_max_upper;
};


/// \brief the main function that calls the class

int main(int argc, char * argv[])
{
    ros::init(argc, argv, "turtle_interface");
    turtle_interface node;
    ros::spin();
    return 0;
}