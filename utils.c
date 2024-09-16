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

uint64_t safe_get_int(pthread_mutex_t *lock , void *n)
{
    uint64_t value;
    pthread_mutex_lock(lock);
    value = *(uint64_t *)n;
    pthread_mutex_unlock(lock);
    return value;
}

void safe_set_int(pthread_mutex_t *lock , void *n , const uint64_t v)
{
    pthread_mutex_lock(lock);
    *(uint64_t *)n = v;
    pthread_mutex_unlock(lock);
}