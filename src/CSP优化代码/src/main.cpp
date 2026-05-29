#include "function.h"
#include "ECconfig.h"
#include <string.h>
/* For setting the process's priority (setpriority) */
#include <sys/resource.h>
/* For pid_t and getpid() */
#include <unistd.h>
#include <sys/types.h>
/* For using real-time scheduling policy (FIFO) and sched_setaffinity */
#include <sched.h>
/* For using uint32_t format specifier, PRIu32 */
#include <inttypes.h>
#define CONFIG_PDOS
#define DC

#ifdef DC
#define SYNC_REF_TO_MASTER

#endif

#ifdef DC

#define CONFIG_DC

#endif

/*****************************************************************************/

/* One motor revolution increments the encoder by 2^19 -1. */
#define ENCODER_RES 524287
/* The maximum stack size which is guranteed safe to access without faulting. */
#define MAX_SAFE_STACK (8 * 1024)

/* Uncomment to enable performance measurement. */
/* Measure the difference in reference slave's clock timstamp each cycle, and print the result,
   which should be as close to cycleTime as possible. */
/* Note: Only works with DC enabled. */
#define MEASURE_PERF

/* Calculate the time it took to complete the loop. */
#define MEASURE_TIMING

#define SET_CPU_AFFINITY

#define NSEC_PER_SEC (1000000000L)
#define FREQUENCY 1000
/* Period of motion loop, in nanoseconds */
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)

#ifdef DC

#define TIMESPEC2NS(T) ((uint64_t)(T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)

#endif

#ifdef CONFIG_DC

/* SYNC0 event happens halfway through the cycle */
#define SHIFT0 (PERIOD_NS / 2)

#endif

/*****************************************************************************/
/* Note: Anything relying on definition of SYNC_MASTER_TO_REF is essentially copy-pasted from /rtdm_rtai_dc/main.c */

ec_master_t *master;

#ifdef MEASURE_TIMING
inline void timespec_sub(struct timespec *result, struct timespec *time1, struct timespec *time2)
{

	if ((time1->tv_nsec - time2->tv_nsec) < 0)
	{
		result->tv_sec = time1->tv_sec - time2->tv_sec - 1;
		result->tv_nsec = NSEC_PER_SEC - (time1->tv_nsec - time2->tv_nsec);
	}
	else
	{
		result->tv_sec = time1->tv_sec - time2->tv_sec;
		result->tv_nsec = time1->tv_nsec - time2->tv_nsec;
	}
}
#endif

#define EC_NEWTIMEVAL2NANO(TV) \
	(((TV).tv_sec - 946684800ULL) * 1000000000ULL + (TV).tv_nsec)
uint32_t interval_ = (uint32_t)(1000000000.0 / 1000);

extern ec_sync_info_t slave_0_syncs[];
extern ec_pdo_entry_reg_t domain1_regs[];
int main(int argc, char **argv)
{

#ifdef SET_CPU_AFFINITY
	cpu_set_t set;
	/* Clear set, so that it contains no CPUs. */
	CPU_ZERO(&set);
	/* Add CPU (core) 1 to the CPU set. */
	CPU_SET(4, &set);
#endif

	/* 0 for the first argument means set the affinity of the current process. */
	/* Returns 0 on success. */
	if (sched_setaffinity(0, sizeof(set), &set))
	{
		printf("Setting CPU affinity failed!\n");
		return -1;
	}

	/* SCHED_FIFO tasks are allowed to run until they have completed their work or voluntarily yield. */
	/* Note that even the lowest priority realtime thread will be scheduled ahead of any thread with a non-realtime policy;
	   if only one realtime thread exists, the SCHED_FIFO priority value does not matter.
	*/
	struct sched_param param = {};
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	printf("Using priority %i.\n", param.sched_priority);
	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
	{
		perror("sched_setscheduler failed\n");
	}

	/* Lock the program into RAM to prevent page faults and swapping */
	/* MCL_CURRENT: Lock in all current pages.
	   MCL_FUTURE:  Lock in pages for heap and stack and shared memory.
	*/
	if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
	{
		printf("mlockall failed\n");
		return -1;
	}

	/* Allocate the entire stack, locked by mlockall(MCL_FUTURE). */
	stack_prefault();
	/* Register the signal handler function. */
	signal(SIGINT, signal_handler);

	/* Reserve the first master (0) (/etc/init.d/ethercat start) for this program */
	master = ecrt_request_master(0);
	if (!master)
		printf("Requesting master failed\n");

	initDrive(master, 0);

	/* Creates and returns a slave configuration object, ec_slave_config_t*, for the given alias and position. */
	/* Returns NULL (0) in case of error and pointer to the configuration struct otherwise */

	ec_slave_config_t *drive0 = ecrt_master_slave_config(master, alias, position0, vendor_id, product_code); // 从站配置

	ec_slave_config_state_t slaveState0;

	/* If the drive0 = NULL or drive1 = NULL */
	if (!drive0)
	{
		printf("Failed to get slave configuration\n");
		return -1;
	}

#ifdef CONFIG_PDOS
	if (ecrt_slave_config_pdos(drive0, EC_END, slave_0_syncs))
	{
		printf("Failed to configure slave 0 PDOs\n");
		return -1;
	}
#endif
	ec_domain_t *domain1 = ecrt_master_create_domain(master); // 创建域

	/* Registers PDOs for a domain. */
	/* Returns 0 on success. */
	printf("Activating master...\n");

	//(master, 1);
	if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs))
	{
		printf("PDO entry registration failed\n");
		return -1;
	}

#ifdef CONFIG_DC

	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	ecrt_master_application_time(master, EC_NEWTIMEVAL2NANO(t));
	/* Do not enable Sync1 */
	ecrt_slave_config_dc(drive0, 0x0300, PERIOD_NS, 0, 0, 0); // 配置从站的分布式时钟参数

#endif

#ifdef SYNC_REF_TO_MASTER
	/* Initialize master application time. */
	struct timespec masterInitTime;
	clock_gettime(CLOCK_MONOTONIC, &masterInitTime);
	ecrt_master_application_time(master, TIMESPEC2NS(masterInitTime));
#endif
	/* Up to this point, we have only requested the master. See log messages */
	printf("Activating master...\n");

	if (ecrt_master_activate(master))
		return -1;

	uint8_t *domain1_pd;
	/* Returns a pointer to (I think) the first byte of PDO data of the domain */
	if (!(domain1_pd = ecrt_domain_data(domain1)))
		return -1;

	struct timespec wakeupTime;

#ifdef DC
	struct timespec time;
#endif

	struct timespec cycleTime = {0, PERIOD_NS};
	clock_gettime(CLOCK_MONOTONIC, &wakeupTime);

	/* The slaves (drives) enter OP mode after exchanging a few frames. */
	/* We exchange frames with no RPDOs (targetPos) untill all slaves have
	   reached OP state, and then we break out of the loop.
	*/
	while (1)
	{

		timespec_add(&wakeupTime, &wakeupTime, &cycleTime);
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTime, NULL);

		ecrt_master_receive(master);

		ecrt_slave_config_state(drive0, &slaveState0);

		if (slaveState0.operational)
		{
			printf("All slaves have reached OP state\n");
			// initDrive(master, 0);
			break;
		}

		ecrt_domain_queue(domain1);

#ifdef SYNC_REF_TO_MASTER
		/* Syncing reference slave to master:
		   1- The master's (PC) clock is the reference.
		   2- Sync the reference slave's clock to the master's.
		   3- Sync the other slave clocks to the reference slave's.
		*/

		clock_gettime(CLOCK_MONOTONIC, &time);
		ecrt_master_application_time(master, TIMESPEC2NS(time));
		/* Queues the DC reference clock drift compensation datagram for sending.
		   The reference clock will by synchronized to the **application (PC)** time provided
		   by the last call off ecrt_master_application_time().
		*/
		ecrt_master_sync_reference_clock(master);
		/* Queues the DC clock drift compensation datagram for sending.
		   All slave clocks will be synchronized to the reference slave clock.
		*/
		ecrt_master_sync_slave_clocks(master);
#endif

		ecrt_master_send(master);
	}

	int32_t actPos0, targetPos0;
	int32_t actVel0, targetVel0;
#ifdef MEASURE_PERF
	/* The slave time received in the current and the previous cycle */
	uint32_t t_cur, t_prev;
#endif

	/* Sleep is how long we should sleep each loop to keep the cycle's frequency as close to cycleTime as possible. */
	struct timespec sleepTime;
#ifdef MEASURE_TIMING
	struct timespec execTime, endTime;
#endif

	/* Wake up 1 msec after the start of the previous loop. */
	sleepTime = cycleTime;
	/* Update wakeupTime = current time */
	clock_gettime(CLOCK_MONOTONIC, &wakeupTime);

	actPos0 = EC_READ_S32(domain1_pd + actual_position); // 读取实际位置
	targetPos0 = actPos0;				     // 设置目标位置 0x607A 等于当前实际位置值
	while (1)
	{
#ifdef MEASURE_TIMING
		clock_gettime(CLOCK_MONOTONIC, &endTime);
		/* wakeupTime is also start time of the loop. */
		/* execTime = endTime - wakeupTime */
		timespec_sub(&execTime, &endTime, &wakeupTime);
		printf("Execution time: %lu ns\n", execTime.tv_nsec);
#endif

		/* wakeupTime = wakeupTime + sleepTime */
		timespec_add(&wakeupTime, &wakeupTime, &sleepTime);
		/* Sleep to adjust the update frequency */
		/* Note: TIMER_ABSTIME flag is key in ensuring the execution with the desired frequency.
		 *
		 *
		   We don't have to conider the loop's execution time (as long as it doesn't get too close to 1 ms),
		   as the sleep ends cycleTime (=1 msecs) *after the start of the previous loop*.
		*/
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTime, NULL);
		/* Fetches received frames from the newtork device and processes the datagrams. */
		ecrt_master_receive(master);
		/* Evaluates the working counters of the received datagrams and outputs statistics,
		   if necessary.
		   This function is NOT essential to the receive/process/send procedure and can be
		   commented out
		*/
		ecrt_domain_process(domain1);

#ifdef MEASURE_PERF
		ecrt_master_reference_clock_time(master, &t_cur);
#endif

		/********************************************************************************/

		/* Read PDOs from the datagram */
		actPos0 = EC_READ_S32(domain1_pd + actual_position);
		actVel0 = EC_READ_S32(domain1_pd + actual_velocity);

		// Read status word
		uint16_t statusWord = EC_READ_U16(domain1_pd + statusword);
		uint16_t state = getDriveState(statusWord);
		uint16_t cw = 0;

		// State machine for enabling the drive
		switch (state)
		{
		case STATE_FAULT:
			cw = CONTROL_WORD_FAULT_RESET;
			printf("Fault state, sending reset command\n");
			break;

		case STATE_SWITCH_ON_DISABLED:
			cw = CONTROL_WORD_SHUTDOWN;
			printf("Switch on disabled, sending shutdown command\n");
			break;

		case STATE_READY_TO_SWITCH_ON:
			cw = CONTROL_WORD_SWITCH_ON;
			printf("Ready to switch on, sending switch on command\n");
			break;

		case STATE_SWITCHED_ON:
			cw = CONTROL_WORD_ENABLE_OPERATION;
			EC_WRITE_S32(domain1_pd + target_position, targetPos0);
			printf("Switched on, sending enable operation command\n");
			break;

		case STATE_OPERATION_ENABLED:
			cw = CONTROL_WORD_ENABLE_OPERATION;
			printf("Operating...\n");
			break;

		default:
			cw = CONTROL_WORD_SHUTDOWN;
			printf("Unknown state (0x%04x), trying shutdown\n", state);
			break;
		}

		// Write control word after switch statement
		EC_WRITE_U16(domain1_pd + controlword, cw);
		// Periodic sending of target location
		if (state == STATE_OPERATION_ENABLED)
		{
			targetPos0 += 10;
			EC_WRITE_S32(domain1_pd + target_position, targetPos0);
		}
		printf("targetPos=%d actualPos=%d\n", targetPos0, actPos0);

		/* Queues all domain datagrams in the master's datagram queue.
		   Call this function to mark the domain's datagrams for exchanging at the
		   next call of ecrt_master_send()
		*/
		ecrt_domain_queue(domain1);

#ifdef SYNC_REF_TO_MASTER
		/* Distributed clocks */
		clock_gettime(CLOCK_MONOTONIC, &time);
		ecrt_master_application_time(master, TIMESPEC2NS(time));
		ecrt_master_sync_reference_clock(master);
		ecrt_master_sync_slave_clocks(master);
#endif

		/* Sends all datagrams in the queue.
		   This method takes all datagrams that have been queued for transmission,
		   puts them into frames, and passes them to the Ethernet device for sending.
		*/
		ecrt_master_send(master);

#ifdef MEASURE_PERF
		printf("\nTimestamp diff: %" PRIu32 " ns\n\n", t_cur - t_prev);
		t_prev = t_cur;
#endif
	}

	return 0;
}
