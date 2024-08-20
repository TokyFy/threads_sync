/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   index.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: franaivo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 08:29:29 by franaivo          #+#    #+#             */
/*   Updated: 2024/08/20 08:29:37 by franaivo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

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

typedef struct fork {
    int id;
    pthread_mutex_t mutex;
} t_fork;

typedef struct philo {
    int id;
    pthread_t thread;
    t_fork *left_fork;
    t_fork *right_fork;
} t_philo;


typedef struct simulation {
    int philo_numbers;
	t_philo **philos;
    t_fork  **forks;
} t_simulation;

int init_philos(t_simulation *s)
{
    int i = 0;
    t_philo** philos = malloc(sizeof(t_philo*) * s->philo_numbers);
    while(i < s->philo_numbers)
    {
        philos[i] = malloc(sizeof(t_philo));
        (philos[i])->id = i;
        i++;
    }
    s->philos = philos;
    return 1;
}

int init_forks(t_simulation *s)
{
   int i = 0;
    
   t_fork **forks = malloc(sizeof(t_fork*) * s->philo_numbers);
   int philo_nbr = s->philo_numbers;
   while(i < s->philo_numbers)
   {
        forks[i] = malloc(sizeof(t_fork));
        forks[i]->id = i;
        s->philos[i]->left_fork = forks[i];
        s->philos[((i - 1) % philo_nbr + philo_nbr) % philo_nbr]->right_fork = forks[i];
        pthread_mutex_init(&(forks[i])->mutex , NULL);
        i++;
   }
   s->forks = forks;
}

int main()
{
    t_simulation dinning;
    dinning.philo_numbers = 5;

    init_philos(&dinning);
    init_forks(&dinning);
    return 0;
}
