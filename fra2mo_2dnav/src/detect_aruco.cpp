#include "../include/tf_nav.h"
#include <tf/transform_broadcaster.h>


// Global variables
std::vector<double> aruco_pose(7,0.0);
bool aruco_pose_available = false;
int score = 0;

TF_NAV::TF_NAV() {

    _position_pub = _nh.advertise<geometry_msgs::PoseStamped>( "/fra2mo/pose", 1 );
    _cur_pos << 0.0, 0.0, 0.0;
    _cur_or << 0.0, 0.0, 0.0, 1.0;
    _goal_pos << 0.0, 0.0, 0.0;
    _goal_or << 0.0, 0.0, 0.0, 1.0;
    _home_pos << -18.0, 2.0, 0.0;
    _aruco_pos << 0.0, 0.0, 0.0;
    _aruco_or<< 0.0, 0.0, 0.0, 1.0;
}

void arucoPoseCallback(const geometry_msgs::PoseStamped & msg)
{
    aruco_pose_available = true;
    aruco_pose.clear();
    aruco_pose.push_back(msg.pose.position.x);
    aruco_pose.push_back(msg.pose.position.y);
    aruco_pose.push_back(msg.pose.position.z);
    aruco_pose.push_back(msg.pose.orientation.x);
    aruco_pose.push_back(msg.pose.orientation.y);
    aruco_pose.push_back(msg.pose.orientation.z);
    aruco_pose.push_back(msg.pose.orientation.w);
}

void TF_NAV::tf_listener_fun() {
    ros::Rate r( 5 );
    tf::TransformListener listener;
    tf::StampedTransform transform;
    
    while ( ros::ok() )
    {
        try {
            listener.waitForTransform( "map", "base_footprint", ros::Time(0), ros::Duration(10.0) );
            listener.lookupTransform( "map", "base_footprint", ros::Time(0), transform );

        }
        catch( tf::TransformException &ex ) {
            ROS_ERROR("%s", ex.what());
            r.sleep();
            continue;
        }
        
        _cur_pos << transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z();
        _cur_or << transform.getRotation().w(),  transform.getRotation().x(), transform.getRotation().y(), transform.getRotation().z();
        position_pub();
        r.sleep();
    }

}

void TF_NAV::position_pub() {

    geometry_msgs::PoseStamped pose;

    pose.header.stamp = ros::Time::now();
    pose.header.frame_id = "map";

    pose.pose.position.x = _cur_pos[0];
    pose.pose.position.y = _cur_pos[1];
    pose.pose.position.z = _cur_pos[2];

    pose.pose.orientation.w = _cur_or[0];
    pose.pose.orientation.x = _cur_or[1];
    pose.pose.orientation.y = _cur_or[2];
    pose.pose.orientation.z = _cur_or[3];

    _position_pub.publish(pose);
}

void TF_NAV::goal_listener() {
    ros::Rate r( 1 );
    tf::TransformListener listener;
    tf::StampedTransform transform;

    while ( ros::ok() )
    {
        try
        {
            listener.waitForTransform( "map", "goal9", ros::Time( 0 ), ros::Duration( 10.0 ) );
            listener.lookupTransform( "map", "goal9", ros::Time( 0 ), transform );
        }
        catch( tf::TransformException &ex )
        {
            ROS_ERROR("%s", ex.what());
            r.sleep();
            continue;
        }

        _goal_pos << transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z();
        _goal_or << transform.getRotation().w(),  transform.getRotation().x(), transform.getRotation().y(), transform.getRotation().z();
        r.sleep();
    }    
}

void TF_NAV::tf_aruco_fun() {
    ros::Rate r( 5 );
    tf::TransformListener listener;
    tf::StampedTransform transform;
    
    while ( ros::ok() )
    {
        try {
            listener.waitForTransform( "map", "aruco_marker_frame", ros::Time(0), ros::Duration(10.0) );
            listener.lookupTransform( "map", "aruco_marker_frame", ros::Time(0), transform );

        }
        catch( tf::TransformException &ex ) {
            ROS_ERROR("%s", ex.what());
            r.sleep();
            continue;
        }
        
        _aruco_pos << transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z();
        _aruco_or << transform.getRotation().w(),  transform.getRotation().x(), transform.getRotation().y(), transform.getRotation().z();
        r.sleep();
    }

}

void TF_NAV::send_goal() {
    ros::Rate r( 5 );
    int cmd;
    move_base_msgs::MoveBaseGoal goal;

    while ( ros::ok() )
    {
        std::cout<<"\nInsert 1 to send goal from TF "<<std::endl;
        std::cout<<"Insert 2 to send home position goal "<<std::endl;
        std::cout<<"Inser your choice"<<std::endl;
        std::cin>>cmd;

        if ( cmd == 1) {
            MoveBaseClient ac("move_base", true);
            while(!ac.waitForServer(ros::Duration(5.0))){
            ROS_INFO("Waiting for the move_base action server to come up");
            }
            if (score == 0){
		    goal.target_pose.header.frame_id = "map";
		    goal.target_pose.header.stamp = ros::Time::now();
		    
		    goal.target_pose.pose.position.x = _goal_pos[0];
		    goal.target_pose.pose.position.y = _goal_pos[1];
		    goal.target_pose.pose.position.z = _goal_pos[2];

		    goal.target_pose.pose.orientation.w = _goal_or[0];
		    goal.target_pose.pose.orientation.x = _goal_or[1];
		    goal.target_pose.pose.orientation.y = _goal_or[2];
		    goal.target_pose.pose.orientation.z = _goal_or[3];

		    ROS_INFO("Sending goal");
		    ac.sendGoal(goal);

		    ac.waitForResult();

		    if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
		    ROS_INFO("The mobile robot arrived near the aruco marker");
		    score++;
		    }
		    else
		        ROS_INFO("The base failed to move for some reason");
		    
		    boost::thread tf_aruco_fun( &TF_NAV::tf_aruco_fun, this );
	    	    
	    	    // Publish aruco TF
	    	    //tf::Vector3 aruco_position(aruco_pose[0], aruco_pose[1], aruco_pose[2]);
   	 	    //tf::Quaternion aruco_orientation(aruco_pose[0], aruco_pose[1], aruco_pose[2], aruco_pose[3]);
   		    tf::Vector3 aruco_position(_aruco_pos[0], _aruco_pos[1], _aruco_pos[2]);
   	 	    tf::Quaternion aruco_orientation(_aruco_or[0], _aruco_or[1], _aruco_or[2], _aruco_or[3]);
		    TF_NAV::publishTF(aruco_position, aruco_orientation);
	     }
	     else{
	    	    while(!ac.waitForServer(ros::Duration(5.0))){
		    ROS_INFO("Waiting for the move_base action server to come up");
		    }
		    
		    goal.target_pose.header.frame_id = "map";
		    goal.target_pose.header.stamp = ros::Time::now();
		    //std::cout<<"first goal:"<<std::endl<<_goal_pos[0]<<std::endl<<_goal_pos[1]<<std::endl<<_goal_pos[2]<<std::endl;
		    //std::cout<<"second goal pos:"<<std::endl<<_aruco_pos[0]+1<<std::endl<<_aruco_pos[1]<<std::endl<<_goal_pos[2]<<std::endl;
		    
		    //goal.target_pose.pose.position.x = aruco_pose[0] + 1;
		    //goal.target_pose.pose.position.y = aruco_pose[1];
		    goal.target_pose.pose.position.x = _aruco_pos[0] + 1;
		    goal.target_pose.pose.position.y = _aruco_pos[1];
		    goal.target_pose.pose.position.z = _goal_pos[2];

		    goal.target_pose.pose.orientation.w = _goal_or[0];
		    goal.target_pose.pose.orientation.x = _goal_or[1];
		    goal.target_pose.pose.orientation.y = _goal_or[2];
		    goal.target_pose.pose.orientation.z = _goal_or[3];

		    ROS_INFO("Sending goal");
		    ac.sendGoal(goal);

		    ac.waitForResult();

		    if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
		    ROS_INFO("The mobile robot arrived at the second goal with:\nx = aruco_pos.x +1\ny = aruco_pos.y");
		    else
		        ROS_INFO("The base failed to move for some reason");
	     }
	}
    	    
        else if ( cmd == 2 ) {
            MoveBaseClient ac("move_base", true);
            while(!ac.waitForServer(ros::Duration(5.0))){
            ROS_INFO("Waiting for the move_base action server to come up");
            }
            goal.target_pose.header.frame_id = "map";
            goal.target_pose.header.stamp = ros::Time::now();
            
            goal.target_pose.pose.position.x = _home_pos[0];
            goal.target_pose.pose.position.y = _home_pos[1];
            goal.target_pose.pose.position.z = _home_pos[2];

            goal.target_pose.pose.orientation.w = 1.0;
            goal.target_pose.pose.orientation.x = 0.0;
            goal.target_pose.pose.orientation.y = 0.0;
            goal.target_pose.pose.orientation.z = 0.0;

            ROS_INFO("Sending HOME position as goal");
            ac.sendGoal(goal);

            ac.waitForResult();

            if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
            ROS_INFO("The mobile robot arrived in the HOME position");
            else
                ROS_INFO("The base failed to move for some reason");
        }
         else {
            ROS_INFO("Wrong input!");
        }
        r.sleep();
    }
    
}

void TF_NAV::run() {
    boost::thread tf_listener_fun_t( &TF_NAV::tf_listener_fun, this );
    boost::thread tf_listener_goal_t( &TF_NAV::goal_listener, this );
    boost::thread send_goal_t( &TF_NAV::send_goal, this );
    ros::spin();
}

void TF_NAV::publishTF(const tf::Vector3& aruco_position, const tf::Quaternion& aruco_orientation){

   static tf::TransformBroadcaster br;
   tf::Transform transform;
   transform.setOrigin(aruco_position);
   transform.setRotation(aruco_orientation);
   br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "map", "aruco_marker_frame"));
 }

int main( int argc, char** argv ) {
    // Init node
    ros::init(argc, argv, "tf_navigation");
    ros::NodeHandle n;
    
    // Subscriber
    ros::Subscriber aruco_pose_sub = n.subscribe("/aruco_single/pose", 1, arucoPoseCallback);
    
    // Start navigation
    TF_NAV tfnav;
    tfnav.run();

    return 0;
}
