#ifndef MELEEDS_H
#define MELEEDS_H

#include "package.h"
#include "card.h"
#include "meleecard.h"

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>

class MeleeDSPackage: public Package{
    Q_OBJECT

public:
    MeleeDSPackage();
};

class BingzhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BingzhuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZuanjiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZuanjiCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class RongjieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE RongjieCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShenyuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShenyuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YaogunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YaogunCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LieqiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LieqiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class AnqiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE AnqiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class FanxiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FanxiangCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LingfuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LingfuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LeishenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LeishenCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class XueshenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XueshenCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class AnitaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE AnitaCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class AnitaAnweiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE AnitaAnweiCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JiushuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiushuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class QiyueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QiyueCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhipeiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhipeiCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ZhipeiGiveCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhipeiGiveCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GuanwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuanwuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZhicaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhicaiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YoubuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YoubuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GuiyanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuiyanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YuanhunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuanhunCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LeyuanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LeyuanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // MELEEDS_H
