#include "MarketAccounting.h"
#include "dbmanager.h"
#include <QApplication>
#include <QMessageBox>
#include <QStyleFactory>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // Arabic RTL support
    a.setLayoutDirection(Qt::RightToLeft);
    a.setFont(QFont("Segoe UI", 10));

    // Force native Windows style on all widgets
    a.setStyle(QStyleFactory::create("Windows"));

    // Connect to MySQL
    if (!DbManager::instance()->connect("localhost", "market_accounting", "root", "Mayarima3")) {
        QMessageBox::critical(nullptr, "Database Error",
            "Failed to connect to MySQL. Make sure the server is running.\n\n"
            "Error: " + DbManager::instance()->lastError().text());
    }

    MarketAccounting w;
    w.show();

    return a.exec();
}