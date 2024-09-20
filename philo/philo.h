/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: franaivo <franaivo@student.42antanana      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 14:57:16 by franaivo          #+#    #+#             */
/*   Updated: 2024/09/20 14:57:17 by franaivo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H
# include <limits.h>
# include <pthread.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_fork
{
	int				id;
	pthread_mutex_t	mutex;
}					t_fork;

typedef struct s_philo
{
	int				id;
	pthread_t		thread;
	t_fork			*left_fork;
	t_fork			*right_fork;
	void			*dinning;
	uint64_t		eat_time;
	int				meals_numbers;
	uint64_t		start_time;
	pthread_mutex_t	eat_time_lock;
	pthread_mutex_t	meals_numbers_lock;
}					t_philo;

typedef struct s_simulation
{
	int				philo_numbers;
	int				philo_fullup_numbers;
	t_philo			**philos;
	t_fork			**forks;
	uint64_t		t_t_eat;
	uint64_t		t_t_sleep;
	uint64_t		t_t_die;
	int				meals_limit;
	int				stoped;
	pthread_mutex_t	start_time_lock;
	pthread_mutex_t	stoped_lock;
	pthread_mutex_t	philo_fullup_lock;
	pthread_mutex_t	print_lock;
}					t_simulation;

typedef enum e_mode
{
	THINKING,
	EATING,
	SLEEPING,
	PICKING_FORK,
	DYING
}					t_mode;

int					ft_atoi(char *s);
uint64_t			gettimeofday_ms(void);
uint64_t			timestamp_in_ms(void);
uint64_t			safe_get_int(pthread_mutex_t *lock, void *n);
void				safe_set_int(pthread_mutex_t *lock, void *n,
						const uint64_t v);
int					ft_usleep(t_simulation *t, uint64_t ms);
void				logging(t_philo *philo, t_mode mode, char *msg);

int					init_philos(t_simulation *s);
int					init_forks(t_simulation *s);
void				init_simulation(t_simulation *dinning, int argc,
						char **argv);

void				take_forks(t_philo *p);
void				eating(void *arg);
int					is_philo_fullup_or_die(t_philo *philo);
void				*worker(void *arg);

void				monitor(t_simulation *dinning);

void				free_simulation(t_simulation *dinning);

int					is_number(char *str);
int					args_check(int argc, char **argv);

#endif
