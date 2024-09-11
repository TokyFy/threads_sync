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

#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

typedef struct fork
{
	int				id;
	pthread_mutex_t	m_taken;
	pthread_mutex_t	mutex;
}					t_fork;

typedef struct philo
{
	int				id;
	pthread_t		thread;
	t_fork			*left_fork;
	t_fork			*right_fork;
	void			*dinning;
	uint64_t		eat_time;
	int				meals_numbers;
	pthread_mutex_t	eat_time_lock;
	pthread_mutex_t	meals_numbers_lock;
}					t_philo;

typedef struct simulation
{
	int				philo_numbers;
	int             philo_fullup_numbers;
	t_philo			**philos;
	t_fork			**forks;
	uint64_t		t_t_eat;
	uint64_t		t_t_sleep;
	uint64_t		t_t_die;
	int				meals_limit;
	uint64_t		start_time;
	int				stoped;
	pthread_mutex_t	start_time_lock;
	pthread_mutex_t	stoped_lock;
	pthread_mutex_t philo_fullup_lock;
}					t_simulation;

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

uint64_t	timestamp_in_ms(void)
{
	static pthread_mutex_t	m = PTHREAD_MUTEX_INITIALIZER;
	static uint64_t			created_at = 0;
	uint64_t				time;

	pthread_mutex_lock(&m);
	time = 0;
	if (created_at == 0)
		created_at = gettimeofday_ms();
	time = (gettimeofday_ms() - created_at);
	pthread_mutex_unlock(&m);
	return (time);
}

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
		s->philos[i]->right_fork = forks[i];
		s->philos[(i + philo_nbr - 1) % philo_nbr]->left_fork = forks[i];
		pthread_mutex_init(&(forks[i])->mutex, NULL);
		pthread_mutex_init(&(forks[i])->m_taken, NULL);
		i++;
	}
	s->forks = forks;
	return (1);
}

typedef enum e_mode
{
	THINKING,
	EATING,
	SLEEPING,
	PICKING_FORK,
	DYING
}					t_mode;

void	logging(t_philo *philo, t_mode mode)
{
	static pthread_mutex_t	m = PTHREAD_MUTEX_INITIALIZER;
	t_simulation			*dinning;
	uint64_t				time;

	pthread_mutex_lock(&m);
	dinning = philo->dinning;
	time = gettimeofday_ms();
	if (mode == THINKING)
	{
		printf("%ld %d is thinking\n", time - dinning->start_time, philo->id);
	}
	else if (mode == EATING)
	{
		pthread_mutex_lock(&philo->eat_time_lock);
		philo->eat_time = time;
		pthread_mutex_unlock(&philo->eat_time_lock);
		printf("%ld %d is eating\n", time - dinning->start_time, philo->id);
	}
	else if (mode == PICKING_FORK)
	{
		printf("%ld %d has taken a fork\n", time - dinning->start_time,
			philo->id);
	}
	else if (mode == SLEEPING)
	{
		printf("%ld %d is sleeping\n", time - dinning->start_time, philo->id);
	}
	else if (mode == DYING)
	{
		printf("%ld %d died\n", time - dinning->start_time, philo->id);
	}
	pthread_mutex_unlock(&m);
}

void	eating(void *arg)
{
	t_philo			*p;
	t_simulation	*dinning;
	int				taken;

	p = arg;
	dinning = p->dinning;
	taken = 0;
	if (p->left_fork->id < p->right_fork->id)
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
		pthread_mutex_lock(&p->left_fork->mutex);
		logging(p, PICKING_FORK);
	}
	logging(p, EATING);
	pthread_mutex_lock(&p->meals_numbers_lock);
	p->meals_numbers++;
	if(p->meals_numbers >= dinning->meals_limit)
	{
	   pthread_mutex_lock(&dinning->philo_fullup_lock);
	   dinning->philo_fullup_numbers++;
	   pthread_mutex_unlock(&dinning->philo_fullup_lock);
	}
	pthread_mutex_unlock(&p->meals_numbers_lock);
	usleep(dinning->t_t_eat * 1000);
	pthread_mutex_unlock(&p->right_fork->mutex);
	pthread_mutex_unlock(&p->left_fork->mutex);
}

void	*worker(void *arg)
{
	t_philo			*p;
	t_simulation	*dinning;

	p = arg;
	dinning = p->dinning;
	pthread_mutex_lock(&dinning->start_time_lock);
	if (dinning->start_time == 0)
		dinning->start_time = gettimeofday_ms();
	if (p->eat_time == 0)
		p->eat_time = dinning->start_time;
	pthread_mutex_unlock(&dinning->start_time_lock);
	if (p->id % 2 != 0)
		usleep(20 * 1000);
	while (1)
	{
		pthread_mutex_lock(&dinning->stoped_lock);
		if (dinning->stoped)
		{
		    pthread_mutex_unlock(&dinning->stoped_lock);
			return (NULL);
		}
		logging(p, THINKING);
		pthread_mutex_unlock(&dinning->stoped_lock);
		eating(p);
		logging(p, SLEEPING);
		usleep(dinning->t_t_sleep * 1000);
	}
	return (NULL);
}

int	main(int argc, char **argv)
{
	t_simulation	dinning;
	int				i;
	int				hungry_time;

	if (!(argc == 5 || argc == 6))
	{
		printf("Usage : ./philo nbr_of_philo t_t_die t_t_eat t_t_sleep [nbr_of_t_each_philo_must_eat]\n");
		return (EXIT_FAILURE);
	}
	dinning.philo_numbers = ft_atoi(argv[1]);
	dinning.t_t_die = ft_atoi(argv[2]);
	dinning.t_t_eat = ft_atoi(argv[3]);
	dinning.t_t_sleep = ft_atoi(argv[4]);
	dinning.stoped = 0;
	dinning.start_time = 0;
	pthread_mutex_init(&dinning.stoped_lock, NULL);
	pthread_mutex_init(&dinning.start_time_lock, NULL);
	pthread_mutex_init(&dinning.philo_fullup_lock, NULL);
	init_philos(&dinning);
	init_forks(&dinning);
	dinning.meals_limit = INT_MAX;
	dinning.philo_fullup_numbers = 0;
	if(argc == 6)
	   dinning.meals_limit = ft_atoi(argv[5]);
	i = 0;
	while (i < dinning.philo_numbers)
	{
		pthread_create(&dinning.philos[i]->thread, NULL, worker,
			dinning.philos[i]);
		i++;
	}
	while (1)
	{
		usleep(200 * 1000);
		i = 0;
		while (i < dinning.philo_numbers)
		{
			pthread_mutex_lock(&dinning.philos[i]->eat_time_lock);
			hungry_time = (gettimeofday_ms() - dinning.philos[i]->eat_time);
			pthread_mutex_unlock(&dinning.philos[i]->eat_time_lock);

			pthread_mutex_lock(&dinning.philo_fullup_lock);
			int nbr_full_philo = dinning.philo_fullup_numbers;
			pthread_mutex_unlock(&dinning.philo_fullup_lock);

			if (hungry_time > dinning.t_t_die || nbr_full_philo > dinning.philo_numbers)
			{
				pthread_mutex_lock(&dinning.stoped_lock);
				dinning.stoped = 1;
				pthread_mutex_unlock(&dinning.stoped_lock);
				if(hungry_time > dinning.t_t_die)
				    logging(dinning.philos[i], DYING);
				break ;
			}
			i++;
		}

		if (dinning.stoped)
			break ;
	}

	return (0);
}
