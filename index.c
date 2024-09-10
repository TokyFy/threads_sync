/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   index.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: franaivo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 08:29:29 by franaivo          #+#    #+#             */
/*   Updated: 2024/09/10 13:48:03 by franaivo         ###   ########.fr       */
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
    int taken;
    pthread_mutex_t m_taken;
    pthread_mutex_t mutex;
} t_fork;

typedef struct philo {
    int id;
    pthread_t thread;
    t_fork *left_fork;
    t_fork *right_fork;
    uint64_t eat_time;
    void *dinning;
} t_philo;

typedef struct simulation {
    int philo_numbers;
    t_philo **philos;
    t_fork **forks;
    uint64_t t_t_eat;
    uint64_t t_t_sleep;
    uint64_t t_t_die;
    int meals_limit;
    uint64_t start_time;
    int stoped;
    pthread_mutex_t start_time_lock;
    pthread_mutex_t stoped_lock;
} t_simulation;

int ft_atoi(char *s) {
    int acum = 0;
    int factor = 1;

    if (*s == '-' || *s == '+') {
        if (*s == '-')
            factor = -1;
        s++;
    }
    while ((*s >= '0') && (*s <= '9')) {
        acum = acum * 10;
        acum = acum + (*s - 48);
        s++;
    }
    return (factor * acum);
}

uint64_t gettimeofday_ms(void) {
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&m);
    uint64_t time = 0;
    struct timeval t;
    gettimeofday(&t, NULL);
    time = ((t.tv_sec * (uint64_t) 1000) + (t.tv_usec / 1000));
    pthread_mutex_unlock(&m);
    return time;
}

uint64_t timestamp_in_ms(void) {
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    static uint64_t created_at = 0;
    pthread_mutex_lock(&m);
    uint64_t time = 0;
    if (created_at == 0)
        created_at = gettimeofday_ms();
    time = (gettimeofday_ms() - created_at);
    pthread_mutex_unlock(&m);
    return time;
}

int init_philos(t_simulation *s) {
    int i = 0;
    t_philo **philos = malloc(sizeof(t_philo *) * s->philo_numbers);

    while (i < s->philo_numbers) {
        philos[i] = malloc(sizeof(t_philo));
        (philos[i])->id = i;
        (philos[i])->eat_time = 0;
        (philos[i])->dinning = s;
        i++;
    }
    s->philos = philos;
    return 1;
}

int init_forks(t_simulation *s) {
    int i = 0;

    t_fork **forks = malloc(sizeof(t_fork *) * s->philo_numbers);
    int philo_nbr = s->philo_numbers;
    while (i < s->philo_numbers) {
        forks[i] = malloc(sizeof(t_fork));
        forks[i]->id = i + 1;
        forks[i]->taken = 0;
        s->philos[i]->right_fork = forks[i];
        s->philos[(i + philo_nbr - 1) % philo_nbr]->left_fork = forks[i];
        pthread_mutex_init(&(forks[i])->mutex, NULL);
        pthread_mutex_init(&(forks[i])->m_taken, NULL);
        i++;
    }
    s->forks = forks;
    return 1;
}

typedef enum e_mode {
    THINKING,
    EATING,
    SLEEPING,
    PICKING_FORK,
    DYING
} t_mode;

void logging(t_philo *philo, t_mode mode) {
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&m);
    t_simulation *dinning = philo->dinning;
    uint64_t time = gettimeofday_ms();
    if (mode == THINKING) {
        printf("%ld %d is thinking\n", time - dinning->start_time , philo->id);
    } else if (mode == EATING) {
        philo->eat_time = time;
        printf("%ld %d is eating\n", time - dinning->start_time , philo->id);
    } else if (mode == PICKING_FORK) {
        printf("%ld %d has taken a fork\n", time - dinning->start_time , philo->id);
    } else if (mode == SLEEPING) {
        printf("%ld %d is sleeping\n", time - dinning->start_time , philo->id);
    } else if (mode == DYING)
        {
        printf("%ld %d died\n", time - dinning->start_time , philo->id);
    }
    pthread_mutex_unlock(&m);
}


void eating(void *arg)
{
    t_philo *p = arg;
    t_simulation *dinning = p->dinning;
    int taken = 0;

    if (p->left_fork->id < p->right_fork->id) {
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
    usleep(dinning->t_t_eat * 1000);
    pthread_mutex_unlock(&p->right_fork->mutex);
    pthread_mutex_unlock(&p->left_fork->mutex);
}

void *worker(void *arg)
{
    t_philo *p = arg;
    t_simulation *dinning = p->dinning;

    pthread_mutex_lock(&dinning->start_time_lock);
    if(dinning->start_time == 0)
        dinning->start_time = gettimeofday_ms();

    if(p->eat_time == 0)
        p->eat_time = dinning->start_time;

    pthread_mutex_unlock(&dinning->start_time_lock);

    while (1) {
        pthread_mutex_lock(&dinning->stoped_lock);
        if (dinning->stoped)
            return NULL;
        logging(p, THINKING);
        pthread_mutex_unlock(&dinning->stoped_lock);
        eating(p);
        logging(p, SLEEPING);
        usleep(dinning->t_t_sleep * 1000);
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (!(argc == 5 || argc == 6)) {
        printf("Usage : ./philo nbr_of_philo t_t_die t_t_eat t_t_sleep [nbr_of_t_each_philo_must_eat]\n");
        return EXIT_FAILURE;
    }

    t_simulation dinning;
    dinning.philo_numbers = ft_atoi(argv[1]);
    dinning.t_t_die = ft_atoi(argv[2]);
    dinning.t_t_eat = ft_atoi(argv[3]);
    dinning.t_t_sleep = ft_atoi(argv[4]);
    dinning.meals_limit = 0;
    dinning.stoped = 0;
    dinning.start_time = 0;
    pthread_mutex_init(&dinning.stoped_lock, NULL);
    pthread_mutex_init(&dinning.start_time_lock, NULL);

    init_philos(&dinning);
    init_forks(&dinning);

    int i = 0;
    while (i < dinning.philo_numbers) {
        pthread_create(&dinning.philos[i]->thread, NULL, worker, dinning.philos[i]);
        i++;
    }
    while (1) {
        usleep(200 * 1000);
        i = 0;
        while (i < dinning.philo_numbers) {
            if ((gettimeofday_ms() - dinning.philos[i]->eat_time) > dinning.t_t_die) {
                pthread_mutex_lock(&dinning.stoped_lock);
                dinning.stoped = 1;
                pthread_mutex_unlock(&dinning.stoped_lock);
                logging(dinning.philos[i], DYING);
                break;
            }
            i++;
        }
        if (dinning.stoped)
            break;
    }
    return 0;
}
