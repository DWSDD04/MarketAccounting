#pragma once
#ifndef STYLEDLINEEDIT_H
#define STYLEDLINEEDIT_H

#include <QLineEdit>

class StyledLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit StyledLineEdit(QWidget* parent = nullptr);
};

#endif