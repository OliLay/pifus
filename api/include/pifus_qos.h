#ifndef PIFUS_QOS_H
#define PIFUS_QOS_H

enum pifus_qos {
    QOS_BULK = 0,
    QOS_REAL_TIME = 1,
};

enum pifus_priority {
    PRIORITY_LOW = 0,
    PRIORITY_MEDIUM = 1,
    PRIORITY_HIGH = 2,
};

#endif
