#ifndef TOOLS_LOG_H
#define TOOLS_LOG_H
void InfoLog(const char*s);
void ErrorLog(const char*s);
#ifdef DEBUG
void debug_log(const char*s);
#define DebugLog(s) do{debug_log(s);}while(0)
#else
#define DebugLog(s) (void(0))
#endif
#endif