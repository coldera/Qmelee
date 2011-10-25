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

//--------------------------------------------------------------------------------------------------------------End

MeleeSFPackage::MeleeSFPackage()
    :Package("meleesf")
{
    General *gouki;
    
    gouki = new General(this, "gouki", "yuan");
    gouki->addSkill(new Shayi);
    gouki->addSkill(new Shankong);
    gouki->addSkill(new Shunyu);

    addMetaObject<ShunyuCard>();
}

ADD_PACKAGE(MeleeSF);


