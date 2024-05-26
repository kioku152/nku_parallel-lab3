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
const int DIV_NUM = 4;

typedef struct{
    int t_id;
}threadParam_t;

pthread_barrier_t barrier_Division;
pthread_barrier_t barrier_Elimination;


void *threadFunc(void *param){
    threadParam_t *p=(threadParam_t*)param;
    int t_id=p->t_id;

    for(int k=0;k<n;++k){
        if(t_id<DIV_NUM)
        {
            for(int j=k+1+t_id;j<n;j+=DIV_NUM)
                factor[j]=A[j][k]/A[k][k];
        }

        pthread_barrier_wait(&barrier_Division);

        for(int i=k+1+t_id;i<n;i+=NUM_THREADS){
             for(int j=k+1;j<n;++j)
                A[i][j]=A[i][j]-factor[i]*A[k][j];
            B[i]=B[i]-factor[i]*B[k];
        }

        pthread_barrier_wait(&barrier_Elimination);

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

    pthread_barrier_init(&barrier_Division,NULL,NUM_THREADS);
    pthread_barrier_init(&barrier_Elimination,NULL,NUM_THREADS);

    pthread_t handles[NUM_THREADS];
    threadParam_t param[NUM_THREADS];
    for(int t_id=0;t_id<NUM_THREADS;t_id++){
        param[t_id].t_id=t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,(void *)&param[t_id]);
    }

    for(int t_id=0;t_id<NUM_THREADS;t_id++){
        pthread_join(handles[t_id],NULL);
    }

    pthread_barrierattr_destroy(&barrier_Division);
    pthread_barrierattr_destroy(&barrier_Elimination);



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

    while(count1<1)
    {
        count1+=1;
        time_sum+=run();
        cout<<count1<<endl;
    }

    cout<<time_sum/1.0<<endl;


    return 0;
}
