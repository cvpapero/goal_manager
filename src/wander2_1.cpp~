#include <ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>

#include <tf/transform_listener.h>

#include <cstdlib>
#include <ctime>

#include <geometry_msgs/Pose.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>

using namespace std;
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MBClient;


map<int, geometry_msgs::Pose> pose;

class Wander
{
private:
  ros::NodeHandle n;
  MBClient ac;
  move_base_msgs::MoveBaseGoal goal;
  ros::Subscriber goal_sub;
  //int pre_goal_id;
  //int next_goal_id;
  //double pass[6][6]; 

public:
  Wander() : ac("move_base", true)
  {
    ROS_INFO("Waiting for action server to start.");
    ac.waitForServer();
    ROS_INFO("Action server started, sending goal.");

 

    geometry_msgs::Pose tmp;
    tmp.position.x = 0.0;
    tmp.position.y = 0.0; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[0] = tmp;

    tmp.position.x = -1.08;
    tmp.position.y = 5.77; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[1] = tmp;

    tmp.position.x = 0.0;
    tmp.position.y = 10.4; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[2] = tmp;

    tmp.position.x = 2.83;
    tmp.position.y = 4.84; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[3] = tmp;

    /*
    tmp.position.x = 2.83;
    tmp.position.y = 4.84; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[4] = tmp;

    tmp.position.x = 3.24;
    tmp.position.y = -3.54; 
    tmp.orientation.z = 0;
    tmp.orientation.w = 1;
    pose[5] = tmp;
    */

  }

  ~Wander()
  {
    pose.clear();
  }

  //ゴール地点を決定する
  geometry_msgs::Pose goalDecider()
  {
    //pre_goal_id = input_id;
    //ROS_INFO("goal decider, pre_goal_id : %d", pre_goal_id);
    int output_id;

    srand((unsigned)time(NULL));

    if((double)rand()/RAND_MAX < 1./4.)
      {
	output_id = 0;
      }
    else if((double)rand()/RAND_MAX > 1./4. && (double)rand()/RAND_MAX < 2./4.)
      {
	output_id = 1;
      }
    else if((double)rand()/RAND_MAX > 2./4. && (double)rand()/RAND_MAX < 3./4.)
      {
	output_id = 2;
      }
    else 
      {
	output_id = 3;
      }

    return pose[ output_id ];
    //ROS_INFO("goal decider, next_goal_id : %d",next_goal_id);

    //goalSender( next_goal_id );
  }

  //ゴール地点番号を与えられたら、ゴールをその地点番号に結びついた値にセットする
  void goalSender(geometry_msgs::Pose goal_pose)
  {

    ROS_INFO("goal send to ");
    cout<< goal_pose <<endl;
    //now_goal_id = id;
    goal.target_pose.pose = goal_pose;
    goal.target_pose.header.frame_id = "map";
    goal.target_pose.header.stamp = ros::Time::now();
    ac.sendGoal(goal,
		boost::bind(&Wander::doneCb, this, _1, _2),
		boost::bind(&Wander::activeCb, this),
		boost::bind(&Wander::feedbackCb, this, _1));
		//MBClient::SimpleFeedbackCallback());

    //boost::bind(&Wander::activeCb, this),
    //		boost::bind(&Wander::feedbackCb, this, _1));
  }

  void callback(const geometry_msgs::PoseConstPtr& msg)
  {
    /*0.現在のゴール状態を取得、もしactive以外なら以下実行
    1.現在の姿勢状態を取得
    2.姿勢をyawに変換
    

    目的地を決める
    自分の位置を見る
    ゴール地点の決定
    */
    /*
    if( (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) 
	|| (ac.getState() == actionlib::SimpleClientGoalState::ABORTED) )
      {
	goalSender( goalDecider() );
      }
    */
    //if(!msg->pose)

    geometry_msgs::Pose pose;
    pose.position = msg->position;
    pose.orientation = msg->orientation;
    if( pose.orientation.w )
      {
	cout<< "goal sub"<<endl;
	goalSender( pose );
      }
  }
  
  void doneCb(const actionlib::SimpleClientGoalState& state,
	      const move_base_msgs::MoveBaseResultConstPtr& result)
  {
    ROS_INFO("Goal is [%s]",state.toString().c_str());
    //ここで、もし、成功以外なら、元の場所に戻る
    //もし成功したなら、選択肢を交換して選択

    goalSender( goalDecider() );

  }
  
  void activeCb()
  {
    ROS_INFO("Goal just went active.");
  }

  void feedbackCb(const move_base_msgs::MoveBaseFeedbackConstPtr& feedback)
  {
    std::cout << "feedback: " <<feedback << std::endl; 

    //ros::goal_sub = n.subscribe("goal_pose", 1, &Wander::callback, this);
    geometry_msgs::PoseConstPtr msg = ros::topic::waitForMessage<geometry_msgs::Pose>("goal_pose");
    geometry_msgs::Pose pose;
    pose.position = msg->position;
    pose.orientation = msg->orientation;
    if( pose.orientation.w )
      {
	cout<< "goal sub"<<endl;
	goalSender( pose );
      }
  }

};

int main(int argc, char** argv)
{
  ros::init( argc, argv, "wander" );
  Wander WObj;
  WObj.goalSender(WObj.goalDecider());

  ros::spin();
  return 0;
}
