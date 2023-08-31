namespace yarp roboticslab

service FollowMeHeadCommands
{
    oneway void enableFollowing();
    oneway void disableFollowing();
    double getOrientationAngle();
    bool stop();
}

service FollowMeArmCommands
{
    oneway void doGreet();
    oneway void doSignalLeft();
    oneway void doSignalRight();
    oneway void enableArmSwinging();
    oneway void disableArmSwinging();
    bool stop();
}
