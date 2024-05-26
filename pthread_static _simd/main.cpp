#include <iostream>
#include <pthread.h>
#include<windows.h>
#include<stdio.h>
#include<cstdlib>
#include <semaphore.h>


using namespace std;

const int n=3000;
static float A[n][n];
static float B[n];
static float X[n];
static float factor[n];

const int NUM_THREADS = 6;

typedef struct{
    int t_id;
}threadParam_t;

sem_t sem_main;
sem_t sem_workerstart[NUM_THREADS];
sem_t sem_workerend[NUM_THREADS];

void *threadFunc(void *param){
    threadParam_t *p=(threadParam_t*)param;
    int t_id=p->t_id;

    for(int k=0;k<n;++k){

    sem_wait(&sem_workerstart[t_id]);

    for(int i=k+1+t_id;i<n;i+=NUM_THREADS)
    {
        for(int j=k+1;j<n;j+=8){
            __m256 Aij = _mm256_loadu_ps(&A[i][j]);
            __m256 Akj = _mm256_loadu_ps(&A[k][j]);
            Aij = _mm256_sub_ps(Aij, _mm256_mul_ps( _mm256_set1_ps(factor[i]), Akj));
            _mm256_storeu_ps(&A[i][j], Aij);
        }
        B[i]=B[i]-factor[i]*B[k];

    }

    sem_post(&sem_main);
    sem_wait(&sem_workerend[t_id]);

    }
    pthread_exit(NULL);

}



float run(){
    long long head, tail, freq;



    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            A[i][j]=0;
    }
    for(int i=0;i<n;i++)
    {
        B[i]=1.0;
        A[i][i]=1.0;
        for(int j=i+1;j<n;j++)
            {
                A[i][j]= (rand() % 100)-50;
                B[i]+=A[i][j];
            }
    }

    for(int k=0;k<n;k++)
        for(int i=k+1;i<n;i++)
            {
                for(int j=0;j<n;j++)
                    A[i][j]+=A[k][j];
                B[i]+=B[k];
            }




    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

    QueryPerformanceCounter((LARGE_INTEGER*)&head);

    sem_init(&sem_main,0,0);
    for(int i=0;i<NUM_THREADS;++i){
        sem_init(&sem_workerstart[i],0,0);
        sem_init(&sem_workerend[i],0,0);
    }

    pthread_t handles[NUM_THREADS];
    threadParam_t param[NUM_THREADS];
    for(int t_id=0;t_id<NUM_THREADS;t_id++){
        param[t_id].t_id=t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,(void *)&param[t_id]);
    }

    for(int k=0;k<n;++k){
        for(int j=k+1;j<n;j+=8){
            __m256 Ajk = _mm256_loadu_ps(&A[j][k]);
            __m256 factor_ = _mm256_div_ps(Ajk, _mm256_set1_ps(A[k][k]));
            _mm256_storeu_ps(&factor[j], factor_);
        }



        for(int t_id=0;t_id<NUM_THREADS;++t_id){
            sem_post(&sem_workerstart[t_id]);
        }
        for(int t_id=0;t_id<NUM_THREADS;++t_id){
            sem_wait(&sem_main);
        }
        for(int t_id=0;t_id<NUM_THREADS;++t_id){
            sem_post(&sem_workerend[t_id]);
        }
    }

    for(int t_id=0;t_id<NUM_THREADS;t_id++){
        pthread_join(handles[t_id],NULL);
    }

    sem_destroy(&sem_main);
    sem_destroy(sem_workerstart);
    sem_destroy(sem_workerend);


    X[n-1]=B[n-1]/A[n-1][n-1];
    for(int i=n-2;i>-1;i--)
    {
        float sum=B[i];
        for(int j=i+1;j<n;j++)
            sum=sum-A[i][j]*X[j];
        X[i]=sum/A[i][i];


    }


    QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    float answer=(float)((tail - head) * 1000.0 / freq);

   // for(int i=0;i<n;i++)
   //     cout<<X[i]<<endl;


    return answer ;
}

int main()
{
    float time_sum;
    int count1=0;

    while(count1<6)
    {
        count1+=1;
        time_sum+=run();
        cout<<count1<<endl;
    }

    cout<<time_sum/6.0<<endl;


    return 0;
}
