#include <iostream>
#include <unistd.h>
#include <mpi.h>
using namespace std;

//multi-process print message
void testHello();

//test each process cost time 
void testTime();


//test multi-process send-recv data with eachother
void test_sendrecv_data();

//测试点对点通信的非阻塞方式
void test_sendrecv_data_I();

//多个进程进程向0号进程发送数据，0号进程接收所有数据然后处理
void test_sr_handle_data();

int main(){
    //testHello();
    //testTime();
    //test_sendrecv_data();
    test_sendrecv_data_I();
    //test_sr_handle_data();
    return 0;
}

void testHello(){
    int my_rank;
    int my_size;
    int nameLen;
    char nameProcess[128];

    //Init MPI Pro
    MPI_Init(NULL, NULL);

    //rank of process
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    //number of process
    MPI_Comm_size(MPI_COMM_WORLD, &my_size);

    //get name of process
    MPI_Get_processor_name(nameProcess, &nameLen);

    //get version of MPI
    int version, sub_version;
    MPI_Get_version(&version, &sub_version);

    //print
    cout<<"rank:"<<my_rank<<"  of size:"<<my_size<<"  name:"<<nameProcess<<"  mess:Hello MPI!!!"<<endl;
    
    if(my_rank == 0){
        cout<<"rank_0 print MPI VERSION:"<<version<<"."<<sub_version<<endl;
    }
    //shut down MPI
    MPI_Finalize();


    return ;
}


void testTime(){

    int rank, size;
    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
     
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    double m_start,m_end;
    if(rank == 0){
        m_start = MPI_Wtime();
        cout<<"process - "<<rank<<" start at: "<<m_start<<endl;
        //sleep(1000);秒级
        usleep(1e6);//微秒级
        m_end = MPI_Wtime();
        cout<<"process - "<<rank<<" end at: "<<m_end<<endl;
    }

     if(rank == 1){
        m_start = MPI_Wtime();
        cout<<"process - "<<rank<<" start at: "<<m_start<<endl;
        //sleep(2000); // 秒级
        usleep(2e6);//微秒级
        m_end = MPI_Wtime();
        cout<<"process - "<<rank<<" end at: "<<m_end<<endl;
    }

    
    for(int i=0; i<1e4;++i){
        float a = i*1.0 / (i+1);
    }

    //同步所有进程， 这里进程0会等待 进程1运行到同步点，然后再继续运行
    MPI_Barrier(MPI_COMM_WORLD);

    m_end = MPI_Wtime();
    cout<<">>Final  process - "<<rank<<" end at: "<<m_end<<endl;
    MPI_Finalize();
    return;
}

void test_sendrecv_data(){
    int rank, size;

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
     
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Status status;

    int sendValue = 10;
    int recvValue;
    if(size <2){
        cout<<"Error: process size less than 2, send data failed!!"<<endl;
        return;
    }

   //交接传递信息  process 0 -> process 1 -> process 2 ...
    if(rank == 0){
        MPI_Send(&sendValue,  //send buffer
                1,  //send number of data
                MPI_INT, //send type of data
                rank+1, //dest process
                0,  // send message tag
                MPI_COMM_WORLD  //comm region
                );
        cout<<"rank: "<<rank<<" send: "<<sendValue<<" to "<<"   rank "<<rank+1<<endl;
    }else{
        MPI_Recv(&recvValue, // get buffer
                1, //get number of data
                MPI_INT, //get type of data
                rank-1, // source process
                0, //get message tag,   [if tag != send tag, cannot recv data]
                MPI_COMM_WORLD, // comm region
                &status
        );
         cout<<"--rank: "<<rank<<" recv: "<<recvValue<<" from "<<"  rank "<<rank-1<<endl;
         // 假设4个进程[ 0 1 2 3]，  进程3 就不发送了,因为没有接收进程了
         if(rank < size-1){
            MPI_Send(&sendValue,  //send buffer
                    1,  //send number of data
                    MPI_INT, //send type of data
                    rank+1, //dest process
                    0,  // send message tag
                    MPI_COMM_WORLD  //comm region
                    );
            cout<<"rank: "<<rank<<" send: "<<sendValue<<" to "<<"   rank "<<rank+1<<endl;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return;
}

void test_sendrecv_data_I(){
    int rank, size;

    int data[10]={1,2,3,4,5,6,7,8,9,10};
    int data_1[10]={};
    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
     
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Status status;
    int tag = 1234;

    if(rank == 0){
        MPI_Request request;
        MPI_Isend(data,  //send buffer
                10,  //send number of data
                MPI_INT, //send type of data
                1, //dest process
                tag,  // send message tag
                MPI_COMM_WORLD,  //comm region
                &request);

        //wait the message send out
        MPI_Wait(&request, &status);
    }

    if(rank ==1){
        MPI_Probe(MPI_ANY_SOURCE, tag,  MPI_COMM_WORLD, &status);
        if(status.MPI_SOURCE == 0){
            MPI_Recv(data_1, // get buffer
                    10, //get number of data
                    MPI_INT, //get type of data
                    0, // source process
                    tag, //get message tag,   [if tag != send tag, cannot recv data]
                    MPI_COMM_WORLD, // comm region
                    &status
            );
        }

        for (int i = 0; i < 10; i++)
        {
            /* code */
            cout<<"rank-"<<rank<<"  recv data:"<<data_1[i]<<endl;
        }
        
    }

    MPI_Finalize();
    return;

}
void test_sr_handle_data(){
     int rank, size;
     MPI_Status status;

    //初始化数据放在Init外，防止多进程重复执行
    int data[16];
     for (int i = 0; i < 16; i++)
     {
         data[i]=i+10;
     }
    

     MPI_Init(NULL, NULL);
     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
     MPI_Comm_size(MPI_COMM_WORLD, &size);
   

     //process [1,, n-1] send data to process 0； send 11 12 ...
     if(rank != 0){
         
           MPI_Send(data+rank,  //send buffer
                    1,  //send number of data
                    MPI_INT, //send type of data
                    0, //dest process
                    0,  // send message tag
                    MPI_COMM_WORLD  //comm region
                    );
           // cout<<"rank: "<<rank<<" send: "<<sendValue<<" to "<<"   rank "<<rank+1<<endl;
     }else{
         //process 0 handle recv data
         int sumValue=0;
         for (int id = 1; id < size; id++)
         {
             int getValue;
             MPI_Recv(&getValue, // get buffer
                    1, //get number of data
                    MPI_INT, //get type of data
                    id, // source process
                    0, //get message tag,   [if tag != send tag, cannot recv data]
                    MPI_COMM_WORLD, // comm region
                    &status 
            );
            cout<<"rank: "<<rank<<"  recv data: "<<getValue<<endl;
            sumValue += getValue;
         }//for
        cout<<"rank: "<<rank<<" ,sumValue is: "<<sumValue<<endl;
         
     }//if-else

     MPI_Finalize();
     return;
}

