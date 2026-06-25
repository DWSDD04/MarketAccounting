#include "styledlineedit.h"

StyledLineEdit::StyledLineEdit(QWidget* parent) : QLineEdit(parent) {
    setStyleSheet(
        "QLineEdit { "
        "  background-color: #FFFFFF; "
        "  border: 1px solid #9DB9D2; "
        "  padding: 6px 10px; "
        "  font-size: 13px; "
        "  color: #000000; "
        "}"
        "QLineEdit:focus { "
        "  border: 1px solid #5B8DB8; "
        "  background-color: #FFFFFF; "
        "}"
    );
    setMinimumHeight(28);
}