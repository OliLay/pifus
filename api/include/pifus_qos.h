#ifndef PIFUS_QOS_H
#define PIFUS_QOS_H

enum pifus_priority {
    PRIORITY_LOW = 0,
    PRIORITY_MEDIUM = 1,
    PRIORITY_HIGH = 2,
};

const char* prio_str(enum pifus_priority prio);

enum pifus_priority str_to_prio(char *str);

#endif
