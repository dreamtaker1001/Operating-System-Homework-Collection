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
    struct userlist *head;
    int count;
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
        tmp = tmp->next;
    }
    /* to_add is not in watch yet, critical section starts*/
    tmp = (struct watchlist*)malloc(sizeof(struct watchlist));
    strcpy (tmp->name, to_add);
    tmp->head = NULL;
    tmp->count = -1;
    if (watchlist_head == NULL) {
        watchlist_head = tmp;
        watchlist_head -> next = NULL;
    }
    else {
        tmp -> next = watchlist_head;
        watchlist_head = tmp;
    }
    printf("Added user to watch: %s\n", to_add);
    /* added, critical section over */
}

void
watchlist_remove(char* to_remove)
{
    struct watchlist *tmp, *tmplast = NULL;
    tmp = watchlist_head;
    int flag = 0;
    /* search */
    while (tmp) {
        if (strcmp(to_remove, tmp->name) == 0) {
            flag = 1;
            break;
        }
        if (tmp->next) {
            tmplast = tmp;
            tmp = tmp->next;
        }
    }
    if (flag = 0) {
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

/* adds a record to a watchlist element */
void
ut_insert(struct watchlist *tmpwatch,
            struct userlist *list_head, 
            struct utmpx to_insert)
{
    struct userlist *tmp = (struct userlist*)malloc(sizeof(struct userlist));
    tmp->next = list_head;
    list_head = tmp;

    /* doing the first time copy */
    tmp->element = to_insert;
    //debug information
    printf("inserted user=%s, line=%s, host=%s\n", (tmp->element).ut_user, (tmp->element).ut_line, (tmp->element).ut_host);
}

/* deletes a record from a watchlist element */
void
ut_remove(struct watchlist *tmpwatch,
            struct userlist *list_head,
            struct userlist *to_remove)
{
    struct userlist *tmp = list_head;
    struct userlist *tmp2;
    while(tmp) {
        if (tmp == to_remove) {
            if (tmp2 == NULL) {
                list_head->next = tmp->next;
                free(tmp);
            }
            else {
                tmp2->next = tmp->next;
                free(tmp);
            }
            break;
        }
        tmp2 = tmp;
        tmp = tmp -> next;
    }
}

/* Do the initialization for a new watched user */
void
new_user_init(struct watchlist *tmpwatch)
{
    struct userlist* tmpuser = user_head;
    tmpwatch->head = NULL;
    while (tmpuser) {
        /* match user */
        if (strcmp(tmpwatch->name, (tmpuser->element).ut_user) == 0) {
            ut_insert(tmpwatch, tmpwatch->head, tmpuser->element);
        }
        tmpuser = tmpuser->next;
    }
    if (tmpwatch->count == -1)
        tmpwatch->count = 1;
}

/* search for new logins */
void
scan_for_new_login(struct userlist* tmplist,
                    struct watchlist *tmpwatch)
{
    struct userlist *tmpu = user_head;
    struct userlist *tmpw = tmplist;
    int flag = 0;
    while(tmpu) {
        //debug information
        //printf("In tmpu\n");
        if (strcmp((tmpu->element).ut_user, tmpwatch->name) != 0) {
            tmpu = tmpu -> next;
            continue;
        }
        flag = 0;
        tmpw = tmplist;
        while(tmpw) {
            //debug information
            printf("in tmpw: u.user=%s, w.user=%s; u.line=%s, w.line=%s; u.host=%s, w.host=%s\n", (tmpw->element).ut_user, (tmpu->element).ut_user, (tmpw->element).ut_line, (tmpu->element).ut_line, (tmpw->element).ut_host, (tmpu->element).ut_host);

            if (strcmp((tmpw->element).ut_user, (tmpu->element).ut_user) == 0 &&
                    strcmp((tmpw->element).ut_line, (tmpu->element).ut_line) == 0 &&
                    strcmp((tmpw->element).ut_host, (tmpu->element).ut_host) == 0) {
                flag = 1;
                break;
            }
            tmpw = tmpw->next;
        }
        if (flag == 0) {
            printf("YuqiShell: watchuser: %s has logged on %s from %s!\n", 
                    (tmpu->element).ut_user,
                    (tmpu->element).ut_line,
                    (tmpu->element).ut_host);
        }
        tmpu = tmpu->next;
    }
}

/* search for new exit */
void
scan_for_new_exit(struct userlist* tmplist,
                    struct watchlist* tmpwatch)
{
    struct userlist *tmpu = user_head;
    struct userlist *tmpw = tmplist;
    int flag = 1;
    while(tmpw) {
        while(tmpu) {
            if (strcmp((tmpu->element).ut_user, tmpwatch->name) != 0) {
                tmpu = tmpu -> next;
                continue;
            }
            flag = 0;
            if (strcmp((tmpw->element).ut_user, (tmpu->element).ut_user) == 0 &&
                    strcmp((tmpw->element).ut_line, (tmpu->element).ut_line) == 0 &&
                    strcmp((tmpw->element).ut_host, (tmpu->element).ut_host) == 0) {
                flag = 1;
                break;
            }
            tmpu = tmpu->next;
        }
        if (flag == 0) {
            printf("YuqiShell: watchuser: %s has logged off %s from %s!\n", 
                    (tmpw->element).ut_user,
                    (tmpw->element).ut_line,
                    (tmpw->element).ut_host);
        }
        tmpw = tmpw->next;
    }
}

/* Do the comparison for old entries */
void
compare_old(struct watchlist *tmpwatch)
{
    struct userlist* tmplist = tmpwatch->head;
    //debug information
    assert(tmplist);
    scan_for_new_login(tmplist, tmpwatch);
    scan_for_new_exit(tmplist, tmpwatch);
}

/* Compare the value of watched users and assign new values */
void
compare_value()
{
    char usernow[32];
    struct watchlist* tmpwatch = watchlist_head;
    while(tmpwatch) {
        strcpy (usernow, tmpwatch -> name);
        //debug information
        printf("Comparing value for %s!\n", usernow);
        /* If the user was just added into watchlist, first time scan */
        if (tmpwatch->count == -1) {
            //debug information
            printf("This is a new user, calling new_user_init()\n");
            new_user_init(tmpwatch);
        }
        /* Scan old entry for existence and compare the value */
        else compare_old(tmpwatch); 
        tmpwatch = tmpwatch->next;
    }
    tmpwatch = watchlist_head;
}

/* copy the new status to watchlist */
void
adjust_watchlist()
{
    struct watchlist *tmpwatch = watchlist_head;
    struct userlist *tmpuser = user_head;
    struct userlist *tmplist;
    while (tmpwatch) {
        tmpwatch->head = NULL;
        tmplist = tmpwatch->head;
        while (tmpuser) {
            if (strcmp((tmpuser->element).ut_user, tmpwatch->name) == 0)
                ut_insert (tmpwatch, tmplist, tmpuser->element);
            tmpuser = tmpuser->next;
        }
        tmpwatch = tmpwatch->next;
    }
}

void
*watch_user_deamon(void)
{
    while(1) {
        user_head = NULL;
        get_all_users();
        //show_all_users();
        compare_value();
        adjust_watchlist();
        sleep(2);
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
            if (user_head == NULL) {
                tmp->next = NULL;
                user_head = tmp;
            }
            else {
                tmp->next = user_head;
                user_head = tmp;
            }
            tmp -> element = *up;
        }
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
