
#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include "humans_msgs/HumanSrv.h"
#include <std_msgs/Int32.h>

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
    //ac.waitForServer();
    ROS_INFO("Action server started, sending goal.");

    g_sub = n.subscribe("chatter", 1, &GoalSetter::callback, this);
    human_srv = n.serviceClient<humans_msgs::HumanSrv>("okao_srv");

    // m_goal.pose.position.x = 0;
    //m_goal.pose.position.y = 0;
  }

  void callback(const std_msgs::Int32ConstPtr& goalId)
  {
    std::cout << goalId->data << std::endl;
    humans_msgs::HumanSrv hs;
    humans_msgs::Person person;
    //person. = goalId->data;
    hs.request.src.header.frame_id = "map";
    hs.request.src.max_okao_id = goalId->data; 
    hs.request.src.face.persons.push_back( person );
    if( human_srv.call( hs ) )
      {
	std::cout << "Go to " << goalId->data << std::endl;
	std::cout << "Go to " << hs.response.dst << std::endl;
	move_base_msgs::MoveBaseGoal goal;
	goal.target_pose.header.frame_id = "map";
	goal.target_pose.header.stamp = ros::Time::now();

	goal.target_pose.pose.position = hs.response.dst.p;
	goal.target_pose.pose.orientation.w = 1;
 
	ac.sendGoal( goal );
      }
    else
      {
	ROS_INFO("no such name: %d", goalId->data);
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
