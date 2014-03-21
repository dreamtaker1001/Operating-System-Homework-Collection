#ifndef H_UTIL
#define H_UTIL

/* the data structure for background jobs */
struct bg {
  pid_t pid;
  int valid;
};

/* the initialization function for background execution */
void 
bg_init(void);

/* add a new background job to the list */
int
bg_add(pid_t);

/* remove a background job (will go to foreground)*/
int
bg_remove(int);

/* show all background jobs */
void
bg_show();

/* test whether a given process (PID) exists */
int
pid_exist(pid_t);

/* check each pid's status in the bg list */
void
bg_checkall();

/* check the load of the system and action if necessary */
static void
*checkload();

/* gets the system load for 1 min */
int 
get_load(double*);

#endif
