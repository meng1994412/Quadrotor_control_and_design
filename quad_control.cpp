#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <stdint.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <curses.h>

//gcc -o week1 week_1.cpp -lwiringPi -lncurses -lm

#define frequency 25000000.0
#define CONFIG           0x1A
#define SMPLRT_DIV       0x19
#define GYRO_CONFIG      0x1B
#define ACCEL_CONFIG     0x1C
#define ACCEL_CONFIG2    0x1D
#define USER_CTRL        0x6A  // Bit 7 enable DMP, bit 3 reset DMP
#define PWR_MGMT_1       0x6B // Device defaults to the SLEEP mode
#define PWR_MGMT_2       0x6C


enum Ascale {
  AFS_2G = 0,
  AFS_4G,
  AFS_8G,
  AFS_16G
};
 
enum Gscale {
  GFS_250DPS = 0,
  GFS_500DPS,
  GFS_1000DPS,
  GFS_2000DPS
};

//week2
typedef struct{
  char key_press;
  int heartbeat;
  int version;
}Keyboard;
 
int setup_imu();
void calibrate_imu();      
void read_imu();    
void update_filter();
void setup_keyboard();
void trap(int signal);
void safety_check(Keyboard *keyp);


//global variables
int imu;
float x_gyro_calibration=0;
float y_gyro_calibration=0;
float z_gyro_calibration=0;
float roll_calibration=0;
float pitch_calibration=0;
float accel_z_calibration=0;
float imu_data[6]; //gyro xyz, accel xyz
long time_curr;
long time_prev;
struct timespec te;
float yaw=0;
float pitch_angle=0; //pitch_acceleration
float roll_angle=0;  //roll_acceleration
float Roll=0;        //roll values from combination of accelerometer and gyro 
float Pitch=0;        //pitch values from combination of accelerometer and gyro 
float A=0.02;        //complementary coefficicent
//week2
Keyboard* shared_memory; 
int run_program=1;
//week2
struct timeval tem;
long beat_timer;
int oldheartbeat=-1;
 
int main (int argc, char *argv[])
{
    //week2
    gettimeofday(&tem,NULL);
    beat_timer=tem.tv_sec*1000LL+tem.tv_usec/1000;
    
    setup_imu();
    calibrate_imu();
    
    setup_keyboard();
    signal(SIGINT, &trap);
    
    //to refresh values from shared memory first 
    Keyboard keyboard=*shared_memory;
    
    while(run_program==1)
    {
      read_imu();      
      update_filter();
      
      //printf("x_gyro: %8.4f, y_gyro: %8.4f, z_gyro: %8.4f, roll_angle: %8.4f, pitch_angle: %8.4f\n",imu_data[0],imu_data[1],imu_data[2], roll_angle, pitch_angle);
      //printf("Roll: Roll_x: %f. Gyro_x: %f\n", roll_angle, imu_data[0]);
      //printf("Pitch: Pitch_y: %f. Gyro_y: %f\n", pitch_angle, );
      //printf("%f %f %f\n",imu_data[3],imu_data[4],imu_data[5]);
      //printf(" %f.   %f\n", roll_angle, pitch_angle);
      
      keyboard=*shared_memory;
      
      printf("%d \n",keyboard.heartbeat); 
      //printf("%c \n",keyboard.key_press);
      
      safety_check(&keyboard);

     
    }
      
    return 0;
   
  
}

void calibrate_imu()
{
  int times=3000;
  float sum[6]={0.0,0.0,0.0,0.0,0.0,0.0};  
  for (int i=0;i<times;i++)
  {
    read_imu();
    for (int j=0;j<3;j++)
    {
      sum[j]+=imu_data[j];
    }
    sum[3]+=roll_angle;
    sum[4]+=pitch_angle; 
  }
  x_gyro_calibration=-sum[0]/times;
  y_gyro_calibration=-sum[1]/times;
  z_gyro_calibration=-sum[2]/times;
  
  roll_calibration=-sum[3]/times;
  pitch_calibration=-sum[4]/times;
  /*
  accel_z_calibration=??
  */
  //printf("calibration complete, %f %f %f %f %f %f\n\r",x_gyro_calibration,y_gyro_calibration,z_gyro_calibration,roll_calibration,pitch_calibration,accel_z_calibration);


}

void read_imu()
{
  int address=59;//todo: set address value for accel x value 
  float ax=0;
  float az=0;
  float ay=0; 
  int vh,vl;
  
  //read in data
  vh=wiringPiI2CReadReg8(imu,address);
  vl=wiringPiI2CReadReg8(imu,address+1);
  //convert 2 complement
  int vw=(((vh<<8)&0xff00)|(vl&0x00ff))&0xffff;
  if(vw>0x8000)
  {
    vw=vw ^ 0xffff;
    vw=-vw-1;
  }          
  imu_data[3]=1.0*vw/32767*2;//  todo: convert vw from raw values to "g's"
  
  
  address=61;//todo: set address value for accel y value
  vh=wiringPiI2CReadReg8(imu,address);
  vl=wiringPiI2CReadReg8(imu,address+1);
  vw=(((vh<<8)&0xff00)|(vl&0x00ff))&0xffff;
  if(vw>0x8000)
  {
    vw=vw ^ 0xffff;
    vw=-vw-1;
  }          
  imu_data[4]=-1.0*vw/32767*2;//Todo: convert vw from raw valeus to "g's"
  
  
  address=63;//todo: set addres value for accel z value;
  vh=wiringPiI2CReadReg8(imu,address);
  vl=wiringPiI2CReadReg8(imu,address+1);
  vw=(((vh<<8)&0xff00)|(vl&0x00ff))&0xffff;
  if(vw>0x8000)
  {
    vw=vw ^ 0xffff;
    vw=-vw-1;
  }          
  imu_data[5]=-1.0*vw/32767*2;//todo: convert vw from raw values to g's
  
  
  address=67;//todo: set addres value for gyro x value;
  vh=wiringPiI2CReadReg8(imu,address);
  vl=wiringPiI2CReadReg8(imu,address+1);
  vw=(((vh<<8)&0xff00)|(vl&0x00ff))&0xffff;
  if(vw>0x8000)
  {
    vw=vw ^ 0xffff;
    vw=-vw-1;
  }          
  imu_data[0]=x_gyro_calibration+1.0*vw/32767*500;////todo: convert vw from raw values to degrees/second
  
  address=69;//todo: set addres value for gyro y value;
  vh=wiringPiI2CReadReg8(imu,address);
  vl=wiringPiI2CReadReg8(imu,address+1);
  vw=(((vh<<8)&0xff00)|(vl&0x00ff))&0xffff;
  if(vw>0x8000)
  {
    vw=vw ^ 0xffff;
    vw=-vw-1;
  }          
 imu_data[1]=y_gyro_calibration+1.0*vw/32767*500;////todo: convert vw from raw values to degrees/second
  
  address=71;////todo: set addres value for gyro z value;
  vh=wiringPiI2CReadReg8(imu,address);
  vl=wiringPiI2CReadReg8(imu,address+1);
  vw=(((vh<<8)&0xff00)|(vl&0x00ff))&0xffff;
  if(vw>0x8000)
  {
    vw=vw ^ 0xffff;
    vw=-vw-1;
  }          
  imu_data[2]=z_gyro_calibration+1.0*vw/32767*500;////todo: convert vw from raw values to degrees/second
  
  //Roll & Pitch angle
  roll_angle = atan2(imu_data[4],imu_data[5])*180/M_PI+roll_calibration;
  pitch_angle = atan2(imu_data[3],imu_data[5])*180/M_PI+pitch_calibration;
  


}

void update_filter()
{

  //get current time in nanoseconds
  timespec_get(&te,TIME_UTC);
  time_curr=te.tv_nsec;
  //compute time since last execution
  float imu_diff=time_curr-time_prev;           
  
  //check for rollover
  if(imu_diff<=0)
  {
    imu_diff+=1000000000;
  }
  //convert to seconds
  imu_diff=imu_diff/1000000000;
  time_prev=time_curr;
  
  //comp. filter for roll, pitch here: 
  Roll=roll_angle*A+(1-A)*(imu_data[0]*imu_diff+Roll);
  Pitch=pitch_angle*A+(1-A)*(imu_data[1]*imu_diff+Pitch);
  
  //print out the values
  //printf("%10.5f %10.5f %10.5f\n", Roll, roll_angle, imu_data[0]); //Roll values, roll_acceleration values, gyro values.
  //printf("%10.5f %10.5f %10.5f\n", Pitch, pitch_angle, imu_data[1]); //Pitch values, pitch_acceleration values, gyro values.
}


int setup_imu()
{
  wiringPiSetup ();
  
  
  //setup imu on I2C
  imu=wiringPiI2CSetup (0x68) ; //accel/gyro address
  
  if(imu==-1)
  {
    printf("-----cant connect to I2C device %d --------\n",imu);
    return -1;
  }
  else
  {
  
    printf("connected to i2c device %d\n",imu);
    printf("imu who am i is %d \n",wiringPiI2CReadReg8(imu,0x75));
    
    uint8_t Ascale = AFS_2G;     // AFS_2G, AFS_4G, AFS_8G, AFS_16G
    uint8_t Gscale = GFS_500DPS; // GFS_250DPS, GFS_500DPS, GFS_1000DPS, GFS_2000DPS
    
    
    //init imu
    wiringPiI2CWriteReg8(imu,PWR_MGMT_1, 0x00);
    printf("                    \n\r");
    wiringPiI2CWriteReg8(imu,PWR_MGMT_1, 0x01);
    wiringPiI2CWriteReg8(imu, CONFIG, 0x00);  
    wiringPiI2CWriteReg8(imu, SMPLRT_DIV, 0x00); //0x04        
    int c=wiringPiI2CReadReg8(imu,  GYRO_CONFIG);
    wiringPiI2CWriteReg8(imu,  GYRO_CONFIG, c & ~0xE0);
    wiringPiI2CWriteReg8(imu, GYRO_CONFIG, c & ~0x18);
    wiringPiI2CWriteReg8(imu, GYRO_CONFIG, c | Gscale << 3);       
    c=wiringPiI2CReadReg8(imu, ACCEL_CONFIG);
    wiringPiI2CWriteReg8(imu,  ACCEL_CONFIG, c & ~0xE0); // Clear self-test bits [7:5] 
    wiringPiI2CWriteReg8(imu,  ACCEL_CONFIG, c & ~0x18); // Clear AFS bits [4:3]
    wiringPiI2CWriteReg8(imu,  ACCEL_CONFIG, c | Ascale << 3);      
    c=wiringPiI2CReadReg8(imu, ACCEL_CONFIG2);         
    wiringPiI2CWriteReg8(imu,  ACCEL_CONFIG2, c & ~0x0F); //
    wiringPiI2CWriteReg8(imu,  ACCEL_CONFIG2,  c | 0x00);
  }
  return 0;
}

//function to add
void setup_keyboard()
{

  int segment_id;   
  struct shmid_ds shmbuffer; 
  int segment_size; 
  const int shared_segment_size = 0x6400; 
  int smhkey=33222;
  
  /* Allocate a shared memory segment.  */ 
  segment_id = shmget (smhkey, shared_segment_size,IPC_CREAT | 0666); 
  /* Attach the shared memory segment.  */ 
  shared_memory = (Keyboard*) shmat (segment_id, 0, 0); 
  printf ("shared memory attached at address %p\n", shared_memory); 
  /* Determine the segment's size. */ 
  shmctl (segment_id, IPC_STAT, &shmbuffer); 
  segment_size  =               shmbuffer.shm_segsz; 
  printf ("segment size: %d\n", segment_size); 
  /* Write a string to the shared memory segment.  */ 
  //sprintf (shared_memory, "test!!!!."); 

}


//when cntrl+c pressed, kill motors

void trap(int signal)

{

   printf("\n Ending program\n\r");

   run_program=0;
}

 
void safety_check(Keyboard *keyp)
{
  /*
Any gyro rate > 300 degrees/sec
Any Accelerometer value >1.8 g’s
All Accelerometer values <.25 g’s
Roll angle > 45 or <-45
Pitch angle >45 or <-45
Keyboard press of space
Keyboard timeout
Control+c is pressed in student code

  */
  
  //week2

  gettimeofday(&tem,NULL);
  long curr_time=tem.tv_sec*1000LL+tem.tv_usec/1000;  
  
  if (imu_data[0] > 300)
  {
     printf("Program ends: gyro x rate larger than 300 deg/sec.\n\r");
     run_program=0;
  }
  else if (imu_data[1] > 300)
  {
     printf("Program ends: gyro y rate larger than 300 deg/sec.\n\r");
     run_program=0;
  }
  else if (imu_data[2] > 300)
  {
     printf("Program ends: gyro z rate larger than 300 deg/sec.\n\r");
     run_program=0;
  }
  else if (imu_data[3] > 1.8)
  {
     printf("Program ends: x accel value larger than 1.8 g.\n\r");
     run_program=0;
  }
  else if (imu_data[4] > 1.8)
  {
     printf("Program ends: y accel value larger than 1.8 g.\n\r");
     run_program=0;
  }  
  else if (imu_data[5] > 1.8)
  {
     printf("Program ends: z accel value larger than 1.8 g.\n\r");
     run_program=0;
  }
  else if ((imu_data[3] < 0.25) & (imu_data[4] < 0.25) & (imu_data[5] < 0.25))
  {
     printf("Program ends: all accel values smaller than 0.25 g.\n\r");
     run_program=0;
  }
  else if (abs(Roll) > 45)
  {
     printf("Program ends: Roll angle > 45 or < -45.\n\r");
     run_program=0;
  }
  else if (abs(Pitch) > 45)
  {
     printf("Program ends: Pitch angle > 45 or < -45.\n\r");
     run_program=0;
  }
  
  
  if(keyp->key_press == ' ')
  {
    printf("Space pressed. \n");
    run_program=0;
  }
  gettimeofday(&tem,NULL);
  curr_time=tem.tv_sec*1000LL+tem.tv_usec/1000;
  
  //printf("%ld   %ld\n",curr_time,beat_timer);  
  
  
  if (keyp->heartbeat != oldheartbeat)
  {
    oldheartbeat=keyp->heartbeat;
    beat_timer=curr_time;
  }
  else if(curr_time-beat_timer>250)
  {
    printf("Keyboard Timeout. \n");
    run_program=0;      
  }
}

