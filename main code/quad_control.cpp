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
#include "vive.h"

//gcc -o W_M_quad quad_control.cpp -lwiringPi -lncurses -lm

#define frequency 25000000.0
#define CONFIG           0x1A
#define SMPLRT_DIV       0x19
#define GYRO_CONFIG      0x1B
#define ACCEL_CONFIG     0x1C
#define ACCEL_CONFIG2    0x1D
#define USER_CTRL        0x6A  // Bit 7 enable DMP, bit 3 reset DMP
#define PWR_MGMT_1       0x6B // Device defaults to the SLEEP mode
#define PWR_MGMT_2       0x6C
//week3
#define PWM_MAX 1950
#define frequency 25000000.0
#define LED0 0x6			
#define LED0_ON_L 0x6		
#define LED0_ON_H 0x7		
#define LED0_OFF_L 0x8		
#define LED0_OFF_H 0x9		
#define LED_MULTIPLYER 4	


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
/*
typedef struct{
  char key_press;
  int heartbeat;
  int version;
}Keyboard;
*/

//week 4

struct Keyboard {
  char key_press;
  float pitch;
	float roll;
	float yaw;
	float thrust;
  int heartbeat;
};

 
int setup_imu();
void calibrate_imu();      
void read_imu();    
void update_filter();
void setup_keyboard();
void trap(int signal);
void safety_check(Keyboard *keyp, Position *lp);
void init_pwm();
void init_motor(uint8_t channel);
void set_PWM( uint8_t channel, float time_on_us);
void pid_update(/*FILE *f*/);
void get_joystick(Keyboard *keyp);
void vive_control(Position *lp);



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
float A=0.004;        //complementary coefficicent
//week2
Keyboard* shared_memory; 
int run_program=1;
//week2
struct timeval tem;
long beat_timer;
int oldheartbeat=-1;
//week3
int pwm;
float pwm_control[4];
//week4
float previous_pitch = 0; //previous pitch velocity for differential control
float pitch_integral = 0; //integral pitch for integral control 
float previous_roll = 0; //previous roll velocity for differential control
float roll_integral = 0; //integral roll for integral control 

//Feb 2nd
//FILE *f; // write a csv file to store some values 
//week5
float Thrust;
float Desire_pitch;
float Desire_roll;
float Desire_pitch_joystick;
float Desire_roll_joystick;
//week6
float Desire_yaw = 0;
//week8
Position local_p;
long version_timer;
int oldversion = -1;
//week9
float Desire_pitch_vive;
float Desire_roll_vive;

float Desire_x = 0;
float x_vive_prev;
float x_vive_estimate;
float D_term_x = 0;
//week10
float Desire_y = 0;
float y_vive_prev;
float y_vive_estimate;
float D_term_y;

 
int main (int argc, char *argv[])
{
    //week8
    init_shared_memory();
    //now you can use the vive sensor values: 
    //local_p.version
    //local_p.x
    //local_p.y
    //local_p.z
    //local_p.yaw
    //week2
    gettimeofday(&tem,NULL);
    beat_timer=tem.tv_sec*1000LL+tem.tv_usec/1000;
    version_timer = beat_timer;
    
    //week3
    init_pwm();
    init_motor(0);
    init_motor(1);
    init_motor(2);
    init_motor(3);
    delay(1000);
    
    setup_imu();
    calibrate_imu();
    
    setup_keyboard();
    signal(SIGINT, &trap);
    
    //to refresh values from shared memory first 
    Keyboard keyboard=*shared_memory;
    /*while(run_program==1)
    {
     keyboard=*shared_memory;
    printf("%c  %f  %f  %f  %f  %d\n", keyboard.key_press, keyboard.pitch, keyboard.roll, keyboard.yaw, keyboard.thrust, keyboard.heartbeat);
    }
    //Feb 2nd
    //FILE *f = fopen("Pitch_Data.csv", "w");
    return 0; */
    
    //week5
    int flag = -1;
    local_p = *position;
    x_vive_estimate = local_p.x;
    x_vive_prev = x_vive_estimate;
    y_vive_estimate = local_p.y;
    y_vive_prev = y_vive_estimate;
    
    
    while(run_program==1)
    { 
      local_p = *position;
      keyboard=*shared_memory;
      
      
      if (keyboard.key_press == '!') 
      {
        flag = 1;
      } 
      else if (keyboard.key_press == '"')
      {
        flag = 0;
      }
      
      if (flag == 1) 
      {
        set_PWM(0, 1000); // motor 0
        set_PWM(1, 1000); // motor 1
        set_PWM(2, 1000); // motor 2
        set_PWM(3, 1000); // motor 3
        if (keyboard.key_press == '#')
        {
          printf("Calibration is starting, please wait a moment\n");
          calibrate_imu();
          printf("Calibration finished!\n");
        }
      }
      else if (flag == 0) {
      
      read_imu();      
      update_filter();
      
      //printf("x_gyro: %8.4f, y_gyro: %8.4f, z_gyro: %8.4f, roll_angle: %8.4f, pitch_angle: %8.4f\n",imu_data[0],imu_data[1],imu_data[2], roll_angle, pitch_angle);
      //printf("Roll: Roll_x: %f. Gyro_x: %f\n", roll_angle, imu_data[0]);
      //printf("Pitch: Pitch_y: %f. Gyro_y: %f\n", pitch_angle, );
      //printf("%f %f %f\n",imu_data[3],imu_data[4],imu_data[5]);
      //printf(" %f.   %f\n", roll_angle, pitch_angle);
      //printf("%8.4f\n", Pitch);
      //printf("%d  \n", local_p.version);
      //printf("%f  %f  %f  %f  %d  \n", local_p.x, local_p.y, local_p.z, local_p.yaw, local_p.version); 
      
      //keyboard=*shared_memory;
      
      //printf("%d \n",keyboard.heartbeat); 
      //printf("%c \n",keyboard.key_press);
      //printf("%f \n", keyboard.pitch);
      
      //safety_check(&keyboard);
      get_joystick(&keyboard);
      vive_control(&local_p);
      pid_update(/*f*/);
      
      set_PWM(0, pwm_control[0]);    //speed between 1000 and PWM_MAX, motor 0-3
      set_PWM(1, pwm_control[1]);    //speed between 1000 and PWM_MAX, motor 0-3
      set_PWM(2, pwm_control[2]);    //speed between 1000 and PWM_MAX, motor 0-3
      set_PWM(3, pwm_control[3]);    //speed between 1000 and PWM_MAX, motor 0-3
      
      //printf("%10.5f    %10.5f    %10.5f    %10.5f    %10.5f\n", Pitch, pwm_control[0], pwm_control[1], pwm_control[2], pwm_control[3]);
      //printf("%10.5f    %10.5f    %10.5f\n", Pitch, pwm_control[0], pwm_control[1]);
      safety_check(&keyboard, &local_p);
      }
    }
    set_PWM(0, 1000); // motor 0
    set_PWM(1, 1000); // motor 1
    set_PWM(2, 1000); // motor 2
    set_PWM(3, 1000); // motor 3
    
    //Feb 2nd
    //fclose(f);
    
    return 0;
   
  
}

void calibrate_imu()
{
  int times=3000;
  float sum[6]={0.0,0.0,0.0,0.0,0.0,0.0};
  x_gyro_calibration = 0;
  y_gyro_calibration = 0;
  z_gyro_calibration = 0;
  roll_calibration = 0;
  pitch_calibration = 0;  
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
  Pitch=roll_angle*A+(1-A)*(imu_data[0]*imu_diff+Pitch);
  Roll=pitch_angle*A+(1-A)*(imu_data[1]*imu_diff+Roll);
  
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
   set_PWM(0, 1000); // motor 0
   set_PWM(1, 1000); // motor 1
   set_PWM(2, 1000); // motor 2
   set_PWM(3, 1000); // motor 3
   printf("\n Ending program\n\r");

   run_program=0;
}

 
void safety_check(Keyboard *keyp, Position *lp)
{
  /*
Any gyro rate > 300 degrees/sec
Any Accelerometer value >1.8 g�s
All Accelerometer values <.25 g�s
Roll angle > 45 or <-45
Pitch angle >45 or <-45
Keyboard press of space
Keyboard timeout
Control+c is pressed in student code

  */
  
  //week2

  gettimeofday(&tem,NULL);
  long curr_time=tem.tv_sec*1000LL+tem.tv_usec/1000;  
  //printf("%ld    \n", tem.tv_sec);
  
  if (abs(imu_data[0]) > 300)
  {
     printf("Program ends: gyro x absolute rate larger than 300 deg/sec.\n\r");
     run_program=0;
  }
  else if (abs(imu_data[1]) > 300)
  {
     printf("Program ends: gyro y absolute rate larger than 300 deg/sec.\n\r");
     run_program=0;
  }
  else if (abs(imu_data[2]) > 300)
  {
     printf("Program ends: gyro z absolute rate larger than 300 deg/sec.\n\r");
     run_program=0;
  }/*
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
  }*/
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
  
  if (abs(lp->x - 0) > 1000) 
  {
    printf("Exceed 1000 distance from desired x. \n\r");
    run_program = 0;
  } 
  else if (abs(lp->y - 0) > 1000) 
  {
    printf("Exceed 1000 distance from desired y. \n\r");
    run_program = 0;
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
  
  if (lp->version != oldversion)
  {
    oldversion=lp->version;
    version_timer=curr_time;
  } 
  else if(curr_time-version_timer>500)
  {
    printf("Vive Timeout. \n");
    run_program=0;      
  }
}

/*
Week 3 
Functions
*/

void init_pwm()
{

    pwm=wiringPiI2CSetup (0x40);
    if(pwm==-1)
    {
      printf("-----cant connect to I2C device %d --------\n",pwm);
     
    }
    else
    {
  
      float freq =400.0*.95;
      float prescaleval = 25000000;
      prescaleval /= 4096;
      prescaleval /= freq;
      prescaleval -= 1;
      uint8_t prescale = floor(prescaleval+0.5);
      int settings = wiringPiI2CReadReg8(pwm, 0x00) & 0x7F;
      int sleep	= settings | 0x10;
      int wake 	= settings & 0xef;
      int restart = wake | 0x80;
      wiringPiI2CWriteReg8(pwm, 0x00, sleep);
      wiringPiI2CWriteReg8(pwm, 0xfe, prescale);
      wiringPiI2CWriteReg8(pwm, 0x00, wake);
      delay(10);
      wiringPiI2CWriteReg8(pwm, 0x00, restart|0x20);
    }
}



void init_motor(uint8_t channel)
{
	int on_value=0;

	int time_on_us=900;
	uint16_t off_value=round((time_on_us*4096.f)/(1000000.f/400.0));

	wiringPiI2CWriteReg8(pwm, LED0_ON_L + LED_MULTIPLYER * channel, on_value & 0xFF);
	wiringPiI2CWriteReg8(pwm, LED0_ON_H + LED_MULTIPLYER * channel, on_value >> 8);
	wiringPiI2CWriteReg8(pwm, LED0_OFF_L + LED_MULTIPLYER * channel, off_value & 0xFF);
	wiringPiI2CWriteReg8(pwm, LED0_OFF_H + LED_MULTIPLYER * channel, off_value >> 8);
	delay(100);

	 time_on_us=1200;
	 off_value=round((time_on_us*4096.f)/(1000000.f/400.0));

	wiringPiI2CWriteReg8(pwm, LED0_ON_L + LED_MULTIPLYER * channel, on_value & 0xFF);
	wiringPiI2CWriteReg8(pwm, LED0_ON_H + LED_MULTIPLYER * channel, on_value >> 8);
	wiringPiI2CWriteReg8(pwm, LED0_OFF_L + LED_MULTIPLYER * channel, off_value & 0xFF);
	wiringPiI2CWriteReg8(pwm, LED0_OFF_H + LED_MULTIPLYER * channel, off_value >> 8);
	delay(100);

	 time_on_us=1000;
	 off_value=round((time_on_us*4096.f)/(1000000.f/400.0));

	wiringPiI2CWriteReg8(pwm, LED0_ON_L + LED_MULTIPLYER * channel, on_value & 0xFF);
	wiringPiI2CWriteReg8(pwm, LED0_ON_H + LED_MULTIPLYER * channel, on_value >> 8);
	wiringPiI2CWriteReg8(pwm, LED0_OFF_L + LED_MULTIPLYER * channel, off_value & 0xFF);
	wiringPiI2CWriteReg8(pwm, LED0_OFF_H + LED_MULTIPLYER * channel, off_value >> 8);
	delay(100);

}


void set_PWM( uint8_t channel, float time_on_us)
{
  //if(run_program==1)
  //{
    if(time_on_us>PWM_MAX)
    {
      time_on_us=PWM_MAX;
    }
    else if(time_on_us<1000)
    {
      time_on_us=1000;
    }
  	uint16_t off_value=round((time_on_us*4096.f)/(1000000.f/400.0));
  	wiringPiI2CWriteReg16(pwm, LED0_OFF_L + LED_MULTIPLYER * channel,off_value);
  //}
}

void pid_update(/*FILE *f*/)
{ 
  // PID for pitch 
  float P_pitch = 12;//22; ///13; //8.5; //9.5;      //P = 8
  float D_pitch = 300;//580; //200; //180;      //150;      //D = 150
  float I_pitch = 0.02;//0.7;  //0.02;     //0.012;    //I = 0.015
  // PID for roll
  float P_roll = 12;//22;
  float D_roll = 300;//560;
  float I_roll = 0.02;//0.7;
  // week 6
  // P control for yaw
  float P_yaw = 1;
  float P_yaw_vive = 100;
  /*
  // proportional control for motor 0 and 2, which use + 
  pwm_control[0] = neutral_power + (Pitch - 0) * P;
  pwm_control[2] = neutral_power + (Pitch - 0) * P;
  // proportional control for motor 1 and 3, which use -
  pwm_control[1] = neutral_power - (Pitch - 0) * P;
  pwm_control[3] = neutral_power - (Pitch - 0) * P;
  */
  float pitch_error = Pitch - Desire_pitch;
  float roll_error = Roll - Desire_roll;
  float yaw_error = imu_data[2] - Desire_yaw;  
  float pitch_velocity;
  float roll_velocity;
  pitch_velocity = Pitch - previous_pitch;
  roll_velocity = Roll - previous_roll;
  //float roll_error = Roll - 0;
  //float roll_velocity;
  //roll_velocity = Roll - previous_roll;  
  //printf("%f    %f    \n", Desire_pitch, Pitch);
  //printf("%f      %f    \n", Desire_roll, Roll);
  //printf("%f      %f      \n", Desire_yaw, imu_data[2]);
  //printf("%f      %f    \n", Pitch, Roll);
  
  
  /*
  // differential control for motor 0 and 2, which use + 
  pwm_control[0] = neutral_power + pitch_velocity * D;
  pwm_control[2] = neutral_power + pitch_velocity * D;
  // differential control for motor 1 and 3, which use - 
  pwm_control[1] = neutral_power - pitch_velocity * D;
  pwm_control[3] = neutral_power - pitch_velocity * D;
  */
  //printf("%10.5f    %10.5f    %10.5f    %10.5f    %10.5f\n", pitch_velocity, pwm_control[0], pwm_control[1], pwm_control[2], pwm_control[3]);
  int limit = 150;
  
  //Do we need to add another limit?
  
  
  pitch_integral += pitch_error * I_pitch;
  
  roll_integral += roll_error * I_roll;
  
  if (fabs(pitch_integral) > limit) 
  {
    pitch_integral = (pitch_integral/fabs(pitch_integral)) * limit;
  }
  
  if (fabs(roll_integral) > limit) 
  {
    roll_integral = (roll_integral/fabs(roll_integral)) * limit;
  }
  
  //combination of Proportional, Differential and Integral control
  //for motor 0 and 2
  pwm_control[0] = Thrust + pitch_error * P_pitch + pitch_velocity * D_pitch + pitch_integral + roll_error * P_roll + roll_velocity * D_roll + roll_integral - yaw_error * P_yaw;
  pwm_control[2] = Thrust + pitch_error * P_pitch + pitch_velocity * D_pitch + pitch_integral - roll_error * P_roll - roll_velocity * D_roll - roll_integral + yaw_error * P_yaw;
  //for motor 1 and 3
  pwm_control[1] = Thrust - pitch_error * P_pitch - pitch_velocity * D_pitch - pitch_integral + roll_error * P_roll + roll_velocity * D_roll + roll_integral + yaw_error * P_yaw;
  pwm_control[3] = Thrust - pitch_error * P_pitch - pitch_velocity * D_pitch - pitch_integral - roll_error * P_roll - roll_velocity * D_roll - roll_integral - yaw_error * P_yaw;
  
  //We need to modify(add) Roll PID to the PWM values.
  
  previous_pitch = Pitch;
  previous_roll = Roll;
  
  
  //fprintf(f, "%f,%f,%f,%f,%f\n", Pitch, pwm_control[0] - neutral_power, pitch_error * P, pitch_velocity * D, pitch_integral);
  //printf("%10.5f    %10.5f    %10.5f    %10.5f    %10.5f\n", Pitch, pwm_control[0] - 1400, pitch_error * P, pitch_velocity * D, pitch_integral); 
}

void get_joystick(Keyboard *keyp) 
{
  float neutral_power = 1650;
  Thrust = neutral_power - ((keyp->thrust - 128) / 112) * 100;
  //float Desire_pitch = 0;
  Desire_pitch_joystick = -((keyp->pitch - 128) / 112) * 5;
  Desire_roll_joystick = ((keyp->roll - 128) / 112) * 5;
  //Desire_yaw = ((keyp->yaw - 128) / 112) * 90;
  
  //printf("%f    %f    \n", Desire_pitch_joystick, Desire_roll_joystick);
  //printf("%c  %f  %f  %f  %f  %d\n", keyp->key_press, keyp->pitch, keyp->roll, keyp->yaw, keyp->thrust, keyp->heartbeat);
  //printf("%f  \n", keyp->thrust);
  //Do we need to add another series of PID values?;
}

void vive_control(Position *lp) 
{
  //week8
  Desire_yaw = lp->yaw * 200;
  if (Desire_yaw > 90) {
    Desire_yaw = 90;
  }
  else if (Desire_yaw < -90)
  {
    Desire_yaw = -90;
  }
  //week9
  float P_x_vive = 0.05; //0.03;
  float P_y_vive = 0.05;
  float D_x_vive = 1.6; //0.7; //0.4;
  float D_y_vive = 1.5; //0.7;
  x_vive_estimate = x_vive_estimate * 0.6 + lp->x * 0.4;
  y_vive_estimate = y_vive_estimate * 0.6 + lp->y * 0.4;
  if (lp->version != oldversion)
  {
    D_term_x = D_x_vive * (x_vive_estimate - x_vive_prev);
    D_term_y = D_y_vive * (y_vive_estimate - y_vive_prev);
  }
  Desire_roll_vive = P_x_vive * (-1) * (x_vive_estimate - Desire_x) - D_term_x;
  Desire_pitch_vive = P_y_vive * (y_vive_estimate - Desire_y) + D_term_y;
  if (abs(Desire_roll_vive) > 5) {
    Desire_roll_vive = 5 * Desire_roll_vive / abs(Desire_roll_vive);
  }
  if (abs(Desire_pitch_vive) > 5) {
    Desire_pitch_vive = 5 * Desire_pitch_vive / abs(Desire_pitch_vive);
  }
  
  Desire_roll = Desire_roll_joystick * 0.5 + Desire_roll_vive * 0.5;
  Desire_pitch = Desire_pitch_joystick * 0.5 + Desire_pitch_vive * 0.5;
  
  

  //printf("%f     %f      %f      %f\n", x_vive_estimate, x_vive_prev, P_x_vive * (-1) * (x_vive_estimate - Desire_x), - D_term_x);
  printf("%f    %f    %f    %f    %f    %f\n", lp->x, lp->y, P_x_vive * (-1) * (x_vive_estimate - Desire_x), - D_term_x, P_y_vive * (y_vive_estimate - Desire_y), D_term_y);
  x_vive_prev = lp->x;
  y_vive_prev = lp->y;
}