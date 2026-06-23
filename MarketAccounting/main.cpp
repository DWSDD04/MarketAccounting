#include "MarketAccounting.h"
#include "dbmanager.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // Arabic RTL support
    a.setLayoutDirection(Qt::RightToLeft);
    a.setFont(QFont("Segoe UI", 10));

    // Connect to MySQL
    if (!DbManager::instance()->connect("localhost", "market_accounting", "root", "Mayarima3")) {
        QMessageBox::critical(nullptr, "Database Error",
            "Failed to connect to MySQL. Make sure the server is running.\n\n"
            "Error: " + DbManager::instance()->lastError().text());
        // Continue without DB for UI testing, or return 1;
    }

    MarketAccounting w;
    w.show();

    return a.exec();
}