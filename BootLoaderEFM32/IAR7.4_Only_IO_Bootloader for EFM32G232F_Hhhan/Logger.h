#ifndef __PROFILER__
#define __PROFILER__  
  #ifdef ENABLE_LOGGER
    //RETURNS THE SIZE OF THE STRING POINTED BY THE CHAR POINTER
    unsigned short size_of_string(unsigned char *ptr)
    { 
       unsigned short charCnt = 0;     
       while (*ptr++ != '\0')
            charCnt++; 
       return charCnt;
    }
  
    char message[36];

    unsigned char *tempPointer = 0;
    
    void profileLog(unsigned char* str)
    {
      Com_Write(READ_REGISTER, GET_LOGGER_DATA, size_of_string(tempPointer), tempPointer);
    }
  
    #define log(x) profileLog(tempPointer = x)
  #else
    #define log(x)
  #endif
#endif