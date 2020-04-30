#ifndef _INPUT_DEV_H_
#define _INPUT_DEV_H_

#define DEVICES_NUM 10
#define MAX_LENTH 256
#define NAME_LEN 40
#define HANDLER_LEN 7

#define INPUT_DEV_PATH "/proc/bus/input/devices"

typedef struct input_dev {
    char name[NAME_LEN];
    char handler[HANDLER_LEN];
} input_dev_t;

typedef struct input_dev_handler {
    char handler[HANDLER_LEN];
} input_dev_handler_t;


int input_dev_search(void);
int input_dev_parsing(char *str, input_dev_handler_t *handler, int num, int max);


#endif
