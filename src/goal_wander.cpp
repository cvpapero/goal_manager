#include <ros/ros.h>
#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/PoseStamped.h>
#include <std_msgs/String.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>

#include "humans_msgs/Humans.h"
#include "humans_msgs/HumanSrv.h"

#include "okao_client/OkaoName.h"

using namespace std;
map<int, geometry_msgs::Pose> pose;


class GoalWander
{
private:
  ros::NodeHandle n;
  //ros::Subscriber goal_sub;
  ros::Publisher goal_pub;
  ros::ServiceClient people_srv_;
  ros::ServiceServer goal_srv_;
  geometry_msgs::PoseStamped goal_pose;

public:
  GoalWander()
  {
    //goal_sub = 
    //  n.srbscriber("chatter", 1, &GoalWander::callback, this);

    goal_pub =    
      n.advertise<geometry_msgs::PoseStamped>("move_base_simple/goal",10);
    
    people_srv_ = 
      n.serviceClient<humans_msgs::HumanSrv>("okao_srv");

    goal_srv_ = 
      n.advertiseService("goal_req", 
    			 &GoalWander::goal_req, this);

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


  }
  ~GoalWander()
  {
    pose.clear();
  }


  bool goal_req(okao_client::OkaoName::Request &req,
		okao_client::OkaoName::Response &res)
  {
    
    humans_msgs::HumanSrv hs;
    humans_msgs::Person sp;
    hs.request.src.max_okao_id = req.okao_id;
    //hs.request.src.face.persons.push_back(sp); 
    if( people_srv_.call(hs) )
      {
	goal_pose.pose.position = hs.response.dst.p;
	goal_pose.pose.orientation.w = 1;
	goal_pose.header.stamp = ros::Time::now();
	goal_pose.header.frame_id = "map";
	
	cout<< req.okao_id << " send goal" << endl;
	goal_pub.publish(goal_pose);
	return true;
      }
    else
      {
	cout<< req.okao_id << " has not database" << endl;
	return false;
      }
  }

  //ゴール地点を決定する
  geometry_msgs::Pose goal_decider()
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

  }


  //名前まち
  void goal_sender()
  {
    cout << "now sender"<<endl;
    //std_msgs::StringConstPtr name 
    //  = ros::topic::waitForMessage<std_msgs::String>("chatter");
    //name->dataに値が入ってるなら、その名前をキーにして
    //人物位置を取得し、それをパブリッシュする

    //値が入ってなかったら、ゴール状態を確かめる
    //目的地についていたら(true)、新しい目的を決定する
    //もし目的地についていなかったらスルー
    if( goal_judge() )
      {
	cout << "send to new goal" <<endl;
	//geometry_msgs::PoseStamped goal_pose;
	goal_pose.pose = goal_decider();
	goal_pose.pose.orientation.w = 1;
	goal_pose.header.stamp = ros::Time::now();
	goal_pose.header.frame_id = "map";
	
	goal_pub.publish( goal_pose );
      }
    else
      {
	cout << "I'm going to goal" <<endl;
      }
  }
 

  //ロボットが目的地についたかどうかを判定する
  bool goal_judge()
  {
    geometry_msgs::PoseWithCovarianceStampedConstPtr amcl_pose 
      = ros::topic::waitForMessage<
	geometry_msgs::PoseWithCovarianceStamped>("amcl_pose"); 
    
    geometry_msgs::PoseStamped robot_pose;
    robot_pose.pose = amcl_pose->pose.pose;

    //double mx =  msg->pose.pose.position.x;
    //double my =  msg->pose.pose.position.y;

    float xdiff 
      = fabs( goal_pose.pose.position.x - robot_pose.pose.position.x);
    float ydiff 
      = fabs( goal_pose.pose.position.y -  robot_pose.pose.position.y);
    if( (xdiff < 1.0) && (ydiff < 1.0) )
      {
	cout << "goal true" << endl;
	return true;
      }
    cout << "goal false" << endl;    
    return false;
  }



};


int main(int argc, char** argv)
{
  ros::init(argc, argv, "goal_wander");
  GoalWander GWObj;
  while(ros::ok)
    {
      GWObj.goal_sender();
    }
  //ros::spin();
  return 0;
}
