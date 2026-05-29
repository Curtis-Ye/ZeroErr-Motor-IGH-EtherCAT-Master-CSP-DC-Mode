#pragma once
#include <ecrt.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/mman.h>

#define PP 0x01
#define PV 0x03
#define PT 0x04
#define CSP 0x08
#define CSV 0x09
#define CST 0x0A

void ODwrite(ec_master_t *master, uint16_t slavePos, uint16_t index, uint8_t subIndex, uint8_t objectValue);
void initDrive(ec_master_t *master, uint16_t slavePos, uint8_t mode);
inline void timespec_add(struct timespec *result, struct timespec *time1, struct timespec *time2);
// inline void timespec_sub(struct timespec *result, struct timespec *time1, struct timespec *time2);
void signal_handler(int sig);
void stack_prefault(void);
