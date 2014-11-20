
#include "class.h"

//******************************************************************************
//класс логов
//******************************************************************************
void MyLog::MyCreateNode(int number, int addr,int par,float x, float y)
{

    string name = "Node № ";
    if (number == 0) name = "Pan ";

    LogFile << name<< number << " "<<"MAC - "<<number<< " Logical - "<<addr<<" Parent - "<<par<<" [" << x<<";"<<y<<"];"<<endl;
};

void MyLog::MyLogRow(string str)
{
    LogFile << str ;
};

void MyLog::MyLogRecord(string str)
{
    LogFile <<" "<<  str;

};

void MyLog::put(string str, logMsgType type)
{
    if(type == log_header)	LogFile << str << endl;
    if(type == log_simulator) LogFile <<" - "<<  str << endl;
};

MyLog::~MyLog()
{
    LogFile.close();
}

