#include "productspage.h"
#include <QLabel>
#include <QVBoxLayout>

ProductsPage::ProductsPage(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* lbl = new QLabel("Products - Coming Soon");
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("font-size: 24px; color: #1a1a2e;");
    layout->addWidget(lbl);
}