/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: franaivo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 11:52:14 by franaivo          #+#    #+#             */
/*   Updated: 2024/09/20 11:52:19 by franaivo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
		if (i == 0)
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
