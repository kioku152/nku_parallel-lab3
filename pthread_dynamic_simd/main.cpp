#include <iostream>
#include <pthread.h>
#include<windows.h>
#include<stdio.h>
#include<cstdlib>


using namespace std;

const int n=1000;
static float A[n][n];
static float B[n];
static float X[n];
static float factor[n];

typedef struct{
    int k;
    int t_id;
}threadParam_t;

void *threadFunc(void *param){
    threadParam_t *p=(threadParam_t*)param;
    int k=p->k;
    int t_id=p->t_id;
    int i=k+t_id+1;

             for(int j=k+1;j<n;j+=8){
            __m256 Aij = _mm256_loadu_ps(&A[i][j]);
            __m256 Akj = _mm256_loadu_ps(&A[k][j]);
            Aij = _mm256_sub_ps(Aij, _mm256_mul_ps( _mm256_set1_ps(factor[i]), Akj));
            _mm256_storeu_ps(&A[i][j], Aij);}
        B[i]=B[i]-factor[i]*B[k];


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

    for(int k=0;k<n;k++)
    {
        for(int j=k+1;j<n;j+=8){
            __m256 Ajk = _mm256_loadu_ps(&A[j][k]);
            __m256 factor_ = _mm256_div_ps(Ajk, _mm256_set1_ps(A[k][k]));
            _mm256_storeu_ps(&factor[j], factor_);
        }


        int worker_count = n-1-k;
        pthread_t* handles=new pthread_t[worker_count];
        threadParam_t* param = new threadParam_t[worker_count];


        for(int t_id=0;t_id<worker_count;t_id++)
        {
            param[t_id].k=k;
            param[t_id].t_id=t_id;
        }
        for(int t_id=0;t_id<worker_count;t_id++)
        {
            pthread_create(&handles[t_id],NULL,threadFunc,(void *)&param[t_id]);
        }

        for(int t_id=0;t_id<worker_count;t_id++){
            pthread_join(handles[t_id],NULL);
        }



    }




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
    return answer ;
}

int main()
{
    float time_sum;
    int count1=0;

    while(count1<3)
    {
        count1+=1;
        time_sum+=run();
        cout<<count1<<endl;
    }

    cout<<time_sum/3.0<<endl;


    return 0;
}
