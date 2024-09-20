/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: franaivo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 11:52:44 by franaivo          #+#    #+#             */
/*   Updated: 2024/09/20 11:52:45 by franaivo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	ft_atoi(char *s)
{
	int	acum;
	int	factor;

	acum = 0;
	factor = 1;
	if (*s == '-' || *s == '+')
	{
		if (*s == '-')
			factor = -1;
		s++;
	}
	while ((*s >= '0') && (*s <= '9'))
	{
		acum = acum * 10;
		acum = acum + (*s - 48);
		s++;
	}
	return (factor * acum);
}

uint64_t	safe_get_int(pthread_mutex_t *lock, void *n)
{
	uint64_t	value;

	pthread_mutex_lock(lock);
	value = *(uint64_t *)n;
	pthread_mutex_unlock(lock);
	return (value);
}

void	safe_set_int(pthread_mutex_t *lock, void *n, const uint64_t v)
{
	pthread_mutex_lock(lock);
	*(uint64_t *)n = v;
	pthread_mutex_unlock(lock);
}

void	logging(t_philo *philo, t_mode mode, char *msg)
{
	t_simulation	*dinning;
	uint64_t		time;

	dinning = philo->dinning;
	pthread_mutex_lock(&dinning->print_lock);
	time = gettimeofday_ms();
	if (safe_get_int(&dinning->stoped_lock, &dinning->stoped) && mode != DYING)
	{
		pthread_mutex_unlock(&dinning->print_lock);
		return ;
	}
	printf(msg, time - philo->start_time, philo->id);
	pthread_mutex_unlock(&dinning->print_lock);
}
