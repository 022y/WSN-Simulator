#include "class.h"
#include <sstream>

Environment::Environment()
{
    currTime = 0;
    finishTime = STOP_TIME;
    version = 1;
    std::ostringstream str;
    str << "Начало симуляции БСС v." << version;
    Log.put(str.str(), log_header);
}

Environment::~Environment()
{
    string str = "Конец симуляции БСС.";
    for(int i = 0; i < NUM_NODES; i ++)	delete MyNodeArray[i];
    Log.put(str, log_header);
}

void Environment::createNode()
{
    srand(time(NULL));
    double x = rand() % MAX_X + (double)rand() / RAND_MAX;
    double y = rand() % MAX_Y + (double)rand() / RAND_MAX;
    MyNodeArray[0] = new MyNode(0,x,y,this);
    MyNodeArray[0]->setLogicalAddress(0);
    std::ostringstream str;
    str << "\tMAC-адрес:";
    str.width(3); str << std::right << 0;
    str <<" \tКоординаты: (";
    str.width(10); str << std::right << MyNodeArray[0]->getX();
    str << ", ";
    str.width(10); str << std::right << MyNodeArray[0]->getY();
    str << "). - сервер";
    Log.put(str.str(),log_simulator);
    for(int i = 1; i != NUM_NODES; i++)
    {
        int dontHear = 1, haveAddress = 0;
        int currentCounterNode = 0;
        unsigned int Address, Parent;
        while (dontHear == 1 && haveAddress == 0)
        {
            dontHear = 1;
            haveAddress = 0;
            x = rand() % MAX_X + (double)rand() / RAND_MAX;
            y = rand() % MAX_Y + (double)rand() / RAND_MAX;

            int counterChild = WEB_POWER-1;
            while(counterChild != -1 && haveAddress == 0)
            {
                for(int currentCounterNode = 0; currentCounterNode != i;currentCounterNode++)
                {
                    if(MyNodeArray[currentCounterNode]->getChild() == counterChild)
                    {
                        float r = sqrt(pow(MyNodeArray[currentCounterNode]->getX()-x,2)+pow(MyNodeArray[currentCounterNode]->getY()-y,2));
                        if(r<COVERAGE_AREA)
                        {
                            Parent = MyNodeArray[currentCounterNode]->getLogicalAddress();
                            //Address = MyNodeArray[currentCounterNode]->ConnectionQuery();
                            haveAddress = 1;
                            dontHear=0;
                        }
                    }
                    if(haveAddress == 1)break;
                }
                counterChild-= 1;
            }
        }
        MyNodeArray[i] = new MyNode(i,x,y,this);
        std::ostringstream str;
        str << "\tMAC-адрес:";
        str.width(3); str << std::right << i;
        str <<" \tКоординаты: (";
        str.width(10); str << std::right << MyNodeArray[i]->getX();
        str << ", ";
        str.width(10); str << std::right << MyNodeArray[i]->getY();
        str << "). ";
        Log.put(str.str(), log_simulator);
    }

    cl_event event;

    for (int i = 1; i != NUM_NODES; i++)
    {
        event.timeEvent = rand();
        event.type = start;
        event.node = MyNodeArray[i];
        this->eventQueue.insertEvent(event);
    }
    string s = "Очередь событий.";
    Log.put(s, log_header);
    for (std::multiset<cl_event,cl_event_comp>::iterator it = (this->eventQueue.queue.begin()) ; it != this->eventQueue.queue.end(); it++ )
    {
        event = *it;
        std::ostringstream str;
        if(event.type!=shot)
        {
            str << "\tMAC-адрес:";
            str.width(3); str << std::right << event.node->getMAC();
        }else
        {
            str << "\tКоординаты звука: " << "[ " << shotX <<" , "<< shotY<< " ] ";
        }
        str << "\tТип:";
        str.width(7); str << std::right << event.type;
        str <<" \tВремя события: ";
        str.width(10); str << std::right << event.timeEvent;
        Log.put(str.str(), log_simulator);
    }
    s = "Конец очереди событий.";
    Log.put(s, log_header);
}
void Environment::createTableHearing()
{
    string s = "Таблица слышимости узлов:";
    Log.put(s, log_header);
    for (int i = 0; i != NUM_NODES; i++)
    {
        int x = i;
        string s = "\tNode № ";
        char tmp[20];
        sprintf(tmp, "%d", x);
        s = s+tmp+ ":";
        for(int j = 0; j != NUM_NODES; j++)
        {
            if(i != j )
            {
                float r = sqrt(pow(MyNodeArray[i]->getX()-MyNodeArray[j]->getX(),2)+pow(MyNodeArray[i]->getY()-MyNodeArray[j]->getY(),2));
                if (r < COVERAGE_AREA)
                {
                    int x = j;
                    s = s+" № ";
                    char tmp[20];
                    sprintf(tmp, "%d", x);
                    s = s + tmp;
                }
            }
        }
        Log.put(s, log_simulator);
    }
    s = "Конец таблицы слышимости.";
    Log.put(s, log_header);

    s = "Матрица слышимости узлов.";
    Log.put(s, log_header);
    TableHear[0][0] = 1;
    for (int i = 0; i != NUM_NODES; i++)
    {
        for(int j = 0; j != NUM_NODES; j++)
        {
            if(i != j )
            {
                float r = sqrt(pow(MyNodeArray[i]->getX()-MyNodeArray[j]->getX(),2)+pow(MyNodeArray[i]->getY()-MyNodeArray[j]->getY(),2));
                if (r < COVERAGE_AREA)
                {
                    TableHear[i][j] = 1;
                    TableHear[j][i] = 1;
                }
                else
                {
                    TableHear[i][j] = 0;
                    TableHear[j][i] = 0;
                }
            }else TableHear[i][j] = 1;
        }
    }

    for (int i = 0; i != NUM_NODES; i++)
    {
        std::ostringstream str;
        str << "\t";
        for (int j = 0; j != NUM_NODES; j++)
        {
            str  << TableHear[i][j] << " ";
        }
        Log.put(str.str(), log_simulator);
    }
    s = "Конец матрицы слышимости.";
    Log.put(s, log_header);

    s = "Параметры CSMA/CA";
    Log.put(s, log_header);
    std::ostringstream str1;
    str1 << "\tmacMinBE = " << macMinBE << " ";
    str1 << "macMaxBE = " << macMaxBE << " ";
    str1 << "aUnitBackoffPeriod = " << aUnitBackoffPeriod << " ";
    str1 << "macMaxCSMAbackoffs = " << macMaxCSMAbackoffs << " \n";
    Log.put(str1.str(), log_simulator);
}

void Environment::releaseEvent()
{
    std::ostringstream str;
    logMsgType type;
    cl_event event = this->eventQueue.getFirst();

    //str << "Среда: ";
//    str.width(12); str << std::right << event.timeEvent;
//    str << "\t";
    if(event.type!=shot)
    {
//        str << "Узел MAC - ";
//        str.width(3);  str << std::right << event.node->getMAC();
//            str << "\t";
//        str << "Узел LOG - ";
//        str.width(3);  str << std::right << event.node->getLogicalAddress();
//            str << "\t";
    }


    type=log_simulator;

    switch (event.type)
    {
    case start:
        //str << "Узел включен.";
        break;
    case TimeoutCSMACA:
        //str << "Алгоритм CSMA/CA.";
        break;
    case TX_begin:
        //str << "Начало отправки сообщения: " << event.msg.type << " Длина сообщения: " << event.msg.getLenth();
        break;
    case TX_end:
        //str << "Конец отправки сообщения.";
        break;
    case RX_begin:
        //str << "Начало приема сообщения: " << event.msg.type << " с узла MAC-" << event.msg.from;
        break;
    case RX_end:
        //str << "Конец приема сообщения: "<< event.msg.type << " с узла MAC-" << event.msg.from;
        break;
    case shot:
        processShotEvent(event);
        str << "Обработка события выстрела завершена" ;
        break;
    case hearTheShot:
        str << "Услышал выстрел" ;
        break;
    case checkConnection:
        str << "Проверка подключения к сети" ;
        break;
    }
    //Log.put(str.str(),type);

    this->currTime = event.timeEvent;
    if(event.type!=shot)
        event.node->nodeEvent(event);

    this->eventQueue.deleteEvent();
}

void Environment::sendMessage(int mac,cl_message pMsg)
{
    cl_event newevent;

    pMsg.data = pMsg.data + hexsimbolTime*pMsg.getLenth();

    newevent.timeEvent = this->currTime;
    newevent.type = TX_begin;
    newevent.node = MyNodeArray[mac];
    newevent.msg = pMsg;
    this->eventQueue.insertEvent(newevent);

    newevent.timeEvent = this->currTime+hexsimbolTime*pMsg.getLenth();
    newevent.type = TX_end;
    newevent.node = MyNodeArray[mac];
    newevent.msg = pMsg;
    this->eventQueue.insertEvent(newevent);

    for(int i = 0; i != NUM_NODES; i++)
    {
        if(TableHear[mac][i] == 1 && mac != i)
        {
            newevent.timeEvent = this->currTime;
            newevent.type = RX_begin;
            newevent.node = MyNodeArray[i];
            newevent.msg = pMsg;
            this->eventQueue.insertEvent(newevent);

            newevent.timeEvent = this->currTime+hexsimbolTime*pMsg.getLenth();
            newevent.type = RX_end;
            newevent.node = MyNodeArray[i];
            newevent.msg = pMsg;
            this->eventQueue.insertEvent(newevent);
        }
    }
}

void Environment::createShotEvent()
{
    cl_event newevent;

    newevent.timeEvent = shotTime;
    newevent.type = shot;

    this->eventQueue.insertEvent(newevent);
}

void Environment::processShotEvent(cl_event event)
{
    std::ostringstream str;

    str << "\t\t\tВыстрел!\t";
    str.width(12);str << std::right << event.timeEvent;
    str << "\t";
    str <<"Громкость: " << shotVolume << " dB Координаты: [ " << shotX << " , " << shotY << " ] ";
    str << "\tОбработка события выстрела\n";
    int counterConnected = 0;
    for(int i = 1; i != NUM_NODES; i++)
    {
        if(MyNodeArray[i]->isConnected)counterConnected++;
        if(!MyNodeArray[i]->isConnected&&MyNodeArray[i]->connectionAttemptsCount>MAX_NUMBER_OF_CONNECTION_ATTEMPTS)return;
    }
    if(counterConnected!=NUM_NODES-1)
    {
        cl_event newevent;

        newevent.timeEvent = event.timeEvent + shotTime;
        newevent.type = shot;
        this->eventQueue.insertEvent(newevent);
        str << "\t\t\tПодключено узлов к сети: "<< counterConnected << "\n";
        Log.put(str.str(), log_simulator);
        return;
    }

    for(int i = 1; i != NUM_NODES; i++)
    {
        double r = sqrt(pow(MyNodeArray[i]->getX()-shotX,2)+pow(MyNodeArray[i]->getY()-shotY,2));
        double N = shotVolume - 20*log10(r);

        unsigned long long int eventTime = event.timeEvent + r/soundSpeed;

        str << "\t\tУзел №"<< i <<" улышал выстрел громоксти "<< N << "dB \n";
        str << "\t\t\tРасстояние "<< r << "\n";

        str << "\t\t\tВремя хода звука к узлу "<< str.width(12); str << std::right << r/soundSpeed << "\n";
        str << "\t\t\tВремя обнаружения узлом "<< str.width(12); str << std::right << eventTime << "\n";

        if(N>=minShotVolume)
        {
            cl_event newevent;
            newevent.timeEvent = eventTime;
            newevent.type = hearTheShot;
            newevent.node = MyNodeArray[i];
            this->eventQueue.insertEvent(newevent);
        }

    }
    Log.put(str.str(), log_simulator);
}



