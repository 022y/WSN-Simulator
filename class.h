#include <iostream>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <string>
#include <cmath>
#include <sys/timeb.h>
#include <set>
#include <stdlib.h>

using namespace std;

class MyNode;
class MyPan;
class Environment;
class MyEvent;
class MyLog;
class cl_message;
class cl_event;


enum logMsgType
{
log_header,
log_simulator
};
enum eventType
{
    RX_begin=5,
    RX_end=6,
    TX_begin=3,
    TX_end=4,
    start = 0,
    hello = 1,
    TimeoutCSMACA = 2,
    TX_check,
    checkConnection = 7,
    shot = 8,
    hearTheShot = 9,
    PROGRAM
};
enum msg_type
{
    WebJoin = 0,
    LogicalAddressTransmitted = 1,
    LogicalAddressRecieved = 2,
    hearTheShotTransmitted = 3,
    hearTheShotRecieved = 4

};

#define hexsimbolTime 32 //250kbit/s: 4*(1000000000/(250*1000))

#define shotX  470
#define shotY  682
#define shotTime  10000000

#define MAX_NUMBER_OF_CONNECTION_ATTEMPTS  10

#define shotVolume 120
#define minShotVolume 60
#define soundSpeed 0.000330

#define macMinBE 3
#define macMaxBE 8
#define aUnitBackoffPeriod 20
#define macMaxCSMAbackoffs 4

#define MAX_X 1000
#define MAX_Y 1000
#define NUM_NODES 5
#define COVERAGE_AREA 486
#define STOP_TIME 1000000000000
#define WEB_POWER 3
#define FILE_LOG "log.txt"

class MyLog
{
    ofstream LogFile;
    time_t currentDateTime;

    public:
        MyLog(){remove(FILE_LOG);LogFile.open(FILE_LOG,ios::app);};
        void MyCreateNode(int, int,int,float, float);
        void MyLogRow(string);
        void MyLogRecord(string);
        void put(string, logMsgType);
        ~MyLog();
};

class MyNode
{


    public:
        bool txSignal,canRX,collision,pendingConfirmation;
        int NB ;
        int BE ;
        int messageCount;
        int connectionAttemptsCount = 0;
        int SignalCount;
        long long unsigned int InfoTable[NUM_NODES]= {0};

        double x,y;
        bool isConnected;
        unsigned int MACAddress,Child, Parent,tmpCild=-1;
        int LogicalAddress;
        Environment* environment;

        MyNode(){x=0.0; y=0.0; MACAddress=0;Child=0;txSignal=false;canRX=true;isConnected=true;pendingConfirmation = false;}
        MyNode(int addr, double x1, double y1,Environment* env)
        {
            x=x1;
            y=y1;
            if(addr==0)
                isConnected = true;
            else
                isConnected = false;
            pendingConfirmation = false;
            MACAddress=addr;
            Child=0;

            if(addr==0)
                LogicalAddress = 0;
            else
                LogicalAddress = -1;

            environment = env;
            txSignal=false;
            canRX=true;
            NB = 0;
            BE = macMinBE;
            messageCount = 0;
            SignalCount = 0;
            collision = false;
        }
        double getX(){return x;}
        double getY(){return y;}
        int getMAC(){return MACAddress;}
        int getChild(){return Child;}
        int getLogicalAddress(){return LogicalAddress;}
        unsigned int ConnectionQuery (){Child = Child + 1; return LogicalAddress*WEB_POWER+Child;}
        void nodeEvent(cl_event);
        void setLogicalAddress(int addr){LogicalAddress = addr;}
        void messageProcessing(cl_event event);
        bool displayInfoAboutShot();
        bool cca(int );
        bool determinationOfTheCoordinates();
};
//сервер
class MyPan
{
    double x,y;
    unsigned int MACAddress,LogicalAddress,Child;
    Environment* environment;

    public:
        bool txSignal,canRX, collision,pendingConfirmation;
        int NB;
        int BE;
        int messageCount;
        int SignalCount;


        MyPan(){x=0.0; y=0.0; MACAddress=0; LogicalAddress = 0;Child = 0;txSignal=false;canRX=true;}
        MyPan(double x1, double y1,Environment* env)
        {
            x=x1; y=y1;
            MACAddress=0;
            LogicalAddress = -1;
            Child = 0;
            environment = env;
            txSignal=false;
            canRX=true;
            NB = 0;
            BE = macMinBE;
            messageCount = 0;
            SignalCount = 0;
            collision = false;
        }
        double getX(){return x;}
        double getY(){return y;}
        int getChild(){return Child;}
        unsigned int ConnectionQuery (){return LogicalAddress*WEB_POWER+Child;}
        void nodeEvent(cl_event);

        bool cca(int );
};

class cl_message
{
    public:
        msg_type type;
        bool isRequest;
//        char Data[115];
        int from;
        int to;
        int addr;
        long long unsigned int data = 0;

        int getLenth()
        {
            int len;
            return len = 4 + 1 + 1 + 2 + 1 + 8 /* + 2*/ + 2;
        };
};

class cl_event
{
public:
    long long unsigned int timeEvent;
    eventType type;
    MyNode* node;
    MyPan* pan;
    cl_message msg;
};

class cl_event_comp
{
public:
    bool operator() (const cl_event& lhs,const cl_event& rhs) const {return lhs.timeEvent<rhs.timeEvent;}
};

class cl_eventQueue
{
public:
    std::multiset <cl_event,cl_event_comp> queue;
    void insertEvent(cl_event);
    void deleteEvent();
    cl_event getFirst();
};



//******************************************************************************
//******************************************************************************
//среда
class Environment
{
    public:
        MyLog Log;
        MyNode* MyNodeArray[NUM_NODES];
        MyPan* Pan;
        int TableHear[NUM_NODES+1][NUM_NODES+1];
        long long unsigned int currTime;
        long long unsigned int finishTime;
        cl_eventQueue eventQueue;
        int version;

        Environment();
        ~Environment();
        bool timeIsOver(){return this->finishTime <= this->currTime;}
        bool queueIsOver(){return this->eventQueue.queue.empty();}
        void releaseEvent();
        void createNode();
        void createTableHearing();
        void sendMessage(int mac,cl_message);
        void createShotEvent();
        void processShotEvent(cl_event event);
};

