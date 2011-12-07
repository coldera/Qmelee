#ifndef STAT_H
#define STAT_H

#include "gamerule.h"

class StatRule : public GameRule {
public:
    StatRule(QObject *parent);
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;
};

#endif // STAT_H
