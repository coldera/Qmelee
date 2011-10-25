#ifndef ROOMTHREAD_H
#define ROOMTHREAD_H

#include <QThread>
#include <QSemaphore>
#include <QVariant>

#include <csetjmp>

#include "structs.h"

struct LogMessage{
    LogMessage();
    QString toString() const;

    QString type;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
    QString card_str;
    QString arg;
    QString arg2;
};

class RoomThread : public QThread{
    Q_OBJECT

public:
    explicit RoomThread(Room *room);
    void constructTriggerTable(const GameRule *rule);
    bool trigger(TriggerEvent event, ServerPlayer *target, QVariant &data);
    bool trigger(TriggerEvent event, ServerPlayer *target);

    void addPlayerSkills(ServerPlayer *player, bool invoke_game_start = false);

    void addTriggerSkill(const TriggerSkill *skill);
    void delay(unsigned long msecs = 1000);
    void end();
    void run3v3();
    void action3v3(ServerPlayer *player);

protected:
    virtual void run();

private:
    Room *room;
    jmp_buf env;
    QString order;

    QList<const TriggerSkill *> skill_table[NumOfEvents];
    QSet<const TriggerSkill *> skillSet;
};

#endif // ROOMTHREAD_H
