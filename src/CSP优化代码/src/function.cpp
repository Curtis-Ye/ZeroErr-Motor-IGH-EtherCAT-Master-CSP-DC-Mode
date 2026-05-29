#include "function.h"

void ODwrite(ec_master_t *master, uint16_t slavePos, uint16_t index, uint8_t subIndex, uint8_t objectValue)
{
        /* Blocks until a reponse is received */
        uint8_t retVal = ecrt_master_sdo_download(master, slavePos, index, subIndex, &objectValue, sizeof(objectValue), NULL);
        /* retVal != 0: Failure */
        if (retVal)
                printf("OD write unsuccessful\n");
}

void initDrive(ec_master_t *master, uint16_t slavePos, uint8_t mode)
{
        ODwrite(master, slavePos, 0x6060, 0x00, mode);
        /* Reset alarm */
        ODwrite(master, slavePos, 0x6040, 0x00, 0x80);
}

inline void timespec_add(struct timespec *result, struct timespec *time1, struct timespec *time2)
{

        if ((time1->tv_nsec + time2->tv_nsec) >= NSEC_PER_SEC)
        {
                result->tv_sec = time1->tv_sec + time2->tv_sec + 1;
                result->tv_nsec = time1->tv_nsec + time2->tv_nsec - NSEC_PER_SEC;
        }
        else
        {
                result->tv_sec = time1->tv_sec + time2->tv_sec;
                result->tv_nsec = time1->tv_nsec + time2->tv_nsec;
        }
}

// inline void timespec_sub(struct timespec *result, struct timespec *time1, struct timespec *time2)
// {

//         if ((time1->tv_nsec - time2->tv_nsec) < 0)
//         {
//                 result->tv_sec = time1->tv_sec - time2->tv_sec - 1;
//                 result->tv_nsec = NSEC_PER_SEC - (time1->tv_nsec - time2->tv_nsec);
//         }
//         else
//         {
//                 result->tv_sec = time1->tv_sec - time2->tv_sec;
//                 result->tv_nsec = time1->tv_nsec - time2->tv_nsec;
//         }
// }

void signal_handler(int sig)
{
        printf("\nReleasing master...\n");
        ecrt_release_master(master);
        pid_t pid = getpid();
        kill(pid, SIGKILL);
}

void stack_prefault(void)
{
        unsigned char dummy[MAX_SAFE_STACK];
        memset(dummy, 0, MAX_SAFE_STACK);
}