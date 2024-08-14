/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   index.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: franaivo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/14 11:14:15 by franaivo          #+#    #+#             */
/*   Updated: 2024/08/14 11:16:38 by franaivo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#define PHILO_NUMBER 50

uint64_t	gettimeofday_ms(void)
{
	static struct timeval	t;
	gettimeofday(&t, NULL);
	return ((t.tv_sec * (uint64_t)1000) + (t.tv_usec / 1000));
}

uint64_t	timestamp_in_ms(void)
{
	static uint64_t	created_at;

	if (created_at == 0)
		created_at = gettimeofday_ms();
	return (gettimeofday_ms() - created_at);
}

typedef struct state {
	pthread_t philo[PHILO_NUMBER];
	pthread_mutex_t forks[PHILO_NUMBER];
	uint64_t time_start;
} t_state;

pthread_mutex_t print_mutex;

void *worker(void *data)
{
	while(1)
	{
		pthread_mutex_lock(&print_mutex);
		printf("%ld : %d\n" , timestamp_in_ms()  ,  *(int *)data);
		pthread_mutex_unlock(&print_mutex);
		usleep(10000);
	}
	return NULL;
}

void *int_copy(int n)
{
	int *x = malloc(sizeof(int));
	*x = n;
	return x;
}

int main()
{
	int i = 0;
	t_state table;
	table.time_start = gettimeofday_ms();	

	pthread_mutex_init(&print_mutex , NULL);

	while(i < PHILO_NUMBER)
	{
		pthread_mutex_init(&table.forks[i] , NULL);
		i++;
	} 

	i = 0;
	while(i < PHILO_NUMBER)
	{
		pthread_create(&table.philo[i] , NULL , worker , int_copy(i));
		i++;
	}
	
	while(1)
	;;
}
