/*************************************************************************
	> File Name: work.c
	> Author: 
	> Mail: 
	> Created Time: Fri 10 Jul 2020 06:43:02 PM CST
 ************************************************************************/

#include "head.h"

extern struct User *rteam;
extern struct User *bteam;

void send_all(struct ChatMsg *msg) {
    for (int i = 0; rteam[i].online; i++) {
        send(rteam[i].fd, (void *)msg, sizeof(*msg), 0);
    }
    for (int i = 0; bteam[i].online; i++) {
        send(bteam[i].fd, (void *)msg, sizeof(*msg), 0);
    }
}

void send_one(struct ChatMsg *msg, struct User * user) {
    send(user->fd, (void *)msg, sizeof(*msg), 0);
}

struct User *find_receiver(char *name) {
    for (int i = 0; bteam[i].online == 1; i++) {
        if (strcmp(name, bteam[i].name) == 0) {
            return &bteam[i];
        }
    }
    for (int i = 0; rteam[i].online == 1; i++) {
        if (strcmp(name, rteam[i].name) == 0) {
            return &rteam[i];
        }
    }

    return NULL;
}

void show_members(struct User * user) {
    struct ChatMsg msg;
    bzero(&msg, sizeof(msg));
    msg.type = CHAT_FUNC;

    sprintf(msg.msg, BLUE"Blue team:"NONE);
    send_one(&msg, user);
    
    for (int i = 0; bteam[i].online; i++) {
        sprintf(msg.msg,"%s", bteam[i].name);
        send_one(&msg, user);
    }
    sprintf(msg.msg, L_RED"Red team:"NONE);
    send_one(&msg, user);
    
    for (int i = 0; rteam[i].online; i++) {
        sprintf(msg.msg,"%s", rteam[i].name);
        send_one(&msg, user);
    }
}