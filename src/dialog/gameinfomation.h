#ifndef GAMEINFOMATION_H
#define GAMEINFOMATION_H

#include <QDialog>

class QTextEdit;

class GameInfomation: public QDialog{
    Q_OBJECT

public:
    GameInfomation(QWidget *parent);

private:
    QTextEdit *content_box;

};

#endif // GAMEINFOMATION_H
