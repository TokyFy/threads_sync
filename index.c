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
    pthread_mutex_t *l_print;
} t_philo;


typedef struct simulation {
    int philo_numbers;
    t_philo **philos;
    t_fork  **forks;
    pthread_mutex_t *l_print;
} t_simulation;

int init_philos(t_simulation *s)
{
    int i = 0;
    t_philo** philos = malloc(sizeof(t_philo*) * s->philo_numbers);
    s->l_print = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(s->l_print , NULL);

    while(i < s->philo_numbers)
    {
        philos[i] = malloc(sizeof(t_philo));
        philos[i]->l_print = s->l_print;
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
        s->philos[(i + philo_nbr - 1) % philo_nbr]->right_fork = forks[i];
        pthread_mutex_init(&(forks[i])->mutex , NULL);
        i++;
    }
    s->forks = forks;
    return 1;
}


void p_print(pthread_mutex_t *l_print , char *str  , int id)
{
        pthread_mutex_lock(l_print);
        printf(str , timestamp_in_ms(), id);
        pthread_mutex_unlock(l_print);
}

void eating(void *arg)
{
    t_philo *p = arg;
    pthread_mutex_t *l_print = p->l_print;

    if (p->left_fork->id < p->right_fork->id)
    {
        pthread_mutex_lock(&p->left_fork->mutex);
        p_print(l_print , "%ld %d has taken a fork\n",  p->id); 
        pthread_mutex_lock(&p->right_fork->mutex);
        p_print(l_print , "%ld %d has taken a fork\n",  p->id);
    }
    else
    {
        pthread_mutex_lock(&p->right_fork->mutex);
        p_print(l_print , "%ld %d has taken a fork\n", p->id) ;
        pthread_mutex_lock(&p->left_fork->mutex);
        p_print(l_print , "%ld %d has taken a fork\n", p->id) ;
    }

    pthread_mutex_lock(l_print);
    printf("%ld %d is eating\n", timestamp_in_ms() , p->id);
    pthread_mutex_unlock(l_print);

    pthread_mutex_unlock(&p->right_fork->mutex);
    pthread_mutex_unlock(&p->left_fork->mutex);

    usleep(100 * 1000);
}

void *worker(void *arg)
{
    t_philo *p = arg;
    pthread_mutex_t *l_print = p->l_print;
    
    while(1)
    {
        eating(arg);
        p_print(l_print , "%ld %d is sleeping\n", p->id);
        usleep(100 * 1000);
        p_print(l_print , "%ld %d is thinking\n", p->id);
    }
    return NULL;
}

int main()
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
