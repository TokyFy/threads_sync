/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   worker.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: franaivo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 11:52:54 by franaivo          #+#    #+#             */
/*   Updated: 2024/09/20 11:52:55 by franaivo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	take_forks(t_philo *p)
{
	t_simulation	*dinning;

	dinning = p->dinning;
	if (p->id % 2 == 0)
	{
		pthread_mutex_lock(&p->left_fork->mutex);
		logging(p, PICKING_FORK, "%ld %d has taken a fork\n");
		pthread_mutex_lock(&p->right_fork->mutex);
		logging(p, PICKING_FORK, "%ld %d has taken a fork\n");
	}
	else
	{
		pthread_mutex_lock(&p->right_fork->mutex);
		logging(p, PICKING_FORK, "%ld %d has taken a fork\n");
		if (dinning->philo_numbers == 1)
		{
			ft_usleep(dinning->t_t_die * 2);
			pthread_mutex_unlock(&p->right_fork->mutex);
			return ;
		}
		pthread_mutex_lock(&p->left_fork->mutex);
		logging(p, PICKING_FORK, "%ld %d has taken a fork\n");
	}
}

void	eating(void *arg)
{
	t_philo			*p;
	t_simulation	*dinning;

	p = arg;
	dinning = p->dinning;
	take_forks(p);
	logging(p, EATING, "%ld %d is eating\n");
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

int	is_philo_fullup_or_die(t_philo *philo)
{
	uint64_t		hungry_time;
	int				nbr_full_philo;
	t_simulation	*dinning;

	dinning = philo->dinning;
	hungry_time = (gettimeofday_ms() - safe_get_int(&philo->eat_time_lock,
				&philo->eat_time));
	nbr_full_philo = safe_get_int(&dinning->philo_fullup_lock,
			&dinning->philo_fullup_numbers);
	if (hungry_time >= dinning->t_t_die)
		return (1);
	if (nbr_full_philo > dinning->philo_numbers)
		return (2);
	return (0);
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
		logging(p, SLEEPING, "%ld %d is sleeping\n");
		ft_usleep(dinning->t_t_sleep);
		logging(p, THINKING, "%ld %d is thinking\n");
		if (dinning->philo_numbers % 2 != 0)
			ft_usleep(dinning->t_t_eat * 2 - dinning->t_t_sleep);
	}
	return (NULL);
}
