#include "debtspage.h"
#include <QLabel>
#include <QVBoxLayout>

DebtsPage::DebtsPage(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* lbl = new QLabel("Debts & Accounting - Coming Soon");
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("font-size: 24px; color: #1a1a2e;");
    layout->addWidget(lbl);
}