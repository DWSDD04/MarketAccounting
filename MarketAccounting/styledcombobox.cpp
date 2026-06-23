#include "styledcombobox.h"

StyledComboBox::StyledComboBox(QWidget* parent) : QComboBox(parent) {
    setStyleSheet(
        "QComboBox {"
        "  background-color: #ffffff;"
        "  border: 2px solid #e9ecef;"
        "  border-radius: 8px;"
        "  padding: 10px 14px;"
        "  font-size: 13px;"
        "  color: #212529;"
        "  min-height: 38px;"
        "}"
        "QComboBox:focus { border: 2px solid #16213e; }"
        "QComboBox::drop-down { border: none; width: 30px; }"
        "QComboBox::down-arrow { image: none; border-left: 5px solid transparent; "
        "border-right: 5px solid transparent; border-top: 6px solid #16213e; }"
        "QComboBox QAbstractItemView {"
        "  background-color: #ffffff;"
        "  border: 1px solid #e9ecef;"
        "  selection-background-color: #16213e;"
        "  selection-color: white;"
        "}"
    );
}