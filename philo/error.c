/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: franaivo <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 11:52:07 by franaivo          #+#    #+#             */
/*   Updated: 2024/09/20 11:52:09 by franaivo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	free_simulation(t_simulation *dinning)
{
	t_fork	**forks;
	t_philo	**philos;
	int		i;

	pthread_mutex_destroy(&dinning->start_time_lock);
	pthread_mutex_destroy(&dinning->stoped_lock);
	pthread_mutex_destroy(&dinning->philo_fullup_lock);
	pthread_mutex_destroy(&dinning->print_lock);
	forks = dinning->forks;
	philos = dinning->philos;
	i = 0;
	while (i < dinning->philo_numbers)
	{
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
	return ;
}

int	is_number(char *str)
{
	if (*str == '+' || *str == '-')
		str++;
	while (*str)
	{
		if (!(*str >= '0' && *str <= '9'))
			return (0);
		str++;
	}
	return (1);
}

int	args_check(int argc, char **argv)
{
	if (!(argc == 5 || argc == 6))
	{
		printf("Usage : ./philo n_philo die eat sleep [n_eat]\n");
		return (0);
	}
	while (argc > 1)
	{
		if (!is_number(argv[argc - 1]) || ft_atoi(argv[argc - 1]) <= 0)
		{
			printf("Arg [%d] { %s } : Non valid value\n", argc - 1, argv[argc
				- 1]);
			return (0);
		}
		argc--;
	}
	return (1);
}
