#include "dashboardpage.h"
#include <QLabel>
#include <QVBoxLayout>

DashboardPage::DashboardPage(QWidget* parent) : QWidget(parent) {
    setStyleSheet("background-color: #E8F0F8;");
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* lbl = new QLabel("Dashboard - Coming Soon");
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("font-size: 20px; color: #1a3a5c;");
    layout->addWidget(lbl);
}