#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>

#include <tf/transform_listener.h>

#include <cstdlib>
#include <ctime>

#include <geometry_msgs/Pose.h>

using namespace std;
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MBClient;

/*
class GoalPose
{
public:
  double px;
  double py;
  double qz;
  double qw;
}
*/

map<int, geometry_msgs::Pose> pose;

class Wander
{
private:
  MBClient ac;
  move_base_msgs::MoveBaseGoal goal;
  int pre_goal_id;
  int next_goal_id;
  //double pass[6][6]; 

public:
  Wander() : ac("move_base", true)
  {
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


  }

  ~Wander()
  {
    pose.clear();
  }

  //ゴール地点番号を与えられたら、ゴールをその地点番号に結びついた値にセットする
  void goalSender(int id)
  {
    /*
    ROS_INFO("rotation start");
    tf::TransformListener tfl(ros::Duration(10));
    costmap_2d::Costmap2DROS global_costmap("grobal_costmap", tfl);
    costmap_2d::Costmap2DROS local_costmap("local_costmap", tfl);

    rotate_recovery::RotateRecovery rr;
    rr.initialize("my_rotate_recovery", &tfl, &global_costmap, &local_costmap);
    rr.runBehavior();
    ROS_INFO("rotation end");
    */
    //goal = goalDecicer( id );

    ROS_INFO("goal sender : %d", id);
    //now_goal_id = id;
    goal.target_pose.pose = pose[ id ];
    goal.target_pose.header.frame_id = "map";
    goal.target_pose.header.stamp = ros::Time::now();
    ac.sendGoal(goal,
		boost::bind(&Wander::doneCb, this, _1, _2),
                MBClient::SimpleActiveCallback(),
		//boost::bind(&Wander::feedbackCb, this, _1));
                MBClient::SimpleFeedbackCallback());

    //boost::bind(&Wander::activeCb, this),
    //		boost::bind(&Wander::feedbackCb, this, _1));
  }


  //ゴール地点を決定する
  void goalDecider(int input_id)
  {
    pre_goal_id = input_id;
    ROS_INFO("goal decider, pre_goal_id : %d", pre_goal_id);
    int output_id;

    srand((unsigned)time(NULL));

    switch( pre_goal_id )
      {
	/*
      case 0:
	output_id = 1;
	break;
	*/
      case 1:
	/*
	if((double)rand()/RAND_MAX < 1./3.)
	  {
	    output_id = 0;
	    }*/
	if((double)rand()/RAND_MAX > 1./2.)
	  {
	    output_id = 2;
	  }
	else
	  {
	    output_id = 3;
	  }
	break;
      case 2:
	if((double)rand()/RAND_MAX < 1./2.)
	  {
	    output_id = 1;
	  }
	else 
	  {
	    output_id = 4;
	  }
	break;
      case 3:
	if((double)rand()/RAND_MAX < 1./3.)
	  {
	    output_id = 1;
	  }
	else if((double)rand()/RAND_MAX > 2./3.)
	  {
	    output_id = 4;
	  }
	else
	  {
	    output_id = 5;
	  }
	break;
      case 4:
	if((double)rand()/RAND_MAX < 1./2.)
	  {
	    output_id = 2;
	  }
	else 
	  {
	    output_id = 3;
	  }
	break;
      case 5:
	output_id = 3;
	break;
      default:
	break;
      }
    next_goal_id = output_id;
    ROS_INFO("goal decider, next_goal_id : %d",next_goal_id);
    goalSender( next_goal_id );
  }

  void doneCb(const actionlib::SimpleClientGoalState& state,
	      const move_base_msgs::MoveBaseResultConstPtr& result)
  {
    ROS_INFO("Goal[%d] to Goal[%d] is [%s]", pre_goal_id, next_goal_id ,state.toString().c_str());
    //ここで、もし、成功以外なら、元の場所に戻る
    //もし成功したなら、選択肢を交換して選択
    if(!(state == actionlib::SimpleClientGoalState::SUCCEEDED))
      {
	//中断したら、さっきいたゴール地点に戻る。
	goalDecider( pre_goal_id );
      }
    else
      {
	//もし成功したら、再びゴール地点を探す
	goalDecider( next_goal_id );
      }
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
  ros::init( argc, argv, "wander" );
  Wander WObj;
  WObj.goalDecider(4);

  ros::spin();
  return 0;
}
