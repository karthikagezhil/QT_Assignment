#include "player.h"

#include <QApplication>

int main(int argc, char *argv[])
{ 
    QApplication app(argc, argv); //class provides an event loop for Qt applications without UI 
    Player player;
    player.show();
    return app.exec();
}































