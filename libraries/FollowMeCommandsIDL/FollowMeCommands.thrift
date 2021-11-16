namespace yarp roboticslab

service FollowMeHeadCommandsIDL
{
    oneway void enableFollowing();
    oneway void disableFollowing();
    double getOrientationAngle();
    bool stop();
}

service FollowMeArmCommandsIDL
{
}
