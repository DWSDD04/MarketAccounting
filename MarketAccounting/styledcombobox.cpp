#include "styledcombobox.h"

StyledComboBox::StyledComboBox(QWidget* parent) : QComboBox(parent) {
    setStyleSheet(
        "QComboBox { "
        "  background-color: #FFFFFF; "
        "  border: 1px solid #9DB9D2; "
        "  padding: 6px 10px; "
        "  font-size: 13px; "
        "  color: #000000; "
        "  min-height: 28px; "
        "}"
        "QComboBox:focus { border: 1px solid #5B8DB8; }"
        "QComboBox::drop-down { border: none; width: 24px; }"
        "QComboBox::down-arrow { "
        "  image: none; "
        "  border-left: 4px solid transparent; "
        "  border-right: 4px solid transparent; "
        "  border-top: 5px solid #1a3a5c; "
        "}"
        "QComboBox QAbstractItemView { "
        "  background-color: #FFFFFF; "
        "  border: 1px solid #9DB9D2; "
        "  selection-background-color: #B8D4E8; "
        "  selection-color: #000000; "
        "}"
    );
}