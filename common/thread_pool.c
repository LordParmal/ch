/*************************************************************************
	> File Name: thread_pool.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Thu 09 Jul 2020 02:50:28 PM CST
 ************************************************************************/

#include "head.h"
extern int repollfd, bepollfd;

extern struct User *rteam;
extern struct User *bteam;

void do_work(struct User *user){
    struct ChatMsg msg;
    bzero(&msg, sizeof(msg));
    recv(user->fd, (void *)&msg, sizeof(msg), 0);

    if (msg.type & CHAT_WALL) {
        printf(L_PINK"<%s> ~ "NONE"%s \n", user->name, msg.msg);
        send_all(&msg);
    } else if (msg.type & CHAT_MSG) {
        int i;
        char receiver[20] = {0};
        struct ChatMsg send_msg;
        bzero(&send_msg, sizeof(send_msg));

        for (i = 0; msg.msg[i + 1] != ' '; i++) {
            receiver[i] = msg.msg[i + 1]; 
        }

        struct User *recv;

        if ((recv = find_receiver(receiver)) == NULL) {
            sprintf(send_msg.msg, L_GREEN"私聊格式错误，或者对方不在线"NONE);
            strcpy(send_msg.name, msg.name);
            send_msg.type = CHAT_MSG;
            send_one(&send_msg, user);
        } else {
            char temp[1024] = {0};

            strcpy(send_msg.name, msg.name);
            send_msg.type = CHAT_MSG;
            strncpy(temp, msg.msg + (i + 1), strlen(msg.msg + (i + 1)));
            sprintf(send_msg.msg, YELLOW"<%s> --> <%s> $"BLUE"  %s"NONE, user->name, recv->name, temp);

            send_one(&send_msg, user);
            send_one(&send_msg, recv);
            printf("%s\n", send_msg.msg);
        }
    } else if (msg.type & CHAT_FIN) {
        user->online = 0;
        int epollfd = user->team ? bepollfd : repollfd;
        del_event(epollfd, user->fd);
        printf(GREEN"Server Info"NONE" : %s logout\n", user->name);
        struct ChatMsg send_msg;
        bzero(&send_msg, sizeof(send_msg));
        send_msg.type = CHAT_SYS;
        sprintf(send_msg.msg, L_YELLOW"%s下线了"NONE, user->name);
        send_all(&send_msg);

        close(user->fd);
    } else if (msg.type & CHAT_FUNC) {
        switch(msg.msg[1]) {
            case '1' : show_members(user); break;
            default : break;
        }
    }
}


void task_queue_init(struct task_queue *taskQueue, int sum, int epollfd) {
    taskQueue->sum = sum;
    taskQueue->epollfd = epollfd;
    taskQueue->team = calloc(sum, sizeof(void *));
    taskQueue->head = taskQueue->tail = 0;
    pthread_mutex_init(&taskQueue->mutex, NULL);
    pthread_cond_init(&taskQueue->cond, NULL);
}

void task_queue_push(struct task_queue *taskQueue, struct User *user) {
    pthread_mutex_lock(&taskQueue->mutex);
    taskQueue->team[taskQueue->tail] = user;
    DBG(L_GREEN"Thread Pool"NONE" : Task push %s\n", user->name);
    if (++taskQueue->tail == taskQueue->sum) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
        taskQueue->tail = 0;
    }
    pthread_cond_signal(&taskQueue->cond);
    pthread_mutex_unlock(&taskQueue->mutex);
}


struct User *task_queue_pop(struct task_queue *taskQueue) {
    pthread_mutex_lock(&taskQueue->mutex);
    while (taskQueue->tail == taskQueue->head) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue Empty, Waiting For Task\n");
        pthread_cond_wait(&taskQueue->cond, &taskQueue->mutex);
    }
    struct User *user = taskQueue->team[taskQueue->head];
    DBG(L_GREEN"Thread Pool"NONE" : Task Pop %s\n", user->name);
    if (++taskQueue->head == taskQueue->sum) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
        taskQueue->head = 0;
    }
    pthread_mutex_unlock(&taskQueue->mutex);
    return user;
}

void *thread_run(void *arg) {
    pthread_detach(pthread_self());
    struct task_queue *taskQueue = (struct task_queue *)arg;
    while (1) {
        struct User *user = task_queue_pop(taskQueue);
        do_work(user);
    }
}

