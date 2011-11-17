#ifndef MELEESS_H
#define MELEESS_H

#include "package.h"
#include "card.h"
#include "meleecard.h"

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>

class MeleeSSPackage: public Package{
    Q_OBJECT

public:
    MeleeSSPackage();
};


class TianbaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TianbaCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GoutongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GoutongCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    // virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class YingxuanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YingxuanCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LiulianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LiulianCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QuwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QuwuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YanwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YanwuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YinghuaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YinghuaCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SudiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SudiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SiyueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SiyueCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JiefangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiefangCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YanmieBang: public FireBang{
    Q_OBJECT

public:
    Q_INVOKABLE YanmieBang(Card::Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};


class BaoshaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BaoshaCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class RenquanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE RenquanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DianguangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DianguangCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BingrenBang: public IceBang{
    Q_OBJECT

public:
    Q_INVOKABLE BingrenBang(Card::Suit suit, int number);
};

class BingrenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BingrenCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ChuixueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChuixueCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ChuixueBang: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChuixueBang();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class KongchanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE KongchanCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ChenyinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChenyinCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ShuangyueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShuangyueCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ErdaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ErdaoCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DaoquCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DaoquCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class WuyuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WuyuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class MeiyuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MeiyuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BaoyuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BaoyuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ChaoxiuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChaoxiuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DuchuiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuchuiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // MELEESS_H
