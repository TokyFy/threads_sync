/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   index.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: franaivo <franaivo@student.42antanana      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 14:58:40 by franaivo          #+#    #+#             */
/*   Updated: 2024/09/20 14:58:41 by franaivo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	monitor(t_simulation *dinning)
{
	int	i;
	int	status;

	while (1)
	{
		i = 0;
		while (i < dinning->philo_numbers)
		{
			status = is_philo_fullup_or_die(dinning->philos[i]);
			if (status)
			{
				safe_set_int(&dinning->stoped_lock, &dinning->stoped, 1);
				if (status == 1)
					logging(dinning->philos[i], DYING, "%ld %d died\n");
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

	if (!args_check(argc, argv))
		return (EXIT_FAILURE);
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
