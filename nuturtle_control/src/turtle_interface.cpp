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
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <vector>
#include <cstdlib>

class turtle_interface
{
    public:
        turtle_interface() {
            nh.getParam("/wheel_radius",wheel_radius);
            nh.getParam("/track_width",track_width);
            nh.getParam("/collision_radius",collision_radius);
            nh.getParam("/motor_cmd_to_radsec",motor_cmd_to_radsec);
            nh.getParam("/encoder_ticks_to_rad",encoder_ticks_to_rad);
            nh.getParam("/motor_cmd_max",motor_cmd_max);

            // cmd_vel_sub = nh.subscribe("/cmd_vel", 1000, &turtle_interface::callback, this);
        }
          
    private:
    ros::NodeHandle nh;
    double wheel_radius;
    double track_width;
    double collision_radius;
    double motor_cmd_to_radsec;
    double encoder_ticks_to_rad;
    double motor_cmd_max;
    // ros::Subscriber cmd_vel_sub;

};


int main(int argc, char * argv[])
{
    ros::init(argc, argv, "turtle_interface");
    turtle_interface node;
    ros::spin();
    return 0;
}