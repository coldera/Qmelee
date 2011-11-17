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

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        Room *room = player->getRoom();

        if(!effect.to->isNude() && player->askForSkillInvoke(objectName(), data)){
            int num = effect.to->getCards("e").length()+1;
            
            LogMessage log;
            log.type = "$ChaoxiuSilie";
            log.to << effect.to;
            
            while(!effect.to->isNude() && (num--)) {
                int card_id = room->askForCardChosen(player, effect.to, "he", objectName());

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
//----------------------------------------------------------end

ExclusiveCardPackage::ExclusiveCardPackage()
    :Package("exclusivecard")
{   
    
    QList<Card *> cards;
    
    cards 
        << new Chaoxiu(Card::Club, 0)
    ;
          
    foreach(Card *card, cards) {
        card->setExclusive(true);
        card->setParent(this);
    }

    type = CardPack;
}

ADD_PACKAGE(ExclusiveCard)
