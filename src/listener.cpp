#include "ros/ros.h"
#include "std_msgs/String.h"
#include "geometry_msgs/PoseWithCovarianceStamped.h"
#include <geometry_msgs/Twist.h>
#include <tf/transform_broadcaster.h>
#include "/home/eranda/fuerte_workspace/sandbox/razor_imu_9dof/razor_imu_9dof/msg_gen/cpp/include/razor_imu_9dof/RazorImu.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

//----------------------------------------------------------------------------
ros::Publisher publisher;
double distance_x;
double distance_y;
double inputAngle;
double myangle; // - 0.523598776; //target angle
geometry_msgs::Twist base_msg;
geometry_msgs::Twist last_published;
boost::mutex publish_mutex;
ros::Timer timer;
bool done = false;
bool firsts = false;
bool firsts_2 = false;
bool ss = false;
bool startangle = false;
bool overAllDone = false;
double startangleVal;
double currAngle;
double angleVelocity;
double solidAngle;
ros::Time now1;
ros::Time now1_2;

void chatterCallback(razor_imu_9dof::RazorImu msg) {
	if (inputAngle < 0) {
		myangle = inputAngle + 0.191986218;
	} else if (inputAngle == 0) {
		myangle = 0;
		done = 1;
	} else if (0.191986218<=inputAngle<0) {
		myangle = 0;
		done = 1;
	}else {
		myangle = inputAngle - 0.191986218;
	}

	double distance = sqrt(
			(distance_x * distance_x) + (distance_y * distance_y));
//ROS_INFO("Message %f",msg.yaw);
	if (!startangle) {
		startangleVal = msg.yaw;
		solidAngle = startangleVal;
		startangle = 1;
	}
	currAngle = msg.yaw;

	if (ss || (distance == 0)) {
		base_msg.linear.x = 0;
		base_msg.angular.z = 0;
		if (!firsts_2) {
			now1_2 = ros::Time::now();
			firsts_2 = 1;
		}
		ros::Time now2_2 = ros::Time::now();
		ros::Duration d2 = now2_2 - now1_2;
		if(d2.toSec()>3.0)
		{
			overAllDone = true;
		}

	}
	if (!done) {
		base_msg.angular.z = angleVelocity;
	} else {

		if (!firsts) {
			now1 = ros::Time::now();
			firsts = 1;
		}
		ros::Time now2 = ros::Time::now();
		ros::Duration d = now2 - now1;
		if ((d.toSec() > (distance*12))) {
			ss = 1;
			ROS_INFO("yyyy%f", d.toSec());

			base_msg.linear.x = 0.0;
			//return;
		} else
		{
			base_msg.linear.x = 0.025;
			if(currAngle < (solidAngle))
				base_msg.angular.z = -0.023;
			/*if(currAngle > (solidAngle + 0.0174532925))
				base_msg.angular.z = 0.1;*/
			else
				base_msg.angular.z = 0;
		}

		//base_msg.angular.z = 0;

	}
	//publisher.publish(base_msg);
	last_published = base_msg;

	double dangle;
	if (inputAngle < 0) {
		dangle = currAngle - startangleVal;
	} else {
		dangle = currAngle - startangleVal; //the rotation diff
	}

	ROS_INFO("start angle: %f", solidAngle);
	ROS_INFO("ending angle: %f", currAngle);
	ROS_INFO("dangle: %f", dangle);
	ROS_INFO("my angle: %f", myangle);
	ROS_INFO("angular vel: %f", base_msg.angular.z);

	if (inputAngle < 0) {
		dangle = dangle * (-1);
		myangle = myangle * (-1);
	}
	if (dangle >= (myangle)) {
		solidAngle = currAngle;
		done = 1;
	}
}

void publish()
{
	//boost::mutex::scoped_lock lock(publish_mutex);
	if(overAllDone || ss)
		{
			last_published.linear.x = 0;
		}
	publisher.publish(last_published);
}

int main(int argc, char** argv)
{
	ros::init(argc, argv,"listener");
	ros::NodeHandle nh;
	angleVelocity = -0.191986218;
	if(argv[1] == NULL)
	{
		distance_x = 0.0;
	}
	if(argv[2] == NULL)
	{
		distance_y = 0.0;
	}
	if(argv[3] == NULL)
	{
		inputAngle = 0.0;
	}
	distance_x = atof(argv[1]);
	distance_y = atof(argv[2]);
	inputAngle = atof(argv[3]);

	if(inputAngle<0)
	{
		angleVelocity = 0.191986218;
	}	

	base_msg.linear.x = 0;
	base_msg.angular.z = 0;
	//ros::Rate r(100);
	ros::Subscriber sub = nh.subscribe("imuRaw",1000,chatterCallback);
	publisher = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);
	timer = nh.createTimer(ros::Duration(0.1),boost::bind(publish));
	
	while(nh.ok() && !overAllDone)
{
	ros::spinOnce();
	//r.sleep();
}
	last_published.linear.x = 0;
	ROS_INFO("Status: %d", overAllDone);
	return 1 ?overAllDone:0;
	//ros::spin();
}

