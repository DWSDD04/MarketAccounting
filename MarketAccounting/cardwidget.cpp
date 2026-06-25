#include "cardwidget.h"

CardWidget::CardWidget(QWidget* parent) : QFrame(parent) {
    setStyleSheet(
        "QFrame {"
        "  background-color: #E8F0F8;"
        "  border: 1px solid #9DB9D2;"
        "  border-radius: 4px;"
        "}"
    );
    setFrameShape(QFrame::StyledPanel);
}