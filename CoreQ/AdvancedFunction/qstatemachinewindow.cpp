#include "qstatemachinewindow.h"

//State Machine Examples
//The Event Transitions Example 说明了QEventTransition怎么在状态机使用。把QEvent继承到状态机框架中使用。
//Factorial 说明QSignalTransition、QFinalState、QState怎么使用
//pingpong 说明了平行状态怎么用

QStateMachineWindow::QStateMachineWindow(QWidget *parent)
    : QWidget{parent}
{}
