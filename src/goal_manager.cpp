#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include "humans_msgs/GoalJudgeAction.h"

typedef actionlib::SimpleActionClient<humans_msgs::GoalJudgeAction> GJClient;


void doneCB()
{

}

void activeCB()
{

}

void feedbackCB()
{

}


int main(int argc, char** argv)
{
  ros::init(argc, argv, "goal_manager");

  GJClient ac("goal_judge", true);

  ac.waitForServer();

  humans_msgs::GoalJudgeGoal goal;
  goal.okao_id = 8;
  ac.sendGoal(goal);

  //actionlib::SimpleClientGoalState state = ;


  ros::Rate loop_rate(1);
  while( ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED )
    {
      ROS_INFO("now goal state :%s", ac.getState().toString().c_str());
      loop_rate.sleep();
    }
  //ros::spin();
  return 0;
}
