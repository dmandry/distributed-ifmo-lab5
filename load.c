/**
 * @file     load.h
 * @Author   Andrey Dmitriev (dmandry92@gmail.com)
 * @date     September, 2015
 * @brief    Buisness-process functionality
 */

#include "load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pa2345.h"
#include "process_msg.h"
#include "process_transmission.h"

extern int mutexl;
extern FILE *eventlog;
extern timestamp_t lamport_time;
extern int8_t num_processes;
extern int8_t reply_num;
extern int reply[11];

char buff[100];
int dr[11] = {0,0,0,0,0,0,0,0,0,0,0};
int8_t num_running_processes;
timestamp_t request_time;

int request_cs(const void * self)
{
	local_id id = ((request_queue_t *)(self))->id;
	local_id req_id = ((request_queue_t *)(self))->req_id;
	timestamp_t req_time = ((request_queue_t *)(self))->req_time;
    if (request_time > req_time ||
		(request_time == req_time && id > req_id))
	{
		process_send(id, req_id, CS_REPLY);
	}
	else
	{
		dr[req_id] = 1;
	}
    return 0;
}

int release_cs(const void * self)
{
	local_id i;
	local_id id = ((request_queue_t *)(self))->id;
	for (i = 0; i<num_processes+1; i++)
	{
		if (dr[i])
		{
			process_send(id, i, CS_REPLY);
			dr[i] = 0;
		}
		reply[i] = 0;
	}
	reply_num = 0;
	return 0;
}

void load(local_id id)
{
	int i;
	request_queue_t *rqe = NULL;
	for (i=1; i<=id*5;i++)
	{
		sprintf(buff, log_loop_operation_fmt, id, i, id*5);
		if (mutexl)
		{
			process_send_multicast(id, CS_REQUEST);
			request_time = get_lamport_time();
			process_recieve_all(num_processes, id, CS_REPLY);
		}
		print(buff);
		if (mutexl)
		{
			process_send_multicast(id, CS_RELEASE);
			rqe = malloc(sizeof(request_queue_t));
			rqe->id = id;
			rqe->req_id = id;
			release_cs(rqe);
			free(rqe);
		}
	}
}
