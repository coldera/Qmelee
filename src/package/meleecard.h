#ifndef MELEECARD_H
#define MELEECARD_H

#include "standard.h"

class Bang: public Slash{
    Q_OBJECT

public:
    Q_INVOKABLE Bang(Card::Suit suit, int number);
    DamageStruct::Nature getNature() const;

protected:
    DamageStruct::Nature nature;
};


class FireBang: public NatureSlash{
    Q_OBJECT

public:
    Q_INVOKABLE FireBang(Card::Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;

    static TriggerSkill *Skill;
};


class ThunderBang: public NatureSlash{
    Q_OBJECT

public:
    Q_INVOKABLE ThunderBang(Card::Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    
    static TriggerSkill *Skill;
};


class AirBang: public NatureSlash{
    Q_OBJECT

public:
    Q_INVOKABLE AirBang(Card::Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    
    static TriggerSkill *Skill;
};


class PoisonBang: public NatureSlash{
    Q_OBJECT

public:
    Q_INVOKABLE PoisonBang(Card::Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    
    static TriggerSkill *Skill;
};


class IceBang: public NatureSlash{
    Q_OBJECT

public:
    Q_INVOKABLE IceBang(Card::Suit suit, int number);
    // virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    
    // static TriggerSkill *Skill;
};


class Dodge: public Jink{
    Q_OBJECT

public:
    Q_INVOKABLE Dodge(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual bool isAvailable(const Player *player) const;
};


class HolyWater: public Peach{
    Q_OBJECT

public:
    Q_INVOKABLE HolyWater(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;
};


class Schnapps: public Analeptic{
    Q_OBJECT

public:
    Q_INVOKABLE Schnapps(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;

    static bool IsAvailable(const Player *player);

    virtual bool isAvailable(const Player *player) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class AlienInvasion:public AOE{
    Q_OBJECT

public:
    Q_INVOKABLE AlienInvasion(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class  ThousandsArrowsShot:public AOE{
    Q_OBJECT

public:
    Q_INVOKABLE ThousandsArrowsShot(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class  Roar:public AOE{
    Q_OBJECT

public:
    Q_INVOKABLE Roar(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class WoodElf:public GlobalEffect{
    Q_OBJECT

public:
    Q_INVOKABLE WoodElf(Card::Suit suit, int number);
    virtual bool isCancelable(const CardEffectStruct &effect) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class ReadyToGo:public GlobalEffect{
    Q_OBJECT

public:
    Q_INVOKABLE ReadyToGo(Card::Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class PoisonMarish:public GlobalEffect{
    Q_OBJECT

public:
    Q_INVOKABLE PoisonMarish(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class NothingIsSomething: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE NothingIsSomething(Card::Suit suit, int number);
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class DestroyAll: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE DestroyAll(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class BorrowWeapon:public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE BorrowWeapon(Card::Suit suit, int number);
    virtual bool isAvailable(const Player *player) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};


class Grab:public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Grab(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class Cure:public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Cure(Card::Suit suit, int number);
    
    virtual bool isAvailable(const Player *player) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class Unassailable:public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Unassailable(Card::Suit suit, int number);

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool isAvailable(const Player *player) const;
};


class Burn: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Burn(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class PK:public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE PK(Card::Suit suit, int number);
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class SoulChain: public TrickCard{
    Q_OBJECT

public:
    Q_INVOKABLE SoulChain(Card::Suit suit, int number);

    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class EnegyDrain: public DelayedTrick{
    Q_OBJECT

public:
    Q_INVOKABLE EnegyDrain(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void takeEffect(ServerPlayer *target) const;
};


class SoulAwe:public DelayedTrick{
    Q_OBJECT

public:
    Q_INVOKABLE SoulAwe(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void takeEffect(ServerPlayer *target) const;
};


class Icebound:public DelayedTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Icebound(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void takeEffect(ServerPlayer *target) const;
};


class Thunder: public Disaster{
    Q_OBJECT

public:
    Q_INVOKABLE Thunder(Card::Suit suit, int number);
    virtual void takeEffect(ServerPlayer *target) const;
};


class LiyaoSword:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE LiyaoSword(Card::Suit suit, int number);
};


class GreenSteelSword:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE GreenSteelSword(Card::Suit suit, int number);
};


class SoulCane:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE SoulCane(Card::Suit suit, int number);
};


class SnakeSpear:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE SnakeSpear(Card::Suit suit, int number);
};


class PanguAxe:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE PanguAxe(Card::Suit suit, int number);
};


class WushuangHalberd:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE WushuangHalberd(Card::Suit suit, int number);
};


class DeicideBow:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE DeicideBow(Card::Suit suit, int number);
};


class GhostFan: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE GhostFan(Card::Suit suit, int number);
};


class GudingMachete: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE GudingMachete(Card::Suit suit, int number);
};


class WhiteDragon: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE WhiteDragon(Card::Suit suit, int number);
};


class RedDragon: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE RedDragon(Card::Suit suit, int number);
};


class EvilDirk: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE EvilDirk(Card::Suit suit, int number);
};


class TigerSpear: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE TigerSpear(Card::Suit suit, int number);
};


class PeacockTear: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE PeacockTear(Card::Suit suit, int number);
};


class ChaosMirror:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE ChaosMirror(Card::Suit suit, int number);
};


class KingShield: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE KingShield(Card::Suit suit, int number);
};


class VineArmor: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE VineArmor(Card::Suit suit, int number);
};


class HolyWing: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE HolyWing(Card::Suit suit, int number);
    virtual void onInstall(ServerPlayer *player) const;
};


class BloodGuard: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE BloodGuard(Card::Suit suit, int number);

    virtual void onUninstall(ServerPlayer *player) const;
};


class QinglinHorse: public DefensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE QinglinHorse(Card::Suit suit, int number, int correct);
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

};


class Rhinoceros: public OffensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Rhinoceros(Card::Suit suit, int number, int correct);
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

};


class Leopard: public DefensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Leopard(Card::Suit suit, int number, int correct);
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

};


class Fox: public DefensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Fox(Card::Suit suit, int number, int correct);
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

};


class XunleiBeast: public OffensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE XunleiBeast(Card::Suit suit, int number, int correct);
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

};


class Boar: public OffensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Boar(Card::Suit suit, int number, int correct);
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

};


class Tiger: public DefensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Tiger(Card::Suit suit, int number, int correct);
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

};


class FloodDragon: public OffensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE FloodDragon(Card::Suit suit, int number, int correct);
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

};


class Deer: public DefensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Deer(Card::Suit suit, int number, int correct);
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

};


class SpiderQueen: public OffensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE SpiderQueen(Card::Suit suit, int number, int correct);
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

};

class SpiderQueenSkillCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SpiderQueenSkillCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};


class Cattle: public DefensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Cattle(Card::Suit suit, int number, int correct);
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

};



class Relic:public EquipCard{
    Q_OBJECT

public:
    Relic(Suit suit, int number);

    virtual QString getSubtype() const;

    virtual Location location() const;
    virtual QString label() const;

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;

protected:
    bool attach_skill;
};


class NvwaStone:public Relic{
    Q_OBJECT

public:
    Q_INVOKABLE NvwaStone(Card::Suit suit, int number);
};


class Blunderbuss:public Relic{
    Q_OBJECT

public:
    Q_INVOKABLE Blunderbuss(Card::Suit suit, int number);
};

class BlunderbussSkillCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BlunderbussSkillCard();
    
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class SoulSpirit:public Relic{
    Q_OBJECT

public:
    Q_INVOKABLE SoulSpirit(Card::Suit suit, int number);    
};


class Miracle:public Relic{
    Q_OBJECT

public:
    Q_INVOKABLE Miracle(Card::Suit suit, int number);
};


class Shtm:public Relic{
    Q_OBJECT

public:
    Q_INVOKABLE Shtm(Card::Suit suit, int number);
    
    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;
    
    virtual bool willThrow() const;
    virtual bool isAvailable(const Player *player) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const;
};

class CheatCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CheatCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class MeleeCardPackage: public Package{
    Q_OBJECT

public:
    MeleeCardPackage();
};

#endif // MELEECARD_H
