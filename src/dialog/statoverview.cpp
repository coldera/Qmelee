#include "statoverview.h"
#include "ui_statoverview.h"
#include "engine.h"

#include <QMessageBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QCommandLinkButton>
#include <QClipboard>
#include <QFile>

StatOverview::StatOverview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StatOverview)
{
    ui->setupUi(this);
}

void StatOverview::loadAllStat(){

    QFile file("etc/total_stat.txt");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        
        QHash<QString, QHash<QString, int> > total_stat;
        QHash<QString, int> stat;
        QStringList items = Sanguosha->getStatItems();
        int i = 0;
        
        while(!stream.atEnd()){
            QString general_name;
            stream >> general_name;
            
            stat.empty();
            
            foreach(QString item, items) {
                int value;
                stream >> value;
                
                stat.insert(item, value);
            }
            
            total_stat.insert(general_name, stat);
            
            i++;
        }
        
        file.close();
        
        if(total_stat.isEmpty())
            return;
        
        static QStringList table_items;
        if(table_items.isEmpty())
            table_items  << "appearance" << "offensive" << "resistance" << "assist" << "obstruct" << "kill" << "death" << "win" << "lose"
                               << "slash" << "miss" << "cure" << "holywater";
        
        QHashIterator<QString, QHash<QString, int> > itor(total_stat);
        QString name;
        QHash<QString, int> hash;
        
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(i-1);
        
        int j = 0;
        while (itor.hasNext()) {
            itor.next();
            name = Sanguosha->translate(itor.key());
            hash = itor.value();
            
            if(hash["appearance"]) {
                QTableWidgetItem *name_item = new QTableWidgetItem(name);
                name_item->setTextAlignment(Qt::AlignCenter);
                name_item->setData(Qt::UserRole, name);
                
                ui->tableWidget->setItem(j, 0, name_item);
                
                int k = 1;
                foreach(const QString &item_name, table_items) {
                    QString value = QString::number(hash[item_name]);
                    
                    QTableWidgetItem *item = fillTableItem(value);
                    ui->tableWidget->setItem(j, k++, item);
                }
                
                QString total = QString::number(
                    (hash["offensive"] + hash["resistance"] +
                    (hash["assist"] + hash["obstruct"]) * 2 +
                    (hash["kill"] - hash["death"]) * 10 +
                    (hash["win"] - hash["lose"]) * 10) / hash["appearance"]
                );
                QTableWidgetItem *total_item = new QTableWidgetItem(total);
                total_item->setTextAlignment(Qt::AlignCenter);
                ui->tableWidget->setItem(j, k, total_item);
                
                j++;
            }
        }
        
        ui->tableWidget->setCurrentItem(ui->tableWidget->item(0,0));
        
       
        
        return;
    }
    
    file.close();
}

QTableWidgetItem *StatOverview::fillTableItem(const QString &value) {
    QTableWidgetItem *item = new QTableWidgetItem(value);
    item->setTextAlignment(Qt::AlignCenter);
    
    return item;
}

StatOverview::~StatOverview()
{
    delete ui;
}
