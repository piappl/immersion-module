#ifndef ROBOT_STATE_H
#define ROBOT_STATE_H

#include <QtGlobal>

struct RobotState
{
    bool emergency_on_{false};
    bool turtle_mode_on_{false};
    bool reverse_mode_on_{false};
    quint8 battery_level_{100};
};

#endif // ROBOT_STATE_H