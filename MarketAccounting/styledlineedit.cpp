#include "styledlineedit.h"

StyledLineEdit::StyledLineEdit(QWidget* parent) : QLineEdit(parent) {
    setStyleSheet(
        "QLineEdit {"
        "  background-color: #ffffff;"
        "  border: 2px solid #e9ecef;"
        "  border-radius: 8px;"
        "  padding: 10px 14px;"
        "  font-size: 13px;"
        "  color: #212529;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #16213e;"
        "}"
    );
    setMinimumHeight(38);
}