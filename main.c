#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>

# define PHILO_MAX 256

typedef struct s_philo
{

	int				id;
	int				eating;
	int				meals_eaten;
	int				num_of_philos;
	int				meals_numbers;
	int				*dead;
	pthread_t		thread;
	uint64_t			last_meal;
	uint64_t			t_t_die;
	uint64_t			t_t_eat;
	uint64_t			t_t_sleep;
	uint64_t			start_time;
	pthread_mutex_t	*right_fork;
	pthread_mutex_t	*left_fork;
	pthread_mutex_t	*l_print;
	pthread_mutex_t	*l_dead;
	pthread_mutex_t	*l_meal;
}					t_philo;
typedef struct s_simulation
{
	int				have_dead;
	t_philo			*philos;
	pthread_mutex_t	l_dead;
	pthread_mutex_t	l_meal;
	pthread_mutex_t	l_print;
}					t_simulation;

int	ft_strlen(const char *s)
{
	if (s[0] == '\0')
		return (0);
	return (1 + ft_strlen(++s));
}

int	ft_atoi(const char *s)
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

void	free_simulation(t_simulation *program, pthread_mutex_t *forks)
{
	int	i;

	i = 0;
	pthread_mutex_destroy(&program->l_print);
	pthread_mutex_destroy(&program->l_meal);
	pthread_mutex_destroy(&program->l_dead);
	while (i < program->philos[0].num_of_philos)
	{
		pthread_mutex_destroy(&forks[i]);
		i++;
	}
}

uint64_t	gettimeofday_ms(void)
{
	struct timeval	time;
	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000 + time.tv_usec / 1000);
}

int	ft_usleep(uint64_t ms)
{
	uint64_t	start;
	start = gettimeofday_ms();
	while ((gettimeofday_ms() - start) < ms)
		usleep(200);
	return (0);
}


void	init_input(t_philo *philo, char **argv)
{
	philo->t_t_die = ft_atoi(argv[2]);
	philo->t_t_eat = ft_atoi(argv[3]);
	philo->t_t_sleep = ft_atoi(argv[4]);
	philo->num_of_philos = ft_atoi(argv[1]);
	if (argv[5])
		philo->meals_numbers = ft_atoi(argv[5]);
	else
		philo->meals_numbers = -1;
}

// Initializing the philosophers

void	init_philos(t_philo *philos, t_simulation *program, pthread_mutex_t *forks,
		char **argv)
{
	int	i;

	i = 0;
	while (i < ft_atoi(argv[1]))
	{
		philos[i].id = i + 1;
		philos[i].eating = 0;
		philos[i].meals_eaten = 0;
		init_input(&philos[i], argv);
		philos[i].start_time = gettimeofday_ms();
		philos[i].last_meal = gettimeofday_ms();
		philos[i].l_print = &program->l_print;
		philos[i].l_dead = &program->l_dead;
		philos[i].l_meal = &program->l_meal;
		philos[i].dead = &program->have_dead;
		philos[i].left_fork = &forks[i];
		if (i == 0)
			philos[i].right_fork = &forks[philos[i].num_of_philos - 1];
		else
			philos[i].right_fork = &forks[i - 1];
		i++;
	}
}

void	init_forks(pthread_mutex_t *forks, int philo_num)
{
	int	i;

	i = 0;
	while (i < philo_num)
	{
		pthread_mutex_init(&forks[i], NULL);
		i++;
	}
}

void	init_simulation(t_simulation *program, t_philo *philos)
{
	program->have_dead = 0;
	program->philos = philos;
	pthread_mutex_init(&program->l_print, NULL);
	pthread_mutex_init(&program->l_dead, NULL);
	pthread_mutex_init(&program->l_meal, NULL);
}

int	is_dead(t_philo *philo)
{
	pthread_mutex_lock(philo->l_dead);
	if (*philo->dead == 1)
		return (pthread_mutex_unlock(philo->l_dead), 1);
	pthread_mutex_unlock(philo->l_dead);
	return (0);
}

void	logging(char *str, t_philo *philo)
{
	uint64_t	time;

	pthread_mutex_lock(philo->l_print);
	time = gettimeofday_ms() - philo->start_time;
	if (!is_dead(philo))
		printf("%zu %d %s\n", time, philo->id, str);
	pthread_mutex_unlock(philo->l_print);
}

int	should_die(t_philo *philo, uint64_t time_to_die)
{
	pthread_mutex_lock(philo->l_meal);
	if (gettimeofday_ms() - philo->last_meal >= time_to_die
		&& philo->eating == 0)
		return (pthread_mutex_unlock(philo->l_meal), 1);
	pthread_mutex_unlock(philo->l_meal);
	return (0);
}

int	have_someone_die(t_philo *philos)
{
	int	i;

	i = 0;
	while (i < philos[0].num_of_philos)
	{
		if (should_die(&philos[i], philos[i].t_t_die))
		{
			logging("died", &philos[i]);
			pthread_mutex_lock(philos[0].l_dead);
			*philos->dead = 1;
			pthread_mutex_unlock(philos[0].l_dead);
			return (1);
		}
		i++;
	}
	return (0);
}

int	is_all_fullup(t_philo *philos)
{
	int	i;
	int	finished_eating;

	i = 0;
	finished_eating = 0;
	if (philos[0].meals_numbers == -1)
		return (0);
	while (i < philos[0].num_of_philos)
	{
		pthread_mutex_lock(philos[i].l_meal);
		if (philos[i].meals_eaten >= philos[i].meals_numbers)
			finished_eating++;
		pthread_mutex_unlock(philos[i].l_meal);
		i++;
	}
	if (finished_eating == philos[0].num_of_philos)
	{
		pthread_mutex_lock(philos[0].l_dead);
		*philos->dead = 1;
		pthread_mutex_unlock(philos[0].l_dead);
		return (1);
	}
	return (0);
}

// Monitor thread routine

void	*monitor(void *pointer)
{
	t_philo	*philos;

	philos = (t_philo *)pointer;
	while (1)
	{
		if (have_someone_die(philos) == 1 || is_all_fullup(philos) == 1)
			break ;
	}
	return (pointer);
}


int	check_arg_content(char *arg)
{
	int	i;

	i = 0;
	while (arg[i] != '\0')
	{
		if (arg[i] < '0' || arg[i] > '9')
			return (1);
		i++;
	}
	return (0);
}

void	p_think(t_philo *philo)
{
	logging("is thinking", philo);
}

void	p_sleep(t_philo *philo)
{
	logging("is sleeping", philo);
	ft_usleep(philo->t_t_sleep);
}

void	p_eat(t_philo *philo)
{
	if(philo->id % 2 != 0)
	{
		pthread_mutex_lock(philo->right_fork);
		logging("has taken a fork", philo);
		if (philo->num_of_philos == 1)
		{
			ft_usleep(philo->t_t_die);
			pthread_mutex_unlock(philo->right_fork);
			return ;
		}
		pthread_mutex_lock(philo->left_fork);
		logging("has taken a fork", philo);
	}
	else
	{
		pthread_mutex_lock(philo->left_fork);
		logging("has taken a fork", philo);
		pthread_mutex_lock(philo->right_fork);
		logging("has taken a fork", philo);
	}
	philo->eating = 1;
	logging("is eating", philo);
	pthread_mutex_lock(philo->l_meal);
	philo->last_meal = gettimeofday_ms();
	philo->meals_eaten++;
	pthread_mutex_unlock(philo->l_meal);
	ft_usleep(philo->t_t_eat);
	philo->eating = 0;
	pthread_mutex_unlock(philo->left_fork);
	pthread_mutex_unlock(philo->right_fork);
}

void	*philo_routine(void *pointer)
{
	t_philo	*philo;

	philo = (t_philo *)pointer;
	if (philo->id % 2 != 0)
		ft_usleep(1);
	while (!is_dead(philo))
	{
		p_eat(philo);
		p_sleep(philo);
		p_think(philo);
	}
	return (pointer);
}

// Creates all the threads

int	thread_create(t_simulation *program, pthread_mutex_t *forks)
{
	pthread_t	observer;
	int			i;

	if (pthread_create(&observer, NULL, &monitor, program->philos) != 0)
		free_simulation(program, forks);
	i = 0;
	while (i < program->philos[0].num_of_philos)
	{
		if (pthread_create(&program->philos[i].thread, NULL, &philo_routine,
				&program->philos[i]) != 0)
			free_simulation(program, forks);
		i++;
	}
	i = 0;
	if (pthread_join(observer, NULL) != 0)
		free_simulation(program, forks);
	while (i < program->philos[0].num_of_philos)
	{
		if (pthread_join(program->philos[i].thread, NULL) != 0)
			free_simulation(program, forks);
		i++;
	}
	return (0);
}


int	main(int argc, char **argv)
{
	t_simulation		program;
	t_philo			philos[PHILO_MAX];
	pthread_mutex_t	forks[PHILO_MAX];

	if (argc != 5 && argc != 6)
	{
		printf("Error\n");
		return (EXIT_FAILURE);
	}

	init_simulation(&program, philos);
	init_forks(forks, ft_atoi(argv[1]));
	init_philos(philos, &program, forks, argv);
	thread_create(&program, forks);
	free_simulation(&program, forks);
	return (0);
}
