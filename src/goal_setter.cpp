/*
2015.1.7---------------------
actionlib
うーん、よく考えたらpub/sub形式でも良くない？って思ったよ
なぜなら、その方が即効で問題が解決しそうだし、
そもそも、この目的が達成されたかどうかを知ることは、あんまり意味がない
とりあえずパブリッシュさえすればいいのだから

だけど、ゴール地点は送ったけど、ロボットが動いてない場合とか、あるよね
その場合、どうする？
もし、繰り返しおなじような目的地が送られてきたらそれを無視する、、、っていう処理をしているのだと、
いつまでも動けなくなっちゃわない？
だから、無視する条件は
1.ロボットが目的を実行中であり
2.似ているとき

分別したいのは、まず、
ゴール地点が前回と類似しているか
1.類似していなければsend
2.類似していても、GOAL STATUSが
REJECTED, RECALLED, PREEMPTED, ABORTEFDのいずれならsend

ん、でもまてよ、ゴール状態を受けとるだけなら別にいつだって可能なんじゃないか？
そうだよ、コールバック関数が呼び出されたら、ゴールの状態を問い合わせればええやん！
さすが～ROS

ちょっと疲れたし朝早いので帰るお

*/

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>

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

public:
  GoalSetter()
    :ac("move_base", true)
  {
    ROS_INFO("Waiting for action server to start.");
    ac.waitForServer();
    ROS_INFO("Action server started, sending goal.");

    g_sub = n.subscribe("/goal", 1, &GoalSetter::callback, this);

    m_goal.pose.position.x = 0;
    m_goal.pose.position.y = 0;
  }


  bool judging_valid_goal( geometry_msgs::PoseStamped srca, geometry_msgs::PoseStamped srcb )
  {
    double diff_x = fabs( srca.pose.position.x - srcb.pose.position.x );
    double diff_y = fabs( srca.pose.position.y - srcb.pose.position.y );
    if( diff_x > 1.0 && diff_y > 1.0 )
      {
	return true;
      }
    else
      {
	return false;
      }
  }

  geometry_msgs::PoseStamped setting_goal( geometry_msgs::PoseStamped src )
  {

    geometry_msgs::PoseStamped dst;
    dst.header.frame_id = "map";
    dst.header.stamp = ros::Time::now();
    dst.pose = src.pose;

    return dst;
  }

  void callback(const geometry_msgs::PoseStamped& t_goal)
  {

    move_base_msgs::MoveBaseGoal goal;

    //目的地が前回と類似しているかどうかのチェック(関数)
    if(judging_valid_goal(t_goal, m_goal))
      {
	goal.target_pose = m_goal = setting_goal( t_goal );
	ac.sendGoal( goal );
	ROS_INFO("send goal(x, y) = (%f, %f)", m_goal.pose.position.x, m_goal.pose.position.y);
      }
    else
      {
	//cout << ac.getState() << endl;
	//ここで、ロボットに与えられた指示がどうなっているのかを取得する
	if(ac.getState() == actionlib::SimpleClientGoalState::REJECTED ||
	   ac.getState() == actionlib::SimpleClientGoalState::RECALLED || 
	   ac.getState() == actionlib::SimpleClientGoalState::PREEMPTED || 
	   ac.getState() == actionlib::SimpleClientGoalState::ABORTED)
	  {
	    goal.target_pose = m_goal = setting_goal( t_goal );
	    ac.sendGoal( goal );
	    ROS_INFO("send goal(x, y) = (%f, %f)", m_goal.pose.position.x, m_goal.pose.position.y);
	  }
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
