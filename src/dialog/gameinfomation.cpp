#include "gameinfomation.h"

#include <QListWidget>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>

GameInfomation::GameInfomation(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Game infomation"));
    resize(800, 600);

    content_box = new QTextEdit;
    content_box->setReadOnly(true);

    QHBoxLayout *layout = new QHBoxLayout;

    layout->addWidget(content_box);

    setLayout(layout);
    
    QString filename = "game_infomation.html";
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        QString content = stream.readAll();

        content_box->setHtml(content);
    }
}

