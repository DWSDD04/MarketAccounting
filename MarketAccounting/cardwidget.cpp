#include "cardwidget.h"

CardWidget::CardWidget(QWidget* parent) : QFrame(parent) {
    setStyleSheet(
        "QFrame {"
        "  background-color: #ffffff;"
        "  border-radius: 12px;"
        "  border: 1px solid #e9ecef;"
        "}"
    );
    setFrameShape(QFrame::StyledPanel);
}