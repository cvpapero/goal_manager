#include "ros/ros.h"
#include <geometry_msgs/Pose.h>
#include <sstream>


int main(int argc, char **argv)
{
  
  ros::init(argc, argv, "talker");
  ros::NodeHandle n;
  ros::Publisher chatter_pub = n.advertise<geometry_msgs::Pose>("goal_pose", 1000);


  geometry_msgs::Pose tmp;
  tmp.position.x = 0.0;
  tmp.position.y = 0.0; 
  tmp.orientation.z = 0;
  tmp.orientation.w = 1;
  
  chatter_pub.publish(tmp);

  //ros::Rate loop_rate(10);


  //int count = 0;
  /*
  while (ros::ok())
  {
  
    std_msgs::String msg;

    std::stringstream ss;
    ss << "hello world " << count;
    msg.data = ss.str();

    ROS_INFO("%s", msg.data.c_str());


    chatter_pub.publish(msg);

    ros::spinOnce();

    loop_rate.sleep();
    ++count;
  }
  */

  return 0;
}
