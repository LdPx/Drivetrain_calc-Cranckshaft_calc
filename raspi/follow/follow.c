#include <unistd.h>
#include <fcntl.h>       
#include <stdio.h>  
#include <time.h>     
#include <sys/param.h>

#define SENSOR_FILE "/dev/sensor"
#define MOTOR_LEFT_FILE "/dev/motor-left"
#define MOTOR_RIGHT_FILE "/dev/motor-right"

#define MIN_DIST_UM 200000
#define MAX_DIST_UM 600000
#define AVG_DIST_UM 250000

#define NUM_DIST_VALUES 3
#define NUM_FAR_VALUES 10

int MOTOR_GO = 1;
int MOTOR_STOP = 0;
int MOTOR_BACK = -1; 

enum Mode {ROTATE, FOLLOW_GO, FOLLOW_STOP, FOLLOW_BACK};
const char* mode_name[] = {"ROTATE", "FOLLOW_GO", "FOLLOW_STOP", "FOLLOW_BACK"};


unsigned long get_distance(int sensor){
	unsigned long distance;
	while(read(sensor, &distance, sizeof(distance)) < 0){
		perror("get_distance()");
	}
	return distance;
}


unsigned long get_stable_distance(int sensor, int n){
	unsigned long sum = 0;
	int i;	
	for(i = 0; i < n; i++){
		sum += get_distance(sensor);
	}
	return sum / n;
}

unsigned long median3(unsigned long a, unsigned long b, unsigned long c){
	return MAX(MIN(a,b), MIN(MAX(a,b), c));
}

unsigned long get_stable_distance_med3(int sensor){
	return median3(get_distance(sensor), get_distance(sensor), get_distance(sensor));
}

void set_mode(int motor_left, int motor_right, enum Mode *mode, enum Mode newmode){
	int mode_motor_vals[][2] = {{MOTOR_STOP,MOTOR_GO}, {MOTOR_GO,MOTOR_GO}, {MOTOR_STOP,MOTOR_STOP}, {MOTOR_BACK, MOTOR_BACK}};
	*mode = newmode;
	write(motor_left, &mode_motor_vals[(int)newmode][0], sizeof(mode_motor_vals[(int)newmode][0]));
	write(motor_right, &mode_motor_vals[(int)newmode][1], sizeof(mode_motor_vals[(int)newmode][1]));
}


int main(int argc, char **argv){
	int motor_left = open(MOTOR_LEFT_FILE, O_WRONLY);
	int motor_right = open(MOTOR_RIGHT_FILE, O_WRONLY);
	int sensor = open(SENSOR_FILE, O_RDONLY);
	if(motor_left < 0 || motor_right < 0 || sensor < 0){
		perror("open()");
		return 1;
	}
	enum Mode mode;
	set_mode(motor_left, motor_right, &mode, ROTATE);
	int num_far_values = 0;
	while(1){	
		//unsigned long dist = get_distance(sensor);
		//unsigned long dist = get_stable_distance(sensor, NUM_DIST_VALUES);
		unsigned long dist = get_stable_distance_med3(sensor);
		printf("mode %s, read distance %lu µm\n", mode_name[mode], dist);		
		if(mode == ROTATE /*&& dist >= MIN_DIST_UM*/ && dist <= MAX_DIST_UM){
			num_far_values = 0;
			if(dist <= AVG_DIST_UM){
				//set_mode(motor_left, motor_right, &mode, FOLLOW_STOP);
				set_mode(motor_left, motor_right, &mode, FOLLOW_BACK);	
			}
			else {
				set_mode(motor_left, motor_right, &mode, FOLLOW_GO);
			}
		}
		else if(mode != ROTATE && dist > MAX_DIST_UM) {
			num_far_values += 1;
			if(num_far_values >= NUM_FAR_VALUES){
				set_mode(motor_left, motor_right, &mode, ROTATE);
			}
		}
		else if(mode == FOLLOW_GO && dist <= AVG_DIST_UM){
			//set_mode(motor_left, motor_right, &mode, FOLLOW_STOP);
			set_mode(motor_left, motor_right, &mode, FOLLOW_BACK);				
		}
		//else if(mode == FOLLOW_STOP && dist > AVG_DIST_UM){
		else if(mode == FOLLOW_BACK && dist > AVG_DIST_UM){
			set_mode(motor_left, motor_right, &mode, FOLLOW_GO);	
		}
	}
	return 1;
}

