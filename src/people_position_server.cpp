/*
2015.1.9----------------

今は、ruleに入ってるキーワードを判定した条件分岐で、検索の処理をしている
つまり、キーワードの数だけ条件分岐がいる

 
*/

#include <ros/ros.h>
//#include <geometry_msgs/PointStamped.h>

#include <map>
#include <string>
#include <sstream>
#include <iostream>
//#include <time.h>

//オリジナルメッセージ
#include "humans_msgs/Humans.h"
#include "humans_msgs/HumanSrv.h"
#include "humans_msgs/HumansSrv.h"
//#include "okao_client/OkaoStack.h"

using namespace std;

//map<int, humans_msgs::Human> n_DBHuman;
//map<int, humans_msgs::Human> p_DBHuman;
map<int, humans_msgs::Human> o_DBHuman;

class PeoplePositionServer
{
private:
  ros::NodeHandle n;
  ros::Subscriber rein_sub_;
  ros::ServiceServer track_srv_;
  ros::ServiceServer okao_srv_;
  ros::ServiceServer array_srv_;

  //人物の初期化をする
  //humans_msgs::Human init;
  //o_DBHuman[1] = 


public:
  PeoplePositionServer()
  {
    rein_sub_ 
      = n.subscribe("/humans/RecogInfo", 1, 
		    &PeoplePositionServer::callback, this);
    track_srv_ 
      = n.advertiseService("track_srv", 
			   &PeoplePositionServer::resTrackingId, this);
    okao_srv_ 
      = n.advertiseService("okao_srv", 
			   &PeoplePositionServer::resOkaoId, this);

    array_srv_ 
      = n.advertiseService("array_srv", 
			   &PeoplePositionServer::resHumans, this);
  }
  
  ~PeoplePositionServer()
  {
    //n_DBHuman.clear();
    //p_DBHuman.clear();
    o_DBHuman.clear();
  }

  void callback(const humans_msgs::HumansConstPtr& rein)
  {

    if( rein->num == 0 )
      {
	//ここで、見えなくなった人物のd_idをパブリッシュしてもいいかも
	//もし、N_DBHumanの中にデータが入っていたら、d_idをpeople_recog_infoにreq/resして、.clear()する
	//n_DBHuman.size();
      }
    else
      {

	for(int i = 0; i < rein->num; ++i)
	  {
	    humans_msgs::Human ah;
	    ah.header.stamp = ros::Time::now();
	    ah = rein->human[ i ];
	    //n_DBHuman[ i ] = ah;
	    //p_DBHuman[ rein->human[ i ].d_id ] = ah;
	    o_DBHuman[ rein->human[ i ].max_okao_id ] = ah;
	    ROS_INFO("people data update! okao_id: %d", rein->human[ i ].max_okao_id);
	  } 
      } 
  }

  bool resTrackingId(humans_msgs::HumanSrv::Request &req,
		     humans_msgs::HumanSrv::Response &res)
  {
    cout<<"tracking_id"<<endl;
    humans_msgs::Human h_res;
    //o_DBHuman内から、tracking_idをキーにして検索
    map<int, humans_msgs::Human>::iterator it_o = o_DBHuman.begin();
    while( it_o != o_DBHuman.end() )
      {
	if( it_o->second.body.tracking_id == req.src.body.tracking_id )
	  {
	    cout << "now looking: " << it_o->second.max_okao_id << endl;
	    res.dst = it_o->second;
	    return true;
	  }
	++it_o;
      }
    return false;
  }

  bool resOkaoId(humans_msgs::HumanSrv::Request &req,
		 humans_msgs::HumanSrv::Response &res)
    
  {
    //n_DBHumanについての検索
    //もしそれでもみつからなかったらp_DBHumanから検索する
    //どのデータベースから見つかったかをラベルづけして返す(.srvに記述しておく)
    cout<<"okao_id"<<endl;

    map<int, humans_msgs::Human>::iterator it_o = o_DBHuman.begin();
    while( it_o != o_DBHuman.end() )
      {
	if( it_o->second.max_okao_id == req.src.max_okao_id )
	  {
	    cout << "now looking: " << it_o->second.max_okao_id << endl;
	    res.dst = it_o->second;
	    return true;
	  }
	++it_o;
      }
    return false;
  }

  bool resHumans(humans_msgs::HumansSrv::Request &req,
		 humans_msgs::HumansSrv::Response &res)
  {
    ROS_INFO("all people request!");
    humans_msgs::Humans hums;
    map<int, humans_msgs::Human>::iterator it = o_DBHuman.begin();
    int num = 0;
    while( it != o_DBHuman.end() )
      { 
	//ここで、mapが持っている値を次の値につめて返す
	humans_msgs::Human hum;
	hum = it->second;  
	hums.human.push_back( hum );
	++num;
      }
    //hums.header
    hums.num = num;
    res.dst = hums;
    return true;
  }
  
 
};
  
int main(int argc, char** argv)
{
  ros::init(argc, argv, "people_position_server");
  PeoplePositionServer PPS;
  ros::spin();
  return 0;
}
