#!/bin/sh
cd /sys/class/gpio
echo 8 > export                                                               
echo 23 > export                                                              
echo 24 > export                                                              
echo 25 > export                                                              
echo out > gpio8/direction                                                    
echo out > gpio23/direction                                                   
echo out > gpio24/direction                                                   
echo out > gpio25/direction                                                   
echo 0 > gpio8/value                                                          
echo 0 > gpio23/value                                                         
echo 0 > gpio24/value                                                         
echo 0 > gpio25/value  

