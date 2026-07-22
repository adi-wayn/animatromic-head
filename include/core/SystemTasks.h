#ifndef SYSTEM_TASKS_H
#define SYSTEM_TASKS_H

void kinematicsTask(void *pvParameters);
void staggeredBootTask(void *pvParameters);
void networkTask(void *pvParameters);

#endif
