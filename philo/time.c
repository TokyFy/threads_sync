/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: franaivo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 11:53:14 by franaivo          #+#    #+#             */
/*   Updated: 2024/09/20 11:53:15 by franaivo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

uint64_t	gettimeofday_ms(void)
{
	static pthread_mutex_t	m = PTHREAD_MUTEX_INITIALIZER;
	uint64_t				time;
	struct timeval			t;

	pthread_mutex_lock(&m);
	time = 0;
	gettimeofday(&t, NULL);
	time = ((t.tv_sec * (uint64_t)1000) + (t.tv_usec / 1000));
	pthread_mutex_unlock(&m);
	return (time);
}

int	ft_usleep(t_simulation *t, uint64_t ms)
{
	uint64_t	start;

	start = gettimeofday_ms();
	while ((gettimeofday_ms() - start) < ms)
	{
		if (safe_get_int(&t->stoped_lock, &t->stoped))
			return (0);
		usleep(500);
	}
	return (0);
}
