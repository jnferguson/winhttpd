#ifndef USERTAB2_H
#define USERTAB2_H

#include <QWidget>
#include "ui_usertab2.h"

%NAMESPACE_BEGIN%class userTab2_t : public QWidget
{
    Q_OBJECT

public:
    userTab2_t(QWidget *parent = 0);
    ~userTab2_t();

private:
    Ui::userTab2_t ui;
};

%NAMESPACE_END%#endif // USERTAB2_H
