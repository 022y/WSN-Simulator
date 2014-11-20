#include "class.h"

using namespace std;

/* run this program using the console pauser or add your own getch, system("pause") or input loop */
int main(int argc, char** argv) {

    Environment MyEnvironment;

    MyEnvironment.Log.put("Начало журнала.",log_header);
    MyEnvironment.createShotEvent();
    MyEnvironment.createNode();
    MyEnvironment.createTableHearing();


    while(!(MyEnvironment.timeIsOver() || MyEnvironment.queueIsOver()))
    {
        MyEnvironment.releaseEvent();
    }
    //MyEnvironment.MyNodeArray[0]->displayInfoAboutShot();
    MyEnvironment.MyNodeArray[0]->determinationOfTheCoordinates();
    MyEnvironment.Log.put("Конец журнала.",log_header);
    return 0;
}





