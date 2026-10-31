#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <string>
#include <climits>
using byte=uint8_t; using boolean=bool; using std::min; using std::max;
inline unsigned long host_millis=0;
inline unsigned long millis(){return host_millis;}
inline unsigned long micros(){return host_millis*1000;}
inline void delay(unsigned long ms){host_millis+=ms;}
inline void delayMicroseconds(unsigned int){}
inline void yield(){}
inline void pinMode(int,int){} inline void digitalWrite(int,int){}
inline int digitalRead(int){return 0;} inline int analogRead(int){return 0;}
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define INPUT 0
#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define PROGMEM
#define F(x) x
#define PI 3.14159265358979323846
#define DEG_TO_RAD (PI/180.0)
#define pgm_read_word(p) (*(const uint16_t*)(p))
#define pgm_read_byte(p) (*(const uint8_t*)(p))
#define bitRead(v,b) (((v)>>(b))&1)
#define bitWrite(v,b,x) ((x)?((v)|=(1UL<<(b))):((v)&=~(1UL<<(b))))
#define constrain(x,a,b) ((x)<(a)?(a):((x)>(b)?(b):(x)))
inline long map(long x,long a,long b,long c,long d){return (x-a)*(d-c)/(b-a)+c;}
class String {std::string s; public: String(const char* x=""):s(x?x:""){} String(int x):s(std::to_string(x)){} unsigned int length()const{return s.size();} const char* c_str()const{return s.c_str();} void toCharArray(char* b,unsigned int n)const{if(n){std::strncpy(b,s.c_str(),n);b[n-1]=0;}}};
class Print { public: virtual ~Print()=default; template<class T> void print(const T&){} template<class T> void println(const T&){} void println(){} };
struct HostSerial: Print{template<class T>void print(const T&){} template<class T>void println(const T&){} void println(){} void begin(unsigned long){} operator bool()const{return true;}};
inline HostSerial Serial;
