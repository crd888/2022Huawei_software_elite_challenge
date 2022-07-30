#ifndef _HEAD_H
#define _HEAD_H
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/ioctl.h>
#include <math.h>
#include <sys/stat.h>
#include <time.h>
#include <string>
#include <string.h>
#include <fstream>
#include <unordered_map>
#include <iomanip>
#include "Node.h"
#include <algorithm>
#ifdef DBG
#define MyPint(fmt, args...) printf(fmt, ##args)
#else
#define MyPint(fmt, args...)
#endif

#endif