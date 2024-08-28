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
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

typedef struct fork {
    int id;
    pthread_mutex_t mutex;
} t_fork;

typedef struct philo {
    int id;
    pthread_t thread;
    t_fork *left_fork;
    t_fork *right_fork;
    uint64_t eat_time;
    uint64_t think_time;
    uint64_t sleeping_time;
} t_philo;


typedef struct simulation {
    int philo_numbers;
    t_philo **philos;
    t_fork  **forks;
} t_simulation;

uint64_t	gettimeofday_ms(void)
{
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&m);
    uint64_t time = 0;
    struct timeval	t;
    gettimeofday(&t, NULL);
    time = ((t.tv_sec * (uint64_t)1000) + (t.tv_usec / 1000));
    pthread_mutex_unlock(&m);
    return time;
}

uint64_t	timestamp_in_ms(void)
{
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    static uint64_t	created_at = 0;
    pthread_mutex_lock(&m);
    uint64_t time = 0;
    if (created_at == 0)
        created_at = gettimeofday_ms();
    time = (gettimeofday_ms() - created_at);
    pthread_mutex_unlock(&m);
    return time;
}

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
        s->philos[i]->right_fork = forks[i];
        s->philos[(i + philo_nbr - 1) % philo_nbr]->left_fork = forks[i];
        pthread_mutex_init(&(forks[i])->mutex , NULL);
        i++;
    }
    s->forks = forks;
    return 1;
}

typedef enum e_mode {
    THINKING,
    EATING,
    SLEEPING,
    PICKING_FORK
} t_mode;

void logging(t_philo *philo , t_mode mode)
{
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&m);
    uint64_t time = timestamp_in_ms();
    if(mode == THINKING)
    {
        philo->think_time = time;
        printf("%ld %d is thinking\n" , philo->think_time , philo->id);
    }
    else if (mode == EATING) {
        philo->eat_time = time;
        printf("%ld %d is eating\n" , philo->eat_time , philo->id);
    }
    else if (mode == PICKING_FORK) {
        printf("%ld %d has taken a fork\n" , time , philo->id);
    }
    else if(mode == SLEEPING)
    {
        philo->sleeping_time = time;
        printf("%ld %d is sleeping\n" , philo->sleeping_time , philo->id);
    }
    pthread_mutex_unlock(&m);
}


void eating(void *arg)
{
    t_philo *p = arg;

    if (p->left_fork->id < p->right_fork->id)
    {
        pthread_mutex_lock(&p->left_fork->mutex);
        logging(p, PICKING_FORK);
        pthread_mutex_lock(&p->right_fork->mutex);
        logging(p, PICKING_FORK);
    }
    else
    {
        pthread_mutex_lock(&p->right_fork->mutex);
        logging(p, PICKING_FORK);
        pthread_mutex_lock(&p->left_fork->mutex);
        logging(p, PICKING_FORK);
    }

    logging(p, EATING);
    usleep(200 * 1000);
    pthread_mutex_unlock(&p->right_fork->mutex);
    pthread_mutex_unlock(&p->left_fork->mutex);
}

void *worker(void *arg)
{
    t_philo *p = arg;

    while(1)
    {
        logging(p, THINKING);
        eating(p);
        logging(p, SLEEPING);
        usleep(200 * 1000);
    }
    return NULL;
}

int main(int argc , char **argv)
{
    t_simulation dinning;
    dinning.philo_numbers = 5;
    init_philos(&dinning);
    init_forks(&dinning);

    int i = 0;
    while(i < dinning.philo_numbers)
    {
        pthread_create(&dinning.philos[i]->thread , NULL , worker , dinning.philos[i]);
        i++;
    }
    while(1)
       ;;
    return 0;
}
