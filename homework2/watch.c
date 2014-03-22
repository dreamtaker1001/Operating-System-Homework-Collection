#include "shell.h"
#include "cmd.h"
#include "util.h"
#include <utmpx.h>
#include <pthread.h>

struct utmpx *up;
pthread_t user_tid = 0;

struct userlist {
    struct utmpx element;
    struct userlist *next;
};
struct userlist *user_head = NULL;
struct userlist *tmp = NULL;
struct userlist *tmp2 = NULL;

int watchlist_count;
struct watchlist {
    char name[255];
    struct utmpx element;
    struct watchlist *next;
};
struct watchlist *watchlist_head;

void
show_all_users(void);

/* the deamon for watching users */
void
*watch_user_deamon(void);

/* init function for watchlist */
void
watchlist_init();

/* add new element to watchlist */
void
watchlist_add(char*);

/* remove element from watchlist */
void
watchlist_remove(char*);

/* cmd_watchuser() function. */
void
cmd_watchuser(int argc, char **argv)
{
    if (argc > 3) {
        printf("YuqiShell: watchuser: syntax error!\n");
        return;
    }
    /* start the thread if not exist */
    if (user_tid == 0) {
        watchlist_init();
        pthread_create(&user_tid, NULL, watch_user_deamon, "watchuser");
    }
    /* watchuser username */
    if (argc == 2) {
        watchlist_add(argv[1]);
        return;
    }
    /* watchuser username off */
    if (argc == 3) {
        if (strcmp (argv[2], "off") == 0) {
            watchlist_remove(argv[1]);
        }
    }
}

void
watchlist_add(char* to_add)
{
    struct watchlist *tmp;
    tmp = watchlist_head;
    while (tmp) {
        if (strcmp(to_add, tmp->name) == 0) {
            printf("YuqiShell: username %s already in watch!\n", to_add);
            return;
        }
        if (tmp->next)
            tmp = tmp->next;
    }
    /* to_add is not in watch yet, critical section starts*/
    tmp = (struct watchlist*)malloc(sizeof(struct watchlist));
    strcpy (tmp->name, to_add);
    strcpy ((tmp->element).ut_user, "__init");
    if (watchlist_head == NULL) {
        watchlist_head = tmp;
        watchlist_head -> next = NULL;
    }
    tmp -> next = watchlist_head -> next;
    watchlist_head -> next = tmp;
    /* added, critical section over */
}

void
watchlist_remove(char* to_remove)
{
    struct watchlist *tmp, *tmplast = NULL;
    tmp = watchlist_head;
    /* search */
    while (tmp) {
        if (strcmp(to_remove, tmp->name) == 0) {
            break;
        }
        if (tmp->next) {
            tmplast = tmp;
            tmp = tmp->next;
        }
    }
    if (tmp->next == NULL) {
        printf("YuqiShell: there's no user called %s in watch!", to_remove);
        return;
    }
    /* remove, critical section start */
    if (tmplast == NULL) {
        free(tmp);
        watchlist_head = NULL;
    }
    else {
        tmplast->next = tmp->next;
        free(tmp);
    }
    watchlist_count--;
    /* critical section over */
}

/* Compare the value of watched users and assign new values */
void
compare_value()
{
    char usernow[32];
    struct watchlist* tmpwatch = watchlist_head;
    struct userlist* tmpuser = user_head;
    while(tmpwatch) {
        strcpy (usernow, tmpwatch -> name);
        tmpuser = user_head;
        while(tmpuser && strcmp ((tmpuser->element).ut_user, usernow) == 0) {
            if (strcmp ((tmpwatch->element).ut_user, "__init") != 0) {    
                if (strcmp((tmpwatch->element).ut_line, (tmpuser->element).ut_line) != 0 ||
                         strcmp((tmpwatch->element).ut_host, (tmpuser->element).ut_host) != 0) {
                    printf("YuqiShell: watchuser: %s has logged on %s from %s \n", (tmpuser->element).ut_user, (tmpuser->element).ut_line, (tmpuser->element).ut_host);
                }
                strcpy( (tmpwatch->element).ut_user, (tmpuser->element).ut_user);
                strcpy( (tmpwatch->element).ut_line, (tmpuser->element).ut_line);
                strcpy( (tmpwatch->element).ut_host, (tmpuser->element).ut_host);
            }
            if (tmpuser->next)
                tmpuser = tmpuser->next;
            else break;
        }
        if (tmpwatch->next)
            tmpwatch = tmpwatch->next;
    }
}

void
*watch_user_deamon(void)
{
    while(1) {
        get_all_users();
        compare_value();
        sleep(5);
    }
}

void
watchlist_init()
{
    watchlist_count = 0;
    watchlist_head = NULL;
}

/* gets the currently logged-in users */
void
get_all_users()
{
    setutxent();
    while (up = getutxent() ) {
        if (up -> ut_type == USER_PROCESS) {
            tmp = (struct userlist*)malloc(sizeof(struct userlist));
            tmp -> next = NULL;
            if (user_head == NULL)
                user_head = tmp;
            else {
                tmp2 = user_head;
                while(tmp2 -> next)
                    tmp2 = tmp2 -> next;
                tmp2 -> next = tmp;
            }
            tmp -> element = *up;
        }
    }
    if (user_head) { 
        free(user_head);
        user_head = NULL;
    }
}

void
show_all_users()
{
    tmp = user_head;
    while(tmp) {
        printf("%s has logged on %s from %s\n", (tmp->element).ut_user, (tmp->element).ut_line, (tmp->element).ut_host);
        tmp = tmp->next;
    }
}
