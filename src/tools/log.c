#include<stdio.h>
#include<time.h>
#include<string.h>
#include<stdarg.h>
#include<stdlib.h>
#include"reversi/log.h"
const const char* LOG_FILE="Reversi.log";
/*TODO：多线程？*/
#define TIME_BUF_LEN 256
void get_time(char*time){
    struct timespec ts;
    if(0!=clock_gettime(CLOCK_REALTIME,&ts)) {
        perror("log: clock_gettime failed");
        time[0]='\0';
        return;
    }
    time_t now=ts.tv_sec;
    struct tm tm_now;
    if(NULL==localtime_r(&now,&tm_now)){
        perror("log: localtime_r failed");
        time[0]='\0';
        return;
    }
    char buf[80];
    if(0==strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&tm_now)){
        fputs("log: strftime failed",stderr);
        time[0]='\0';
        return;
    }
    int len=snprintf(time,TIME_BUF_LEN,"%s.%09ld\n",buf,ts.tv_nsec);
    if(len<0){
        fputs("log: snprintf format string failed!",stderr);
    }else if(len>=TIME_BUF_LEN){
        fputs("log: snprintf time string too long!",stderr);
    }else{
        time[len-1]=0;/*清空末尾换行符*/
    }
}
void log(const char*level,const char*s){
    char time[TIME_BUF_LEN];
    get_time(time);
    FILE *fp=fopen(LOG_FILE,"a");
    if(NULL==fp){
        perror("log: fopen failed!");
        return;
    }
    if(EOF==fputs(time,fp)){
        perror("log: fputs time failed");
        fclose(fp);
        return;
    }
    if(EOF==fputc(' ',fp)){
        perror("log: fputc space failed");
        fclose(fp);
        return;
    }
    if(EOF==fputs(level,fp)){
        perror("log: fputs level failed");
        fclose(fp);
        return;
    }
    if(EOF==fputs(": ",fp)){
        perror("log: fputs delimiter failed");
        fclose(fp);
        return;
    }
    if(EOF==fputs(s,fp)){
        perror("log: fputs content failed");
        fclose(fp);
        return;
    }
    if(EOF==fputc('\n',fp)){
        perror("log: fputc line break failed");
        fclose(fp);
        return;
    }
    if(EOF==fclose(fp)){
        perror("log: fclose failed");
        return;
    }
}
#undef TIME_BUF_LEN
void InfoLog(const char*s,...){
    va_list ap;
    va_list ap2;
    va_start(ap,s);
    va_copy(ap2,ap);
    int len=vsnprintf(NULL,0,s,ap);
    va_end(ap);
    if(len<0){
        va_end(ap2);
        return;
    }
    char *buf=(char*)malloc((len+1)*sizeof(char));
    int n=vsnprintf(buf,(len+1)*sizeof(char),s,ap2);
    va_end(ap2);
    log("Info",buf);
    free(buf);
}
void ErrorLog(const char*s,...){
    va_list ap;
    va_list ap2;
    va_start(ap,s);
    va_copy(ap2,ap);
    int len=vsnprintf(NULL,0,s,ap);
    va_end(ap);
    if(len<0){
        va_end(ap2);
        return;
    }
    char *buf=(char*)malloc((len+1)*sizeof(char));
    int n=vsnprintf(buf,(len+1)*sizeof(char),s,ap2);
    va_end(ap2);
    log("Error",buf);
    free(buf);
}
#ifdef DEBUG
void debug_log(const char*s,...){
    va_list ap;
    va_list ap2;
    va_start(ap,s);
    va_copy(ap2,ap);
    int len=vsnprintf(NULL,0,s,ap);
    va_end(ap);
    if(len<0){
        va_end(ap2);
        return;
    }
    char *buf=(char*)malloc((len+1)*sizeof(char));
    int n=vsnprintf(buf,(len+1)*sizeof(char),s,ap2);
    va_end(ap2);
    log("Debug",buf);
    free(buf);
}
#endif