/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   index.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: franaivo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 08:29:29 by franaivo          #+#    #+#             */
/*   Updated: 2024/09/11 09:43:10 by franaivo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>

void	logging(t_philo *philo, t_mode mode , char *msg)
{
	t_simulation			*dinning;
	uint64_t				time;
	dinning = philo->dinning;
	pthread_mutex_lock(&dinning->print_lock);
	time = gettimeofday_ms();
	if (safe_get_int(&dinning->stoped_lock, &dinning->stoped) && mode != DYING)
	{
	    pthread_mutex_unlock(&dinning->print_lock);
		return ;
	}

	printf(msg , time - philo->start_time , philo->id);
	pthread_mutex_unlock(&dinning->print_lock);
}

void	take_forks(t_philo *p)
{
	t_simulation	*dinning;

	dinning = p->dinning;
	if (p->id % 2 == 0)
	{
		pthread_mutex_lock(&p->left_fork->mutex);
		logging(p, PICKING_FORK , "%ld %d has taken a fork\n");
		pthread_mutex_lock(&p->right_fork->mutex);
		logging(p, PICKING_FORK , "%ld %d has taken a fork\n");
	}
	else
	{
		pthread_mutex_lock(&p->right_fork->mutex);
		logging(p, PICKING_FORK , "%ld %d has taken a fork\n");
		if (dinning->philo_numbers == 1)
		{
			ft_usleep(dinning->t_t_die * 2);
			pthread_mutex_unlock(&p->right_fork->mutex);
			return ;
		}
		pthread_mutex_lock(&p->left_fork->mutex);
		logging(p, PICKING_FORK , "%ld %d has taken a fork\n");
	}
}

void	eating(void *arg)
{
	t_philo			*p;
	t_simulation	*dinning;

	p = arg;
	dinning = p->dinning;
	take_forks(p);
	logging(p, EATING , "%ld %d is eating\n");
	safe_set_int(&p->eat_time_lock, &p->eat_time, gettimeofday_ms());
	pthread_mutex_lock(&p->meals_numbers_lock);
	p->meals_numbers++;
	if (p->meals_numbers >= dinning->meals_limit)
	{
		pthread_mutex_lock(&dinning->philo_fullup_lock);
		dinning->philo_fullup_numbers++;
		pthread_mutex_unlock(&dinning->philo_fullup_lock);
	}
	pthread_mutex_unlock(&p->meals_numbers_lock);
	ft_usleep(dinning->t_t_eat);
	pthread_mutex_unlock(&p->right_fork->mutex);
	pthread_mutex_unlock(&p->left_fork->mutex);
}

void	*worker(void *arg)
{
	t_philo			*p;
	t_simulation	*dinning;

	p = arg;
	dinning = p->dinning;
	if (p->id % 2 == 0)
		usleep(500);
	while (1)
	{
		if (safe_get_int(&dinning->stoped_lock, &dinning->stoped))
			return (NULL);
		eating(p);
		logging(p, SLEEPING , "%ld %d is sleeping\n");
		ft_usleep(dinning->t_t_sleep);
		logging(p, THINKING , "%ld %d is thinking\n");
		if (dinning->philo_numbers % 2 != 0)
			ft_usleep(dinning->t_t_eat * 2 - dinning->t_t_sleep);
	}
	return (NULL);
}

void	init_simulation(t_simulation *dinning, int argc, char **argv)
{
	dinning->philo_numbers = ft_atoi(argv[1]);
	dinning->t_t_die = ft_atoi(argv[2]);
	dinning->t_t_eat = ft_atoi(argv[3]);
	dinning->t_t_sleep = ft_atoi(argv[4]);
	dinning->stoped = 0;
	pthread_mutex_init(&dinning->stoped_lock, NULL);
	pthread_mutex_init(&dinning->start_time_lock, NULL);
	pthread_mutex_init(&dinning->philo_fullup_lock, NULL);
	pthread_mutex_init(&dinning->print_lock, NULL);
	init_philos(dinning);
	init_forks(dinning);
	dinning->meals_limit = INT_MAX;
	dinning->philo_fullup_numbers = 0;
	if (argc == 6)
		dinning->meals_limit = ft_atoi(argv[5]);
}

void	monitor(t_simulation *dinning)
{
	int			i;
	uint64_t	hungry_time;
	int			nbr_full_philo;

	while (1)
	{
		i = 0;
		while (i < dinning->philo_numbers)
		{
			hungry_time = (gettimeofday_ms()
					- safe_get_int(&dinning->philos[i]->eat_time_lock,
						&dinning->philos[i]->eat_time));
			nbr_full_philo = safe_get_int(&dinning->philo_fullup_lock,
					&dinning->philo_fullup_numbers);
			if (hungry_time >= dinning->t_t_die
				|| nbr_full_philo > dinning->philo_numbers)
			{
				safe_set_int(&dinning->stoped_lock, &dinning->stoped, 1);
				if (hungry_time >= dinning->t_t_die)
					logging(dinning->philos[i], DYING , "%ld %d died\n");
				break ;
			}
			i++;
		}
		if (dinning->stoped)
			break ;
	}
}

void free_simulation(t_simulation *dinning)
{
    pthread_mutex_destroy(&dinning->start_time_lock);
    pthread_mutex_destroy(&dinning->stoped_lock);
    pthread_mutex_destroy(&dinning->philo_fullup_lock);
    pthread_mutex_destroy(&dinning->print_lock);
    t_fork **forks = dinning->forks;
    t_philo **philos = dinning->philos;
    int i = 0;
    while (i < dinning->philo_numbers) {
        pthread_mutex_destroy(&forks[i]->mutex);
        pthread_mutex_destroy(&forks[i]->m_taken);
        free(forks[i]);

        pthread_mutex_destroy(&philos[i]->eat_time_lock);
        pthread_mutex_destroy(&philos[i]->meals_numbers_lock);
        free(philos[i]);
        i++;
    }
    free(forks);
    free(philos);
    return;
}

int	main(int argc, char **argv)
{
	t_simulation	dinning;
	int				i;

	if (!(argc == 5 || argc == 6))
	{
		printf("Usage : ./philo nbr_of_philo t_t_die t_t_eat t_t_sleep [nbr_of_t_each_philo_must_eat]\n");
		return (EXIT_FAILURE);
	}
	init_simulation(&dinning, argc, argv);
	i = 0;
	while (i < dinning.philo_numbers)
	{
		dinning.philos[i]->eat_time = gettimeofday_ms();
		pthread_create(&dinning.philos[i]->thread, NULL, worker,
			dinning.philos[i]);
		i++;
	}
	monitor(&dinning);
	i = 0;
	while (i < dinning.philo_numbers)
		pthread_join(dinning.philos[i++]->thread, NULL);
	free_simulation(&dinning);
	return (0);
}
