#include "meleecard.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"

//----------------------------------------------------------Bang

Bang::Bang(Suit suit, int number): Slash(suit, number)
{
    setObjectName("bang");
    nature = DamageStruct::Normal;
}

//----------------------------------------------------------FireBang

class FireBangSkill: public TriggerSkill{
public:
    FireBangSkill():TriggerSkill("fire_bang"){
        frequency = Compulsory;
        events << Damage << CardFinished;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        
        if(event == Damage) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(damage.nature == DamageStruct::Fire && damage.to && !damage.to->getEquip(1) && damage.to->getCardCount(false)) {
                
                room->getThread()->delay();
                
                int card_id = room->askForCardChosen(damage.from, damage.to, "h", objectName());                
                
                LogMessage log;
                log.type = "$FireBangEffect";
                log.to << damage.to;
                log.card_str = QString::number(card_id);
                room->sendLog(log);
                
                room->throwCard(card_id);
            }
        }
        
        room->detachSkillFromPlayer(player, objectName());
        
        return false;
    }
};

FireBang::FireBang(Suit suit, int number)
    :NatureSlash(suit, number, DamageStruct::Fire)
{
    setObjectName("fire_bang");
    nature = DamageStruct::Fire;
}

TriggerSkill *FireBang::Skill = new FireBangSkill;

void FireBang::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &targets) const{
    if(player->getPhase() == Player::Play) {
        room->getThread()->addTriggerSkill(this->Skill);        
        room->attachSkillToPlayer(player, objectName());
    }
    NatureSlash::use(room, player, targets);
}

//----------------------------------------------------------ThunderBang

class ThunderBangSkill: public TriggerSkill{
public:
    ThunderBangSkill():TriggerSkill("thunder_bang"){
        frequency = Compulsory;
        events << Damage << CardFinished;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        
        if(event == Damage) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(damage.nature == DamageStruct::Thunder && damage.to && damage.to->isAlive()) {
                const EquipCard *card = damage.to->getEquip(0);
                
                if(card) {
                    
                    LogMessage log;
                    log.type = "#ThunderBangJudge";
                    log.to << damage.to;
                    log.arg = "spade";
                    room->sendLog(log);
                    
                    JudgeStruct judge;
                    judge.pattern = QRegExp("(.*):(spade):(.*)");
                    judge.good = false;
                    judge.reason = objectName();
                    judge.who = damage.to;

                    room->judge(judge);

                    if(judge.isBad()){
                        // room->setEmotion(damage.to, "bad");
                        room->throwCard(card);
                        
                        LogMessage log;
                        log.type = "$ThunderBangEffect";
                        log.to << damage.to;
                        log.card_str = card->toString();
                        room->sendLog(log);
                    }                 
                }            
            }
        }
        
        room->detachSkillFromPlayer(player, objectName());
        //room->getThread()->removeTriggerSkill(this);
        
        return false;
    }
};

ThunderBang::ThunderBang(Suit suit, int number)
    :NatureSlash(suit, number, DamageStruct::Thunder)
{
    setObjectName("thunder_bang");
    nature = DamageStruct::Thunder;
}

TriggerSkill *ThunderBang::Skill = new ThunderBangSkill;

void ThunderBang::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &targets) const{
    if(player->getPhase() == Player::Play) {    
        room->getThread()->addTriggerSkill(this->Skill);        
        room->attachSkillToPlayer(player, objectName());
    }
    NatureSlash::use(room, player, targets);
}

//----------------------------------------------------------AirBang

class AirBangSkill: public TriggerSkill{
public:
    AirBangSkill():TriggerSkill("air_bang"){
        frequency = Compulsory;
        events << Damage << CardFinished;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        
        if(event == Damage) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(damage.nature == DamageStruct::Air && damage.to && damage.to->isAlive()) {
                const EquipCard *card = damage.to->getEquip(1);
                
                if(card) {
                    Room *room = damage.to->getRoom();

                    LogMessage log;
                    log.type = "#AirBangChoice";
                    log.to << damage.to;
                    room->sendLog(log);
                
                    static QStringList choices;
                    if(choices.isEmpty()) {
                        choices << "AirEffectHp" << "AirEffectArmor"; 
                    }                    
                                        
                    QString choice = room->askForChoice(damage.to, objectName(), choices.join("+"));
                    
                    if(choice =="AirEffectHp") {
                        room->loseHp(damage.to,1);
                    }else if(choice == "AirEffectArmor") {
                        room->throwCard(card);
                        
                        LogMessage log;
                        log.type = "$AirBangEffect";
                        log.to << damage.to;
                        log.card_str = card->toString();
                        room->sendLog(log);
                    }
                }                
            }
        }
        
        player->getRoom()->detachSkillFromPlayer(player, objectName());
        
        return false;
    }
};

AirBang::AirBang(Suit suit, int number)
    :NatureSlash(suit, number, DamageStruct::Air)
{
    setObjectName("air_bang");
    nature = DamageStruct::Air;
}

TriggerSkill *AirBang::Skill = new AirBangSkill;

void AirBang::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &targets) const{
    if(player->getPhase() == Player::Play) {    
        room->getThread()->addTriggerSkill(this->Skill);        
        room->attachSkillToPlayer(player, objectName());
    }
    NatureSlash::use(room, player, targets);
}

//----------------------------------------------------------PoisonBang

class PoisonBangSkill: public TriggerSkill{
public:
    PoisonBangSkill():TriggerSkill("poison_bang"){
        frequency = Compulsory;
        events << Damage << CardFinished;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == Damage) {
        
            DamageStruct damage = data.value<DamageStruct>();
            
            if(damage.nature == DamageStruct::Poison && damage.to && damage.to->isAlive()) {
                Room *room = damage.to->getRoom();
                
                LogMessage log;
                log.type = "#PoisonBangJudge";
                log.to << damage.to;
                log.arg = "club";
                room->sendLog(log);                

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(club):(.*)");
                judge.good = false;
                judge.reason = objectName();
                judge.who = damage.to;

                room->judge(judge);

                if(judge.isBad()){
                    
                    LogMessage log;
                    log.type = "$PoisonBangEffect";
                    log.to << damage.to;
                    room->sendLog(log);  
                    
                    room->loseHp(damage.to,1);
                    // room->setEmotion(damage.to, "bad");
                }
            }

        }

        player->getRoom()->detachSkillFromPlayer(player, objectName());

        return false;
    }
};

PoisonBang::PoisonBang(Suit suit, int number)
    :NatureSlash(suit, number, DamageStruct::Poison)
{
    setObjectName("poison_bang");
    nature = DamageStruct::Poison;
}

TriggerSkill *PoisonBang::Skill = new PoisonBangSkill;

void PoisonBang::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &targets) const{
    if(player->getPhase() == Player::Play) {    
        room->getThread()->addTriggerSkill(this->Skill);        
        room->attachSkillToPlayer(player, objectName());
    }
    NatureSlash::use(room, player, targets);
}

//----------------------------------------------------------IceBang

IceBang::IceBang(Suit suit, int number)
    :NatureSlash(suit, number, DamageStruct::Ice)
{
    setObjectName("ice_bang");
    nature = DamageStruct::Ice;
}

//----------------------------------------------------------Dodge

Dodge::Dodge(Suit suit, int number):Jink(suit, number){
    setObjectName("dodge");

    target_fixed = true;
}

QString Dodge::getSubtype() const{
    return "defense_card";
}

bool Dodge::isAvailable(const Player *) const{
    return false;
}

//----------------------------------------------------------HolyWater

HolyWater::HolyWater(Suit suit, int number):Peach(suit, number){
    setObjectName("holy_water");
    target_fixed = true;
}

QString HolyWater::getSubtype() const{
    return "recover_card";
}

QString HolyWater::getEffectPath(bool is_male) const{
    return Card::getEffectPath();
}

void HolyWater::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    if(targets.isEmpty())
        room->cardEffect(this, source, source);
    else
        room->cardEffect(this, source, targets.first());
}

void HolyWater::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    // do animation
    room->broadcastInvoke("animate", QString("peach:%1:%2")
                          .arg(effect.from->objectName())
                          .arg(effect.to->objectName()));

    // recover hp
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.to, recover);
}

bool HolyWater::isAvailable(const Player *player) const{
    return player->isWounded();
}

//----------------------------------------------------------Schnapps

Schnapps::Schnapps(Card::Suit suit, int number)
    :Analeptic(suit, number)
{
    setObjectName("schnapps");
    target_fixed = true;
    once = true;
}

QString Schnapps::getSubtype() const{
    return "buff_card";
}

QString Schnapps::getEffectPath(bool ) const{
    return Card::getEffectPath();
}

bool Schnapps::IsAvailable(const Player *player){
    return ! player->hasUsed("Schnapps");
}

bool Schnapps::isAvailable(const Player *player) const{
    return IsAvailable(player);
}

void Schnapps::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    room->cardEffect(this, source, source);
}

void Schnapps::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    // do animation
    QString who = effect.to->objectName();
    QString animation_str = QString("analeptic:%1:%2").arg(who).arg(who);
    room->broadcastInvoke("animate", animation_str);

    if(effect.to->hasFlag("dying")){
        // recover hp
        RecoverStruct recover;
        recover.card = this;
        recover.who = effect.from;
        room->recover(effect.to, recover);
    }else{
        LogMessage log;
        log.type = "#Drank";
        log.from = effect.from;
        room->sendLog(log);

        room->setPlayerFlag(effect.to, "drank");
    }
}

//----------------------------------------------------------AlienInvasion

AlienInvasion::AlienInvasion(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("alien_invasion");
}

void AlienInvasion::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *slash = room->askForCard(effect.to, "slash", "alien-invasion-slash:" + effect.from->objectName());
    if(slash)
        room->broadcastInvoke("animate", "bang:"+effect.to->objectName());
    else{
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.to = effect.to;
        damage.nature = DamageStruct::Normal;

        damage.from = effect.from;
        room->damage(damage);
    }
}

//----------------------------------------------------------ThousandsArrowsShot

ThousandsArrowsShot::ThousandsArrowsShot(Card::Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("thousands_arrows_shot");
}

void ThousandsArrowsShot::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *jink = room->askForCard(effect.to, "jink", "thousands-arrows-shot-jink:" + effect.from->objectName());
    if(jink)
        room->broadcastInvoke("animate", "dodge:"+effect.to->objectName());
    else{
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.from = effect.from;
        damage.to = effect.to;
        damage.nature = DamageStruct::Normal;

        room->damage(damage);
    }
}

//----------------------------------------------------------Roar

Roar::Roar(Card::Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("roar");
}

void Roar::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    
    if(!effect.to->hasEquip()) {
        return;
    }
    
    if(effect.to->hasSkill("flood_dragon")) {   
        LogMessage log;
        log.type = "$FloodDragonEffect";
        log.from = effect.to;
        log.card_str = effect.card->toString();
        room->sendLog(log);
        return;
    }
    
    static QStringList choices1;
    if(choices1.isEmpty()) {
        choices1 << "RoarEffect1" << "RoarEffect2"; 
    }
    
    QString choice1 = room->askForChoice(effect.to, objectName(), choices1.join("+"));
    
    if(choice1== "RoarEffect1") {

        int card_id = room->askForCardChosen(effect.to, effect.to, "e", objectName());
        room->throwCard(card_id);
        
    }else if(choice1 == "RoarEffect2") {
        
        DamageStruct damageMaker;
        damageMaker.card = effect.card;
        damageMaker.from = effect.from;
        damageMaker.to = effect.to;
        damageMaker.damage = 1;
        damageMaker.nature = DamageStruct::Air;
        room->damage(damageMaker);
        
    }
}

//----------------------------------------------------------WoodElf

WoodElf::WoodElf(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("wood_elf");
}

bool WoodElf::isCancelable(const CardEffectStruct &effect) const{
    return effect.to->isWounded();
}

void WoodElf::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    room->recover(effect.to, recover);
}

//----------------------------------------------------------ReadyToGo

ReadyToGo::ReadyToGo(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("ready_to_go");
}

void ReadyToGo::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);

    QList<ServerPlayer *> players = room->getAllPlayers();
    QList<int> card_ids = room->getNCards(players.length());
    room->fillAG(card_ids);

    QVariantList ag_list;
    foreach(int card_id, card_ids)
        ag_list << card_id;
    room->setTag("ReadyToGo", ag_list);

    GlobalEffect::use(room, source, players);

    ag_list = room->getTag("ReadyToGo").toList();

    // throw the rest cards
    foreach(QVariant card_id, ag_list){
        room->takeAG(NULL, card_id.toInt());
    }

    room->broadcastInvoke("clearAG");
}

void ReadyToGo::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QVariantList ag_list = room->getTag("ReadyToGo").toList();
    QList<int> card_ids;
    foreach(QVariant card_id, ag_list)
        card_ids << card_id.toInt();

    int card_id = room->askForAG(effect.to, card_ids, false, objectName());
    card_ids.removeOne(card_id);

    room->takeAG(effect.to, card_id);
    ag_list.removeOne(card_id);

    room->setTag("ReadyToGo", ag_list);
}

//----------------------------------------------------------PoisonMarish

PoisonMarish::PoisonMarish(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("poison_marish");
}

void PoisonMarish::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    
    if(effect.to->hasSkill("flood_dragon")) {
        LogMessage log;
        log.type = "$FloodDragonEffect";
        log.from = effect.to;
        log.card_str = effect.card->toString();
        room->sendLog(log);
        return;
    }

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(club):(.*)");
    judge.good = false;
    judge.reason = objectName();
    judge.who = effect.to;

    room->judge(judge);

    if(judge.isBad()){
        
        DamageStruct damageMaker;
        damageMaker.to = effect.to;
        damageMaker.damage = 1;
        damageMaker.nature = DamageStruct::Poison;
        room->damage(damageMaker);
        
        // room->setEmotion(effect.to, "bad");
        
    }
}

//----------------------------------------------------------NothingIsSomething

NothingIsSomething::NothingIsSomething(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("nothing_is_something");
    target_fixed = true;
}

void NothingIsSomething::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);

    CardEffectStruct effect;
    effect.from = effect.to = source;
    effect.card = this;

    room->cardEffect(effect);
}

void NothingIsSomething::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(2);
}

//----------------------------------------------------------DestroyAll

DestroyAll::DestroyAll(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("destroy_all");
}

bool DestroyAll::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->isAllNude())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void DestroyAll::onEffect(const CardEffectStruct &effect) const{
    if(effect.to->isAllNude())
        return;

    Room *room = effect.to->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "hej", objectName());
    room->throwCard(card_id);

    LogMessage log;
    log.type = "$destroy_all";
    log.from = effect.from;
    log.card_str = QString::number(card_id);
    room->sendLog(log);
}

//----------------------------------------------------------BorrowWeapon

BorrowWeapon::BorrowWeapon(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("borrow_weapon");
}

bool BorrowWeapon::isAvailable(const Player *player) const{
    QList<const Player*> players = player->parent()->findChildren<const Player *>();
    foreach(const Player *p, players){
        if(p->getWeapon() != NULL && p != player)
            return true;
    }

    return false;
}

bool BorrowWeapon::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

bool BorrowWeapon::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty()){
        return to_select->getWeapon() && to_select != Self;
    }else if(targets.length() == 1){
        const Player *first = targets.first();
        return first != Self && first->getWeapon() && first->canSlash(to_select);
    }else
        return false;
}

void BorrowWeapon::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *killer = targets.at(0);
    QList<ServerPlayer *> victims = targets;
    if(victims.length() > 1)
        victims.removeAt(0);
    const Weapon *weapon = killer->getWeapon();

    if(weapon == NULL)
        return;

    bool on_effect = room->cardEffect(this, source, killer);
    if(on_effect){
        QString prompt = QString("borrow-weapon-slash:%1:%2")
                         .arg(source->objectName()).arg(victims.first()->objectName());
        const Card *slash = room->askForCard(killer, "slash", prompt);
        if(slash){
            CardUseStruct use;
            use.card = slash;
            use.from = killer;
            use.to = victims;
            room->useCard(use);
        }else{
            source->obtainCard(weapon);
        }
    }
}

//----------------------------------------------------------Grab

Grab::Grab(Suit suit, int number):SingleTargetTrick(suit, number, true) {
    setObjectName("grab");
}

bool Grab::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->isAllNude())
        return false;

    if(to_select == Self)
        return false;

    if(Self->distanceTo(to_select) > 1)
        return false;

    return true;
}

void Grab::onEffect(const CardEffectStruct &effect) const{
    if(effect.to->isAllNude())
        return;

    Room *room = effect.to->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "hej", objectName());

    if(room->getCardPlace(card_id) == Player::Hand)
        room->moveCardTo(Sanguosha->getCard(card_id), effect.from, Player::Hand, false);
    else
        room->obtainCard(effect.from, card_id);
}

//----------------------------------------------------------Cure

Cure::Cure(Suit suit, int number):SingleTargetTrick(suit, number, true) {
    setObjectName("cure");
}

bool Cure::isAvailable(const Player *player) const{
    QList<const Player*> players = player->parent()->findChildren<const Player *>();
    foreach(const Player *p, players){
        if(player->distanceTo(p) <= 1 && p->isWounded())
            return true;
    }

    return false;
}

bool Cure::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(Self->distanceTo(to_select) > 1)
        return false;
    
    if(!to_select->isWounded())
        return false;
        
    return true;
}

void Cure::onEffect(const CardEffectStruct &effect) const{
    RecoverStruct recover;
    recover.card = this;
    recover.recover = 2;
    recover.who = effect.from;
    effect.to->getRoom()->recover(effect.to, recover);
}

//----------------------------------------------------------Unassailable

Unassailable::Unassailable(Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("unassailable");
}

void Unassailable::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
}

bool Unassailable::isAvailable(const Player *) const{
    return false;
}

//----------------------------------------------------------Burn

Burn::Burn(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number, true)
{
    setObjectName("burn");
}

bool Burn::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->isKongcheng())
        return false;

    if(to_select == Self)
        return Self->getHandcardNum() >= 2;
    else
        return true;
}

void Burn::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(effect.to->isKongcheng())
        return;

    const Card *card = room->askForCardShow(effect.to, effect.from, objectName());
    room->showCard(effect.to, card->getEffectiveId());

    QString suit_str = card->getSuitString();
    QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
    QString prompt = QString("@burn-card:%1::%2").arg(effect.to->getGeneralName()).arg(suit_str);
    if(room->askForCard(effect.from, pattern, prompt)){
        DamageStruct damage;
        damage.card = this;
        damage.from = effect.from;
        damage.to = effect.to;
        damage.nature = DamageStruct::Fire;

        room->damage(damage);
    }

    if(card->isVirtualCard())
        delete card;
}

//----------------------------------------------------------PK

PK::PK(Suit suit, int number)
    :SingleTargetTrick(suit, number, true)
{
    setObjectName("pk");
}

bool PK::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;

    return targets.isEmpty();
}

void PK::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *first = effect.to;
    ServerPlayer *second = effect.from;
    Room *room = first->getRoom();

    room->setEmotion(first, "duel-a");
    room->setEmotion(second, "duel-b");

    forever{
        const Card *slash = room->askForCard(first, "slash", "duel-slash:" + second->objectName());
        if(slash == NULL)
            break;

        qSwap(first, second);
    }

    DamageStruct damage;
    damage.card = this;
    damage.from = second;
    damage.to = first;

    room->damage(damage);
}

//----------------------------------------------------------SoulChain

SoulChain::SoulChain(Card::Suit suit, int number)
    :TrickCard(suit, number, false)
{
    setObjectName("soul_chain");
}

QString SoulChain::getSubtype() const{
    return "damage_spread";
}

QString SoulChain::getEffectPath(bool is_male) const{
    return QString();
}

bool SoulChain::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;

    return true;
}

bool SoulChain::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= 2 && targets.length() > 0;
}

void SoulChain::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    room->playCardEffect("@tiesuo", source->getGeneral()->isMale());
    TrickCard::use(room, source, targets);
}

void SoulChain::onEffect(const CardEffectStruct &effect) const{
    bool chained = ! effect.to->isChained();
    effect.to->setChained(chained);

    effect.to->getRoom()->broadcastProperty(effect.to, "chained");
}

//----------------------------------------------------------EnegyDrain

EnegyDrain::EnegyDrain(Card::Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("enegy_drain");

    judge.pattern = QRegExp("(.*):(club):(.*)");
    judge.good = true;
    judge.reason = objectName();
}

bool EnegyDrain::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    int distance = Self->distanceTo(to_select);
    return distance <= 1;
}

void EnegyDrain::takeEffect(ServerPlayer *target) const{
    target->skip(Player::Draw);
}

//----------------------------------------------------------SoulAwe

SoulAwe::SoulAwe(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("soul_awe");
    target_fixed = false;

    judge.pattern = QRegExp("(.*):(heart):(.*)");
    judge.good = true;
    judge.reason = objectName();
}

bool SoulAwe::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if(!targets.isEmpty())
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    if(to_select == Self)
        return false;
        
    if(to_select->hasSkill("flood_dragon"))
        return false;

    return true;
}

void SoulAwe::takeEffect(ServerPlayer *target) const{
    target->skip(Player::Play);
}

//----------------------------------------------------------Icebound

Icebound::Icebound(Suit suit, int number)
    :DelayedTrick(suit, number, false, false)
{
    setObjectName("icebound");
    target_fixed = false;
}

bool Icebound::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    if(to_select->containsTrick(objectName()))
        return false;

    int distance = Self->distanceTo(to_select);
    return distance <= 1;
}

void Icebound::takeEffect(ServerPlayer *target) const{
    static QStringList choices;
    if(choices.isEmpty()) {
        choices << "IceboundEffect1" << "IceboundEffect2"; 
    }
    
    Room *room = target->getRoom();
    QString choice = room->askForChoice(target, objectName(), choices.join("+"));
    
    if(choice =="IceboundEffect1") {
        target->skip(Player::Play);
    }else if(choice == "IceboundEffect2") {
        DamageStruct damageMaker;
        damageMaker.to = target;
        damageMaker.damage = 1;
        damageMaker.nature = DamageStruct::Ice;
        room->damage(damageMaker);
    }
}

//----------------------------------------------------------Thunder

Thunder::Thunder(Suit suit, int number):Disaster(suit, number){
    setObjectName("thunder");

    judge.pattern = QRegExp("(.*):(spade):([2-9])");
    judge.good = false;
    judge.reason = objectName();
}

void Thunder::takeEffect(ServerPlayer *target) const{
    DamageStruct damage;
    damage.card = this;
    damage.damage = 3;
    damage.from = NULL;
    damage.to = target;
    damage.nature = DamageStruct::Thunder;

    target->getRoom()->damage(damage);
}

//----------------------------------------------------------LiyaoSword

class LiyaoSwordSkill: public WeaponSkill{
public:
    LiyaoSwordSkill():WeaponSkill("liyao_sword"){
        events << SlashEffect;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = player->getRoom();

        if(effect.from->getGeneral()->getGender() != effect.to->getGeneral()->getGender()){
            if(effect.from->askForSkillInvoke(objectName())){
                bool draw_card = false;

                if(effect.to->isKongcheng())
                    draw_card = true;
                else{
                    QString prompt = "liyao-sword-card:" + effect.from->getGeneralName();
                    const Card *card = room->askForCard(effect.to, ".", prompt);
                    if(card){
                        room->throwCard(card);
                    }else
                        draw_card = true;
                }

                if(draw_card)
                    effect.from->drawCards(1);
            }
        }

        return false;
    }
};

LiyaoSword::LiyaoSword(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("liyao_sword");
    skill = new LiyaoSwordSkill;
}

//----------------------------------------------------------GreenSteelSword

class GreenSteelSwordSkill: public WeaponSkill{
public:
    GreenSteelSwordSkill():WeaponSkill("greensteel_sword"){
        events << SlashEffect;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        effect.to->addMark("qinggang");

        return false;
    }
};

GreenSteelSword::GreenSteelSword(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("greensteel_sword");

    skill = new GreenSteelSwordSkill;
}

//----------------------------------------------------------SoulCane

class SoulCaneSkill : public WeaponSkill{
public:
    SoulCaneSkill():WeaponSkill("soul_cane"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(effect.to->hasSkill("kongcheng") && effect.to->isKongcheng())
            return false;

        Room *room = player->getRoom();
        const Card *card = room->askForCard(player, "slash", "soulcane-slash");
        if(card){
            // if player is drank, unset his flag
            if(player->hasFlag("drank"))
                room->setPlayerFlag(player, "-drank");

            CardUseStruct use;
            use.card = card;
            use.from = player;
            use.to << effect.to;
            room->useCard(use, false);
        }

        return false;
    }
};

SoulCane::SoulCane(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("soul_cane");
    skill = new SoulCaneSkill;
}

//----------------------------------------------------------SnakeSpear

class SnakeSpearSkill: public ViewAsSkill{
public:
    SnakeSpearSkill():ViewAsSkill("snake_spear"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 2 && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        const Card *first = cards.at(0)->getFilteredCard();
        const Card *second = cards.at(1)->getFilteredCard();

        Card::Suit suit = Card::NoSuit;
        if(first->isBlack() && second->isBlack())
            suit = Card::Spade;
        else if(first->isRed() && second->isRed())
            suit = Card::Heart;

        Bang *slash = new Bang(suit, 0);
        slash->setSkillName(objectName());
        slash->addSubcard(first);
        slash->addSubcard(second);

        return slash;
    }
};

SnakeSpear::SnakeSpear(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("snake_spear");
    attach_skill = true;
}

//----------------------------------------------------------PanguAxe

class PanguAxeViewAsSkill: public ViewAsSkill{
public:
    PanguAxeViewAsSkill():ViewAsSkill("pangu_axe"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@axe";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2)
            return false;

        if(to_select->getCard() == Self->getWeapon())
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        DummyCard *card = new DummyCard;
        card->setSkillName(objectName());
        card->addSubcards(cards);
        return card;
    }
};

class PanguAxeSkill: public WeaponSkill{
public:
    PanguAxeSkill():WeaponSkill("pangu_axe"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        Room *room = player->getRoom();
        CardStar card = room->askForCard(player, "@axe", "@axe:" + effect.to->objectName());
        if(card){
            QList<int> card_ids = card->getSubcards();
            foreach(int card_id, card_ids){
                LogMessage log;
                log.type = "$DiscardCard";
                log.from = player;
                log.card_str = QString::number(card_id);

                room->sendLog(log);
            }

            LogMessage log;
            log.type = "#AxeSkill";
            log.from = player;
            log.to << effect.to;
            room->sendLog(log);

            room->slashResult(effect, NULL);
        }

        return false;
    }
};

PanguAxe::PanguAxe(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("pangu_axe");
    skill = new PanguAxeSkill;
    attach_skill = true;
}

//----------------------------------------------------------WushuangHalberd

WushuangHalberd::WushuangHalberd(Suit suit, int number)
    :Weapon(suit, number, 4)
{
    setObjectName("wushuang_halberd");
}

//----------------------------------------------------------DeicideBow

class DeicideBowSkill: public WeaponSkill{
public:
    DeicideBowSkill():WeaponSkill("deicide_bow"){
        events << SlashHit;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        
        const Horse *horse = effect.to->getHorse();
        Room *room = player->getRoom();
        
        if(horse) {
            if(player->askForSkillInvoke(objectName(), data)) {
                room->throwCard(horse);
                
                LogMessage log;
                log.type = "$DeicideBowEffect";
                log.to << effect.to;
                log.card_str = horse->toString();
                room->sendLog(log);
            }
        }

        return false;
    }
};

DeicideBow::DeicideBow(Suit suit, int number)
    :Weapon(suit, number, 5)
{
    setObjectName("deicide_bow");
    skill = new DeicideBowSkill;
}

//----------------------------------------------------------GhostFan

class GhostFanSkill: public TriggerSkill{
public:
    GhostFanSkill():TriggerSkill("ghost_fan"){
        events << SlashHit;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasWeapon(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        Room *room = player->getRoom();

        if(!effect.to->isNude() && player->askForSkillInvoke("ghost_fan", data)){
            int card_id = room->askForCardChosen(player, effect.to, "he", "ghost_fan");
            room->throwCard(card_id);

            if(!effect.to->isNude()){
                card_id = room->askForCardChosen(player, effect.to, "he", "ghost_fan");
                room->throwCard(card_id);
            }

            return true;
        }

        return false;
    }
};

GhostFan::GhostFan(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("ghost_fan");
    skill = new GhostFanSkill;
}

//----------------------------------------------------------GudingMachete

class GudingMacheteSkill: public WeaponSkill{
public:
    GudingMacheteSkill():WeaponSkill("guding_machete"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") &&
            damage.to->isKongcheng())
        {
            Room *room = damage.to->getRoom();

            LogMessage log;
            log.type = "#GudingMacheteEffect";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

GudingMachete::GudingMachete(Suit suit, int number):Weapon(suit, number, 2){
    setObjectName("guding_machete");
    skill = new GudingMacheteSkill;
}

//----------------------------------------------------------WhiteDragon

class WhiteDragonSkill: public WeaponSkill{
public:
    WhiteDragonSkill():WeaponSkill("white_dragon"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Bang") &&
        damage.nature == DamageStruct::Normal) {
        
            static QStringList choices;
            if(choices.isEmpty()) {
                choices << "WhiteDragonEffect1" << "WhiteDragonEffect2" << "NoEffectChoice"; 
            }
            
            Room *room = damage.from->getRoom();
            QString choice = room->askForChoice(damage.from, objectName(), choices.join("+"));
            
            if(choice =="WhiteDragonEffect1") {
                damage.nature = DamageStruct::Ice;
                data = QVariant::fromValue(damage);
                
                LogMessage log;
                log.type = "#DamageToIce";
                log.to << damage.to;
                log.arg = objectName();
                room->sendLog(log);
                
            }else if(choice == "WhiteDragonEffect2") {
                RecoverStruct recover;
                room->recover(damage.from, recover);
                
                LogMessage log;
                log.type = "$WhiteDragonRecover";
                log.from = damage.from;
                room->sendLog(log);
            }
        }

        return false;
    }
};

WhiteDragon::WhiteDragon(Suit suit, int number):Weapon(suit, number, 1){
    setObjectName("white_dragon");
    skill = new WhiteDragonSkill;
}

//----------------------------------------------------------RedDragon

class RedDragonSkill: public WeaponSkill{
public:
    RedDragonSkill():WeaponSkill("red_dragon"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.card && damage.card->inherits("Bang") &&
        damage.nature == DamageStruct::Normal) {
        
            if(damage.from->askForSkillInvoke(objectName(), data)) {
                Room *room = player->getRoom();

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;

                room->judge(judge);

                if(judge.isGood()){
                
                    LogMessage log;
                    log.type = "$RedDragonDamage";
                    log.from = damage.from;
                    room->sendLog(log);
                    
                    DamageStruct damageMaker;
                    damageMaker.card = damage.from->getWeapon();
                    damageMaker.from = damage.from;
                    damageMaker.to = damage.to;
                    damageMaker.damage = 1;
                    damageMaker.nature = DamageStruct::Fire;
                    room->damage(damageMaker);
                    
                    // room->setEmotion(damage.to, "bad");
                }
                // else
                    // room->setEmotion(damage.to, "good");
            }
        }

        return false;
    }
};

RedDragon::RedDragon(Suit suit, int number):Weapon(suit, number, 1){
    setObjectName("red_dragon");
    skill = new RedDragonSkill;
}

//----------------------------------------------------------EvilDirk

class EvilDirkSkill: public WeaponSkill{
public:
    EvilDirkSkill():WeaponSkill("evil_dirk"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.card && damage.card->inherits("Bang") &&
        damage.nature == DamageStruct::Normal) {
        
            Room *room = damage.to->getRoom();
            
            static QStringList choices;
            if(choices.isEmpty()) {
                choices << "fire_nature" << "thunder_nature" << "air_nature" << "poison_nature" << "ice_nature" << "normal_nature"; 
            }
            QString choice = room->askForChoice(damage.from, objectName(), choices.join("+"));
            
            if(choice =="fire_nature") {
                damage.nature = DamageStruct::Fire;
            }else if(choice =="thunder_nature") {
                damage.nature = DamageStruct::Thunder;
            }else if(choice =="air_nature") {
                damage.nature = DamageStruct::Air;
            }else if(choice =="poison_nature") {
                damage.nature = DamageStruct::Poison;
            }else if(choice =="ice_nature") {
                damage.nature = DamageStruct::Ice;
            }
            
            if(damage.nature != DamageStruct::Normal) {
                LogMessage log;
                log.type = "#EvilDirkEffect";
                log.arg = choice;
                room->sendLog(log);
                
                data = QVariant::fromValue(damage);
            }
        }

        return false;
    }
};

EvilDirk::EvilDirk(Suit suit, int number):Weapon(suit, number, 1){
    setObjectName("evil_dirk");
    skill = new EvilDirkSkill;
}

//----------------------------------------------------------TigerSpear

class TigerSpearSkill: public WeaponSkill{
public:
    TigerSpearSkill():WeaponSkill("tiger_spear"){
        events << CardFinished << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        CardStar card = NULL;
        if(event == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card == NULL || !card->isBlack())
            return false;

        Room *room = player->getRoom();
        room->askForUseCard(player, "slash", "@tiger-spear-slash");

        return false;
    }
};

TigerSpear::TigerSpear(Suit suit, int number):Weapon(suit, number, 3){
    setObjectName("tiger_spear");
    skill = new TigerSpearSkill;
}

//----------------------------------------------------------PeacockTear

class PeacockTearSkill: public WeaponSkill{
public:
    PeacockTearSkill():WeaponSkill("peacock_tear"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()) {
        
            int x = 5 - damage.to->getHandcardNum();
            if(x < 0)
                return false;
                
            LogMessage log;
            log.type = "#PeacockTearEffect";
            log.to << damage.to;
            log.arg = QString::number(x);
            player->getRoom()->sendLog(log);
                
            damage.to->drawCards(x);
        }

        return false;
    }
};

PeacockTear::PeacockTear(Suit suit, int number):Weapon(suit, number, 5){
    setObjectName("peacock_tear");
    skill = new PeacockTearSkill;
}

//----------------------------------------------------------ChaosMirror

class ChaosMirrorSkill: public ArmorSkill{
private:
    ChaosMirrorSkill():ArmorSkill("chaos_mirror"){
        events << CardAsked;
    }
public:
    static ChaosMirrorSkill *GetInstance(){
        static ChaosMirrorSkill *instance = NULL;
        if(instance == NULL)
            instance = new ChaosMirrorSkill;

        return instance;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        QStringList prompt = data.toString().split(":");
        QString asked = prompt.at(0);
        QString reason = prompt.at(1);
        
        if(asked == "jink"){
            Room *room = player->getRoom();
            if(room->askForSkillInvoke(player, objectName(), reason)){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;

                room->judge(judge);
                if(judge.isGood()){
                    Dodge *jink = new Dodge(Card::NoSuit, 0);
                    jink->setSkillName(objectName());
                    room->provide(jink);
                    // room->setEmotion(player, "good");

                    return true;
                }
                // else
                    // room->setEmotion(player, "bad");
            }
        }
        return false;
    }
};

ChaosMirror::ChaosMirror(Suit suit, int number)
    :Armor(suit, number){
    setObjectName("chaos_mirror");
    skill = ChaosMirrorSkill::GetInstance();
}

//----------------------------------------------------------KingShield

class KingShieldSkill: public ArmorSkill{
public:
    KingShieldSkill():ArmorSkill("king_shield"){
        events << SlashEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->isBlack()){
        
            LogMessage log;
            log.type = "$KingShieldNullify";
            log.from = player;
            log.to << effect.to;
            log.card_str = effect.slash->toString();
            player->getRoom()->sendLog(log);

            return true;
        }else
            return false;
    }
};

KingShield::KingShield(Suit suit, int number)
    :Armor(suit, number)
{
    setObjectName("king_shield");
    skill = new KingShieldSkill;
}

//----------------------------------------------------------VineArmor

class VineArmorSkill: public ArmorSkill{
public:
    VineArmorSkill():ArmorSkill("vine_armor"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            LogMessage log;
            log.type = "#VineDamage";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            player->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }else if(damage.nature == DamageStruct::Normal) {
            LogMessage log;
            log.from = player;
            log.type = "#VineArmorNullify";
            player->getRoom()->sendLog(log);
            return true;
        }
        
        return false;
    }
};

VineArmor::VineArmor(Suit suit, int number):Armor(suit, number){
    setObjectName("vine_armor");
    skill = new VineArmorSkill;
}

//----------------------------------------------------------BloodGuard

class BloodGuardSkill: public ArmorSkill{
public:
    BloodGuardSkill():ArmorSkill("blood_guard"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.damage > 1){
            LogMessage log;
            log.type = "#BloodGuard";
            log.from = player;
            log.arg = QString::number(damage.damage);
            player->getRoom()->sendLog(log);

            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

BloodGuard::BloodGuard(Suit suit, int number):Armor(suit, number){
    setObjectName("blood_guard");
    skill = new BloodGuardSkill;
}

void BloodGuard::onUninstall(ServerPlayer *player) const{
    if(player->isAlive() && player->getMark("qinggang") == 0){
        RecoverStruct recover;
        recover.card = this;
        player->getRoom()->recover(player, recover);
    }
    EquipCard::onUninstall(player);
}

//----------------------------------------------------------HolyWing

class HolyWingSkill: public ArmorSkill{
public:
    HolyWingSkill():ArmorSkill("holy_wing"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature != DamageStruct::Normal) {
            LogMessage log;
            log.from = player;
            log.to << damage.to;
            log.type = "#HolyWingNullify";
            player->getRoom()->sendLog(log);
            
            return true;
        }
        
        return false;
    }
};

HolyWing::HolyWing(Suit suit, int number):Armor(suit, number){
    setObjectName("holy_wing");
    skill = new HolyWingSkill;
}

void HolyWing::onInstall(ServerPlayer *player) const{
    
    LogMessage log;
    log.type = "#HolyWingCost";
    log.from = player;
    player->getRoom()->sendLog(log);
    
    player->getRoom()->loseHp(player,1);
    EquipCard::onInstall(player);    
}

//----------------------------------------------------------QinglinHorse

class QinglinHorseSkill: public TriggerSkill{
public:
    QinglinHorseSkill():TriggerSkill("qinglin_horse"){
        can_forbid = false;
        events << Predamaged;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getHorse() && target->getHorse()->objectName() == "qinglin_horse";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.nature == DamageStruct::Poison) {
            LogMessage log;
            log.type = "#DamageToNormal";
            log.to << damage.to;
            log.arg = "poison_nature";
            player->getRoom()->sendLog(log);
            
            damage.nature = DamageStruct::Normal;
            
            data = QVariant::fromValue(damage);
        }
        
        return false;
    }
};

QinglinHorse::QinglinHorse(Suit suit, int number, int correct)
    :DefensiveHorse(suit, number, correct){
    
    setObjectName("qinglin_horse");
    skill = new QinglinHorseSkill;
    
}

void QinglinHorse::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        room->getThread()->addTriggerSkill(skill);
}

void QinglinHorse::onUninstall(ServerPlayer *player) const{

    if(skill)
        player->getRoom()->detachSkillFromPlayer(player, skill->objectName());
        //room->getThread()->removeTriggerSkill(skill);
    
    QString kingdom = player->getKingdom();
    if(kingdom == "yuan") {
        player->updateMp(3);
    }
}

//----------------------------------------------------------Rhinoceros

class RhinocerosSkill: public TriggerSkill{
public:
    RhinocerosSkill():TriggerSkill("rhinoceros"){
        can_forbid = false;
        events << Predamage;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getHorse() && target->getHorse()->objectName() == "rhinoceros";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.nature != DamageStruct::Normal) {
            LogMessage log;
            log.type = "#RhinocerosEffect";
            log.to << damage.from;
            player->getRoom()->sendLog(log);
            
            damage.nature = DamageStruct::Normal;
            
            data = QVariant::fromValue(damage);
        }
        
        return false;
    }
};

Rhinoceros::Rhinoceros(Suit suit, int number, int correct)
    :OffensiveHorse(suit, number, correct){
    
    setObjectName("rhinoceros");
    skill = new RhinocerosSkill;
    
}

void Rhinoceros::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        room->getThread()->addTriggerSkill(skill);
    
    room->setPlayerMark(player, "rhinoceros", 1);
}

void Rhinoceros::onUninstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        player->getRoom()->detachSkillFromPlayer(player, skill->objectName());
        //room->getThread()->removeTriggerSkill(skill);
        
    room->setPlayerMark(player, "rhinoceros", 0);
}

//----------------------------------------------------------Leopard

class LeopardSkill: public TriggerSkill{
public:
    LeopardSkill():TriggerSkill("leopard"){
        can_forbid = false;
        events << Predamaged;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getHorse() && target->getHorse()->objectName() == "leopard";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.nature == DamageStruct::Fire) {
            LogMessage log;
            log.type = "#DamageToNormal";
            log.to << damage.to;
            log.arg = "fire_nature";
            player->getRoom()->sendLog(log);
            
            damage.nature = DamageStruct::Normal;
            
            data = QVariant::fromValue(damage);
        }
        
        return false;
    }
};

Leopard::Leopard(Suit suit, int number, int correct)
    :DefensiveHorse(suit, number, correct){
    
    setObjectName("leopard");
    skill = new LeopardSkill;
    
}

void Leopard::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        room->getThread()->addTriggerSkill(skill);
}

void Leopard::onUninstall(ServerPlayer *player) const{

    if(skill)
        player->getRoom()->detachSkillFromPlayer(player, skill->objectName());
        //room->getThread()->removeTriggerSkill(sill);
    
    QString kingdom = player->getKingdom();
    if(kingdom == "nu") {
        player->updateMp(1);
    }
}

//----------------------------------------------------------Fox

class FoxSkill: public TriggerSkill{
public:
    FoxSkill():TriggerSkill("fox"){
        can_forbid = false;
        events << Predamaged;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getHorse() && target->getHorse()->objectName() == "fox";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.nature == DamageStruct::Ice) {
            LogMessage log;
            log.type = "#DamageToNormal";
            log.to << damage.to;
            log.arg = "ice_nature";
            player->getRoom()->sendLog(log);
            
            damage.nature = DamageStruct::Normal;
            
            data = QVariant::fromValue(damage);
        }
        
        return false;
    }
};

Fox::Fox(Suit suit, int number, int correct)
    :DefensiveHorse(suit, number, correct){
    
    setObjectName("fox");
    skill = new FoxSkill;
    
}

void Fox::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        room->getThread()->addTriggerSkill(skill);
}

void Fox::onUninstall(ServerPlayer *player) const{

    if(skill)
        player->getRoom()->detachSkillFromPlayer(player, skill->objectName());
        //room->getThread()->removeTriggerSkill(skill);
    
    QString kingdom = player->getKingdom();
    if(kingdom == "ling") {
        player->updateMp(2);
    }
}

//----------------------------------------------------------XunleiBeast

class XunleiBeastSkill: public TriggerSkill{
public:
    XunleiBeastSkill():TriggerSkill("xunlei_beast"){
        can_forbid = false;
        events << Predamaged;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getHorse() && target->getHorse()->objectName() == "xunlei_beast";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.nature == DamageStruct::Thunder) {
            LogMessage log;
            log.type = "#DamageToNormal";
            log.to << damage.to;
            log.arg = "thunder_nature";
            player->getRoom()->sendLog(log);
            
            damage.nature = DamageStruct::Normal;
            
            data = QVariant::fromValue(damage);
        }
        
        return false;
    }
};

XunleiBeast::XunleiBeast(Suit suit, int number, int correct)
    :OffensiveHorse(suit, number, correct){
    
    setObjectName("xunlei_beast");
    skill = new XunleiBeastSkill;
    
}

void XunleiBeast::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        room->getThread()->addTriggerSkill(skill);
}

void XunleiBeast::onUninstall(ServerPlayer *player) const{

    if(skill)
        player->getRoom()->detachSkillFromPlayer(player, skill->objectName());
        //room->getThread()->removeTriggerSkill(skill);
    
    QString kingdom = player->getKingdom();
    if(kingdom == "kuang") {
        player->updateMp(2);
    }
}

//----------------------------------------------------------Boar

class BoarSkill: public TriggerSkill{
public:
    BoarSkill():TriggerSkill("boar"){
        can_forbid = false;
        events << Damaged;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getHorse() && target->getHorse()->objectName() == "boar";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    
        DamageStruct damage = data.value<DamageStruct>();
        const Horse *horse = damage.to->getHorse();
        Room *room = player->getRoom();
        
        if(damage.from && damage.to->isAlive()) {
            if(player->askForSkillInvoke(objectName(), data)) {
                room->throwCard(horse); 
                
                LogMessage log;
                log.type = "#BoarEffect";
                log.from = damage.from;
                log.arg = QString::number(damage.damage);
                room->sendLog(log);
                
                DamageStruct damageMaker;
                damageMaker.card = horse;
                damageMaker.from = damage.to;
                damageMaker.to = damage.from;
                damageMaker.damage = damage.damage;
                damageMaker.nature = damage.nature;
                room->damage(damageMaker);                
            }
        }
        
        return false;
    }
};

Boar::Boar(Suit suit, int number, int correct)
    :OffensiveHorse(suit, number, correct){
    
    setObjectName("boar");
    skill = new BoarSkill;
    
}

void Boar::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        room->getThread()->addTriggerSkill(skill);
}

void Boar::onUninstall(ServerPlayer *player) const{

    if(skill)
        player->getRoom()->detachSkillFromPlayer(player, skill->objectName());
}

//----------------------------------------------------------Tiger

class TigerSkill: public TriggerSkill{
public:
    TigerSkill():TriggerSkill("tiger"){
        can_forbid = false;
        events << Predamaged;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getHorse() && target->getHorse()->objectName() == "tiger";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.nature == DamageStruct::Air) {
            LogMessage log;
            log.type = "#DamageToNormal";
            log.to << damage.to;
            log.arg = "air_nature";
            player->getRoom()->sendLog(log);
            
            damage.nature = DamageStruct::Normal;
            
            data = QVariant::fromValue(damage);
        }
        
        return false;
    }
};

Tiger::Tiger(Suit suit, int number, int correct)
    :DefensiveHorse(suit, number, correct){
    
    setObjectName("tiger");
    skill = new TigerSkill;
    
}

void Tiger::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        room->getThread()->addTriggerSkill(skill);
}

void Tiger::onUninstall(ServerPlayer *player) const{

    if(skill)
        player->getRoom()->detachSkillFromPlayer(player, skill->objectName());
        //room->getThread()->removeTriggerSkill(skill);
    
    QString kingdom = player->getKingdom();
    if(kingdom == "qi") {
        player->updateMp(2);
    }
}

//----------------------------------------------------------FloodDragon

class FloodDragonSkill: public DrawCardsSkill{
public:
    FloodDragonSkill():DrawCardsSkill("flood_dragon"){
        can_forbid = false;
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        Room *room = player->getRoom();
        
        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
        judge.good = true;
        judge.reason = objectName();
        judge.who = player;

        room->judge(judge);
        
        if(judge.isBad()) {
        
            LogMessage log;
            log.type = "$FloodDragonDraw";
            log.from = player;
            player->getRoom()->sendLog(log);
            
            return n-1;
        }
        
        return n;
    }
};

FloodDragon::FloodDragon(Suit suit, int number, int correct)
    :OffensiveHorse(suit, number, correct){
    
    setObjectName("flood_dragon");
    skill = new FloodDragonSkill;
    
}

void FloodDragon::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    
    if(skill)
        room->acquireSkill(player, skill->objectName());
}

void FloodDragon::onUninstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    
    if(skill)
        room->detachSkillFromPlayer(player, skill->objectName());
}

//----------------------------------------------------------Deer

class DeerSkill:public TriggerSkill{
public:
    DeerSkill():TriggerSkill("deer"){
        frequency = Compulsory;
        can_forbid = false;
        events <<PhaseChange << FinishJudge << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getHorse() && target->getHorse()->objectName() == "deer";
    }
    
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        
        if(event == PhaseChange && player->getPhase() == Player::Start) {
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = player;

            room->judge(judge);
            
            if(judge.isBad()) {
            
                LogMessage log;
                log.type = "$DeerEffectSkip";
                room->sendLog(log);
                
                player->skip(Player::Draw);
            }

        }else if(event == CardAsked && player->getMark("qinggang") == 0 && !player->getArmor()) {
        
            QStringList prompt = data.toString().split(":");
            QString pattern = prompt.at(0);
            QString reason = prompt.at(1);

            if(pattern != "jink")
                return false;

            if(player->askForSkillInvoke("chaos_mirror", reason)){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;

                room->judge(judge);

                if(judge.isGood()){
                    Dodge *jink = new Dodge(Card::NoSuit, 0);
                    jink->setSkillName(objectName());
                    room->provide(jink);
                    return true;
                }
            }
            
        }else if(event == FinishJudge) {
        
            JudgeStar judge = data.value<JudgeStar>();
            player->obtainCard(judge->card);
            
            return true;
        }

        return false;
    }
};

Deer::Deer(Suit suit, int number, int correct)
    :DefensiveHorse(suit, number, correct){
    
    setObjectName("deer");
    skill = new DeerSkill;
    
}

void Deer::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    
    if(skill)
        room->acquireSkill(player, skill->objectName());
}

void Deer::onUninstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    
    if(skill)
        room->detachSkillFromPlayer(player, skill->objectName());
}

//----------------------------------------------------------SpiderQueen

SpiderQueenSkillCard::SpiderQueenSkillCard(){
    setObjectName("spiderqueenskill");
    target_fixed = true;
}

void SpiderQueenSkillCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    const Horse *horse = source->getHorse();
    if(horse && horse->objectName() == "spider_queen") {
        room->throwCard(horse);
        
        ThousandsArrowsShot *card = new ThousandsArrowsShot(Card::NoSuit, 0);
        card->setSkillName(objectName());
        
        CardUseStruct use;
        use.card = card;        
        use.from = source;
        // use.to << room->getOtherPlayers(source);
        room->useCard(use);
    }
}

class SpiderQueenVaskill:public ZeroCardViewAsSkill{
public:
    SpiderQueenVaskill():ZeroCardViewAsSkill("spider_queen"){
        can_forbid = false;
    }
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return true;
    }
    
    virtual const Card *viewAs() const{
        return new SpiderQueenSkillCard;
    }
};

SpiderQueen::SpiderQueen(Suit suit, int number, int correct)
    :OffensiveHorse(suit, number, correct){
    
    setObjectName("spider_queen");
}

void SpiderQueen::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    
    // room->acquireSkill(player, "spider_queen");
    room->attachSkillToPlayer(player, "spider_queen");
}

void SpiderQueen::onUninstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    
    room->detachSkillFromPlayer(player, "spider_queen");
}

//----------------------------------------------------------Cattle

class CattleSkill: public TriggerSkill{
public:
    CattleSkill():TriggerSkill("cattle"){
        can_forbid = false;
        events << Predamaged;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getHorse() && target->getHorse()->objectName() == "cattle";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    
        DamageStruct damage = data.value<DamageStruct>();
        const Horse *horse = damage.to->getHorse();
        Room *room = player->getRoom();
        
        if(damage.to && damage.to->isAlive()) {
            if(player->askForSkillInvoke(objectName(), data)) {
                room->throwCard(horse); 
                
                LogMessage log;
                log.type = "#CattleEffect";
                log.to << damage.to;
                room->sendLog(log);
                
                return true;
           
            }
        }
        
        return false;
    }
};

Cattle::Cattle(Suit suit, int number, int correct)
    :DefensiveHorse(suit, number, correct){
    
    setObjectName("cattle");
    skill = new CattleSkill;
    
}

void Cattle::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();

    if(skill)
        room->getThread()->addTriggerSkill(skill);
}

void Cattle::onUninstall(ServerPlayer *player) const{

    if(skill)
        player->getRoom()->detachSkillFromPlayer(player, skill->objectName());
        //room->getThread()->removeTriggerSkill(skill);
}

//----------------------------------------------------------Relic

Relic::Relic(Suit suit, int number)
    :EquipCard(suit, number), attach_skill(false)
{
}


QString Relic::getSubtype() const{
    return "relic";
}

EquipCard::Location Relic::location() const{
    return RelicLocation;
}

QString Relic::label() const{
    return QString("%1").arg(getName());
}

void Relic::onInstall(ServerPlayer *player) const{
    EquipCard::onInstall(player);
    Room *room = player->getRoom();

    if(attach_skill)
        room->attachSkillToPlayer(player, objectName());
}

void Relic::onUninstall(ServerPlayer *player) const{
    EquipCard::onUninstall(player);
    Room *room = player->getRoom();

    if(attach_skill)
        room->detachSkillFromPlayer(player, objectName());
}

//----------------------------------------------------------NvwaStone

class NvwaStoneSkill: public RelicSkill{
public:
    NvwaStoneSkill():RelicSkill("nvwa_stone"){
        events << Dying;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{

        Room *room = player->getRoom();
        
        if(player->askForSkillInvoke(objectName(), data)) {
            Card::Suit suit1 = room->askForSuit(player, objectName());            
            
            LogMessage log;
            log.type = "#ChoiceSuit";
            log.from = player;
            log.arg = Card::Suit2String(suit1);
            player->getRoom()->sendLog(log);
            
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*)");
            judge.reason = objectName();
            judge.who = player;

            room->judge(judge);
            
            Card::Suit suit2 = judge.card->getSuit();
            
            if(suit1 == suit2){
                RecoverStruct recover;
                recover.card = player->getRelic();
                room->recover(player, recover);
                // room->setEmotion(player, "good");

            }
            // else
                // room->setEmotion(player, "false");
        }

        return false;
    }
};

NvwaStone::NvwaStone(Suit suit, int number)
    :Relic(suit, number)
{
    setObjectName("nvwa_stone");
    skill = new NvwaStoneSkill;
}

//----------------------------------------------------------Blunderbuss

BlunderbussSkillCard::BlunderbussSkillCard(){
    setObjectName("blunderbussskill");
}

bool BlunderbussSkillCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(to_select->hasEquip())
        return true;
        
    return false;
}

void BlunderbussSkillCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    
    room->throwCard(this);
    
    int card_id = room->askForCardChosen(effect.from, effect.to, "e", objectName());
    room->throwCard(card_id);
    
}

class BlunderbussVaskill:public ViewAsSkill{
public:
    BlunderbussVaskill():ViewAsSkill("blunderbuss"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty() || selected.length() == 1){
            return !to_select->isEquipped();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            BlunderbussSkillCard *skillCard = new BlunderbussSkillCard;
            skillCard->addSubcards(cards);
            return skillCard;
        }else
            return NULL;
    }
};

Blunderbuss::Blunderbuss(Suit suit, int number)
    :Relic(suit, number)
{
    setObjectName("blunderbuss");
    attach_skill = true;
}

//----------------------------------------------------------SoulSpirit

class SoulSpiritSkill: public RelicSkill{
public:
    SoulSpiritSkill():RelicSkill("soul_spirit"){
        events << Death;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasRelic(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{

        Room *room = player->getRoom();
        ServerPlayer *source = NULL;
        foreach(ServerPlayer *target, room -> getAllPlayers()){
            if(target->getRelic()&& target->getRelic()->objectName() == "soul_spirit" && target->isAlive()) {
                source = target;
                break;
            }
        }
        
        if(source && source->askForSkillInvoke(objectName(), data)) {
            DummyCard *all_cards = player->wholeHandCards();
            if(all_cards){
                room->moveCardTo(all_cards, source, Player::Hand, false);
                delete all_cards;
            }
        }    
        
        return false;
    }
};

SoulSpirit::SoulSpirit(Suit suit, int number)
    :Relic(suit, number)
{
    setObjectName("soul_spirit");
    skill = new SoulSpiritSkill;
}

//----------------------------------------------------------Miracle

class MiracleSkill: public RelicSkill{
public:
    MiracleSkill():RelicSkill("miracle"){
        events << PhaseChange;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{

        if(player->getPhase() == Player::Start && player->getHandcardNum() == 3) {
            if(player->askForSkillInvoke(objectName(), data)) {
                Room *room = player->getRoom();
                
                player->skip(Player::Draw);
                
                room->drawCards(player, 4);
                
                room->askForDiscard(player, objectName(), 2, false, false);
            }
        }
        
        return false;
    }
};

Miracle::Miracle(Suit suit, int number)
    :Relic(suit, number)
{
    setObjectName("miracle");
    skill = new MiracleSkill;
}

//----------------------------------------------------------Shtm

Shtm::Shtm(Suit suit, int number)
    :Relic(suit, number)
{
    setObjectName("shtm");
}

bool Shtm::willThrow() const{
    return false;
}

bool Shtm::isAvailable(const Player *player) const {
    QList<const Player *> players = player->parent()->findChildren<const Player *>();
    foreach(const Player *p, players){
        if(p->isDead())
            return true;
    }

    return false;
}

void Shtm::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

    ServerPlayer *dead = NULL;
    QStringList deadPlayersName;
    
    foreach(ServerPlayer *target, room -> getServerPlayers()){
        if(target->isDead()) {
            dead = target;
            deadPlayersName << target->getGeneralName();
        }
    }
       
    if(deadPlayersName.length() > 1) {
        QString choice = room->askForChoice(source, objectName(), deadPlayersName.join("+"));        
        dead = room->findPlayer(choice, true);
    }
       
    if(dead) {        
        if(dead->getRelic())
            room->throwCard(dead->getRelic());

        room->moveCardTo(this, dead, Player::Equip, true);
    }   

}

void Shtm::onInstall(ServerPlayer *player) const{
    EquipCard::onInstall(player);
    player->gainMark("@revive_rite");
}

void Shtm::onUninstall(ServerPlayer *player) const{
    EquipCard::onUninstall(player);
    player->loseMark("@revive_rite");
}

class HorseSkill: public DistanceSkill{
public:
    HorseSkill():DistanceSkill("horse"){
        can_forbid = false;
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->getOffensiveHorse())
            correct += from->getOffensiveHorse()->getCorrect();
        if(to->getDefensiveHorse())
            correct += to->getDefensiveHorse()->getCorrect();

        return correct;
    }
};

//----------------------------------------------------------CardPattern

class ToUsePattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card);
    }
    virtual bool willThrow() const{
        return false;
    }
};

class HandcardPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card);
    }
};

class SuitPattern: public CardPattern{
public:
    SuitPattern(Card::Suit suit)
        :suit(suit)
    {
    }

    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card) && card->getSuit() == suit;
    }

private:
    Card::Suit suit;
};

class SlashPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card) && card->inherits("Slash");
    }
};

class NamePattern: public CardPattern{
public:
    NamePattern(const QString &name)
        :name(name)
    {

    }

    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card) && card->objectName() == name;
    }

private:
    QString name;
};

class PAPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card) &&
            (card->objectName() == "holy_water" || card->objectName() == "schnapps");
    }
};

class BasicPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return ! player->hasEquip(card) && card->getTypeId() == Card::Basic;
    }
};

//----------------------------------------------------------cheat

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}

//----------------------------------------------------------end

MeleeCardPackage::MeleeCardPackage()
    :Package("meleecard")
{   
    patterns["."] = new HandcardPattern;
    patterns[".touse"] = new ToUsePattern;
    patterns[".S"] = new SuitPattern(Card::Spade);
    patterns[".C"] = new SuitPattern(Card::Club);
    patterns[".H"] = new SuitPattern(Card::Heart);
    patterns[".D"] = new SuitPattern(Card::Diamond);
    
    // modify by ce
    patterns["slash"] = new SlashPattern;
    // patterns["jink"] = new NamePattern("jink");
    patterns["jink"] = new NamePattern("dodge");
    // patterns["peach"] = new NamePattern("peach");
    patterns["peach"] = new NamePattern("holy_water");
    // patterns["nullification"] = new NamePattern("nullification");
    patterns["nullification"] = new NamePattern("unassailable");
    patterns["peach+analeptic"] = new PAPattern;
    patterns[".basic"] = new BasicPattern;

    QList<Card *> cards;
    
    cards 
        << new Bang(Card::Spade, 7)
        << new Bang(Card::Spade, 8)
        << new Bang(Card::Spade, 8)
        << new Bang(Card::Spade, 9)
        << new Bang(Card::Spade, 9)
        << new Bang(Card::Spade, 10)
        << new Bang(Card::Spade, 10)
        << new Bang(Card::Spade, 10)
        << new Bang(Card::Club, 2)
        << new Bang(Card::Club, 3)
        << new Bang(Card::Club, 4)
        << new Bang(Card::Club, 5)
        << new Bang(Card::Club, 6)
        << new Bang(Card::Club, 6)
        << new Bang(Card::Club, 7)
        << new Bang(Card::Club, 8)
        << new Bang(Card::Club, 8)
        << new Bang(Card::Club, 9)
        << new Bang(Card::Club, 9)
        << new Bang(Card::Club, 10)
        << new Bang(Card::Club, 10)
        << new Bang(Card::Club, 11)
        << new Bang(Card::Club, 11)
        << new Bang(Card::Heart, 8)
        << new Bang(Card::Heart, 10)
        << new Bang(Card::Heart, 10)
        << new Bang(Card::Heart, 11)
        << new Bang(Card::Diamond, 4)
        << new Bang(Card::Diamond, 6)
        << new Bang(Card::Diamond, 7)
        << new Bang(Card::Diamond, 8)
        << new Bang(Card::Diamond, 9)
        << new Bang(Card::Diamond, 10)
        << new Bang(Card::Diamond, 13)
        
        << new FireBang(Card::Heart, 4)
        << new FireBang(Card::Heart, 7)
        << new FireBang(Card::Heart, 10)
        << new FireBang(Card::Diamond, 4)
        << new FireBang(Card::Diamond, 5)
        
        << new ThunderBang(Card::Spade, 4)
        << new ThunderBang(Card::Spade, 5)
        << new ThunderBang(Card::Spade, 6)
        << new ThunderBang(Card::Spade, 7)
        << new ThunderBang(Card::Spade, 8)
        << new ThunderBang(Card::Club, 5)
        << new ThunderBang(Card::Club, 6)
        << new ThunderBang(Card::Club, 7)
        << new ThunderBang(Card::Club, 8)
        
        << new AirBang(Card::Heart, 7)
        << new AirBang(Card::Heart, 7)
        << new AirBang(Card::Diamond, 7)
        
        << new PoisonBang(Card::Club, 3)
        << new PoisonBang(Card::Club, 8)
        << new PoisonBang(Card::Club, 12)
        
        << new IceBang(Card::Spade, 11)
        << new IceBang(Card::Club, 7)
        << new IceBang(Card::Diamond, 2)
        
        << new Dodge(Card::Spade, 3)
        << new Dodge(Card::Spade, 5)
        << new Dodge(Card::Spade, 11)
        << new Dodge(Card::Spade, 13)
        << new Dodge(Card::Club, 9)
        << new Dodge(Card::Heart, 2)
        << new Dodge(Card::Heart, 2)
        << new Dodge(Card::Heart, 8)
        << new Dodge(Card::Heart, 9)
        << new Dodge(Card::Heart, 11)
        << new Dodge(Card::Heart, 12)
        << new Dodge(Card::Heart, 13)
        << new Dodge(Card::Diamond, 2)
        << new Dodge(Card::Diamond, 2)
        << new Dodge(Card::Diamond, 3)
        << new Dodge(Card::Diamond, 4)
        << new Dodge(Card::Diamond, 5)
        << new Dodge(Card::Diamond, 6)
        << new Dodge(Card::Diamond, 6)
        << new Dodge(Card::Diamond, 7)
        << new Dodge(Card::Diamond, 7)
        << new Dodge(Card::Diamond, 8)
        << new Dodge(Card::Diamond, 8)
        << new Dodge(Card::Diamond, 9)
        << new Dodge(Card::Diamond, 10)
        << new Dodge(Card::Diamond, 10)
        << new Dodge(Card::Diamond, 11)
        << new Dodge(Card::Diamond, 11)
        << new Dodge(Card::Diamond, 11)

        << new HolyWater(Card::Heart, 3)
        << new HolyWater(Card::Heart, 4)
        << new HolyWater(Card::Heart, 5)
        << new HolyWater(Card::Heart, 6)
        << new HolyWater(Card::Heart, 6)
        << new HolyWater(Card::Heart, 7)
        << new HolyWater(Card::Heart, 8)
        << new HolyWater(Card::Heart, 9)
        << new HolyWater(Card::Heart, 12)
        << new HolyWater(Card::Diamond, 2)
        << new HolyWater(Card::Diamond, 3)
        << new HolyWater(Card::Diamond, 12)

        << new Schnapps(Card::Spade, 3)
        << new Schnapps(Card::Spade, 9)
        << new Schnapps(Card::Club, 3)
        << new Schnapps(Card::Club, 9)
        << new Schnapps(Card::Diamond, 9)

        << new AlienInvasion(Card::Spade, 7)
        << new AlienInvasion(Card::Spade, 13)
        << new AlienInvasion(Card::Club, 7)
        << new ThousandsArrowsShot(Card::Heart, 1)
        << new Roar(Card::Spade, 13)
        << new Roar(Card::Heart, 13)
        
        << new WoodElf(Card::Heart, 1)
        << new ReadyToGo(Card::Heart, 3)
        << new ReadyToGo(Card::Heart, 4)
        << new PoisonMarish(Card::Spade, 11)
        << new PoisonMarish(Card::Club, 3)

        << new NothingIsSomething(Card::Heart, 7)
        << new NothingIsSomething(Card::Heart, 8)
        << new NothingIsSomething(Card::Heart, 9)
        << new NothingIsSomething(Card::Heart, 11)
        << new DestroyAll(Card::Spade, 3)
        << new DestroyAll(Card::Spade, 4)
        << new DestroyAll(Card::Spade, 12)
        << new DestroyAll(Card::Club, 3)
        << new DestroyAll(Card::Club, 4)
        << new DestroyAll(Card::Heart, 12)
        << new BorrowWeapon(Card::Club, 12)
        << new BorrowWeapon(Card::Club, 13)
        << new Grab(Card::Spade, 3)
        << new Grab(Card::Spade, 4)
        << new Grab(Card::Spade, 11)
        << new Grab(Card::Diamond, 3)
        << new Grab(Card::Diamond, 4)
        << new Unassailable(Card::Spade, 11)
        << new Unassailable(Card::Spade, 13)
        << new Unassailable(Card::Club, 5)
        << new Unassailable(Card::Club, 12)
        << new Unassailable(Card::Club, 13)
        << new Unassailable(Card::Heart, 1)
        << new Unassailable(Card::Heart, 13)
        << new Unassailable(Card::Diamond, 5)
        << new Unassailable(Card::Diamond, 12)
        << new Burn(Card::Heart, 2)
        << new Burn(Card::Heart, 3)
        << new Burn(Card::Diamond, 12)
        << new PK(Card::Spade, 1)
        << new PK(Card::Club, 1)
        << new PK(Card::Diamond, 1)
        << new Cure(Card::Heart, 2)
        << new Cure(Card::Heart, 6)
        << new Cure(Card::Heart, 10)
        << new Cure(Card::Diamond, 4)
        << new Cure(Card::Diamond, 8)
          
        << new SoulChain(Card::Spade, 11)
        << new SoulChain(Card::Spade, 12)
        << new SoulChain(Card::Club, 10)
        << new SoulChain(Card::Club, 11)
        << new SoulChain(Card::Club, 12)
        << new SoulChain(Card::Club, 13)

        << new EnegyDrain(Card::Spade, 10)
        << new EnegyDrain(Card::Club, 4)
        << new SoulAwe(Card::Spade, 6)
        << new SoulAwe(Card::Club, 6)
        << new SoulAwe(Card::Heart, 6)
        << new Icebound(Card::Club, 4)
        << new Icebound(Card::Diamond, 10)

        << new Thunder(Card::Spade, 1)
        << new Thunder(Card::Heart, 12)

        << new LiyaoSword(Card::Spade, 2)
        << new GreenSteelSword(Card::Spade, 6)
        << new SoulCane(Card::Spade, 5)
        << new SnakeSpear(Card::Spade, 12)
        << new PanguAxe(Card::Diamond, 5)
        << new WushuangHalberd(Card::Diamond, 12)
        << new DeicideBow(Card::Heart, 5)
        << new GhostFan(Card::Spade, 2)
        << new GudingMachete(Card::Spade, 1)
        << new WhiteDragon(Card::Club, 1)
        << new RedDragon(Card::Diamond, 1)
        << new EvilDirk(Card::Spade, 11)
        << new TigerSpear(Card::Diamond, 9)
        << new PeacockTear(Card::Heart, 12)

        << new ChaosMirror(Card::Spade, 2)
        << new ChaosMirror(Card::Club, 2)
        << new KingShield(Card::Club, 2)
        << new VineArmor(Card::Spade, 2)
        << new VineArmor(Card::Club, 2)
        << new BloodGuard(Card::Club, 1)
        << new HolyWing(Card::Spade, 9)
        << new HolyWing(Card::Diamond, 3)
        
        << new QinglinHorse(Card::Diamond, 13, +1)
        << new Rhinoceros(Card::Spade, 5, -1)
        << new Leopard(Card::Heart, 5, +1)
        << new Fox(Card::Diamond, 13, +1)
        << new XunleiBeast(Card::Heart, 13, -1)
        << new Boar(Card::Club, 5, -1)
        << new Tiger(Card::Spade, 13, +1)
        << new FloodDragon(Card::Heart, 12, -2)
        << new Deer(Card::Diamond, 2, +2)
        << new SpiderQueen(Card::Club, 6, -1)
        << new Cattle(Card::Heart, 13, +1)
        
        << new NvwaStone(Card::Diamond, 1)
        << new Blunderbuss(Card::Diamond, 1)
        << new SoulSpirit(Card::Spade, 1)
        << new Miracle(Card::Club, 1)
        << new Shtm(Card::Heart, 1)
        ;
          
    foreach(Card *card, cards)
        card->setParent(this);
        
    skills 
        << new HorseSkill 
        << new SnakeSpearSkill 
        << new PanguAxeViewAsSkill 
        << new FloodDragonSkill
        << new DeerSkill
        << new SpiderQueenVaskill
        << new BlunderbussVaskill
        ;

    addMetaObject<SpiderQueenSkillCard>();
    addMetaObject<BlunderbussSkillCard>();
        
    addMetaObject<CheatCard>();

    type = CardPack;
}

ADD_PACKAGE(MeleeCard)
