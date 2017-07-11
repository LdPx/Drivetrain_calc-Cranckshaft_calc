#include <unistd.h>
#include <fcntl.h>       
#include <stdio.h>  
#include <time.h>     

#define MOTOR_LEFT_FILE "/dev/motor-left"
#define MOTOR_RIGHT_FILE "/dev/motor-right"

#define MOTOR_GO 1
#define MOTOR_STOP 0
#define MOTOR_BACK -1 

enum Direction{FOR='f',BACK='b',RIGHT='r',LEFT='l',STOP='s',QUIT='q'};


int main(int argc, char **argv){
	int motor_left = open(MOTOR_LEFT_FILE, O_WRONLY);
	int motor_right = open(MOTOR_RIGHT_FILE, O_WRONLY);
	if(motor_left < 0 || motor_right < 0){
		perror("opening motors failed");
		return 1;
	}
	while(1){
		puts("commands: 'f', 'b', 'r', 'l', 's', 'q'");		
		int motor_left_val;
		int motor_right_val;
		char c = getchar();
		switch(c){
			case FOR:     
				puts("FOR");  
				motor_left_val = MOTOR_GO;
				motor_right_val = MOTOR_GO;
				break;
			case BACK:
				puts("BACK");
				motor_left_val = MOTOR_BACK;
				motor_right_val = MOTOR_BACK;
				break;
			case LEFT:
				puts("LEFT");
				motor_left_val = MOTOR_STOP;
				motor_right_val = MOTOR_GO;
				break;
			case RIGHT:
				puts("RIGHT");
				motor_left_val = MOTOR_GO;
				motor_right_val = MOTOR_STOP;
				break;
			case STOP:
				puts("STOP");
				motor_left_val = MOTOR_STOP;
				motor_right_val = MOTOR_STOP;
				break;
			case QUIT:
				puts("QUIT");
				close(motor_left);
				close(motor_right);
				return 0;
			default:
				continue;
		}		
		write(motor_left, &motor_left_val, sizeof(motor_left_val));
		write(motor_right, &motor_right_val, sizeof(motor_right_val));
		struct timespec timeout = {
			tv_sec: 1,
			tv_nsec: 0
		};
		clock_nanosleep(CLOCK_MONOTONIC, 0, &timeout, NULL);
		write(motor_left, MOTOR_STOP, sizeof(MOTOR_STOP));
		write(motor_right, MOTOR_STOP, sizeof(MOTOR_STOP));
	}
	return 1;
}

