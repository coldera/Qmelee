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

class LongjuanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LongjuanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class BodongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BodongCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JifengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JifengCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ShenglongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShenglongCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class BailieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BailieCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JiqiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiqiCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class QigongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QigongCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DianjiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DianjiCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class HuoyanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuoyanCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ChuansongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChuansongCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XuanfengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XuanfengCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DazhuangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DazhuangCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class JiaodaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiaodaoCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ChongbaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChongbaiCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class MofangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MofangCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YingluoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YingluoCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class XinlingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XinlingCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JiliuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiliuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SangliuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SangliuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZengnuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZengnuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HuqieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuqieCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // MELEESF_H
