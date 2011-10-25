#include "melee-sf.h"

#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "settings.h"

// skill cards

//--------------------------------------------------------------------------------------------------------------gouki

//----------------------------------------------------------------------------- Shayi

class Shayi: public TriggerSkill{
public:
    Shayi():TriggerSkill("shayi"){
        frequency = Compulsory;
        events << CardFinished;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *gouki, QVariant &data) const{
        
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Slash")) {
        
            LogMessage log;
            log.type = "#Shayi";
            log.from = gouki;
            gouki->getRoom()->sendLog(log);
        
            gouki->updateMp(2);
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Shankong

class Shankong: public TriggerSkill{
public:
    Shankong():TriggerSkill("shankong"){
        events << CardResponsed;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMp()>=6;
    }
    
    virtual bool trigger(TriggerEvent event, ServerPlayer *gouki, QVariant &data) const{
        Room *room = gouki->getRoom();
    
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Dodge"))
            return false;
        
        if(room->askForSkillInvoke(gouki, objectName())) {
            gouki->updateMp(-6);            
            
            room->playSkillEffect(objectName());
            
            gouki->drawCards(2);            
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Shunyu

ShunyuCard::ShunyuCard(){
    will_throw = true;
}

bool ShunyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(Self->distanceTo(to_select) > 1) 
        return false;

    return true;
}

void ShunyuCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *gouki = effect.from;
    Room *room = gouki->getRoom();
    
    bool success = false;
    
    for(int i=0; i<3; i++) {
        const Card *card = room->peek();
        
        LogMessage log;
        log.type = "$Peek";
        log.from = gouki;
        log.card_str = card->toString();
        room->sendLog(log);
        
        if(card->isRed()){
            gouki->obtainCard(card);
        }else if(card->isBlack()) {
            room->throwCard(card);
            success = true;
        }
    }
    
    if(success) {
        room->broadcastInvoke("animate", "shunyu:2500");
        
        DamageStruct damageMaker;
        damageMaker.card = this;
        damageMaker.from = gouki;
        damageMaker.to = effect.to;
        damageMaker.damage = 3;
        damageMaker.nature = DamageStruct::Normal;
        room->damage(damageMaker);
    }else {
        LogMessage log;
        log.type = "#ShunyuFail";
        log.from = gouki;
        room->sendLog(log);
    }
}

class ShunyuViewAsSkill: public ViewAsSkill{
public:
    ShunyuViewAsSkill():ViewAsSkill("shunyu"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@shunyu";
    }
    
    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 4)
            return false;

        foreach(CardItem *item, selected){
            if(to_select->getFilteredCard()->getSuit() == item->getFilteredCard()->getSuit())
                return false;
        }

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 4)
            return NULL;

        ShunyuCard *card = new ShunyuCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        
        return card;
    }
};

class Shunyu: public PhaseChangeSkill{
public:
    Shunyu():PhaseChangeSkill("shunyu"){
        view_as_skill = new ShunyuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getCards("he").length()>=4
                && target->getMp()>=18;
    }

    virtual bool onPhaseChange(ServerPlayer *gouki) const{
        Room *room = gouki->getRoom();
        
        if(room->askForUseCard(gouki, "@@shunyu", "@shunyu")){
            gouki->updateMp(-18);
            gouki->skip(Player::Play);
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------ryu

//----------------------------------------------------------------------------- Longjuan

LongjuanCard::LongjuanCard(){
    once = true;
    will_throw = true;
}

bool LongjuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{

    if(to_select == Self)
        return false;
        
    if(targets.length() >= 2)
        return false;
        
    if(Self->distanceTo(to_select) > Self->getAttackRange()+1) 
        return false;

    return true;
}

bool LongjuanCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= 2 && targets.length() > 0;
}

void LongjuanCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *ryu = card_use.from;
    
    CardUseStruct use;
    use.card = Sanguosha->getCard(getSubcards().first());
    use.from = ryu;
    use.to << card_use.to;
    
    LogMessage log;
    log.type = "#LongjuanBang";
    log.from = ryu;
    room->sendLog(log);

    room->useCard(use);
}

class LongjuanViewAsSkill: public OneCardViewAsSkill{
public:
    LongjuanViewAsSkill():OneCardViewAsSkill("longjuan"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@longjuan";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Slash") && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LongjuanCard *card = new LongjuanCard;
        card->addSubcard(card_item->getFilteredCard());
        card->setSkillName(objectName());
        return card;
    }
};

class Longjuan: public TriggerSkill{
public:
    Longjuan():TriggerSkill("longjuan"){
        view_as_skill = new LongjuanViewAsSkill;
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->hasUsed("LongjuanCard");
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *ryu, QVariant &data) const{

        Room *room = ryu->getRoom();
        
        if(room->askForUseCard(ryu, "@@longjuan", "@longjuan")) {
            room->playSkillEffect(objectName());
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Bodong

BodongCard::BodongCard(){
    will_throw = true;
}

bool BodongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(Self->distanceTo(to_select) > Self->getAttackRange()+1) 
        return false;

    return true;
}

void BodongCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();   
	
    room->broadcastInvoke("animate", "bodong");
    
    int dp = 3;
    const Card *first=NULL, *second=NULL, *third=NULL;
    first = room->askForCard(effect.to, "jink", "@bodong-jink:" + effect.from->objectName());
    if(first) {
        second = room->askForCard(effect.to, "jink", "@bodong-jink-more:" + effect.from->objectName());
        dp--;
    }
    if(second) {
        third = room->askForCard(effect.to, "jink", "@bodong-jink-more:" + effect.from->objectName());
        dp--;
    }
    if(third) {
        dp--;
    }
    
    if(dp) {       
        DamageStruct damageMaker;
        damageMaker.card = effect.card;
        damageMaker.from = effect.from;
        damageMaker.to = effect.to;
        damageMaker.damage = dp;
        damageMaker.nature = DamageStruct::Air;
        room->damage(damageMaker);        
    }
}

class BodongViewAsSkill: public OneCardViewAsSkill{
public:
    BodongViewAsSkill():OneCardViewAsSkill("bodong"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@bodong";
    }
    
    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Slash") && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        BodongCard *card = new BodongCard;
        card->addSubcard(card_item->getFilteredCard());
        card->setSkillName(objectName());
        return card;
    }
};

class Bodong: public PhaseChangeSkill{
public:
    Bodong():PhaseChangeSkill("bodong"){
        view_as_skill = new BodongViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMp()>=3;
    }

    virtual bool onPhaseChange(ServerPlayer *ryu) const{
        Room *room = ryu->getRoom();
        
        if(room->askForUseCard(ryu, "@@bodong", "@bodong")){
            ryu->updateMp(-3);
            ryu->skip(Player::Play);
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------End

MeleeSFPackage::MeleeSFPackage()
    :Package("meleesf")
{
    General *gouki, *ryu;
    
    gouki = new General(this, "gouki", "yuan");
    gouki->addSkill(new Shayi);
    gouki->addSkill(new Shankong);
    gouki->addSkill(new Shunyu);

    addMetaObject<ShunyuCard>();
    
    ryu = new General(this, "ryu", "nu");
    ryu->addSkill(new Longjuan);
    ryu->addSkill(new Bodong);
    
    addMetaObject<LongjuanCard>();
    addMetaObject<BodongCard>();
}

ADD_PACKAGE(MeleeSF);


