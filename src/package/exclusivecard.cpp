#include "exclusivecard.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"

//---------------------------------------------------------- Chaoxiu

class ChaoxiuSkill: public TriggerSkill{
public:
    ChaoxiuSkill():TriggerSkill("chaoxiu"){
        events << SlashHit;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasWeapon(objectName()) && target->getGeneralName() == "genan";
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *genan, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        Room *room = genan->getRoom();

        if(!effect.to->isNude() && genan->askForSkillInvoke(objectName(), data)){
            int num = effect.to->getCards("e").length()+1;
            
            LogMessage log;
            log.type = "$ChaoxiuSilie";
            log.to << effect.to;
            
            while(!effect.to->isNude() && (num--)) {
                int card_id = room->askForCardChosen(genan, effect.to, "he", objectName());

                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);
                
                room->throwCard(card_id);
            }

            return true;
        }

        return false;
    }
};

Chaoxiu::Chaoxiu(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("chaoxiu");
    skill = new ChaoxiuSkill;
}

QString Chaoxiu::getSubtype() const{
    return "exclusive";
}

//---------------------------------------------------------- ViolentMask

class ViolentMaskSkill: public TriggerSkill{
public:
    ViolentMaskSkill():TriggerSkill("violent_mask"){
        frequency = Compulsory;
        events << Predamage << PhaseChange;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasArmorEffect(objectName()) && target->getGeneralName() == "tamtam";
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *tamtam, QVariant &data) const{

        Room *room = tamtam->getRoom();
        
        if(event == Predamage) {
            DamageStruct damage = data.value<DamageStruct>();
            
            room->playSkillEffect("mianju", 2);
            
            LogMessage log;
            log.type = "#ViolentMask";
            log.from = tamtam;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);

        }else if(event == PhaseChange && tamtam->getPhase() == Player::Discard) {
        
            if(tamtam->hasRelic("shaman_totem")) {
                
                LogMessage log;
                log.type = "#ShamanTotemEffect";
                log.from = tamtam;
                room->sendLog(log);
            
                return false;
            }
            
            int overflow = tamtam->getHandcardNum() - tamtam->getMaxCards();
            
            if(overflow>0) {
                
                room->playSkillEffect("mianju", 3);
                
                LogMessage log;
                log.type = "#ViolentMaskDiscard";
                log.from = tamtam;
                room->sendLog(log);
            
                for(int i=0; i<overflow; i++) {
                    const Card *random_card = tamtam->getRandomHandCard();
                
                    log.type = "$DiscardCard";
                    log.from = tamtam;
                    log.card_str = random_card->toString();
                    room->sendLog(log);
                    
                    room->throwCard(random_card);
                }
            }
        }else if(event == PhaseChange && tamtam->getPhase() == Player::Draw) {
            if(room->testRandomEvent(tamtam, "violent_mask", 50)) {
                tamtam->drawCards(1);
            }
        }
        
        return false;
    }
};

ViolentMask::ViolentMask(Suit suit, int number)
    :Armor(suit, number)
{
    setObjectName("violent_mask");
    skill = new ViolentMaskSkill;
}

QString ViolentMask::getSubtype() const{
    return "exclusive";
}

//---------------------------------------------------------- ShamanTotem

ShamanTotem::ShamanTotem(Suit suit, int number)
    :Relic(suit, number)
{
    setObjectName("shaman_totem");
}

QString ShamanTotem::getSubtype() const{
    return "exclusive";
}
//----------------------------------------------------------end

ExclusiveCardPackage::ExclusiveCardPackage()
    :Package("exclusivecard")
{   
    
    QList<Card *> cards;
    
    cards 
        << new Chaoxiu(Card::Club, 0)
        << new ViolentMask(Card::Diamond, 0)
        << new ShamanTotem(Card::Heart, 0)
    ;
          
    foreach(Card *card, cards) {
        card->setExclusive(true);
        card->setParent(this);
    }

    type = CardPack;
}

ADD_PACKAGE(ExclusiveCard)
