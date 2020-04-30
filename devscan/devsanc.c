#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "devsanc.h"

static input_dev_t _input_devices[DEVICES_NUM];

int input_dev_search(void)
{
    int num = 0;
    int input_dev_num = 0;
    char *start;
    char buffer[MAX_LENTH];
    
    memset(_input_devices,0,sizeof(_input_devices));

    FILE *pf = fopen(INPUT_DEV_PATH,"r");
    if (pf == NULL) {
        // printf("%s file open file\n", INPUT_DEV_PATH);
    }

    while ((fgets(buffer, MAX_LENTH, pf) != NULL)) {
        if ((start = strstr(buffer, "Name=")) != NULL) {
            if(strlen(_input_devices[input_dev_num].name) == 0) {
                memcpy(_input_devices[input_dev_num].name, start+5, NAME_LEN-1);
                _input_devices[input_dev_num].handler[NAME_LEN-1] = '\0';
                // printf("_input_devices[%d].name = %s", input_dev_num, _input_devices[input_dev_num].name);
            }
        }

        if ((start = strstr(buffer, "Handlers=")) != NULL) {
            if ((start = strstr(buffer, "event")) != NULL) {
                if(strlen(_input_devices[input_dev_num].handler) == 0) {
                    memcpy(_input_devices[input_dev_num].handler, start, HANDLER_LEN-1);
                    _input_devices[input_dev_num].handler[HANDLER_LEN-1] = '\0';
                    // printf("_input_devices[%d].handler = %s\n", input_dev_num, _input_devices[input_dev_num].handler);
                }
            }
        }
        if((strlen(_input_devices[input_dev_num].name) != 0) && (strlen(_input_devices[input_dev_num].handler) != 0)) {
            input_dev_num++;
            if (input_dev_num > DEVICES_NUM) {
                // printf("input_dev_num > DEVICES_NUM");
                goto ret;
            }
        }
        num++;
    }

ret:
    // printf("close %s\n", INPUT_DEV_PATH);
    fclose(pf);
    return num;
}

int input_dev_parsing(char *str, input_dev_handler_t *handler, int num, int max)
{
    int i, h = num;
    for (i = num; i < DEVICES_NUM; i++) {
        if (strstr(_input_devices[i].name, str) != NULL) {
            memcpy(handler[h].handler, _input_devices[i].handler,sizeof(_input_devices[i].handler));
            // printf("handler[%d].handler = %s, len: %ld\n", h, handler[h].handler, sizeof(_input_devices[i].handler));
            h++;
            if(h >= max)
                return h;
        }
    }
    return h;
}


int main(int argc, char **argv)
{
    int i;
    input_dev_handler_t handler[1];

    if (NULL == argv[1]) {
        // printf("usage: %s \"str\" \n", argv[0]);
        exit(-1);
    }

    // printf("\n%s\n",argv[1]);

    memset(handler, 0, sizeof(handler));

    input_dev_search();
    input_dev_parsing(argv[1], handler, 0, 1);
    if (strlen(handler[0].handler)) {
        printf("%s\n",handler[0].handler);
    }
}
