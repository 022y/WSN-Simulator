#include <sstream>
#include "class.h"

void MyNode::nodeEvent(cl_event event)
{
    std::ostringstream str;
    cl_event newEvent;
    long long unsigned int timeCSMACA = 30000 + hexsimbolTime*aUnitBackoffPeriod*(rand()%int(pow(2.0,this->BE)-1));

    str.width(12); str << std::right << event.timeEvent;
    str << "\t";
    str << "Узел MAC - ";
    str.width(3);  str << std::right << getMAC();
        str << "\t";
    str << "Узел LOG - ";
    str.width(3);  str << std::right << getLogicalAddress();
        str << "\t";

    switch(event.type)
    {
        case start:
            newEvent.type = TimeoutCSMACA;
            newEvent.timeEvent = this->environment->currTime + timeCSMACA;
            newEvent.node = this;

            str << "Узел включен.\t\tАлгоритм CSMA/CA. Следующая проверка cca будет в " << newEvent.timeEvent;
            this->environment->Log.put(str.str(),log_simulator);

            newEvent.msg.type = WebJoin;
            newEvent.msg.from = MACAddress;

            this->environment->eventQueue.insertEvent(newEvent);

            this->NB=0;
            this->BE=macMinBE;
            this->canRX = false;

            //cl_event connectionEvent;
            newEvent.type = checkConnection;
            newEvent.timeEvent = this->environment->currTime + rand()/205*rand();//5000000;
            newEvent.node = this;
            this->environment->eventQueue.insertEvent(newEvent);

            break;
        case hearTheShot:
//??

            newEvent.type = TimeoutCSMACA;
            newEvent.timeEvent = this->environment->currTime + timeCSMACA;
            newEvent.node = this;

            str << "Услышал выстрел. Алгоритм CSMA/CA. Следующая проверка cca будет в " << newEvent.timeEvent;
            this->environment->Log.put(str.str(),log_simulator);

            newEvent.msg.type = hearTheShotTransmitted;
            newEvent.msg.from = MACAddress;
            newEvent.msg.to = Parent;
            newEvent.msg.data = event.msg.data + timeCSMACA;

            this->environment->eventQueue.insertEvent(newEvent);

            this->NB=0;
            this->BE=macMinBE;
            this->canRX = false;
            break;
        case checkConnection:
            if(!isConnected&&connectionAttemptsCount<=MAX_NUMBER_OF_CONNECTION_ATTEMPTS)
            {
                newEvent.type = TimeoutCSMACA;
                newEvent.timeEvent = this->environment->currTime + timeCSMACA;
                newEvent.node = this;

                str << "Узел до сих пор не подключен к сети. Алгоритм CSMA/CA. Следующая проверка cca будет в " << newEvent.timeEvent;
                this->environment->Log.put(str.str(),log_simulator);

                connectionAttemptsCount = connectionAttemptsCount + 1;

                newEvent.msg.type = WebJoin;
                newEvent.msg.from = MACAddress;

                this->environment->eventQueue.insertEvent(newEvent);

                this->NB=0;
                this->BE=macMinBE;
                this->canRX = false;

                //cl_event connectionEvent;
                newEvent.type = checkConnection;
                newEvent.timeEvent = this->environment->currTime + 5000000;
                newEvent.node = this;
                this->environment->eventQueue.insertEvent(newEvent);
            }
            break;
        case TX_begin:
            str << "Начало отправки сообщения: " << event.msg.type << " Длина сообщения: " << event.msg.getLenth();
            this->txSignal=true;
            this->environment->Log.put(str.str(),log_simulator);
            break;
        case TX_end:
            //this->environment->Log.put("\t\tОтправка завершена.",log_simulator);
            str << "Конец отправки сообщения.";
            this->environment->Log.put(str.str(),log_simulator);
            this->txSignal=false;
            this->canRX = true;
            break;
        case RX_begin:
            str << "Начало приема сообщения: " << event.msg.type << " с узла MAC-" << event.msg.from;
            SignalCount ++;
            if(SignalCount > 1) collision = true;
            //this->environment->Log.put("\t\tНачат прием сообщения.",log_simulator);
            this->environment->Log.put(str.str(),log_simulator);
            break;
        case RX_end:
            str << "Конец приема сообщения: "<< event.msg.type << " с узла MAC-" << event.msg.from;
            SignalCount --;
            if(collision)
            {
                str <<"\t\tКоллизия. Сообщение не принято";
                this->environment->Log.put(str.str(),log_simulator);
            }
            else
            {
                //this->environment->Log.put("\t\tСообщение принято.",log_simulator);
                str << "\tОбработка сообщения. ";
                this->environment->Log.put(str.str(),log_simulator);
                this->messageProcessing(event);
            }
            if(SignalCount <=0) collision = false;
            break;
        case TimeoutCSMACA:
            if(this->cca(MACAddress))
            {
                str << "Алгоритм CSMA/CA.\t\tКанал свободен. Начинаем отправку.";
                this->environment->Log.put(str.str(),log_simulator);
                this->environment->sendMessage(this->MACAddress, event.msg);
            }
            else
            {
                this->NB++;
                this->BE = std::min(this->BE+1,macMaxBE);
                if(NB > macMaxCSMAbackoffs)
                {
                    str << "Алгоритм CSMA/CA.\t\tКанал занят. Попытки исчерпаны.";
                    this->environment->Log.put(str.str(),log_simulator);
                }
                else
                {
                    newEvent.timeEvent = this->environment->currTime + timeCSMACA - 30000;
                    newEvent.type = TimeoutCSMACA;

                    str << "Алгоритм CSMA/CA.\t\tНеудачных попыток (NB): " << this->NB << ". ";

                    newEvent.msg = event.msg;
                    newEvent.msg.data = event.msg.data + timeCSMACA - 30000;
                    newEvent.node = this;

                    this->environment->eventQueue.insertEvent(newEvent);

                    str << "Канал занят. Новая попытка доступа будет в " << newEvent.timeEvent << ".";
                    this->environment->Log.put(str.str(),log_simulator);

                }
            }
            break;

    }
};

bool MyNode::cca(int mac)
{
   for(int i = 0; i != NUM_NODES; i ++)
   {
        if(i != mac && this->environment->MyNodeArray[i]->txSignal && this->environment->TableHear[mac][i]==1)
        {
            return false;
        }
   }
   return true;
}

void MyNode::messageProcessing(cl_event event)
{
    std::ostringstream str;
    cl_event newEvent;
    long long unsigned int timeCSMACA =  30000+ hexsimbolTime*aUnitBackoffPeriod*(rand()%int(pow(2.0,this->BE)-1));


    switch (event.msg.type)
    {
        case WebJoin:
            if(isConnected&&!pendingConfirmation)
            {
                str << "\t\t\tПросьба выдачи лог. адреса с узла МАС - "<<event.msg.from<<". ";

                if(Child<WEB_POWER)
                {
                    newEvent.node = this;
                    newEvent.type = TimeoutCSMACA;
                    newEvent.timeEvent = this->environment->currTime +30000+ hexsimbolTime*aUnitBackoffPeriod*(rand()%int(pow(2.0,this->BE)-1));

                    str << "\t\tАлгоритм CSMA/CA. Следующая проверка cca будет в " << newEvent.timeEvent;

                    newEvent.msg.type = LogicalAddressTransmitted;
                    newEvent.msg.from = this->getMAC();
                    newEvent.msg.to = event.msg.from;
                    str << "\t\LOG =  " << this->LogicalAddress << " Мощность сети = " << WEB_POWER << " Дочерних = " << this->Child;
                    newEvent.msg.addr = this->ConnectionQuery();

                    this->pendingConfirmation = true;
                    this->tmpCild = event.msg.from;
                    this->NB=0;
                    this->BE=macMinBE;
                    this->canRX = false;

                    this->environment->eventQueue.insertEvent(newEvent);
                    this->environment->Log.put(str.str(),log_simulator);
                }
            }
            break;
        case LogicalAddressTransmitted:
            if(!isConnected)
            {
                if(event.msg.to == this->getMAC())
                {
                    str << "\t\t\tВыдача логического адреса узлом MAC - "<<event.msg.from<< " ";

                    this->setLogicalAddress(event.msg.addr);
                    this->Parent = event.msg.from;
                    this->isConnected = true;
                    str << "\t\tУзел Мас - " << this->getMAC() <<" Логический адрес - "<<this->LogicalAddress<<" Родительский узел - "<<event.msg.from;

                    newEvent.node = this;
                    newEvent.type = TimeoutCSMACA;
                    newEvent.timeEvent = this->environment->currTime +30000+ hexsimbolTime*aUnitBackoffPeriod*(rand()%int(pow(2.0,this->BE)-1));

                    str << "\t\tАлгоритм CSMA/CA. Следующая проверка cca будет в " << newEvent.timeEvent;

                    newEvent.msg.type = LogicalAddressRecieved;
                    newEvent.msg.from = this->getMAC();
                    newEvent.msg.to = event.msg.from;

                    this->environment->eventQueue.insertEvent(newEvent);
                    this->environment->Log.put(str.str(),log_simulator);
                }
            }
            break;

        case LogicalAddressRecieved:
            if(event.msg.from==this->tmpCild)
                this->pendingConfirmation = false;
            if(event.msg.from==this->tmpCild&&event.msg.to != this->getMAC())
                Child = Child - 1;
            if(event.msg.to == this->getMAC())
            {
                str << "\t\t\t Подтверждение получения логического адреса узлом MAC - "<<event.msg.from;
                this->environment->Log.put(str.str(),log_simulator);
            }
            break;
        case hearTheShotTransmitted:
            if(event.msg.to == this->getMAC())
            {
                if(this->getLogicalAddress()==0)
                {
                    str << "\t\t\t Координатор получил информацию о выстреле от узла MAC - "<<event.msg.from;
                    str << " Выстрел услышан  "<<event.msg.data << " мкс назад";
                    InfoTable[event.msg.from]=event.timeEvent - event.msg.data;
                }else
                {
                    newEvent.type = TimeoutCSMACA;
                    newEvent.timeEvent = this->environment->currTime + timeCSMACA;
                    newEvent.node = this;

                    str << "\t\tУслышал выстрел. Алгоритм CSMA/CA. Следующая проверка cca будет в " << newEvent.timeEvent;

                    newEvent.msg.type = hearTheShotTransmitted;
                    newEvent.msg.from = event.msg.from;
                    newEvent.msg.to = Parent;
                    newEvent.msg.data = event.msg.data + timeCSMACA;

                    this->environment->eventQueue.insertEvent(newEvent);

                    this->NB=0;
                    this->BE=macMinBE;
                    this->canRX = false;
                    break;
                }
                this->environment->Log.put(str.str(),log_simulator);
            }
            break;
    }
    //this->environment->Log.put(str.str(),log_simulator);

}

bool MyNode::displayInfoAboutShot()
{
   std::ostringstream str;
   str << "\tТаблица времени идентификации выстрела узлами:\n" ;
   for(int i = 1; i != NUM_NODES; i ++)
   {
        if(InfoTable[i]!=0)
        {
            str << "\t\tУзел MAC - " << i << " услышал выстрел в " << InfoTable[i] << "\n" ;
        }
   }
   this->environment->Log.put(str.str(),log_simulator);
   return true;
}

bool MyNode::determinationOfTheCoordinates()
{
   std::ostringstream str;
   str << "\tОпределение координаты выстрела:\n" ;
   double minS = 9999999;
   int xS = -1,yS = -1;
   for(int x = 1; x < MAX_X; x ++)
   {
        for(int y = 1; y < MAX_Y; y ++)
        {
            double S = 0;
            for(int i = 1; i != NUM_NODES-1; i ++)
            {
                if(InfoTable[i]!=0)
                {
                    for(int j = i+1; j != NUM_NODES; j ++)
                    {
                        if(InfoTable[j]!=0&&i!=j)
                        {
                            //std::ostringstream strTmp;
                            //strTmp << "\tУзел№"<< i <<"  [" << environment->MyNodeArray[i]->getX()<<" , " <<environment->MyNodeArray[i]->getY()<< " ] услышал в "
                                //   <<InfoTable[i]<< " исследуемая координата: [ " <<x<< " , " << y<<" ]" << endl;
                            //strTmp << "\tУзел№"<< j <<"  [" << environment->MyNodeArray[j]->getX()<<" , " <<environment->MyNodeArray[j]->getY()<< " ] услышал в "
                               //    <<InfoTable[j]<< " исследуемая координата: [ " <<x<< " , " << y<<" ]" << endl;
                            double delta;
                            if(InfoTable[i]>InfoTable[j])
                                delta = soundSpeed*(InfoTable[i]-InfoTable[j]);
                            else
                                delta = soundSpeed*(InfoTable[j]-InfoTable[i]);
                            double result = (double)delta;
                            //strTmp << "\tДельта = "<< delta<< endl;
                            S = S + abs(sqrt(pow(environment->MyNodeArray[i]->getX()-x,2)+pow(environment->MyNodeArray[i]->getY()-y,2))-sqrt(pow(environment->MyNodeArray[j]->getX()-x,2)+pow(environment->MyNodeArray[j]->getY()-y,2))) - result;
                           // strTmp << "\tS = "<< S << endl;
                            //this->environment->Log.put(strTmp.str(),log_simulator);
                        }
                    }
                }

            }
            if(minS>abs(S))
            {
                minS=abs(S);
                xS = x;
                yS = y;
            }

        }
   }
   str << "\tКоордината: [ "<< xS <<" , " << yS << " ]";
   this->environment->Log.put(str.str(),log_simulator);
   return true;
}
