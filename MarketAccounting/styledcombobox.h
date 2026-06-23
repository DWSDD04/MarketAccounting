#pragma once
#ifndef STYLEDCOMBOBOX_H
#define STYLEDCOMBOBOX_H

#include <QComboBox>

class StyledComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit StyledComboBox(QWidget* parent = nullptr);
};

#endif