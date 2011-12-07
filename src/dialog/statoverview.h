#ifndef STATOVERVIEW_H
#define STATOVERVIEW_H

class General;

#include <QDialog>
#include <QTableWidgetItem>
#include <QButtonGroup>
#include <QVBoxLayout>

namespace Ui {
    class StatOverview;
}

class StatOverview : public QDialog {
    Q_OBJECT
public:
    StatOverview(QWidget *parent = 0);
    ~StatOverview();
    void loadAllStat();

private:
    Ui::StatOverview *ui;
    
    QTableWidgetItem *fillTableItem(const QString &item_name);

};

#endif // STATOVERVIEW_H
