/*
2015.1.22------------
目的地を指定する

*/

#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <actionlib/server/simple_action_server.h>
#include <move_base_msgs/MoveBaseAction.h>
#include "humans_msgs/GoalJudgeAction.h"

#include <tf/transform_listener.h>


#include <cstdlib>
#include <ctime>

#include <geometry_msgs/Pose.h>

using namespace std;
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MBClient;
typedef actionlib::SimpleActionServer<move_base_msgs::MoveBaseAction> MBServer;


class GoalSetter
{
private:
  ros::NodeHandle n;
  MBClient ac;
  MBServer as;
  std::string action_name;
  move_base_msgs::MoveBaseGoal goal;
  int pre_goal_id;
  int next_goal_id;
  move_base_msgs::MoveBaseActionFeedback feedback;
  move_base_msgs::MoveBaseActionResult result;

public:
  GoalSetter(std::string name) 
    : ac("move_base", true),
      as(n, name, boost::bind(&GoalSetter::executeCb, this, _1), false),
      action_name(name)
  {
    ROS_INFO("Waiting for action server to start.");
    ac.waitForServer();
    ROS_INFO("Action server started, sending goal.");

    as.start();
  }

  ~GoalSetter()
  {

  }

  void executeCb(const move_base_msgs::MoveBaseGoalConstPtr &goal_pose)
  {
    ROS_INFO("goal sender : %d", id);
    //now_goal_id = id;
    goal.target_pose = goal_pose;
    goal.target_pose.header.frame_id = "map";
    goal.target_pose.header.stamp = ros::Time::now();
    ac.sendGoal(goal,
		boost::bind(&Wander::doneCb, this, _1, _2),
                MBClient::SimpleActiveCallback(),
		//boost::bind(&Wander::feedbackCb, this, _1));
                MBClient::SimpleFeedbackCallback());
  }

  void doneCb(const actionlib::SimpleClientGoalState& state,
	      const move_base_msgs::MoveBaseResultConstPtr& result)
  {
    ROS_INFO("Goal[%d] to Goal[%d] is [%s]", 
	     pre_goal_id, next_goal_id ,state.toString().c_str());

    if(!(state == actionlib::SimpleClientGoalState::SUCCEEDED))
      {

      }
    else
      {
	state = actionlib::SimpleClientGoalState::SUCCEEDED);
	as.setSucceeded(result);
      }
  }

};

int main(int argc, char** argv)
{
  ros::init( argc, argv, "wander_action" );
  GoalSetter GSObj(ros::this_node::getName());
  //WObj.goalDecider(4);

  ros::spin();
  return 0;
}
