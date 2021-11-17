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
    oneway void doGreet();
    oneway void doSignalLeft();
    oneway void doSignalRight();
    oneway void enableArmSwinging();
    oneway void disableArmSwinging();
    bool stop();
}
