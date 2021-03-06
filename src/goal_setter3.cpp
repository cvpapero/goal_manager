/*
2015.1.7---------------------
actionlib


*/

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include "humans_msgs/HumanSrv.h"
#include <std_msgs/String.h>

typedef actionlib::SimpleActionClient<
  move_base_msgs::MoveBaseAction> MoveBaseClient;

using namespace std;

class GoalSetter
{
private: 
  ros::NodeHandle n;
  ros::Subscriber g_sub;
  MoveBaseClient ac;
  geometry_msgs::PoseStamped m_goal;
  ros::ServiceClient human_srv;

public:
  GoalSetter()
    :ac("move_base", true)
  {
    ROS_INFO("Waiting for action server to start.");
    ac.waitForServer();
    ROS_INFO("Action server started, sending goal.");

    g_sub = n.subscribe("chatter", 1, &GoalSetter::callback, this);
    human_srv = n.serviceClient<humans_msgs::HumanSrv>("name_srv");

    // m_goal.pose.position.x = 0;
    //m_goal.pose.position.y = 0;
  }

  void callback(const std_msgs::StringConstPtr& goalName)
  {
    std::cout << goalName->data << std::endl;
    humans_msgs::HumanSrv hs;
    humans_msgs::Person person;
    person.name = goalName->data;
    hs.request.src.header.frame_id = "map";
    hs.request.src.face.persons.push_back( person );
    if( human_srv.call( hs ) )
      {
	move_base_msgs::MoveBaseGoal goal;
	goal.target_pose.header.frame_id = "map";
	goal.target_pose.header.stamp = ros::Time::now();

	goal.target_pose.pose.position = hs.response.dst.p;
	goal.target_pose.pose.orientation.w = 1;
 
	ac.sendGoal( goal );
      }
    else
      {
	ROS_INFO("no such name: %s", goalName->data.c_str());
      }

  }
};


int main(int argc, char** argv)
{
  ros::init(argc, argv, "goal_setter");
  GoalSetter GSObject;
  ros::spin();

  return 0;
}
