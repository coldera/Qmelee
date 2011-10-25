#ifndef MELEESF_H
#define MELEESF_H

#include "package.h"
#include "card.h"
#include "meleecard.h"

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>

class MeleeSFPackage: public Package{
    Q_OBJECT

public:
    MeleeSFPackage();
};

class ShunyuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShunyuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // MELEESF_H
