#include "philo.h"

int	init_philos(t_simulation *s)
{
    int		i;
    t_philo	**philos;

    i = 0;
    philos = malloc(sizeof(t_philo *) * s->philo_numbers);
    while (i < s->philo_numbers)
    {
        philos[i] = malloc(sizeof(t_philo));
        (philos[i])->id = i + 1;
        (philos[i])->meals_numbers = 0;
        (philos[i])->eat_time = 0;
        (philos[i])->dinning = s;
        (philos[i])->start_time = gettimeofday_ms();
        (philos[i])->eat_time = gettimeofday_ms();
        pthread_mutex_init(&(philos[i]->eat_time_lock), NULL);
        pthread_mutex_init(&(philos[i]->meals_numbers_lock), NULL);
        i++;
    }
    s->philos = philos;
    return (1);
}

int	init_forks(t_simulation *s)
{
    int		i;
    t_fork	**forks;
    int		philo_nbr;

    i = 0;
    forks = malloc(sizeof(t_fork *) * s->philo_numbers);
    philo_nbr = s->philo_numbers;
    while (i < s->philo_numbers)
    {
        forks[i] = malloc(sizeof(t_fork));
        forks[i]->id = i + 1;
        s->philos[i]->left_fork = forks[i];
        if(i == 0)
            s->philos[philo_nbr - 1]->right_fork = forks[i];
        else
            s->philos[i - 1]->right_fork = forks[i];
        pthread_mutex_init(&(forks[i])->mutex, NULL);
        pthread_mutex_init(&(forks[i])->m_taken, NULL);
        i++;
    }
    s->forks = forks;
    return (1);
}
