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
#include <unistd.h>

void	logging(t_philo *philo, t_mode mode)
{
	static pthread_mutex_t	m = PTHREAD_MUTEX_INITIALIZER;
	t_simulation			*dinning;
	uint64_t				time;

	pthread_mutex_lock(&m);
	dinning = philo->dinning;
	time = gettimeofday_ms();
	if (safe_get_int(&dinning->stoped_lock, &dinning->stoped) && mode != DYING)
	{
		pthread_mutex_unlock(&m);
		return ;
	}
	if (mode == THINKING)
		printf("%ld %d is thinking\n", time - philo->start_time, philo->id);
	else if (mode == EATING)
	{
		safe_set_int(&philo->eat_time_lock, &philo->eat_time, time);
		printf("%ld %d is eating\n", time - philo->start_time, philo->id);
	}
	else if (mode == PICKING_FORK)
		printf("%ld %d has taken a fork\n", time - philo->start_time,
			philo->id);
	else if (mode == SLEEPING)
		printf("%ld %d is sleeping\n", time - philo->start_time, philo->id);
	else if (mode == DYING)
		printf("%ld %d died\n", time - philo->start_time, philo->id);
	pthread_mutex_unlock(&m);
}

void	eating(void *arg)
{
	t_philo			*p;
	t_simulation	*dinning;

	p = arg;
	dinning = p->dinning;
	if (p->id % 2 == 0)
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
		if (dinning->philo_numbers == 1)
		{
			ft_usleep(dinning->t_t_die * 2);
			pthread_mutex_unlock(&p->right_fork->mutex);
			return ;
		}
		pthread_mutex_lock(&p->left_fork->mutex);
		logging(p, PICKING_FORK);
	}
	logging(p, EATING);
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
		logging(p, SLEEPING);
		ft_usleep(dinning->t_t_sleep);
		logging(p, THINKING);
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
			if (hungry_time > dinning->t_t_die
				|| nbr_full_philo > dinning->philo_numbers)
			{
				safe_set_int(&dinning->stoped_lock, &dinning->stoped, 1);
				if (hungry_time > dinning->t_t_die)
					logging(dinning->philos[i], DYING);
				break ;
			}
			i++;
		}
		if (dinning->stoped)
			break ;
	}
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
		dinning.philos[i]->start_time = gettimeofday_ms();
		dinning.philos[i]->eat_time = gettimeofday_ms();
		pthread_create(&dinning.philos[i]->thread, NULL, worker,
			dinning.philos[i]);
		i++;
	}
	monitor(&dinning);
	i = 0;
	while (i < dinning.philo_numbers)
	{
		pthread_join(dinning.philos[i]->thread, NULL);
		i++;
	}
	return (0);
}
