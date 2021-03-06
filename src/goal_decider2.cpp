/*
2015.1.13----------
actionlib serverの内部にactionlib clientを作成する
目的地移動モード
ランダム走行モード
姿勢変更モード

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
#include <actionlib/client/simple_action_client.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>

#include <move_base_msgs/MoveBaseAction.h>

#include "humans_msgs/GoalJudgeAction.h"
#include "humans_msgs/HumanSrv.h"


typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> Runner;
//typedef actionlib::SimpleActionClient<geometry_msgs::PoseStamped> GoalRunner;
//typedef actionlib::SimpleActionClient<geometry_msgs::PoseStamped> PoseRunner;

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

  //ros::Publisher goal_pub_;
  ros::Subscriber amcl_sub_;
  ros::ServiceClient people_srv_;
  geometry_msgs::PoseStamped goal;
  geometry_msgs::PoseStamped pre_goal;

  bool now_looking;
  bool now_random_walking = false;
  ros::Time ts;

  Runner ac;

public:
  GoalDecider( string name )
    :as_( n, name, false ),
     action_name_( name ),
     ac("move_base", true)
  {
    as_.registerGoalCallback( boost::bind( &GoalDecider::goalCB, this ) );
    as_.registerPreemptCallback( boost::bind( &GoalDecider::preemptCB, this ) );
    //goal_pub_ = n.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal", 1);
    //amcl_sub_ = n.subscribe( "amcl_pose", 1, &GoalDecider::robotPoseCB, this );
    people_srv_ = n.serviceClient<humans_msgs::HumanSrv>("okao_srv");
    as_.start();

    //Runner ac("move_base", true);
    ac.waitForServer();
    ROS_INFO("Action server started, sending goal.");
  }

  ~GoalDecider( void )
  {
  }

  //Called once when the goal completes
  void doneCB(const actionlib::SimpleClientGoalState& state,
	      const move_base_msgs::MoveBaseResultConstPtr& result)
  {

  }

  //Called once when the goal becomes goal
  void activeCB()
  {
    now_random_walking = false;
  }

  //Called every time feedback is received for the goal
  void feedbackCB(const move_base_msgs::MoveBaseFeedbackConstPtr& feedback)
  {
    //ここで現在のロボットの位置と、受け取った人物の位置(あるなら)を比較する
    cout << feedback->base_position << endl;

    /*
      1.タイムスタンプtsを見る(5秒以内かどうか)
      2.ロボットと目標位置を見る(1メートル四方かどうか？)
      もし、上の二つを満たしていたら、as_setSuccess();
     */


  }

  geometry_msgs::PoseStamped RandomPosition()
  {
    geometry_msgs::PoseStamped out;
    
    //fill in out
    
    return out;
  }

  void GoalRunner( geometry_msgs::PoseStamped src )
  {
    cout << "now goal runner" << endl; 
    ts = src.header.stamp;

    double diff_x = fabs( src.pose.position.x - pre_goal.pose.position.x );
    double diff_y = fabs( src.pose.position.y - pre_goal.pose.position.y );
    //もし、先ほどとゴールが変わっていたら,ゴール地点の変更
    if( (diff_x > 1.0) && (diff_y > 1.0) )
      {
	cout << "goal update!" <<endl;
	move_base_msgs::MoveBaseGoal g;
	
	//fill in goal
	g.target_pose = src;
	pre_goal = src;
	//ac.waitForSendCansel();	
	ac.sendGoal(g, 
		    boost::bind(&GoalDecider::doneCB, this, _1, _2),
		    boost::bind(&GoalDecider::activeCB, this),
		    boost::bind(&GoalDecider::feedbackCB, this, _1));
      }
  }

  void RandomRunner()
  {

  }



  bool search_from_database(int id, geometry_msgs::PoseStamped* pose)
  {
    humans_msgs::HumanSrv hs;
    //検索するOkao_id
    hs.request.src.max_okao_id = id;
    if( people_srv_.call( hs ) )
      {
	//pose = hs.response.dst;
	pose->header = hs.response.dst.header;
	pose->pose.position = hs.response.dst.p;
	pose->pose.orientation.z = 0;
	pose->pose.orientation.w = 1;
	return true;
      }
    return false;
  }

  bool judge_goal(geometry_msgs::PoseStamped robo, geometry_msgs::PoseStamped db)
  {

    //距離&&時間 = t ならtrue
    //
    ros::Duration diff_t = robo.header.stamp - db.header.stamp;
    double diff_x = fabs( robo.pose.position.x - db.pose.position.x );
    double diff_y = fabs( robo.pose.position.y - db.pose.position.y );

    if( (diff_t < ros::Duration(10)) && (diff_x < 2.0) && (diff_y < 2.0) )
      {
	return true;
      }

    return false;
  }

  void goalCB()
  {
    goal_okao_id = as_.acceptNewGoal()->okao_id;

    // move_base_msgs::MoveBaseGoal goal;
    ros::Rate loop_rate(1);

    while(ros::ok())
      {
	/*
	  このループ内で行うことは
	  1 DBからAを探す関数
	  1.1 Aを見つけた→goalRunner(x,y)
	  1.2 Aを見つけなかった→randomRunner()
	  2 ロボットとAの距離を判定する関数
	  2.1 真なら→setSucceed して break
	  2.2 偽ならループ継続
	*/
	geometry_msgs::PoseStamped db_pose;
	
	if( search_from_database(goal_okao_id, &db_pose) )
	  {
	    GoalRunner( db_pose );
	  }
	else
	  {
	    RandomRunner();
	  }

	//amcl_poseのサブスクライブ
	geometry_msgs::PoseWithCovarianceStampedConstPtr amcl_pose 
	  = ros::topic::waitForMessage<
	    geometry_msgs::PoseWithCovarianceStamped>("amcl_pose"); 

	geometry_msgs::PoseStamped robot_pose;
	robot_pose.pose = amcl_pose->pose.pose;
	robot_pose.header.stamp = ros::Time::now();

	if( judge_goal(robot_pose, db_pose) )
	  {
	    as_.setSucceeded();
	    break;
	  }
	
	loop_rate.sleep();
      }

  }

  void preemptCB()
  {
    ROS_INFO("%s: Preempted", action_name_.c_str());
    as_.setPreempted();
  }

  /*
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

	ROS_INFO("OKAO_ID: %d NOW LOOKING...", goal_okao_id);
	double diff_x = fabs( pose_msg->pose.pose.position.x - goal.pose.position.x );
	double diff_y = fabs( pose_msg->pose.pose.position.y - goal.pose.position.y );
	//もし、現在見ているなら
	//タイムスタンプが何秒以内？
	if( now_looking )
	  {
	    if( diff_x < 1.0 && diff_y < 1.0 )
	      {
		
		as_.setSucceeded( result_ );
		ROS_INFO("GOAL!");
	      }
	    else
	      {
		//PoseRunner();
		GoalRunner( goal );
	      }
	  }
	else
	  {
	    GoalRunner( goal );
	  }
      }
    else
      {
	ROS_INFO("OKAO_ID: %d does not exist in the database.", goal_okao_id);
	//ここで、目的地をランダムに決定して動く
	if( !now_random_walking )
	  {
	    RandomRunner();
	    now_random_walking = true;
	  }
	ROS_INFO("Going random position");
	//as_.setAborted();
      }
  }
  */
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "goal_decider");
  GoalDecider GDObject( ros::this_node::getName() );
  ros::spin();
  return 0;
}
