/*************************************************************************
	> File Name: work.h
	> Author: 
	> Mail: 
	> Created Time: Fri 10 Jul 2020 06:43:08 PM CST
 ************************************************************************/

#ifndef _WORK_H
#define _WORK_H
void send_all(struct ChatMsg *msg);
void send_one(struct ChatMsg *msg, struct User * user);
struct User *find_receiver(char *name);
void show_members(struct User * user);
#endif
