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



class GoalDecider
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
    as_.registerGoalCallback( boost::bind( &GoalDecider::goalCB, this ) );
    as_.registerPreemptCallback( boost::bind( &GoalDecider::preemptCB, this ) );
    goal_pub_ = n.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 1);
    amcl_sub_ = n.subscribe( "amcl_pose", 1, &GoalDecider::robotPoseCB, this );
    people_srv_ = n.serviceClient<humans_msgs::HumanSrv>("okao_srv");
    as_.start();
  }

  ~GoalDecider( void )
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


    humans_msgs::HumanSrv hs;
    //検索するOkao_id
    hs.request.src.max_okao_id = goal_okao_id;
    
    if( people_srv_.call( hs ) )
      {
	ROS_INFO("OKAO_ID: %d exists in the database.", goal_okao_id);
	goal.pose.position = hs.response.dst.p; 
	goal.pose.orientation.z = 0;
	goal.pose.orientation.w = 1;
	//今みているかどうか
	now_looking = hs.response.n;

	//もし、現在見ているなら
	if( now_looking )
	  {
	    ROS_INFO("OKAO_ID: %d NOW LOOKING...", goal_okao_id);
	    double diff_x = fabs( pose_msg->pose.pose.position.x - goal.pose.position.x );
	    double diff_y = fabs( pose_msg->pose.pose.position.y - goal.pose.position.y );
	    if( diff_x < 1.0 && diff_y < 1.0 )
	      {
		as_.setSucceeded( result_ );
		ROS_INFO("GOAL!");
	      }
	  }
      }
    else
      {
	ROS_INFO("OKAO_ID: %d does not exist in the database.", goal_okao_id);
	//ここで、目的地をランダムに決定
	//goal = ?
	//だけど問題として、目的地を定めたら、中断が入るまで目的地に向かうというシステムがやはり必要だと思う
	//そして、その中断とは、目的の人物を発見したときに入れたい
	ROS_INFO("Going random position");
	//as_.setAborted();
      }

    //前回の目的地と比較するif
    goal_pub_.publish( goal );

  }

};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "goal_decider");
  GoalDecider GDObject( ros::this_node::getName() );
  ros::spin();
  return 0;
}
