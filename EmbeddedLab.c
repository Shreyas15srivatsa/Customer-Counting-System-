 /* NAME: Puranjoy saha, Max Zhou, Shreyas
 TITLE: LAB 3
 FUNCTIONALITY OF PROGRAM: This program will count the number  of people entering and exiting a room using the inputs from two laser diodes. When someone walks throught the laser the signal to the diode will be cut and this will allow us to calculate the number of people in and out
 */



#include "gpiolib_addr.h"
#include "gpiolib_reg.h"

#include <signal.h>
#include <unisztd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/time.h>

#include <stdint.h>
#include <stdio.h>        //for the printf() function
#include <fcntl.h>
#include <linux/watchdog.h>     //needed for the watchdog specific constants
#include <unistd.h>         //needed for sleep
#include <sys/ioctl.h>         //needed for the ioctl function
#include <stdlib.h>         //for atoi
#include <time.h>         //for time_t and the time() function

#define PRINT_MSG(file, time, programName, str) \
do{ \
fprintf(file, "%s : %s : %s", time, programName, str); \
fflush(file); \
}while(0)



//HARDWARE DEPENDENT CODE BELOW

#ifndef MARMOSET_TESTING

/* You may want to create helper functions for the Hardware Dependent functions*/
//This function should initialize the GPIO pins

GPIO_Handle initializeGPIO(){
    //This is the same initialization that was done in Lab 2
    GPIO_Handle gpio;
    gpio = gpiolib_init_gpio();
    if(gpio == NULL){
        perror("Could not initialize GPIO");
        
    }
    return gpio;
}

struct parameterPassedToConfigfile{
    int timeout;
    char logFileName[50] = "LOGFILE";
    char statsFileName[50] = "STATSFILE";
    char watchDog[50] = "WATACHDOG";
    char stringInput[50];
    char stringInputAfterEqual[50];
    char programName[50];
    
};

//This function will get the current time using the gettimeofday function
void getTime(char* buffer)
{
    //Create a timeval struct named tv
    struct timeval tv;
    
    //Create a time_t variable named curtime
    time_t curtime;
    
    
    //Get the current time and store it in the tv struct
    gettimeofday(&tv, NULL);
    
    //Set curtime to be equal to the number of seconds in tv
    curtime=tv.tv_sec;
    
    //This will set buffer to be equal to a string that in
    //equivalent to the current date, in a month, day, year and
    //the current time in 24 hour notation.
    strftime(buffer,30,"%m-%d-%Y  %T.",localtime(&curtime));
    
}

char defaultLocation[50] = "/home/pi/LaserLab.log"; ;

//Open file function
FILE* OpenLogFile(char logFileName[],char programName[], FILE* file){
    if(file)
        fclose(file);
    char buffer[20];
    getTime(buffer);
    
    //FILE* logfile;
    file = fopen(logFileName, "a");
    if(!file){
        file = fopen(defaultLocation, "a");
        PRINT_MSG(file, buffer, programName, "failed to open given logfile, using deafult");
    }
    
        return file;
    
}

bool StringCompare(char inputString[], char string[])
{
    int iCounter = 0;
    while (((((int) inputString[iCounter] >= 65) && ((int) inputString[iCounter] <= 90)) || (((int) inputString[iCounter] >= 97) && (((int) inputString[iCounter] <= 122)) && ((int) string[iCounter] >= 65) && ((int) string[iCounter] <= 90)) || (((int) string[iCounter] >= 97) && ((int) string[iCounter] <= 122))))
    {
        if (((int)inputString[iCounter]%32) == ((int)string[iCounter]%32))
        {
            iCounter++;
        }
        else
        {
            return false;
        }
    }
    return true;
}

//bool compareString(){
//    if(logfilename == given input){
//        return true;
//         logfileName = inputstringafter;
//    }
//    //if its comething you
//}

void readConfig(FILE* configFile, parameterPassedToConfigfile *param)/*int* timeout,char* inputString, char*inputStringAfterEqual,char* logFileName,char* statsFileName, char* watchDog)*/{
    //Loop counter
    
    int i = 0;
    
    //A char array to act as a buffer for the file
    char buffer[255];
    
    //This will
    //fgets(buffer, 255, configFile);
    //This will check that the file can still be read from and if it can,
    //then the loop will check to see if the line may have any useful
    //information.
    FILE* file = NULL;
    char time[30];
    OpenLogFile(param->logFileName, param->programName,file);
    enum State{START,WHITESPACE1,WHITESPACE2,WHITESPACE3,WHITESPACE4,PARAMETER,COMMENTEQUAL,VALUE,DONE};
    while(fgets(buffer, 255, configFile) != NULL){
        i = 0;
        int j = 0;
        //If the starting character of the string is a '#',
        //then we can ignore that line
        //if(buffer[i] != '#')
        //{
        State state = START;
        while(buffer[i] != 0)
        {
            switch (state) {
                case START:
                    if(buffer[i] == ' ' || buffer[i]== '\t'){
                        state = WHITESPACE1;
                    }
                    else if((buffer[i] >='A' && buffer[i] <='Z') || (buffer[i] >='a' && buffer[i] <='z')|| (buffer[i] == '/')){
                        param->stringInput[j] = buffer[i];
                        state = PARAMETER;
                        j++;
                    }
                    else{
                        getTime(time);
                        PRINT_MSG(file, time, param->programName, "Unexpected character in configfile");
                    }
                    break;
                case WHITESPACE1:

                    if((buffer[i] >='A' && buffer[i] <='Z') || (buffer[i] >='a' && buffer[i] <='z')){

                        param->stringInput[j] = buffer[i];
                        state = PARAMETER;
                    }
                    else if(buffer[i] == ' ' || buffer[i] == '\t'){
                        state = WHITESPACE1;
                    }
                    else if(buffer[i]=='#'){
                        state=COMMENT;
                    }
                    else{
                        getTime(time);
                        PRINT_MSG(file, time, param->programName, "Unexpected character in configfile");
                    }
                    break;
                case PARAMETER:
                    if((buffer[i] >='A' && buffer[i] <='Z') || (buffer[i] >='a' && buffer[i] <='z')|| (buffer[i] == '/')){
                        param->stringInput[j] = buffer[i];
                        state = PARAMETER;
                        j++;
                    }
                    else if(buffer[i] == ' ' || buffer[i]== '\t'){
                        state = WHITESPACE2;
                    }
                    else if(buffer[i] =='='){
                        state = EQUAL;
                    }
                    else{
                        getTime(time);
                        PRINT_MSG(file, time, param->programName, "Unexpected character in configfile");
                    }
                    break;
                case WHITESPACE2:
                    if(buffer[i] =='='){
                            state = EQUAL;
                    }
                    else if (buffer[i] == ' ' || buffer[i]== '\t'){
                        state = WHITESPACE3;
                    }
                    else{
                        getTime(time);
                        PRINT_MSG(file, time, param->programName, "Unexpected character in configfile");
                    }
                    break;
                case EQUAL:
                    if(buffer[i] == ' ' || buffer[i]== '\t'){

                        state = WHITESPACE3;
                    }
                    if(isprint(buffer[i])){
                        param->stringInputAfterEqual[j] = buffer[i];
                        state=VALUE;
                        j++;
                    }
                    else{
                        getTime(time);
                        PRINT_MSG(file, time, param->programName, "Unexpected character in configfile");
                    }
                    break;
                case WHITESPACE3:
                    if(buffer[i] == ' ' || buffer[i]== '\t'){
                        state = WHITESPACE3;
                    }
                    else if(isprint(buffer[i])){
                        state = VALUE;
                    }
                    else{
                        getTime(time);
                        PRINT_MSG(file, time, param->programName, "Unexpected character in configfile");
                    }
                    break;
                case COMMENT:
                    if(buffer[i]=='#'){
                        state=COMMENT;
                    }
                    else if(buffer[i]==0){
                        state=DONE;
                    }
                    else{
                        getTime(time);
                        PRINT_MSG(file, time, param->programName, "Unexpected character in configfile");
                    }
                    break;
                case VALUE:
                    if(isprint(buffer[i])){
                        param->stringInputAfterEqual[j] = buffer[i];
                        state=VALUE;
                        j++;
                    }
                    else if(buffer[i] == ' ' || buffer[i]== '\t'){
                        state = WHITESPACE4;
                    }
                    else if(buffer[i] == 0){
                        state = DONE;
                    }
                    else{
                        getTime(time);
                        PRINT_MSG(file, time, param->programName, "Unexpected character in configfile");
                    }
                    break;
                case WHITESPACE4:
                    if(buffer[i] == ' ' || buffer[i]== '\t'){
                        state = WHITESPACE4;
                    }
                    else if(buffer[i] == 0){
                        state = DONE;
                    }
                    else{
                        getTime(time);
                        PRINT_MSG(file, time, param->programName, "Unexpected character in configfile");
                    }
                    break;
                case DONE:
                    if(StringCompare(param->logFileName,param->stringInput)){
                        for(int i = 0; param->stringInputAfterEqual[i] != 0; i++ ){
                            param->logFileName[i] = param->stringInputAfterEqual[i];
                        }
                    }
                    else if(StringCompare(param->statsFileName,param->stringInput)){
                            //param.statsFileName = param.stringInputAfterEqual;
                        for(int i = 0; param->stringInputAfterEqual[i] != 0; i++ ){
                            param->statsFileName[i] = param->stringInputAfterEqual[i];
                        }
                    }
                    else if(StringCompare(param->watchDog,param->stringInput)){
                        param->timeout = 0;
                        for(int i = 0; param->stringInputAfterEqual[i] != 0; i++ ){
                            param->timeout = (param->timeout * 10 )+ (param->stringInputAfterEqual[i] - '0');
                        }
                    }
                    break;
                default:
                    break;
            } i++;
        }
    }
}

//This function should accept the diode number (1 or 2) and output
//a 0 if the laser beam is not reaching the diode, a 1 if the laser
//beam is reaching the diode or -1 if an error occurs.

#define LASER1_PIN_NUM 4//This will replace LASER1_PIN_NUM with 4 when compiled
#define LASER2_PIN_NUM 6//This will replace LASER2_PIN_NUM with 4 when compiled

int laserDiodeStatus(GPIO_Handle gpio, int diodeNumber){
    // if there is no input or any other error then -1 will be returned
    if(gpio == NULL){
        
        return -1;
        }
    //outputs the state of the first laser diode
    if(diodeNumber == 1){
        //declareing the level register
        uint32_t level_reg = gpiolib_read_reg(gpio, GPLEV(0));
        
        // this is used to check if light is reaching the photodiode return 1
        if(level_reg & (1 << LASER1_PIN_NUM)){
            return 1;
        }
        // if light is not reaching the photo diode return 0
        else{
            return 0;
            }
        
    }
    //outputs the state of the second laser diode
    if(diodeNumber == 2){
        uint32_t level_reg = gpiolib_read_reg(gpio, GPLEV(0));
        if(level_reg & (1 << LASER2_PIN_NUM)){
            return 1;
        }
        else{
            return 0;
        }
    }
    else{
        return -1;
    }
}

#endif
//END OF HARDWARE DEPENDENT CODE
//laser1Count will be how many times laser 1 is broken (the left laser).
//laser2Count will be how many times laser 2 is broken (the right laser).
//numberIn will be the number  of objects that moved into the room.
//numberOut will be the number of objects that moved out of the room.

void outputMessage(int laser1Count, int laser2Count, int numberIn, int numberOut,char* time,char* programName, FILE* file){
    
    fprintf(file, "%s : %s :Laser 1 was broken %d times \n", time, programName, laser1Count);
    fprintf(file, "%s : %s :Laser 2 was broken %d times \n", time, programName, laser2Count);
    fprintf(file, "%s : %s objects entered the room %d  \n", time, programName, numberIn);
    fprintf(file, "%s : %s objects exitted the room %d  \n", time, programName, numberOut);
   // printf("Laser 2 was broken %d times \n", laser2Count);
    //printf("%d objects entered the room \n", numberIn);
    //printf("%d objects exitted the room \n", numberOut);
    
}


//This function accepts an errorCode. You can define what the corresponding error code
//will be for each type of error that may occur.
void errorMessage(int errorCode){
    fprintf(stderr, "An error occured; the error code was %d \n", errorCode);
}


#ifndef MARMOSET_TESTING

int main(const int argc, const char* const argv[]){
    
    //Create a string that contains the program name
    const char* argName = argv[0];
    
    //These variables will be used to count how long the name of the program is
    int i = 0;
    int fileNameLength = 0;
    
    for(i; argName[i] != 0; i++){
         fileNameLength++;
    }
    char programName[fileNameLength];
    char time1[30];
    getTime(time1);
    
    i = 0;
    //Copy the name of the program without the ./ at the start
    //of argv[0]
    while(argName[i + 2] != 0){
        programName[i] = argName[i + 2];
        i++;
    }
    
    parameterPassedToConfigfile param;
    param.timeout = -3;

    //Create a file pointer named configFile
    FILE* configFile;
    //Set configFile to point to the Lab4Sample.cfg file. It is
    //set to read the file.
    configFile = fopen("/home/pi/Lab4Sample.cfg", "r");
    
    //Output a warning message if the file cannot be openned
    if(!configFile)
    {
        perror("The config file could not be opened");
        return -1;
    }
    
   //Call the readConfig function to read from the config file
    readConfig(configFile, &param);
    
    //Close the configFile now that we have finished reading from it
    fclose(configFile);
    
    if((param.timeout <= 1) || (param.timeout >= 15)){
        param.timeout = 10;
    }
    //Create a new file pointer to point to the log file
    FILE* logFile;
    //Set it to point to the file from the config file and make it append to
    //the file when it writes to it.
    logFile = fopen(param.logFileName, "a");
    
    const char defaultLog[50] = "/home/pi/LaserLab.log";
    
    //Check that the file opens properly.
    if(!logFile){
        logFile = fopen(defaultLog, "a");
        fprintf(logFile, "Given log file %s :unable to open", param.logFileName);
        return -1;
    }
    
    getTime(time1);
    PRINT_MSG(logFile, time1, programName, "program has started");
    
    FILE* statsFile;
    //Set it to point to the file from the config file and make it append to
    //the file when it writes to it.
    statsFile = fopen(param.statsFileName, "a");
    char defaultstat[50] = "/home/pi/LaserLab.stat";
    
    //Check that the stats file opens properly.
    if(!statsFile)
    {
        statsFile = fopen(defaultstat, "a");
        PRINT_MSG(logFile, time1, programName, "The stats filed could not be opened");
        perror("The stats file could not be opened");
        return -1;
    }
    
    
    if(argc< 1){
        getTime(time1);
        PRINT_MSG(logFile, time1, programName, "No time given\n");
    }
    
    
    //This variable will be used to access the /dev/watchdog file, similar to how
    //the GPIO_Handle works
    int watchdog;
    
    //We use the open function here to open the /dev/watchdog file. If it does
    //not open, then we output an error message. We do not use fopen() because we
    //do not want to create a file if it doesn't exist
    if ((watchdog = open("/dev/watchdog", O_RDWR | O_NOCTTY)) < 0) {
        printf("Error: Couldn't open watchdog device! %d\n", watchdog);
        return -1;
    }
    //Get the current time
    getTime(time1);
    //Log that the watchdog file has been opened
    PRINT_MSG(logFile, time1, programName, "The Watchdog file has been opened\n\n");
    
    //This line uses the ioctl function to set the time limit of the watchdog
    //timer to 15 seconds. The time limit can not be set higher that 15 seconds
    //so please make a note of that when creating your own programs.
    //If we try to set it to any value greater than 15, then it will reject that
    //value and continue to use the previously set time limit
    ioctl(watchdog, WDIOC_SETTIMEOUT, param.timeout);
    
    //Get the current time
    getTime(time1);
    //Log that the Watchdog time limit has been set
    PRINT_MSG(logFile, time1, programName, "The Watchdog time limit has been set\n\n");
    
    //The value of timeout will be changed to whatever the current time limit of the
    //watchdog timer is
    ioctl(watchdog, WDIOC_GETTIMEOUT, param.timeout);
    
    //This print statement will confirm to us if the time limit has been properly
    //changed. The \n will create a newline character similar to what endl does.
    printf("The watchdog timeout is %d seconds.\n\n", param.timeout);
    
    
    time_t startTime = time(NULL);
    //This variable will represent the amount of time the program should run for
    
    //int timeLimit = atoi(argv[1]);
    
    //In the while condition, we check the current time minus the start time and
    //see if it is less than the number of seconds that was given from the
    //command line.
    
    enum {BROKEN, UNBROKEN};
    typedef enum{START, PD1BROKEN,PARTIALLYIN, PD2BROKEN, PARTIALLYOUT,PD2BROKENOUT,PD1BROKENOUT}State;
    
    int peopleInCounter = 0;
    int peopleOutCounter = 0;
    int PD1broken = 0;
    int PD2broken = 0;
    
    GPIO_Handle gpio = initializeGPIO();
    State state = START;
    while(1){
         //if((time_t(NULL) - startTime) > 60){
           //  PRINT_MSG(statsFile, time, programName,PD1broken);
         //}
        
        //declare the first state as START
        //These two variables are used to get the status of the photo diode as either being on or off
        int PD1 = laserDiodeStatus(gpio, 1);
        int PD2 = laserDiodeStatus(gpio, 2);
        
        //start of the state machine
        
        fprintf(stderr, "PD1: %i PD2: %i\n", PD1, PD2);
        switch(state){
            case START:
                usleep(100);
                
                //fprintf(stderr, "enterd startstate\n");
                
                if(PD1 == BROKEN && PD2 == UNBROKEN) {
                    PD1broken++;
                    state = PD1BROKEN;
                    
                    
                    break;
                }
                if(PD1 == UNBROKEN && PD2 == BROKEN){
                    state = PD2BROKENOUT;
                    PD2broken++;
                }
                
                break;
                
            case PD1BROKEN:
                
                fprintf(stderr, "pd1 broken\n");
                
                usleep(100);
                
                if(PD1 == UNBROKEN && PD2 == UNBROKEN){
                    
                    state = START;
                    break;
                }
                
                else if(PD1 == BROKEN && PD2 == BROKEN){
                    PD2broken++;
                    state = PARTIALLYIN;
                    
                }
                
                break;
                
            case PARTIALLYIN:
                
                fprintf(stderr, "partlly in\n");
                
                usleep(100);
                
                if(PD1 == UNBROKEN && PD2 == BROKEN){
                    
                    state = PD2BROKEN;
                    break;
                    
                }
                if(PD2 == UNBROKEN && PD1 == BROKEN){
                    state = PD1BROKEN;
                }
                break;
                
            case PD2BROKEN:
                
                fprintf(stderr, "pd2 broken\n");
                
                usleep(100);
                
                if(PD1 == UNBROKEN && PD2 == UNBROKEN){
                    
                    state = START;
                    
                    peopleInCounter++;
                    break;
                }
                
                else if(PD1 == BROKEN && PD2 == BROKEN){
                    
                    PD1broken++;
                    state = PARTIALLYIN;
                    
                }
                
                break;
            case PD2BROKENOUT:
                
                fprintf(stderr, "pd2brokenout\n");
                
                usleep(100);
                
                if(PD1 == BROKEN && PD2 == BROKEN){
                    
                    PD1broken++;
                    
                    state = PARTIALLYOUT;
                    break;
                }
                if(PD2 == UNBROKEN && PD1 == UNBROKEN){
                    state = START;
                }
                break;
                
            case PARTIALLYOUT:
                
                fprintf(stderr, "partiallyout\n");
                
                usleep(100);
                
                if(PD1 == BROKEN && PD2 == UNBROKEN){
                    
                    
                    state = PD1BROKENOUT;
                    break;
                    
                }
                if(PD1 == UNBROKEN && PD2 == BROKEN){
                    state = PD2BROKENOUT;
                }
                
                break;
                
            case PD1BROKENOUT:
                
                fprintf(stderr, "pd1Brokenout\n");
                
                usleep(100);
                
                if(PD1 == UNBROKEN && PD2 == UNBROKEN){
                    
                    state = START;
                    
                    peopleOutCounter++;
                    
                    break;
                }
                
                else if(PD1 == BROKEN && PD2 == BROKEN){
                    
                    PD2broken++;
                    
                    state = PARTIALLYOUT;
                    
                }
                break;
        }
        getTime(time1);
        outputMessage(PD1broken, PD2broken, numberIn, numberOut, time1, programName, statsFile);
        
        ioctl(watchdog, WDIOC_KEEPALIVE, 0);
        getTime(time1);
        //Log that the Watchdog was kicked
        PRINT_MSG(logFile, time1, programName, "The Watchdog was kicked\n\n");
        }
    
    
    write(watchdog, "V", 1);
    getTime(time1);
    //Log that the Watchdog was disabled
    PRINT_MSG(logFile, time1, programName, "The Watchdog was disabled\n\n");
    
    //Close the watchdog file so that it is not accidentally tampered with
    close(watchdog);
    getTime(time1);
    //Log that the Watchdog was closed
    PRINT_MSG(logFile, time1, programName, "The Watchdog was closed\n\n");
    
    //Free the gpio pins
    gpiolib_free_gpio(gpio);
    getTime(time1);
    //Log that the GPIO pins were freed
    PRINT_MSG(logFile, time1, programName, "The GPIO pins have been freed\n\n");
    
    return 0;
    
#endif
    
}
