/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-03-04 02:11:32
 * @LastEditTime: 2020-04-30 10:53:49
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>

#define BUFSZ 1024
#define UEVENT_BUFFER_SIZE 2048

#define NAME "ppp"
#define RUNNING_DIR "/home/root/peripheral/ec20-4g"
#define PROGRAM "/home/root/peripheral/ec20-4g/ppp-on.sh"




#define PLATFORM_NET_PROTO_TCP  0 /**< The TCP transport protocol */
#define PLATFORM_NET_PROTO_UDP  1 /**< The UDP transport protocol */

//ARM 开发板LED设备的路径
#define RLED_DEV_PATH "/sys/class/leds/red/brightness"
#define GLED_DEV_PATH "/sys/class/leds/green/brightness"
#define BLED_DEV_PATH "/sys/class/leds/blue/brightness"

#define RLED_ON     led_control("255", "0", "0")
#define GLED_ON     led_control("0", "255", "0")
#define BLED_ON     led_control("0", "0", "255")
#define RGBLED_OFF  led_control("0", "0", "0")

int ok_flag = 0;

sem_t ppp_sem;
sem_t usbdev_sem;
pthread_t ppp_thread;
pthread_t check_thread;

char buf[UEVENT_BUFFER_SIZE] = { 0 };

int led_control(char *r, char *g, char *b)
{
    int res = 0;
	int r_fd, g_fd, b_fd;

	//获取红灯的设备文件描述符
	r_fd = open(RLED_DEV_PATH, O_WRONLY);
	if(r_fd < 0) {
		printf("Fail to Open %s device\n", RLED_DEV_PATH);
		return -1;
	}

	//获取绿灯的设备文件描述符
	g_fd = open(GLED_DEV_PATH, O_WRONLY);
	if(g_fd < 0) {
		printf("Fail to Open %s device\n", GLED_DEV_PATH);
		return -1;
	}

	//获取蓝灯的设备文件描述符
	b_fd = open(BLED_DEV_PATH, O_WRONLY);
	if(b_fd < 0) {
		printf("Fail to Open %s device\n", BLED_DEV_PATH);
		return -1;
	}

    write(r_fd, r, 3);
    write(g_fd, g, 3);
    write(b_fd, b, 3);

    close(r_fd);
    close(g_fd);
    close(b_fd);
}



int run_ppp_on(char *program)
{
    int res = -1;
    FILE* fp;
    char buf[BUFSZ], command[BUFSZ];

    sprintf(command, "%s &", program);

    fp = popen(command, "r");
    if (fp == NULL)
        return 0;

     while ((fgets(buf, BUFSZ, fp)) != NULL ) {
         printf("++++ %s\n", buf);
        if (buf != "\n") {
            if ('\n' == buf[strlen(buf)-1]) {
                buf[strlen(buf)-1] = '\0';
            }

            if ((strstr(buf, "ERROR")) || (strstr(buf, "failed"))) {
                goto exit;

            } else if (strstr(buf, "/etc/ppp/ip-up finished")) {
                res = 1;
                goto exit;
            } 
            // else if (strstr(buf, "Connection terminated")) {
               
            //     goto exit;
            // }
        } else {
            continue;
        }
     }

exit:
    printf("%s\n", buf);
    pclose(fp);
    if (res != -1) {
        sem_post(&ppp_sem);
        return res;
    }

    RLED_ON;
    return res;
}


int get_pid_by_name(char *name)
{
    int pid = 0;
    FILE* fp;
    char buf[BUFSZ], command[BUFSZ];

    sprintf(command, "ps | grep %s | grep -v grep | awk \'{print $1}\'", name);

    fp = popen(command, "r");
    if (fp == NULL)
        return 0;

    if ((fgets(buf, BUFSZ, fp)) != NULL ) {
        pid = atoi(buf); 
        printf("process id is %d\n", pid);   

    } else {
        printf("not found %s process\n", name);   
    }

    pclose(fp);

    return pid;
}

void kill_process(int pid)
{
    if (pid != 0) {
        printf("kill pid = %d\n", pid);
        kill(pid, SIGKILL);
    }
}

void *usbdev_monitor(void* arg)
{
    struct sockaddr_nl client;
    struct timeval tv;
    int pid, usb_dev, rcvlen, ret;
    fd_set fds;
    int buffersize = 1024;
    
    usb_dev = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);

    printf("fd = %d\n", usb_dev);
    memset(&client, 0, sizeof(client));

    client.nl_family = AF_NETLINK;
    client.nl_pid = getpid();
    client.nl_groups = 1;

    setsockopt(usb_dev, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

    bind(usb_dev, (struct sockaddr*)&client, sizeof(client));

    while (1) {
        FD_ZERO(&fds);
        FD_SET(usb_dev, &fds);

        tv.tv_sec = 1;
        tv.tv_usec = 100 * 1000;
        ret = select(usb_dev + 1, &fds, NULL, NULL, &tv);

        if(ret < 0)
            continue;
        if(!(ret > 0 && FD_ISSET(usb_dev, &fds)))
            continue;

        /* receive data */
        rcvlen = recv(usb_dev, &buf, sizeof(buf), 0);

        if (rcvlen > 0) {
            // printf("%s\n",buf);
            if (strstr(buf, "tty/ttyUSB2")) {
                if (strstr(buf,"remove")) {
                    printf("\nremove ttyUSB2\n");

                regetpid:
                    pid = get_pid_by_name(NAME);
                    kill_process(pid);
                    if (pid > 0)
                        goto regetpid;

                    RGBLED_OFF;
                    ok_flag = 1;

                } else if (strstr(buf,"add")) {

                    printf("\nadd ttyUSB2\n");
                    BLED_ON;
                    sleep(15);
                    ok_flag = 0;
                    sem_post(&usbdev_sem);
                    run_ppp_on(PROGRAM);
                    
                }
            }
        }
    }

    close(usb_dev);
    pthread_exit(NULL);
}


int ppp_connect(const char *host, const char *port, int proto)
{
    int fd, ret = -1;
    struct addrinfo hints, *addr_list, *cur;
    
    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = (proto == PLATFORM_NET_PROTO_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = (proto == PLATFORM_NET_PROTO_UDP) ? IPPROTO_UDP : IPPROTO_TCP;
    
    if (getaddrinfo(host, port, &hints, &addr_list) != 0) {
        return ret;
    }
    
    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0) {
            ret = -1;
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            ret = fd;
            break;
        }

        ret = -1;
    }

    freeaddrinfo(addr_list);
    close(fd);
    return ret;
}

void *check_thread_func(void *arg)
{
    int res;

    while (1)
    {
        /* 进行 P 操作 */
        sem_wait(&ppp_sem);

        printf("check thread is starting\n");

        res = ppp_connect("www.firebbs.cn", "80", PLATFORM_NET_PROTO_TCP);

        if (res < 0) {
            printf("------------------>ppp error\n");
            ok_flag = -1;
            RLED_ON;
        } else {
            printf("------------------>ppp ok\n");
            ok_flag = 1;
            GLED_ON;
        }

        printf("check thread finished\n");

    }

    pthread_exit(NULL);
}


int main(void)
{
    int pid, res;
    RGBLED_OFF;

    chdir(RUNNING_DIR);

    sem_init(&ppp_sem, 0, 0);
    sem_init(&usbdev_sem, 0, 0);
    
    res = pthread_create(&check_thread, NULL, check_thread_func, NULL);
    if (res != 0) {
        printf("Create check thread failed\n");
        exit(res);
    }

    res = pthread_create(&ppp_thread, NULL, usbdev_monitor, NULL);
    if (res != 0) {
        printf("Create ppp thread failed\n");
        exit(res);
    }

    while(1) {
wait:
        sem_wait(&usbdev_sem);
        sleep(15);
        if (ok_flag != 1) {
            printf("------timeout\n");

        rekill:
            pid = get_pid_by_name(NAME);
            kill_process(pid);
            if (pid > 0)
                goto rekill;
            
            run_ppp_on(PROGRAM);
        }
    }
    
    sem_destroy(&ppp_sem);
    return 0;
}

