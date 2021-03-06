#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <actionlib/server/simple_action_server.h>
#include <move_base_msgs/MoveBaseAction.h>
#include "humans_msgs/GoalJudgeAction.h"
#include <std_msgs/String.h>
#include <tf/transform_listener.h>
#include "humans_msgs/HumanSrv.h"

#include <cstdlib>
#include <ctime>

#include <geometry_msgs/Pose.h>

using namespace std;
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MBClient;
//typedef actionlib::SimpleActionServer<humans_msgs::GoalJudgeAction> GJServer;

map<int, geometry_msgs::Pose> pose;

class GoalDecider
{
private:
  ros::NodeHandle n;

  ros::Subscriber goal_sub;
  ros::ServiceClient human_srv;

  MBClient ac;
  //GJServer as;
  std::string action_name;
  move_base_msgs::MoveBaseGoal goal;
  int pre_goal_id;
  int next_goal_id;
  humans_msgs::GoalJudgeActionFeedback feedback;
  humans_msgs::GoalJudgeActionResult result;

public:
  GoalDecider(std::string name) 
    : ac("move_base", true),
      //as(n, name, false),
      action_name(name)
  {
    goal_sub = n.subscribe("chatter", 1, &GoalDecider::callback, this);
    human_srv = n.serviceClient<humans_msgs::HumanSrv>("name_srv");
    ROS_INFO("Waiting for action server to start.");
    ac.waitForServer();
    ROS_INFO("Action server started, sending goal.");

    geometry_msgs::Pose tmp;

    tmp.position.x = 2.28;
    tmp.position.y = 13.5; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[0] = tmp;

    tmp.position.x = 2.28;
    tmp.position.y = 10.9; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[1] = tmp;

    tmp.position.x = -1.25;
    tmp.position.y = 10.4; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[2] = tmp;

    tmp.position.x = 0;
    tmp.position.y = 0; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[3] = tmp;

    tmp.position.x = 3.2;
    tmp.position.y = -0.367; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[4] = tmp;

    tmp.position.x = 3.24;
    tmp.position.y = -3.54; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[5] = tmp;


    //as.registerGoalCallback(boost::bind(&GoalDecider::goalCb, this));
    //as.registerPreemptCallback(boost::bind(&GoalDecider::preemptCb, this));

    //as.start();
  }

  ~GoalDecider()
  {
    pose.clear();
  }

  void callback(const std_msgs::String::ConstPtr& msg)
  {

    humans_msgs::HumanSrv seekhuman;
    humans_msgs::Person person;
    person.name = msg->data;
    seekhuman.request.src.face.persons.push_back( person );// sg->data;
    seekhuman.request.src.header.frame_id = "map";
    if( human_srv.call(seekhuman) )
      {
	//msg->data=okao_idを引数にして人物を検索し、その位置を取得する
	//決定したらgoal_setter()
	geometry_msgs::Pose tmp;

	tmp.position = seekhuman.response.dst.p;
	tmp.orientation.w = 1;

	cout<< seekhuman.response.dst << endl;

	goalSender( tmp );
      }
    else
      {
	//ゴールが実行中かどうかを調べる
	//もし実行中なら,return
	//もし実行中でなければ(preemptかな?)適当にゴールを選ぶ
	actionlib::SimpleClientGoalState state = ac.getState();
	if((state == actionlib::SimpleClientGoalState::ACTIVE))
	  {
	    return;
	  }
	else
	  {
	    //ゴールをランダムに決定する
	    //決定したらgoal_setter()
	    goalSender( randomGoal() );
	  }
      }
  }

  /*
  void goalCb()
  {
    //goalDecider(4);
  }

  void preemptCb()
  {
    ROS_INFO("%s: Preempted", action_name.c_str());
    as.setPreempted();
  }
  */

  //ゴール地点番号を与えられたら、ゴールをその地点番号に結びついた値にセットする
  void goalSender(geometry_msgs::Pose goal_pose)
  {

    //goal = goalDecicer( id );

    //ROS_INFO("goal sender : %d", id);
    //now_goal_id = id;
    goal.target_pose.pose = goal_pose;
    goal.target_pose.header.frame_id = "map";
    goal.target_pose.header.stamp = ros::Time::now();
    ac.sendGoal(goal,
		boost::bind(&GoalDecider::doneCb, this, _1, _2),
                MBClient::SimpleActiveCallback(),
                MBClient::SimpleFeedbackCallback());
  }


  //ゴール地点を決定する
  geometry_msgs::Pose randomGoal()
  {
    //pre_goal_id = input_id;
    //ROS_INFO("goal decider, pre_goal_id : %d", pre_goal_id);
    //int output_id;

    srand((unsigned)time(NULL));

    double prob = (double)rand()/RAND_MAX; 
    int id = 1;
    if( prob < 0.2 )
      {
	id = 1;
      }
    else if( prob > 0.2 && prob < 0.4 )
      {
	id = 2;
      }
    else if( prob > 0.4 && prob < 0.6 )
      {
	id = 3;
      }
    else if( prob > 0.6 && prob < 0.8 )
      {
	id = 4;
      }
    else
      {
	id = 5;
      }

    //next_goal_id = output_id;
    ROS_INFO("goal decider, next goal_id : %d", id);
    //goalSender( next_goal_id );
    return pose[id];
  }

  void doneCb(const actionlib::SimpleClientGoalState& state,
	      const move_base_msgs::MoveBaseResultConstPtr& result)
  {
    ROS_INFO("Goal");//  to Goal[%d] is [%s]", 
    //pre_goal_id, next_goal_id ,state.toString().c_str());
    //cout << "" << endl;
    //ここで、もし、成功以外なら、元の場所に戻る
    //もし成功したなら、選択肢を交換して選択

    /*
    if(!(state == actionlib::SimpleClientGoalState::SUCCEEDED))
      {
	//中断したら、さっきいたゴール地点に戻る。
	//goalDecider( pre_goal_id );

	callback(0);
      }
    else
      {
	//もし成功したら、再びゴール地点を探す
	//goalDecider( next_goal_id );
	callback(0);
      }
    */
  }

  void activeCb()
  {
    ROS_INFO("Goal just went active.");
  }

  void feedbackCb(const move_base_msgs::MoveBaseActionFeedbackConstPtr& feedback)
  {
    std::cout << feedback << std::endl; 
  }

};

int main(int argc, char** argv)
{
  ros::init( argc, argv, "goal_decider" );
  GoalDecider GDObj(ros::this_node::getName());
  //WObj.goalDecider(4);

  ros::spin();
  return 0;
}
