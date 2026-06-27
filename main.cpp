#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SpendingTracker");
    app.setOrganizationName("SpendingTrackerApp");

    MainWindow window;
    window.setWindowTitle("Spending Tracker");
    window.resize(900, 600);
    window.show();

    return app.exec();
}
