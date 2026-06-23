#pragma once
#ifndef CARDWIDGET_H
#define CARDWIDGET_H

#include <QFrame>

class CardWidget : public QFrame
{
    Q_OBJECT
public:
    explicit CardWidget(QWidget* parent = nullptr);
};

#endif