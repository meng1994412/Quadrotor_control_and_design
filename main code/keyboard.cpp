#include <curses.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
//gcc -o keyboard keyboard.cpp -lwiringPi -lncurses -lm


struct Keyboard {
  char key_press;
  int heartbeat;
  int version;
};




int main ()
{
    WINDOW *w =initscr();
    cbreak();
    nodelay(w,TRUE);

    //shared memory init
    int segment_id;
    Keyboard* shared_memory;
    struct shmid_ds shmbuffer;
    int segment_size;
    const int shared_segment_size = 0x6400;
    int smhkey=33222;
    int ch = 0;
    /* Basic initialization of curses lib */
    initscr();
    cbreak();
    noecho(); /* Set this for interactive programs. */
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);

    /* Allocate a shared memory segment.  */
    segment_id = shmget (smhkey, shared_segment_size,IPC_CREAT | 0666);
    /* Attach the shared memory segment.  */
    shared_memory = (Keyboard*) shmat (segment_id, 0, 0);
    printf ("shared memory attached at address %p\n", shared_memory);
    /* Determine the segment's size. */
    shmctl (segment_id, IPC_STAT, &shmbuffer);
    segment_size  =               shmbuffer.shm_segsz;
    printf ("segment size: %d\n", segment_size);


    float roll=0;
    float pitch=0;
    float yaw=0;
    float thrust=1500;
    int key=0;
    shared_memory->key_press=0;
    shared_memory->heartbeat=0;
    shared_memory->version=0;
    int i=0;
    int heartbeat=0;
    int version=0;
    struct timeval te;
    gettimeofday(&te,NULL);
    long curr_time=te.tv_sec*1000LL+te.tv_usec/1000;
    long heart_beat_timer=te.tv_sec*1000LL+te.tv_usec/1000;

    while(1)
    {
        //get current time
        gettimeofday(&te,NULL);
        curr_time=te.tv_sec*1000LL+te.tv_usec/1000;

        key=getch();
        if (key!=ERR)
        {
           shared_memory->key_press=key;
          
            //reset heartbeat timer
            heart_beat_timer=curr_time;
            heartbeat++;
            shared_memory->heartbeat=heartbeat;
            shared_memory->version=version++;
             printf("keypress is %c version is %d \n\r",key,version);
            

        }

        //send heartbeat if its been .2 seconds since last message sent
        if(curr_time>heart_beat_timer+200)
        {
           
            heart_beat_timer=curr_time;
            heartbeat++;
            shared_memory->heartbeat=heartbeat;
            
             printf("heartbeat sent %d\n\r",heartbeat);
        }
        fflush(stdout);
    }


/* Detach the shared memory segment.  */
shmdt (shared_memory);

/* Deallocate the shared memory segment.  */
endwin();

return 0;
}
