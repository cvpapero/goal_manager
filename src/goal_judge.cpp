/*
2015.1.10----------
やること
actionlib serverであること
内部でreq/resできるようにすること
amclからの自己位置をサブスクライブできるようにすること

目的地をパブリッシュすること



*/

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <actionlib/server/simple_action_server.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include "humans_msgs/GoalJudgeAction.h"
#include "humans_msgs/HumanSrv.h"

//actionのヘッダをインクルードする。どういう形になるかは、書いてみないと分からん

using namespace std;

class GoalJudge
{
private:
  ros::NodeHandle n;
  actionlib::SimpleActionServer<humans_msgs::GoalJudgeAction> as_;
  //actionlibにつかういろいろ
  string action_name_;
  string goal_name;
  int goal_okao_id;

  humans_msgs::GoalJudgeFeedback feedback_;
  humans_msgs::GoalJudgeResult result_;

  ros::Publisher goal_pub_;
  ros::Subscriber amcl_sub_;
  ros::ServiceClient people_srv_;
  geometry_msgs::PoseStamped goal;

  bool now_looking;


public:
  GoalJudge( string name )
    :as_( n, name, false ),
     action_name_( name )
  {
    as_.registerGoalCallback( boost::bind( &GoalJudge::goalCB, this ) );
    as_.registerPreemptCallback( boost::bind( &GoalJudge::preemptCB, this ) );
    goal_pub_ = n.advertise<geometry_msgs::PoseStamped>("goal",1);
    amcl_sub_ = n.subscribe( "amcl_pose", 1, &GoalJudge::robotPoseCB, this );
    people_srv_ = n.serviceClient<humans_msgs::HumanSrv>("okao_srv");
    as_.start();
  }

  ~GoalJudge( void )
  {
  }

  void goalCB()
  {
    //ここで目的を記述していく
    //つぎに、Aの位置をreq/resする
    //受け取った情報から、データを更新する
    //今みていたらtrue, みてなかったらfalseになる変数をどっかに用意しておく

    goal_okao_id = as_.acceptNewGoal()->okao_id;

    //name to okao_id;

    humans_msgs::HumanSrv hs;

    hs.request.src.max_okao_id = goal_okao_id;

    if( people_srv_.call( hs ) )
      {
	goal.pose.position = hs.response.dst.p; 
	goal.pose.orientation.z = 0;
	goal.pose.orientation.w = 1;
	//今みているかどうか
	now_looking = hs.response.n;
      }
    else
      {
	ROS_INFO("Such name does not exist in the database.");
	as_.setAborted();
      }
  }

  void preemptCB()
  {
    ROS_INFO("%s: Preempted", action_name_.c_str());
    as_.setPreempted();
  }

  void robotPoseCB(const geometry_msgs::PoseWithCovarianceStampedConstPtr& pose_msg)
  {
    //ロボットの位置がサブスクライブされる度にコールバックされる
    //内部のgoal(x, y)をみていく
    //また。goal_setterに目的位置をpubする

    //ロボットの位置と目的地と誤差1メートル四方であり、かつ、
    //現在、どれかのセンサが人物をみていれば、goalに到達したと判断

    goal_pub_.publish( goal );

    double diff_x = fabs( pose_msg->pose.pose.position.x - goal.pose.position.x );
    double diff_y = fabs( pose_msg->pose.pose.position.y - goal.pose.position.y );

    if( diff_x < 1.0 && diff_y < 1.0 )
      {
	if( now_looking )
	  { 
	    as_.setSucceeded( result_ );
	  }
      }
  }

};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "goal_judge");
  GoalJudge GJObject( ros::this_node::getName() );
  ros::spin();
  return 0;
}
