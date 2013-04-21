#include "ros/ros.h"
#include "std_msgs/String.h"
#include <geometry_msgs/Twist.h>
#include <sstream>

int main(int argc, char** argv)
{
	ros::init(argc,argv,"talker");
	ros::NodeHandle n;
	ros::Publisher chatter = n.advertise<std_msgs::String>("chatter",1000);
	ros::Rate looper(10);

	int count = 0;
	while(ros::ok())
	{
		std_msgs::String msg;
		std::stringstream ss;
		ss<<"Hello World "<<count ;
		msg.data = ss.str();
		chatter.publish(msg);
		looper.sleep();
		count++;
	}
	return 0;
}
