
#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>

#include <tf/transform_listener.h>

#include <cstdlib>
#include <ctime>

#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>




int main(int argc, char** argv)
{
  ros::init( argc, argv, "now_pos" );
  ros::NodeHandle nh;
  ros::Publisher pose_pub = nh.advertise<geometry_msgs::PoseStamped>("now_pose", 10);

  while(ros::ok)
    {
      geometry_msgs::PoseWithCovarianceStampedConstPtr msg = ros::topic::waitForMessage<geometry_msgs::PoseWithCovarianceStamped>("amcl_pose");
  
      geometry_msgs::PoseStamped p;  
      p.pose = msg->pose.pose;
      p.header = msg->header;
      pose_pub.publish(p);
    }

  return 0;
}
