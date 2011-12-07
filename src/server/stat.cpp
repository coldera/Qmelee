#include "engine.h"
#include "clientplayer.h"
#include "client.h"
#include "carditem.h"
#include "stat.h"

StatRule::StatRule(QObject *parent) : GameRule(parent) {
}

bool StatRule::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const {
    Room *room = player->getRoom();
    QString general_name = player->getGeneralName();
    
    switch(event) {
        case GameStart: {
            room->insertStat(general_name, "appearance", 1);
            break;
        }
        
        case CardUsed: {
            if(data.canConvert<CardUseStruct>()){
                CardUseStruct card_use = data.value<CardUseStruct>();
                const Card *card = card_use.card;

                QString item;
                
                if(card->inherits("Slash"))
                    item = "slash";
                else if(card->inherits("HolyWater"))
                    item = "holywater";
                else if(card->inherits("Schnapps"))
                    item = "schnapps";
                else if(card->isNDTrick())
                    item = "trick";
                else if(card->inherits("DelayedTrick"))
                    item = "delay";
                else if(card->inherits("Weapon"))
                    item = "weapon";
                else if(card->inherits("Armor"))
                    item = "armor";
                    
                room->insertStat(general_name, item, 1);
                
                if(card->inherits("Slash") || card->inherits("PK") || card->inherits("Burn") 
                || (card->inherits("AOE") && !card->inherits("Roar"))) {
                    room->insertStat(general_name, "offensive", 1);
                    
                    foreach(ServerPlayer *p, card_use.to){
                        room->insertStat(p->getGeneralName(), "resistance", 1);
                    }
                }else if((card->inherits("DelayedTrick") && !card->inherits("Disaster")) || card->inherits("Roar")) {
                    room->insertStat(general_name, "obstruct", 1);
                }
            }
            
            break;
        }
        
        case CardEffected:{
            
            if(data.canConvert<CardEffectStruct>()){
                CardEffectStruct effect = data.value<CardEffectStruct>();
                    
                const Card *card = effect.card;

                if(card->inherits("Grab") || card->inherits("DestroyAll")) {
                    QString role1 = player->getRole();
                    QString role2 = effect.from->getRole();
                    if(role1 == role2 || (role1 == "lord" && role2 == "loyalist") || (role1 == "loyalist" && role2 == "lord")) {
                        room->insertStat(effect.from->getGeneralName(), "assist", 1);
                    }else {
                        room->insertStat(effect.from->getGeneralName(), "obstruct", 1);
                    }

                }
            }
            
            break;
        }
        
        case SlashMissed:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            
            if(effect.from) {
                room->insertStat(effect.from->getGeneralName(), "miss", 1);
            }
            
            break;
        }
        
        case DamageDone: {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(damage.from) {
                room->insertStat(damage.from->getGeneralName(), "offensive", damage.damage*2);
                room->insertStat(damage.from->getGeneralName(), "damage", damage.damage);
            }
            room->insertStat(general_name, "damaged", damage.damage);
            room->insertStat(general_name, "resistance", damage.damage*2);
            
            break;
        }
        
        case HpLost:{
            int lose = data.toInt();
            
            room->insertStat(general_name, "resistance", lose*2);
            
            break;
        }
        
        case HpRecover:{
            RecoverStruct recover = data.value<RecoverStruct>();
            
            if(recover.who && player != recover.who) {
                int num = qMin(recover.recover, player->getMaxHP()-player->getHp());
                
                room->insertStat(recover.who->getGeneralName(), "assist", num*2);
                room->insertStat(recover.who->getGeneralName(), "cure", num);
            }
            break;
        }
        
        case GameOverJudge: {
        
            room->insertStat(general_name, "death", 1);            
            DamageStar damage = data.value<DamageStar>();
            
            if(damage && damage->from && damage->from != player) {
                room->insertStat(damage->from->getGeneralName(), "kill", 1);
            }
        
            QString winner = getWinner(player);
            if(!winner.isNull()){
                room->updateTotalStat();
                
                LogMessage log;
                log.type = "#StatSaved";
                room->sendLog(log);
            }
            
            break;
        }
        
        default:
            break;
    }

    return GameRule::trigger(event, player, data);
}
