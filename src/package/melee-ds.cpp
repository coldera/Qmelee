#include "melee-ds.h"

#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "settings.h"

// skill cards

//--------------------------------------------------------------------------------------------------------------demitri

//----------------------------------------------------------------------------- Zizun

class Zizun: public TriggerSkill{
public:
    Zizun():TriggerSkill("zizun"){
        frequency = Compulsory;
        events << Damaged << Damage << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) || target->getPhase() == Player::Start;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *demitri, QVariant &data) const{
    
        Room *room = demitri->getRoom();
        
        ServerPlayer *target = demitri;
        if(event == PhaseChange && target->getPhase() == Player::Start) {
            foreach(ServerPlayer *p, room -> getAllPlayers()){
                if(p->hasSkill(objectName())) {
                    room->setPlayerFlag(p, "-zizun_draw");
                    room->setPlayerFlag(p, "-zizun_drawed");
                    room->setPlayerFlag(p, "-zizun_mp");
                    room->setPlayerFlag(p, "-zizun_mp_down");
                    return false;
                }
            }
        }

        //critical skills are invoked in start phase
        if(!demitri->hasSkill(objectName()))
            return false;

        if(event == Damaged && !demitri->hasFlag("zizun_drawed")) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(demitri->hasFlag("zizun_draw") || (!demitri->hasFlag("zizun_draw") && damage.damage>1)) {
                LogMessage log;
                log.type = "#ZizunEffect1";
                log.from = demitri;
                room->sendLog(log);
            
                demitri->drawCards(1);
                
                room->setPlayerFlag(demitri, "-zizun_draw");
                room->setPlayerFlag(demitri, "zizun_drawed");
                
                room->playSkillEffect(objectName(), 1);
                
            }else if(damage.damage == 1) {
                room->setPlayerFlag(demitri, "zizun_draw");
            }            
        }else if(event == Damage) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(demitri->hasFlag("zizun_mp") || (!demitri->hasFlag("zizun_mp") && damage.damage>1)) {
                LogMessage log;
                log.type = "#ZizunEffect2";
                log.from = demitri;
                room->sendLog(log);
            
                demitri->updateMp(-1);
                
                room->setPlayerFlag(demitri, "-zizun_mp");
                room->setPlayerFlag(demitri, "zizun_mp_down");
                
                room->playSkillEffect(objectName(), 2);
                
            }else if(damage.damage == 1) {
                room->setPlayerFlag(demitri, "zizun_mp");
            } 
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Wuye

class Wuye: public PhaseChangeSkill{
public:
    Wuye():PhaseChangeSkill("wuye"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
            && target->getPhase() == Player::Start
            && target->getMp() >= 1;
    }

    virtual bool onPhaseChange(ServerPlayer *demitri) const{
        Room *room = demitri->getRoom();
        
        if(room->askForSkillInvoke(demitri, objectName())){
            
            demitri->updateMp(-1);
            
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade|club):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = demitri;
            
            room->judge(judge);
            
            if(judge.isGood()) {
            
                LogMessage log;
                log.type = "#WuyeOn";
                log.from = demitri;
                room->sendLog(log);  
            
                room->setPlayerFlag(demitri,"wuye_on");
            }
            
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Xixie

class Xixie: public TriggerSkill{
public:
    Xixie():TriggerSkill("xixie"){
        frequency = Compulsory;
        events << Damage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->isWounded();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *demitri, QVariant &data) const{    
    
        Room *room = demitri->getRoom();        
        DamageStruct damage = data.value<DamageStruct>();
        
        if(!damage.card || (!damage.card->inherits("Slash") && !damage.card->inherits("PK")))
            return false;

        int dp = damage.damage;
        
        if(demitri->hasFlag("wuye_on")) {
            dp = dp*2;
        }else {
            if(demitri->distanceTo(damage.to) > 1) 
                return false;
        }
        
        RecoverStruct recover;
        recover.recover = dp;
        room->recover(demitri, recover);
        
        if(damage.to->getGeneral()->isFemale()) {
            if(demitri->getMp()) 
                demitri->updateMp(-1);
                
            room->playSkillEffect(objectName(), 2);
        }else {
            room->playSkillEffect(objectName(), 1);
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------morrigan

//----------------------------------------------------------------------------- Haosheng

class Haosheng: public TriggerSkill{
public:
    Haosheng():TriggerSkill("haosheng"){
        frequency = Compulsory;
        events << Damage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *morrigan, QVariant &data) const{
    
        Room *room = morrigan->getRoom();
        
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.to->getGeneral()->isMale()) {
            room->playSkillEffect(objectName());
            
            LogMessage log;
            log.type = "#Haosheng";
            log.from = morrigan;
            room->sendLog(log);
            
            morrigan->drawCards(1);
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Feixing

class Feixing: public DistanceSkill{
public:
    Feixing():DistanceSkill("feixing") {}

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->hasSkill(objectName()))
            return +1;
        else
            return 0;
    }
};

//----------------------------------------------------------------------------- Huanying

class HuanyingSkill: public TriggerSkill{
public:
    HuanyingSkill():TriggerSkill("#huanying-on"){
        frequency = Compulsory;
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasFlag("huanying_on") && !target->hasFlag("using_huanying_card");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *morrigan, QVariant &data) const{    
    
        Room *room = morrigan->getRoom();
    
        CardUseStruct card_use = data.value<CardUseStruct>();
        CardStar card = card_use.card;
        
        if(card->inherits("BasicCard") || card->isNDTrick()) {
        
            if(card_use.to.length() && card_use.to.first()->getCards("hej").length()<=1 && (card->inherits("Grab") || card->inherits("DestroyAll"))) {
                return false;
            }
        
            room->setPlayerFlag(morrigan, "using_huanying_card");
            
            LogMessage log;
            log.type = "$HuanyingEffect";
            log.from = morrigan;
            log.card_str = card->toString();
            room->sendLog(log);
            
            CardUseStruct new_card_use;
            new_card_use.card = card;
            new_card_use.from = card_use.from;
            new_card_use.to = card_use.to;
            room->useCard(new_card_use);
            
            room->getThread()->delay();
            room->setPlayerFlag(morrigan, "-using_huanying_card");
        }

        return false;
    }
};

class Huanying: public PhaseChangeSkill{
public:
    Huanying():PhaseChangeSkill("huanying"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
            && target->getPhase() == Player::Start
            && target->getMp() >= 6;
    }

    virtual bool onPhaseChange(ServerPlayer *morrigan) const{
        Room *room = morrigan->getRoom();
        
        if(room->askForSkillInvoke(morrigan, objectName())){
            room->broadcastInvoke("animate", "huanying");
            
            morrigan->updateMp(-6);
            
            room->setPlayerFlag(morrigan, "huanying_on");
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------victor

//----------------------------------------------------------------------------- Guzhi

class Guzhi: public PhaseChangeSkill{
public:
    Guzhi():PhaseChangeSkill("guzhi"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
            && target->getPhase() == Player::Start
            && target->getMp() >= 1;
    }

    virtual bool onPhaseChange(ServerPlayer *victor) const{
        Room *room = victor->getRoom();
        
        if(room->askForSkillInvoke(victor, objectName())){
            
            victor->updateMp(-1);
            
            room->setPlayerFlag(victor, "can_not_unassailable");
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Hecheng

class Hecheng: public TriggerSkill{
public:
    Hecheng():TriggerSkill("hecheng"){
        frequency = Compulsory;
        events << Predamaged << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *victor, QVariant &data) const{
    
        Room *room = victor->getRoom();        
        
        if(event == Predamage) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(damage.nature != DamageStruct::Normal) {            

                damage.nature = DamageStruct::Thunder;
                data = QVariant::fromValue(damage);
                
                LogMessage log;
                log.type = "#DamageToThunder";
                log.to << damage.to;
                log.arg = objectName();
                room->sendLog(log);    
            
            }
            
        }else if(event == Predamaged) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(damage.nature != DamageStruct::Normal) {            

                damage.nature = DamageStruct::Normal;
                data = QVariant::fromValue(damage);
                
                LogMessage log;
                log.type = "#DamageToNormal";
                log.to << damage.to;
                log.arg = objectName();
                room->sendLog(log);    
            
            }
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------sasquatch

//----------------------------------------------------------------------------- Xueqiu

class XueqiuRange: public GameStartSkill{
public:
    XueqiuRange():GameStartSkill("#xueqiu-range"){}

    virtual void onGameStart(ServerPlayer *sasquatch) const{
        sasquatch->getRoom()->setPlayerMark(sasquatch, "range_add_one", 1);
    }
};

class Xueqiu: public TriggerSkill{
public:
    Xueqiu():TriggerSkill("xueqiu"){
        frequency = Compulsory;
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *sasquatch, QVariant &data) const{
    
        Room *room = sasquatch->getRoom();        
        
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.card->inherits("Bang")) {

            damage.nature = DamageStruct::Ice;
            data = QVariant::fromValue(damage);
            
            LogMessage log;
            log.type = "#DamageToIce";
            log.to << damage.to;
            log.arg = objectName();
            room->sendLog(log);    
        
        }
            
        return false;
    }
};

//----------------------------------------------------------------------------- Bingzhu

BingzhuCard::BingzhuCard(){
    once = true;
    will_throw = true;
    setObjectName("ice_bang");
}

bool BingzhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    return true;
}

void BingzhuCard::use(Room *room, ServerPlayer *sasquatch, const QList<ServerPlayer *> &targets) const{
    sasquatch->updateMp(-2);
    room->throwCard(this);
    
    IceBang *iceBang = new IceBang(Card::NoSuit, 0);

    LogMessage log;
    log.type = "$BingzhuEffect";
    log.from = sasquatch;
    log.to << targets;
    room->sendLog(log);    
    
    iceBang->use(room, sasquatch, targets);
}

class Bingzhu: public ViewAsSkill{
public:
    Bingzhu():ViewAsSkill("bingzhu"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("BingzhuCard") && player->getMp()>=2 && player->getHandcardNum()>=2;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped() && selected.length() < 2;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        BingzhuCard *card = new BingzhuCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------aulbath

//----------------------------------------------------------------------------- Chaoxi

class Luochao: public TriggerSkill{
public:
    Luochao():TriggerSkill("#luochao"){
        events << Predamaged;
        frequency = Compulsory;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *aulbath, QVariant &data) const{
    
        aulbath->getRoom()->playSkillEffect("chaoxi", 2);
        
        LogMessage log;
        log.type = "#DamageUp";
        log.to << aulbath;
        log.arg = "luochao";
        log.arg2 = "1";
        aulbath->getRoom()->sendLog(log);
        
        DamageStruct damage = data.value<DamageStruct>();        
        damage.damage = damage.damage+1;
        data = QVariant::fromValue(damage);

        return false;
    }
};

class Zhangchao: public TriggerSkill{
public:
    Zhangchao():TriggerSkill("#zhangchao"){
        events << Predamaged;
        frequency = Compulsory;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *aulbath, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.nature != DamageStruct::Ice) {
        
            aulbath->getRoom()->playSkillEffect("chaoxi", 1);
            
            LogMessage log;
            log.type = "#DamageNullify";
            log.to << damage.to;
            log.arg = "zhangchao";
            aulbath->getRoom()->sendLog(log);
            
            return true;
        }

        return false;
    }
};

class Chaoxi: public TriggerSkill{
public:
    Chaoxi():TriggerSkill("chaoxi"){
        events << TurnStart;
        frequency = Compulsory;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->isLord();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        
        Room *room = player->getRoom();
        ServerPlayer *aulbath = NULL;
        foreach(ServerPlayer *p, room -> getAllPlayers()){
            if(p->hasSkill("chaoxi")) {
                aulbath = p;
                break;
            }
        }
        
        if(aulbath) {
            if(aulbath->hasSkill("#zhangchao")) {
                room->detachSkillFromPlayer(aulbath, "#zhangchao");
                room->acquireSkill(aulbath, "#luochao");
                room->setPlayerMark(aulbath, "no_handcard_limit", 0);
                aulbath->gainMark("@luochao");
                if(aulbath->getMark("@zhangchao"))
                    aulbath->loseMark("@zhangchao");
            }else {
                room->detachSkillFromPlayer(aulbath, "#luochao");
                room->acquireSkill(aulbath, "#zhangchao");
                room->setPlayerMark(aulbath, "no_handcard_limit", 1);
                aulbath->gainMark("@zhangchao");
                if(aulbath->getMark("@luochao"))
                    aulbath->loseMark("@luochao");
            }
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Zuanji

ZuanjiCard::ZuanjiCard(){
    target_fixed = true;
}

void ZuanjiCard::use(Room *room, ServerPlayer *aulbath, const QList<ServerPlayer *> &) const{
    aulbath->updateMp(-2);
    room->setPlayerFlag(aulbath, "one_more_bang");
    room->setPlayerFlag(aulbath, "no_range_limit");
}

class Zuanji: public ZeroCardViewAsSkill{
public:
    Zuanji():ZeroCardViewAsSkill("zuanji"){}
    
    virtual bool isEnabledAtPlay(const Player *aulbath) const{
        return !aulbath->hasUsed("ZuanjiCard") && aulbath->getMark("@zhangchao") && aulbath->getMp()>=2;
    }
    
    virtual const Card *viewAs() const{
        ZuanjiCard *card = new ZuanjiCard;
        card->setSkillName(objectName());
        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------qbee

//----------------------------------------------------------------------------- Shihun

class Shihun: public TriggerSkill{
public:
    Shihun():TriggerSkill("shihun"){
        events << Damaged;
        frequency = Compulsory;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        Room *room = target->getRoom();

        ServerPlayer *qbee = NULL;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->hasSkill("shihun")) {
                qbee = p;
                break;
            }
        }
        
        return qbee && target!=qbee && qbee->distanceTo(target) <= 1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        ServerPlayer *qbee = NULL;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->hasSkill("shihun")) {
                qbee = p;
                break;
            }
        }
        
        LogMessage log;
        log.type = "#Shihun";
        log.from = qbee;
        qbee->getRoom()->sendLog(log);
        
        DamageStruct damage = data.value<DamageStruct>();        
        qbee->updateMp(damage.damage);

        return false;
    }
};

//----------------------------------------------------------------------------- Rongjie

RongjieCard::RongjieCard(){
}

bool RongjieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(!to_select->hasEquip())
        return false;
        
    if(!Self->inMyAttackRange(to_select))
        return false;
        
    return true;
}

void RongjieCard::onEffect(const CardEffectStruct &effect) const {
    Room *room = effect.from->getRoom();

    effect.from->updateMp(-3);
    int card_id = room->askForCardChosen(effect.from, effect.to, "e", "rongjie");      

    LogMessage log;
    log.type = "$RongjieDiscard";
    log.to << effect.to;
    log.card_str = Sanguosha->getCard(card_id)->toString();
    room->sendLog(log);
    
    room->throwCard(card_id);
}

class Rongjie: public ZeroCardViewAsSkill{
public:
    Rongjie():ZeroCardViewAsSkill("rongjie"){}
    
    virtual bool isEnabledAtPlay(const Player *qbee) const{
        return qbee->getMp()>=3;
    }
    
    virtual const Card *viewAs() const{
        RongjieCard *card = new RongjieCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Fanyan

class FanyanSkill: public TriggerSkill{
public:
    FanyanSkill():TriggerSkill("#fanyan-protect"){
        events << Predamaged;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@bee");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *qbee, QVariant &data) const{
        
        Room *room = qbee->getRoom();

        if(room->askForSkillInvoke(qbee, objectName(), data)){
            qbee->loseMark("@bee");
            
            DamageStruct damage = data.value<DamageStruct>();
            damage.damage = damage.damage-1;
            data = QVariant::fromValue(damage);            
        }

        return false;
    }
};

class Fanyan: public PhaseChangeSkill{
public:
    Fanyan():PhaseChangeSkill("fanyan"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
            && target->getPhase() == Player::Start
            && target->getMp() >= 6;
    }

    virtual bool onPhaseChange(ServerPlayer *qbee) const{
        
        if(qbee->getRoom()->askForSkillInvoke(qbee, objectName())){
            qbee->updateMp(-6);
            
            qbee->gainMark("@bee");
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------pyron

//----------------------------------------------------------------------------- Zhengfu

class Yanbao: public OneCardViewAsSkill{
public:
    Yanbao():OneCardViewAsSkill("yanbao"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("Yanbao") && Slash::IsAvailable(player);
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        FireBang *firebang = new FireBang(card->getSuit(), card->getNumber());
        firebang->addSubcard(card->getId());
        firebang->setSkillName(objectName());
        return firebang;
    }
};

class Huimie: public OneCardViewAsSkill{
public:
    Huimie():OneCardViewAsSkill("huimie"){}

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        DestroyAll *dismantlement = new DestroyAll(card->getSuit(), card->getNumber());
        dismantlement->addSubcard(card->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};


class ZhengfuEnd: public PhaseChangeSkill{
public:
    ZhengfuEnd():PhaseChangeSkill("#zhengfu-end"){
        frequency = Compulsory;        
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Finish;
    }

    virtual bool onPhaseChange(ServerPlayer *pyron) const{
        
        Room *room = pyron->getRoom();
            
        if(pyron->hasSkill("huimie"))
            room->detachSkillFromPlayer(pyron, "huimie");
            
        if(pyron->hasSkill("yanbao"))
            room->detachSkillFromPlayer(pyron, "yanbao");
        
        if(pyron->getMark("no_handcard_limit"))
            room->setPlayerMark(pyron, "no_handcard_limit", 0);
        
        return false;
    }
};

class Zhengfu: public PhaseChangeSkill{
public:
    Zhengfu():PhaseChangeSkill("zhengfu"){
        frequency = Frequent;        
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *pyron) const{
        
        Room *room = pyron->getRoom();

        JudgeStruct judge;
        judge.pattern = QRegExp("(.*)");
        judge.good = true;
        judge.reason = objectName();
        judge.who = pyron;            
        room->judge(judge);
        
        switch(judge.card->getSuit()){
            case Card::Spade: {
                room->playSkillEffect(objectName(), 1);
                room->acquireSkill(pyron, "huimie");
                break;
            }
            case Card::Heart: {
                room->playSkillEffect(objectName(), 2);
                room->acquireSkill(pyron, "yanbao");
                break;
            }
            case Card::Club:
            case Card::Diamond: {
                room->playSkillEffect(objectName(), 3);
                room->setPlayerMark(pyron, "no_handcard_limit", 1);
                break;
            }
            default: {}
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Shenyu

ShenyuCard::ShenyuCard(){
    target_fixed = true;
    will_throw = false;
}

void ShenyuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{}

class ShenyuViewAsSkill:public OneCardViewAsSkill{
public:
    ShenyuViewAsSkill():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@shenyu";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new ShenyuCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Shenyu: public TriggerSkill{
public:
    Shenyu():TriggerSkill("shenyu"){
        view_as_skill = new ShenyuViewAsSkill;
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) 
            && target->getPhase() == Player::NotActive 
            && !target->isKongcheng() 
            && target->getMp()>0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *pyron, QVariant &data) const{
        Room *room = pyron->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@shenyu-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        pyron->tag["Judge"] = data;
        const Card *card = room->askForCard(pyron, "@shenyu", prompt);

        if(card){
            
            pyron->updateMp(-1);
            
            room->throwCard(judge->card);

            judge->card = Sanguosha->getCard(card->getEffectiveId());
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = pyron;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Yanzhilingyu

class Yanzhilingyu: public TriggerSkill{
public:
    Yanzhilingyu():TriggerSkill("yanzhilingyu$"){
        can_forbid = false;
        events << Predamaged;
    }
    
    virtual int getPriority() const{
        return -1;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->getLord()->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        
        Room *room = player->getRoom();
        
        if(room->testRandomEvent(player, objectName(), 20)) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(damage.nature != DamageStruct::Fire) {

                damage.nature = DamageStruct::Fire;
                data = QVariant::fromValue(damage);
                
                LogMessage log;
                log.type = "#DamageToFire";
                log.to << damage.to;
                log.arg = objectName();
                room->sendLog(log);
            
            }
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------zabal

//----------------------------------------------------------------------------- Yaogun

YaogunCard::YaogunCard(){
    mute = true;
    target_fixed = true;
    once = true;
}

void YaogunCard::use(Room *room, ServerPlayer *zabal, const QList<ServerPlayer *> &) const{
    int x = qMax(room->getAllPlayers().length(), 4);
    
    if(x > zabal->getMp()) {
        
        LogMessage log;
        log.type = "#YaogunFail";
        log.from = zabal;
        room->sendLog(log);
        
        return;
        
    }else {
        
        room->playSkillEffect("yaogun");
        
        zabal->updateMp(-x);
    }
    
    foreach(ServerPlayer *p, room->getOtherPlayers(zabal)){
    
        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(heart):(.*)");
        judge.good = false;
        judge.reason = objectName();
        judge.who = p;
        room->judge(judge);

        if(judge.isBad()){            
            LogMessage log;
            log.type = "#YaogunEffect";
            log.to << p;
            room->sendLog(log);            
        
            room->loseHp(p, 1);
            
            RecoverStruct recover;
            room->recover(zabal, recover);
        }

    }
    
}

class Yaogun: public ZeroCardViewAsSkill{
public:
    Yaogun():ZeroCardViewAsSkill("yaogun"){}
    
    virtual bool isEnabledAtPlay(const Player *zabal) const{
        return !zabal->hasUsed("YaogunCard") && zabal->getMp()>=4;
    }
    
    virtual const Card *viewAs() const{
        YaogunCard *card = new YaogunCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Tusha

class Tusha: public TriggerSkill{
public:
    Tusha():TriggerSkill("tusha"){
        frequency = Compulsory;
        events << Damage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zabal, QVariant &data) const{
    
        DamageStruct damage = data.value<DamageStruct>();
        
        LogMessage log;
        log.type = "#TushaEffect";
        log.from = zabal;
        log.arg = objectName();
        zabal->getRoom()->sendLog(log);
        
        zabal->updateMp(damage.damage*6);
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------lilith

//----------------------------------------------------------------------------- Lieqi

LieqiCard::LieqiCard(){}

bool LieqiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(to_select->getHandcardNum()<=0)
        return false;
        
    return true;
}

void LieqiCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer *lilith = effect.from;
    Room *room = lilith->getRoom();

    lilith->updateMp(-1);
    
    Card::Suit suit1 = room->askForSuit(lilith, objectName());

    LogMessage log;
    log.type = "#ChoiceSuit";
    log.from = lilith;
    log.arg = Card::Suit2String(suit1);
    room->sendLog(log);
    
    int card_id = room->askForCardChosen(lilith, effect.to, "h", objectName());
    room->showCard(effect.to, card_id);
    Card::Suit suit2 = Sanguosha->getCard(card_id)->getSuit();
    
    if(suit1 == suit2){
        room->obtainCard(lilith, card_id);
        //only for ai
        room->setTag("LieqiSuit", ".");
    }else {
        //only for ai
        room->setTag("LieqiSuit", Card::Suit2String(suit2));
    }
}

class Lieqi: public ZeroCardViewAsSkill{
public:
    Lieqi():ZeroCardViewAsSkill("lieqi"){}
    
    virtual bool isEnabledAtPlay(const Player *lilith) const{
        return lilith->getMp()>0;
    }
    
    virtual const Card *viewAs() const{
        LieqiCard *card = new LieqiCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Guisu

class Guisu: public TriggerSkill{
public:
    Guisu():TriggerSkill("guisu"){
        events << Death;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("guisu");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *lilith, QVariant &data) const{
        
        Room *room = lilith->getRoom();

        if(room->askForSkillInvoke(lilith, objectName(), data)){
            Room *room = lilith->getRoom();
            QList<ServerPlayer *> players = room->getOtherPlayers(lilith);

            if(players.isEmpty())
                return false;
                
            ServerPlayer *target;
            target = room->askForPlayerChosen(lilith, players, objectName());
            
            target->drawCards(5);            
        }

        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------leilei

//----------------------------------------------------------------------------- Anqi

class AnqiBuff:public SlashBuffSkill{
public:
    AnqiBuff():SlashBuffSkill("#anqi-buff"){}

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *leilei = effect.from;

        Room *room = leilei->getRoom();
        
        if(leilei->hasFlag("anqi_using")) {
            
            room->setPlayerFlag(leilei, "-anqi_using");

            if(room->testRandomEvent(leilei, "anqi", 50)) {
                room->slashResult(effect, NULL);
                return true;
            }
        }
        
        return false;
    }
};

AnqiCard::AnqiCard(){}

bool AnqiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(Self->distanceTo(to_select) > 4)
        return false;

    return true;
}

void AnqiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *leilei = card_use.from;
    const Card *card = Sanguosha->getCard(getSubcards().first());    
    
    Bang *bang = new Bang(card->getSuit(), card->getNumber());
    bang->setSkillName("anqi");
    
    room->setPlayerFlag(leilei, "anqi_using");
    
    CardUseStruct use;
    use.card = bang;
    use.from = leilei;
    use.to << card_use.to;

    room->useCard(use);
    
    room->throwCard(this);
}

class Anqi: public OneCardViewAsSkill{
public:
    Anqi():OneCardViewAsSkill("anqi"){}
    
    virtual bool isEnabledAtPlay(const Player *leilei) const{
        return Slash::IsAvailable(leilei);
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        AnqiCard *card = new AnqiCard;
        card->setSkillName(objectName());
        card->addSubcard(card_item->getCard()->getId());
        
        return card;
    }
};

//----------------------------------------------------------------------------- Fanxiang

FanxiangCard::FanxiangCard() {}

bool FanxiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(Self == to_select)
        return false;

    return Self->distanceTo(to_select) <= 1;
}

void FanxiangCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->setPlayerFlag(effect.to, "fanxiang_target");
}

class FanxiangViewAsSkill: public ZeroCardViewAsSkill{
public:
    FanxiangViewAsSkill():ZeroCardViewAsSkill("fanxiang"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@fanxiang";
    }
    
    virtual const Card *viewAs() const{
        FanxiangCard *card = new FanxiangCard;
        card->setSkillName(objectName());
        return card;
    }
};

class Fanxiang: public TriggerSkill{
public:
    Fanxiang():TriggerSkill("fanxiang"){
        view_as_skill = new FanxiangViewAsSkill;
        events << CardEffected;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase()== Player::NotActive && target->getMp()>=1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *leilei, QVariant &data) const{

        CardEffectStruct effect = data.value<CardEffectStruct>();
        ServerPlayer *card_user = effect.from;
        
        if(card_user == leilei || !(effect.card->inherits("BasicCard") || effect.card->isNDTrick()))
            return false;
        
        Room *room = leilei->getRoom();
        
        QString prompt = QString("@fanxiang:%1").arg(effect.card->getId());
        if(room->askForUseCard(leilei, "@@fanxiang", prompt)){
            
            leilei->updateMp(-1);
            
            foreach(ServerPlayer *p, room->getOtherPlayers(leilei)){
                if(p->hasFlag("fanxiang_target")){
                    room->setPlayerFlag(p, "-fanxiang_target");
                    effect.to = p;

                    room->cardEffect(effect);
                    
                    p->drawCards(1);
                    
                    return true;
                }
            } 
        }     

        return false;
    }
};

//----------------------------------------------------------------------------- Lingfu

class LingfuOff: public PhaseChangeSkill{
public:
    LingfuOff():PhaseChangeSkill("#lingfu-off"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom(); 
        
        foreach(ServerPlayer *p, room->getAllPlayers()){
            int times = p->getMark("lingfu_on");
            if(times==1) {
                room->allSkillRelease(p);
            }
            
            if(times>0) {
                int rest = times-1;                
                room->setPlayerMark(p, "lingfu_on", rest);
                
                if(rest>0) {
                    LogMessage log;
                    log.type = "#CountDown";
                    log.from = p;
                    log.arg = QString::number(rest);
                    log.arg2 = "skill_forbid";
                    room->sendLog(log);
                }
            }
        }        
       
        return false;
    }
};

LingfuCard::LingfuCard(){
    once = true;
}

bool LingfuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty()
    || to_select == Self
    || to_select->getMark("@skill_forbid")) {        
        return false;
    }
        
    return true;
}

void LingfuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    
    room->throwCard(this);
    
    ServerPlayer *target = effect.to;
    ServerPlayer *leilei = effect.from;
    
    leilei->updateMp(-2);
    
    room->allSkillForbid(target);
    
    room->setPlayerMark(target, "lingfu_on", room->getAllPlayers().length());
    
}

class Lingfu: public OneCardViewAsSkill{
public:
    Lingfu():OneCardViewAsSkill("lingfu"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LingfuCard") && player->getMp()>=2 && player->getHandcardNum()>=1;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LingfuCard *card = new LingfuCard;
        card->setSkillName(objectName());
        card->addSubcard(card_item->getCard()->getId());
        
        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------phobos

//----------------------------------------------------------------------------- Huwei

class Huwei: public TriggerSkill{
public:
    Huwei():TriggerSkill("huwei"){
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        Room *room = target->getRoom();

        ServerPlayer *phobos = NULL;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->hasSkill("huwei")) {
                phobos = p;
                break;
            }
        }
        
        return phobos && target!=phobos && phobos->getPhase() == Player::NotActive && phobos->getMp()>=2;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        
        Room *room = player->getRoom();

        ServerPlayer *phobos = NULL;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->hasSkill("huwei")) {
                phobos = p;
                break;
            }
        }
        
        DamageStruct damage = data.value<DamageStruct>();
        
        if(phobos->askForSkillInvoke(objectName(), data)) {
            phobos->updateMp(-2);
            
            LogMessage log;
            log.type = "#Huwei";
            log.from = phobos;
            log.to << damage.to;
            room->sendLog(log);
            
            damage.to = phobos;
            room->damage(damage);
            
            return true;
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Jijia

class Jijia: public TriggerSkill{
public:
    Jijia():TriggerSkill("jijia"){
        frequency = Compulsory;
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *phobos, QVariant &data) const{
        
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.nature == DamageStruct::Normal || damage.nature == DamageStruct::Poison) {
            
            LogMessage log;
            log.type = "#DamageNullify";
            log.to << damage.to;
            log.arg = objectName();
            phobos->getRoom()->sendLog(log);
            
            return true;
        }

        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------donovan

//----------------------------------------------------------------------------- Huoshen

class Huoshen: public OneCardViewAsSkill{
public:
    Huoshen():OneCardViewAsSkill("huoshen"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("Huoshen") && Slash::IsAvailable(player) && player->getHp()<3 && player->getMp()<=3;
    }
    
    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash" && player->getHp()<3 && player->getMp()<=3;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        FireBang *firebang = new FireBang(card->getSuit(), card->getNumber());
        firebang->addSubcard(card->getId());
        firebang->setSkillName(objectName());
        return firebang;
    }
};

//----------------------------------------------------------------------------- Leishen

LeishenCard::LeishenCard(){
    target_fixed = true;
}

void LeishenCard::use(Room *room, ServerPlayer *donovan, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    room->setPlayerFlag(donovan, "leishen_on");
}

class LeishenViewAsSkill: public OneCardViewAsSkill{
public:
    LeishenViewAsSkill():OneCardViewAsSkill("leishen"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasFlag("leishen_on");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        if(Self->hasFlag("leishen_on")) {
            return to_select->getCard()->inherits("ThunderBang") 
                or (to_select->getCard()->getSuit() == Card::Diamond && to_select->getCard()->inherits("Bang"));
        }else {
            return !to_select->isEquipped();
        }
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@leishen";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        if(Self->hasFlag("leishen_on")) {
            const Card *card = card_item->getCard();
            
            if(card->inherits("ThunderBang")) {
                return card;
            }else {
                ThunderBang *thunderbang = new ThunderBang(card->getSuit(), card->getNumber());
                thunderbang->addSubcard(card->getId());
                thunderbang->setSkillName(objectName());
                return thunderbang;
            }
        }else {
            LeishenCard *leishen = new LeishenCard;
            leishen->addSubcard(card_item->getCard());
            
            return leishen;
        }
    }
};

class Leishen: public PhaseChangeSkill{
public:
    Leishen():PhaseChangeSkill("leishen"){
        view_as_skill = new LeishenViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMp() > 3
                && target->getHandcardNum()>0;
    }

    virtual bool onPhaseChange(ServerPlayer *donovan) const{
        Room *room = donovan->getRoom();
        
       room->askForUseCard(donovan, "@@leishen", "@leishen");
        
        return false;
    }
};

//-----------------------------------------------------------------------------Xueshen

XueshenCard::XueshenCard(){
    once = true;
    setObjectName("ice_bang");
}

bool XueshenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void XueshenCard::use(Room *room, ServerPlayer *donovan, const QList<ServerPlayer *> &targets) const{
    donovan->updateMp(-1);

    const Card *card = Sanguosha->getCard(getSubcards().first());
    
    IceBang *iceBang = new IceBang(card->getSuit(), card->getNumber());

    LogMessage log;
    log.type = "$XueshenEffect";
    log.from = donovan;
    log.to << targets;
    room->sendLog(log);    
    
    iceBang->use(room, donovan, targets);
    
    donovan->getRoom()->throwCard(this);
}

class Xueshen: public OneCardViewAsSkill{
public:
    Xueshen():OneCardViewAsSkill("xueshen"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("XueshenCard") && player->getMp()>4 && Slash::IsAvailable(player) && player->getHandcardNum()>0;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Club && to_select->getCard()->inherits("Bang");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XueshenCard *card = new XueshenCard;
        card->setSkillName(objectName());
        card->addSubcard(card_item->getCard()->getId());
        
        return card;
    }
};

//-----------------------------------------------------------------------------Anita

class AttackAnita: public TriggerSkill{
public:
    AttackAnita():TriggerSkill("#attack-anita"){
        can_forbid = false;
        events << CardEffected;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@anita");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *donovan, QVariant &data) const{
        Room *room = donovan->getRoom();

        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("Slash")){
            QStringList choices;        
            choices << "donovan" << "anita"; 
            
            QString choice = room->askForChoice(effect.from, "attack_anita", choices.join("+"));
            
            if(choice == "anita") {
                
                LogMessage log;
                log.type = "#AttackingAnita";
                log.from = effect.from;
                room->sendLog(log);                
            
                bool injured = true;
                
                if(!donovan->getPile("anita_card").isEmpty()) {
                    
                    QList<int> card_ids;
                    foreach(int card_id, donovan->getPile("anita_card")){
                        if(Sanguosha->getCard(card_id)->inherits("Dodge") || Sanguosha->getCard(card_id)->inherits("HolyWater")) {
                            card_ids << card_id;
                        }
                    }
                    
                    if(card_ids.length()>0) {
                    
                        int ai_delay = Config.AIDelay;
                        Config.AIDelay = 0;
                    
                        room->fillAG(card_ids, donovan);
                        int card_id = room->askForAG(donovan, card_ids, true, "anita_dodge");
                        
                        if(card_id != -1){
                            room->throwCard(card_id);
                            
                            LogMessage log;
                            log.type = "$AnitaCancelAttack";
                            log.from = effect.from;
                            log.card_str = Sanguosha->getCard(card_id)->toString();
                            room->sendLog(log);
                            
                            injured = false;
                        }
                        
                        Config.AIDelay = ai_delay;
                        
                        donovan->invoke("clearAG");
                    }
                    
                }
                
                if(injured) {
                    LogMessage log;
                    log.type = "#AnitaInjured";
                    log.from = effect.from;
                    room->sendLog(log);
                
                    donovan->loseMark("@anita");
                    
                    if(!donovan->getMark("@anita")) {
                    
                        LogMessage log;
                        log.type = "#AnitaDead";
                        log.from = effect.from;
                        room->sendLog(log);
                    
                        if(!donovan->getPile("anita_card").isEmpty()) {
                            const QList<int> anita_cards = donovan->getPile("anita_card");
                            foreach(int card_id, anita_cards){
                                room->throwCard(card_id);
                            }
                        }
                    
                        room->detachSkillFromPlayer(donovan, "anita");
                        room->detachSkillFromPlayer(donovan, "anita_anwei");
                        room->detachSkillFromPlayer(donovan, "anita_xisheng");
                    }
                }
                
                return true;
            }
        }

        return false;
    }
};

AnitaCard::AnitaCard(){
    target_fixed = true;
}

void AnitaCard::use(Room *room, ServerPlayer *donovan, const QList<ServerPlayer *> &) const{
    
    donovan->gainMark("@anita", 2);
    room->acquireSkill(donovan, "anita_anwei");
    room->acquireSkill(donovan, "anita_xisheng");
    room->setPlayerMark(donovan, "anita_called", 1);
    
    room->loseMaxHp(donovan);
}

class Anita: public ZeroCardViewAsSkill{
public:
    Anita():ZeroCardViewAsSkill("anita"){
        can_forbid = false;
    }
    
    virtual bool isEnabledAtPlay(const Player *donovan) const{
        return donovan->getMark("anita_called") <= 0;
    }
    
    virtual const Card *viewAs() const{
        AnitaCard *card = new AnitaCard;
        card->setSkillName(objectName());
        return card;
    }
};

//-----------------------------------------------------------------------------AnitaAnwei

class AnitaAnwei: public TriggerSkill{
public:
    AnitaAnwei():TriggerSkill("anita_anwei"){
        can_forbid = false;
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && (target->getPhase() == Player::Start || target->getPhase() == Player::Discard);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *donovan, QVariant &data) const{
        Room *room = donovan->getRoom();
    
        if(donovan->getPhase() == Player::Start) {
            if(donovan->getMp()>0 && !donovan->getPile("anita_card").isEmpty()) {
                if(room->askForSkillInvoke(donovan, objectName())) {
                    room->playSkillEffect(objectName(), 2);
                
                    donovan->updateMp(-1);
                
                    const QList<int> anita_cards = donovan->getPile("anita_card");
                    foreach(int card_id, anita_cards){
                        room->moveCardTo(Sanguosha->getCard(card_id), donovan, Player::Hand, false);
                    }
                }
            } 
        }else if(donovan->getPhase() == Player::Discard && donovan->getHandcardNum()) {
            const Card *first_card = room->askForCard(donovan, ".touse", "@card_to_anita", data);
            if(first_card) {
                donovan->addToPile("anita_card", first_card->getEffectiveId(), false);
                
                if(donovan->getHandcardNum()) {
                    const Card *second_card = room->askForCard(donovan, ".touse", "@card_to_anita", data);
                    if(second_card) {
                        donovan->addToPile("anita_card", second_card->getEffectiveId(), false);
                    }
                }
            }
        }

        return false;
    }
};

//-----------------------------------------------------------------------------AnitaXisheng

class AnitaXisheng: public TriggerSkill{
public:
    AnitaXisheng():TriggerSkill("anita_xisheng"){
        can_forbid = false;
        events << AskForPeaches;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@anita");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *donovan, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if(dying_data.who != donovan)
            return false;

        Room *room = donovan->getRoom();
        if(donovan->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            
            room->broadcastInvoke("animate", "anita:3000");
            
            room->setPlayerProperty(donovan, "hp", 2);
            
            if(!donovan->getPile("anita_card").isEmpty()) {
                const QList<int> anita_cards = donovan->getPile("anita_card");
                foreach(int card_id, anita_cards){
                    room->moveCardTo(Sanguosha->getCard(card_id), donovan, Player::Hand, false);
                }
            }
            
            donovan->loseMark("@anita", donovan->getMark("@anita"));
            
            room->detachSkillFromPlayer(donovan, "anita");
            room->detachSkillFromPlayer(donovan, "anita_anwei");
            room->detachSkillFromPlayer(donovan, "anita_xisheng");
        }

        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------jedah

//----------------------------------------------------------------------------- Jiushu

class JiushuOff: public TriggerSkill{
public:
    JiushuOff():TriggerSkill("#jiushu-off"){
        frequency = Compulsory;
        events << PhaseChange << Death;
    }

    virtual bool triggerable(const ServerPlayer *jedah) const{
        return jedah->hasSkill("jiushu");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *jedah, QVariant &data) const{
        Room *room = jedah->getRoom();
        
        if(event == PhaseChange && jedah->getPhase()== Player::Start) {
            foreach(ServerPlayer *p, jedah->getRoom()->getOtherPlayers(jedah)){
                int times = p->getMark("jiushu_on");
                if(times==1) {
                    room->killPlayer(p);
                }
                
                if(times>0) {
                    int rest = times-1;
                    
                    room->setPlayerMark(p, "jiushu_on", rest);
                    
                    if(rest>0) {
                        LogMessage log;
                        log.type = "#JiushuOn";
                        log.from = p;
                        log.arg = QString::number(rest);
                        room->sendLog(log);
                    }
                }
            }        
        }else if(event == Death) {
            foreach(ServerPlayer *p, room->getAlivePlayers()){
                if(p->getMark("jiushu_on")) {
                    room->setPlayerMark(p, "jiushu_on", 0);
                    room->killPlayer(p);
                }
            }
        }
        
        return false;
    }
};

JiushuCard::JiushuCard(){
    target_fixed = true;
}

void JiushuCard::use(Room *room, ServerPlayer *jedah, const QList<ServerPlayer *> &) const{
    jedah->updateMp(-26);
    
    ServerPlayer *dead = NULL;
    QStringList deadPlayersName;
    
    foreach(ServerPlayer *target, room->getServerPlayers()){
        if(target->isDead()) {
            dead = target;
            deadPlayersName << target->getGeneralName();
        }
    }
       
    if(deadPlayersName.length() > 1) {
        QString choice = room->askForChoice(jedah, "jiushu-player", deadPlayersName.join("+"));
        dead = room->findPlayer(choice, true);
    }
    
    if(dead) {
        dead->gainMark("@jiushu");
        room->setPlayerMark(dead, "jiushu_on", 3);
        
        if(dead->getMark("@revive_rite"))
            dead->loseMark("@revive_rite");
        
        room->revivePlayer(dead);
        room->setPlayerProperty(dead, "hp", 2);
        dead->drawCards(4);
        
        QString role;
        if(jedah->isLord()) {
            role = "loyalist";
        }else if(jedah->getRole() == "renegade" && room->getMode() == "06_3v3") {
            role = "rebel";
        }else {
            role = jedah->getRole();
        }
        
        room->setPlayerProperty(dead, "role", role);
        
        LogMessage log;
        log.type = "#JiushuEffect";
        log.from = jedah;
        log.to << dead;
        room->sendLog(log);        
    }

}

class Jiushu: public ZeroCardViewAsSkill{
public:
    Jiushu():ZeroCardViewAsSkill("jiushu"){}
    
    virtual bool isEnabledAtPlay(const Player *jedah) const{
        QList<const Player *> players = jedah->parent()->findChildren<const Player *>();
        foreach(const Player *p, players){
            if(p->isDead())
                return jedah->getMp()>=26;
        }

        return false;
    }
    
    virtual const Card *viewAs() const{
        JiushuCard *card = new JiushuCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Qiyue

QiyueCard::QiyueCard(){}

bool QiyueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->getMark("@jiushu")<=0)
        return false;

    return true;
}

void QiyueCard::use(Room *room, ServerPlayer *jedah, const QList<ServerPlayer *> &targets) const{
    room->setPlayerFlag(targets.first(), "jiushu_qiyue");
    if(!jedah->hasSkill("zhipei")) {
        room->loseMaxHp(jedah);
        room->acquireSkill(jedah, "zhipei");
    }
}

class QiyueViewAsSkill: public ZeroCardViewAsSkill{
public:
    QiyueViewAsSkill():ZeroCardViewAsSkill("qiyue"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@qiyue";
    }
    
    virtual const Card *viewAs() const{
        QiyueCard *card = new QiyueCard;
        card->setSkillName(objectName());
        return card;
    }
};

class Qiyue: public PhaseChangeSkill{
public:
    Qiyue():PhaseChangeSkill("qiyue"){
        view_as_skill = new QiyueViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Start) {
            QList<const Player *> players = target->parent()->findChildren<const Player *>();
            foreach(const Player *p, players){
                if(p->getMark("@jiushu")) 
                    return true;
            }
        }
        
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *jedah) const{
        Room *room = jedah->getRoom();
        
        if(room->askForUseCard(jedah, "@@qiyue", "@qiyue")){
            room->playSkillEffect(objectName());
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Zhipei

ZhipeiGiveCard::ZhipeiGiveCard(){
    will_throw = false;
    target_fixed = true;
}

void ZhipeiGiveCard::use(Room *room, ServerPlayer *jedah, const QList<ServerPlayer *> &targets) const {
    
    ServerPlayer *target = NULL;
    foreach(ServerPlayer *p, room->getOtherPlayers(jedah)) {
        if(p->hasFlag("jiushu_qiyue")) {
            target = p;
            break;
        }
    }  
    
    if(target) {
        foreach(int card_id, getSubcards())
            room->moveCardTo(Sanguosha->getCard(card_id), target, Player::Hand, false);
    }
}

class ZhipeiGive: public ViewAsSkill{
public:
    ZhipeiGive():ViewAsSkill("zhipeigive"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        ZhipeiGiveCard *card = new ZhipeiGiveCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        return card;
    }
};

ZhipeiCard::ZhipeiCard(){
    target_fixed = true;
}

void ZhipeiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *target = NULL;
    ServerPlayer *jedah = card_use.from;
    
    foreach(ServerPlayer *p, room->getOtherPlayers(jedah)) {
        if(p->hasFlag("jiushu_qiyue")) {
            target = p;
            break;
        }
    }
    
    if(target) {
        if(target->getHandcardNum()) {
            DummyCard *all_cards = target->wholeHandCards();
            room->moveCardTo(all_cards, jedah, Player::Hand, false);
            delete all_cards;
        }
        if(target->isAlive())
            room->askForUseCard(jedah, "@zhipeigive", "@zhipei-give");
    }
}

class Zhipei: public ZeroCardViewAsSkill{
public:
    Zhipei():ZeroCardViewAsSkill("zhipei"){}
    
    virtual bool isEnabledAtPlay(const Player *jedah) const{
        QList<const Player *> players = jedah->parent()->findChildren<const Player *>();
        foreach(const Player *p, players){
            if(p->hasFlag("jiushu_qiyue"))
                return true;
        }

        return false;
    }
    
    virtual const Card *viewAs() const{
        ZhipeiCard *card = new ZhipeiCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Sizhilingyu

class Sizhilingyu: public TriggerSkill{
public:
    Sizhilingyu():TriggerSkill("sizhilingyu$"){
        can_forbid = false;
        events << Death;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->getLord()->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        
        Room *room = player->getRoom();
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer){
            
            if(room->testRandomEvent(killer, objectName(), 80)) {
                
                LogMessage log;
                log.type = "#Sizhilingyu";
                log.from = killer;
                room->sendLog(log);
                
                room->setPlayerProperty(killer, "hp", killer->getMaxHP());
            }
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------anakaris

//----------------------------------------------------------------------------- Guanwu

class GuanwuOff: public PhaseChangeSkill{
public:
    GuanwuOff():PhaseChangeSkill("#guanwu-off"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom(); 
        
        foreach(ServerPlayer *p, room->getAllPlayers()){
            int times = p->getMark("guanwu_on");
            if(times==1) {
                p->loseMark("@invincible");
            }
            
            if(times>0) {
                int rest = times-1;                
                room->setPlayerMark(p, "guanwu_on", rest);
                
                if(rest>0) {
                    LogMessage log;
                    log.type = "#CountDown";
                    log.from = p;
                    log.arg = QString::number(rest);
                    log.arg2 = "invincible";
                    room->sendLog(log);
                }
            }
        }        
       
        return false;
    }
};

GuanwuCard::GuanwuCard(){
    once = true;
}

bool GuanwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->getMark("@invincible"))
        return false;

    return true;
}

void GuanwuCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *anakaris = effect.from;
    Room *room = anakaris->getRoom();

    anakaris->updateMp(-5);
    
    effect.to->turnOver();
    effect.to->gainMark("@invincible");
    room->setPlayerMark(effect.to, "guanwu_on", room->getAllPlayers().length());
}

class Guanwu: public ZeroCardViewAsSkill{
public:
    Guanwu():ZeroCardViewAsSkill("guanwu"){}
    
    virtual bool isEnabledAtPlay(const Player *anakaris) const{
        return !anakaris->hasUsed("GuanwuCard") && anakaris->getMp()>=5;
    }
    
    virtual const Card *viewAs() const{
        GuanwuCard *card = new GuanwuCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Zhicai

class ZhicaiOff: public TriggerSkill{
public:
    ZhicaiOff():TriggerSkill("#zhicai-off"){
        frequency = Compulsory;
        events << Damaged << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start || target->getMark("zhicai_on");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *source, QVariant &data) const{
        Room *room = source->getRoom();
        
        if(event == PhaseChange && source->getPhase() == Player::Start) {
            foreach(ServerPlayer *p, room->getAllPlayers()){
                int times = p->getMark("zhicai_on");
                if(times==1) {
                    p->loseMark("@card_forbid");
                }
                
                if(times>0) {
                    int rest = times-1;                
                    room->setPlayerMark(p, "zhicai_on", rest);
                    
                    if(rest>0) {
                        LogMessage log;
                        log.type = "#CountDown";
                        log.from = p;
                        log.arg = QString::number(rest);
                        log.arg2 = "card_forbid";
                        room->sendLog(log);
                    }
                }
            } 
        }else if(event == Damaged && source->getMark("zhicai_on")) {
            source->loseMark("@card_forbid");
            room->setPlayerMark(source, "zhicai_on", 0);
            
            LogMessage log;
            log.type = "#ZhicaiOffByDamage";
            log.from = source;
            room->sendLog(log);
            
        }
        
        return false;
    }
};

ZhicaiCard::ZhicaiCard(){
    once = true;
}

bool ZhicaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->getMark("@card_forbid"))
        return false;

    return true;
}

void ZhicaiCard::use(Room *room, ServerPlayer *anakaris, const QList<ServerPlayer *> &targets) const {
    
    ServerPlayer *target = targets.first();
    
    anakaris->updateMp(-18);
    
    target->gainMark("@card_forbid");
    room->setPlayerMark(target, "zhicai_on", room->getAllPlayers().length()*3);
    
}

class Zhicai: public ZeroCardViewAsSkill{
public:
    Zhicai():ZeroCardViewAsSkill("zhicai"){}
    
    virtual bool isEnabledAtPlay(const Player *anakaris) const{
        return !anakaris->hasUsed("ZhicaiCard") && anakaris->getMp()>=18;
    }
    
    virtual const Card *viewAs() const{
        ZhicaiCard *card = new ZhicaiCard;
        card->setSkillName(objectName());
        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------bulleta

//----------------------------------------------------------------------------- Baijin

class Baijin:public DrawCardsSkill{
public:
    Baijin():DrawCardsSkill("baijin"){}

    virtual int getDrawNum(ServerPlayer *bulleta, int n) const{
        Room *room = bulleta->getRoom();
        if(bulleta->getMp()) {
            if(room->askForSkillInvoke(bulleta, objectName())){            
                bulleta->updateMp(-1);
                
                room->playSkillEffect(objectName());
                
                return n + 1;
            }
        }
        
        return n;
    }
};

//----------------------------------------------------------------------------- Shoulie

class Shoulie: public TriggerSkill{
public:
    Shoulie():TriggerSkill("shoulie"){
        events << CardLost;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        
        CardMoveStar move = data.value<CardMoveStar>();
        const Card *card = Sanguosha->getCard(move->card_id);
        if(!card->inherits("Horse") && !card->inherits("Armor")){
            return false;
        }
        
        if(move->to_place == Player::DiscardedPile && (move->from_place == Player::Equip || move->from_place == Player::Hand)) {
            ServerPlayer *bulleta = room->findPlayerBySkillName(objectName());
            if(!bulleta || bulleta->getMp()<=0)
                return false;
            
            if(bulleta->askForSkillInvoke(objectName(), data)){
                bulleta->updateMp(-1);
                room->playSkillEffect(objectName());
                bulleta->obtainCard(card);
            }
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Youbu

YoubuCard::YoubuCard(){
    once = true;
}

void YoubuCard::use(Room *room, ServerPlayer *bulleta, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    int card_id = bulleta->getRandomHandCardId();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target, objectName());

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    target->obtainCard(card);
    room->showCard(target, card_id);

    if(card->getSuit() != suit){
        room->playSkillEffect(objectName(), 1);
        room->loseHp(target, 1);
    }else {
        room->playSkillEffect(objectName(), 2);
    }
    
}

class Youbu:public ZeroCardViewAsSkill{
public:
    Youbu():ZeroCardViewAsSkill("youbu"){}

    virtual bool isEnabledAtPlay(const Player *bulleta) const{
        return bulleta->getHandcardNum()>0 && !bulleta->hasUsed("YoubuCard");
    }

    virtual const Card *viewAs() const{
        return new YoubuCard;
    }
};

//--------------------------------------------------------------------------------------------------------------bishamon

//----------------------------------------------------------------------------- Banruo

class Banruo: public GameStartSkill{
public:
    Banruo():GameStartSkill("banruo"){
        frequency = Compulsory;
    }

    virtual void onGameStart(ServerPlayer *bishamon) const{
        bishamon->gainMark("@banruo");
    }
};

//----------------------------------------------------------------------------- Guiyan

class GuiyanOff: public PhaseChangeSkill{
public:
    GuiyanOff():PhaseChangeSkill("#guiyan-off"){}

    virtual bool triggerable(const ServerPlayer *bishamon) const{
        return PhaseChangeSkill::triggerable(bishamon) && bishamon->getPhase() == Player::Finish;
    }

    virtual bool onPhaseChange(ServerPlayer *bishamon) const{
        Room *room = bishamon->getRoom(); 
        
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->getMark("guiyan_on")) {
                room->setPlayerMark(p, "guiyan_on", 0);
                p->loseMark("@card_forbid");

                break;
            }
        }        
       
        return false;
    }
};

GuiyanCard::GuiyanCard(){
    once = true;
}

bool GuiyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    
    if(to_select == Self)
        return false;

    if(to_select->getMark("@card_forbid"))
        return false;

    return true;
}

void GuiyanCard::use(Room *room, ServerPlayer *bishamon, const QList<ServerPlayer *> &targets) const{
    bishamon->updateMp(-8);
    
    ServerPlayer *target = targets.first();
    
    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(spade|club):(.*)");
    judge.good = false;
    judge.reason = objectName();
    judge.who = target;
    
    room->judge(judge);
    
    if(judge.isBad()) {    
        target->gainMark("@card_forbid");
        room->setPlayerMark(target, "guiyan_on", 1);
    }else {
        LogMessage log;
        log.type = "#GuiyanFail";
        log.from = bishamon;
        room->sendLog(log);        
    }

}

class Guiyan: public ZeroCardViewAsSkill{
public:
    Guiyan():ZeroCardViewAsSkill("guiyan"){}
    
    virtual bool isEnabledAtPlay(const Player *bishamon) const{
        return !bishamon->hasUsed("GuiyanCard") && bishamon->getMp()>=8;
    }
    
    virtual const Card *viewAs() const{
        GuiyanCard *card = new GuiyanCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Yuanhun

YuanhunCard::YuanhunCard(){
    once = true;
    target_fixed = true;
}

void YuanhunCard::use(Room *room, ServerPlayer *bishamon, const QList<ServerPlayer *> &) const{
    bishamon->updateMp(-20);
    
    room->broadcastInvoke("animate", "yuanhun");
    
    if(bishamon->isWounded()) {
        room->setPlayerProperty(bishamon, "hp", bishamon->getMaxHP());
    }
    
    if(bishamon->getHandcardNum()<bishamon->getMaxHP()) {
        bishamon->drawCards(bishamon->getMaxHP()-bishamon->getHandcardNum());
    }
    
    room->loseMaxHp(bishamon);
}

class Yuanhun: public ZeroCardViewAsSkill{
public:
    Yuanhun():ZeroCardViewAsSkill("yuanhun"){}
    
    virtual bool isEnabledAtPlay(const Player *bishamon) const{
        return !bishamon->hasUsed("YuanhunCard") && bishamon->getMp()>=20;
    }
    
    virtual const Card *viewAs() const{
        YuanhunCard *card = new YuanhunCard;
        card->setSkillName(objectName());
        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------felicia

//----------------------------------------------------------------------------- Leyuan

LeyuanCard::LeyuanCard(){}

bool LeyuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{       
    if(targets.length()>=3)
        return false;

    return true;
}

void LeyuanCard::onEffect(const CardEffectStruct &effect) const {
    RecoverStruct recover;
    const Card *card = Sanguosha->getCard(getSubcards().first());
    
    if(card->inherits("Cure")) {
        recover.recover = 2;
    }
    
    effect.from->getRoom()->recover(effect.to, recover);
}

class Leyuan: public OneCardViewAsSkill{
public:
    Leyuan():OneCardViewAsSkill("leyuan"){}
    
    virtual bool isEnabledAtPlay(const Player *felicia) const{
        return felicia->getHandcardNum();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("HolyWater") || to_select->getFilteredCard()->inherits("Cure");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LeyuanCard *card = new LeyuanCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Huhuan

class Huhuan:public TriggerSkill{
public:
    Huhuan():TriggerSkill("huhuan"){
        events << CardAsked;
        default_choice = "ignore";
    }

    virtual bool triggerable(const ServerPlayer *felicia) const{
        return TriggerSkill::triggerable(felicia) && felicia->getMp()>0;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *felicia, QVariant &data) const{
        QStringList prompt = data.toString().split(":");
        QString pattern = prompt.at(0);
        if(pattern != "jink" && pattern != "slash")
            return false;

        Room *room = felicia->getRoom();
        QList<ServerPlayer *> helpers = room->getOtherPlayers(felicia);
        if(helpers.isEmpty())
            return false;

        if(!room->askForSkillInvoke(felicia, objectName()))
            return false;

        room->playSkillEffect(objectName());
        
        QVariant tohelp = QVariant::fromValue((PlayerStar)felicia);
        
        int ai_delay = Config.AIDelay;
        Config.AIDelay = 0;
        const Card *card = NULL;
        
        foreach(ServerPlayer *p, helpers){
            const Card *card = room->askForCard(p, pattern, "@huhuan-card:" + felicia->objectName(), tohelp);
            if(card){
                felicia->updateMp(-1);
                p->drawCards(1);
                room->provide(card);
                break;              
            }
        }
        
        Config.AIDelay = ai_delay;
        if(card) return true;        
        
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------gallon

//----------------------------------------------------------------------------- Manyue

class ManyueDraw:public DrawCardsSkill{
public:
    ManyueDraw():DrawCardsSkill("#manyue-draw"){}

    virtual int getDrawNum(ServerPlayer *gallon, int n) const{
        
        if(gallon->getMark("manyue_on")) {
            return n+2;
        }
        
        return n;
    }
};

class Manyue: public PhaseChangeSkill{
public:
    Manyue():PhaseChangeSkill("manyue"){}
    
    virtual int getPriority() const{
        return -1;
    }    

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
            && ((target->getPhase() == Player::Start && target->getMp() >= 1)
            or (target->getPhase() == Player::Finish && target->getMark("manyue_on")));
            
    }

    virtual bool onPhaseChange(ServerPlayer *gallon) const{
        Room *room = gallon->getRoom();
        
        if(gallon->getPhase() == Player::Start) {
            if(room->askForSkillInvoke(gallon, objectName())){
                
                gallon->updateMp(-1);
                
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = gallon;
                
                room->judge(judge);
                
                if(judge.isGood()) {
                
                    room->playSkillEffect(objectName());
                
                    LogMessage log;
                    log.type = "#ManyueOn";
                    log.from = gallon;
                    room->sendLog(log);  
                
                    room->setPlayerMark(gallon,"manyue_on", 1);
                    room->setPlayerMark(gallon, "no_handcard_limit", 1);
                    
                    gallon->turnOver();
                }else {
                    LogMessage log;
                    log.type = "#ManyueFail";
                    log.from = gallon;
                    room->sendLog(log);   
                }
                
            }
        }else {
            room->setPlayerMark(gallon,"manyue_on", 0);
            room->setPlayerMark(gallon, "no_handcard_limit", 0);
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Huolong

class HuolongEnable: public SlashBuffSkill{
public:
    HuolongEnable():SlashBuffSkill("#huolong-enable"){}

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *gallon = effect.from;
        Room *room = gallon->getRoom();

        if(effect.slash->isRed()) {
            if(gallon->hasFlag("huolong_on")) {
            
                LogMessage log;
                log.type = "$HuolongBang";
                log.from = gallon;
                log.card_str = effect.slash->toString();
                room->sendLog(log);                
                
                room->setPlayerFlag(gallon,"-huolong_on");
            }else {
                room->setPlayerFlag(gallon,"huolong_on");
            }
        }

        return false;
    }
};

class Huolong: public OneCardViewAsSkill{
public:
    Huolong():OneCardViewAsSkill("huolong"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasFlag("huolong_on");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isRed() && to_select->getFilteredCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        return card_item->getCard();
    }
};

//--------------------------------------------------------------------------------------------------------------End

MeleeDSPackage::MeleeDSPackage()
    :Package("meleeds")
{
    General *demitri, *morrigan, *victor, *sasquatch, *aulbath, *qbee, *pyron, *zabal, *lilith, *leilei, *phobos, *donovan, *jedah, *anakaris, *bulleta, *bishamon, *felicia, *gallon;
    
    demitri = new General(this, "demitri", "nu");
    demitri->addSkill(new Zizun);
    demitri->addSkill(new Wuye);
    demitri->addSkill(new Xixie);
    
    morrigan = new General(this, "morrigan", "ling", 3, false);
    morrigan->addSkill(new Haosheng);
    morrigan->addSkill(new Feixing);
    morrigan->addSkill(new Huanying);
    morrigan->addSkill(new HuanyingSkill);
    related_skills.insertMulti("huanying", "#huanying-on");
    
    
    victor = new General(this, "victor", "nu");
    victor->addSkill(new Guzhi);
    victor->addSkill(new Hecheng);
    
    sasquatch = new General(this, "sasquatch", "nu");
    sasquatch->addSkill(new Xueqiu);
    sasquatch->addSkill(new XueqiuRange);
    related_skills.insertMulti("xueqiu", "#xueqiu-range");
    sasquatch->addSkill(new Bingzhu);
    
    addMetaObject<BingzhuCard>();
    
    aulbath = new General(this, "aulbath", "qi");
    aulbath->addSkill(new Chaoxi);
    aulbath->addSkill(new Zuanji);
    
    skills << new Zhangchao << new Luochao;
    related_skills.insertMulti("xhaoxi", "#zhangchao");
    related_skills.insertMulti("xhaoxi", "#luochao");
    
    addMetaObject<ZuanjiCard>();
    
    qbee = new General(this, "qbee", "kuang", 3, false);
    qbee->addSkill(new Shihun);
    qbee->addSkill(new Rongjie);
    qbee->addSkill(new Fanyan);
    qbee->addSkill(new FanyanSkill);
    related_skills.insertMulti("fanyan", "#fanyan-protect");
    
    addMetaObject<RongjieCard>();
    
    pyron = new General(this, "pyron$", "ling", 3);
    pyron->addSkill(new Zhengfu);
    pyron->addSkill(new ZhengfuEnd);
    related_skills.insertMulti("zhengfu", "#zhengfu-end");
    pyron->addSkill(new Shenyu);
    pyron->addSkill(new Yanzhilingyu);
    
    skills << new Huimie << new Yanbao;
    
    addMetaObject<ShenyuCard>();

    zabal = new General(this, "zabal", "yuan", 5);
    zabal->addSkill(new Yaogun);
    zabal->addSkill(new Tusha);
    
    addMetaObject<YaogunCard>();
    
    lilith = new General(this, "lilith", "ling", 4, false);
    lilith->addSkill(new Lieqi);
    lilith->addSkill(new Guisu);
    
    addMetaObject<LieqiCard>();
    
    leilei = new General(this, "leilei", "ling", 3, false);
    leilei->addSkill(new Anqi);
    leilei->addSkill(new AnqiBuff);
    related_skills.insertMulti("anqi", "#anqi-buff");
    leilei->addSkill(new Fanxiang);
    leilei->addSkill(new Lingfu);
    leilei->addSkill(new LingfuOff);
    related_skills.insertMulti("lingfu", "#lingfu-off");
    
    addMetaObject<AnqiCard>();
    addMetaObject<FanxiangCard>();
    addMetaObject<LingfuCard>();
    
    phobos = new General(this, "phobos", "qi");
    phobos->addSkill(new Huwei);
    phobos->addSkill(new Jijia);

    phobos->setGender(General::Neuter);
    
    donovan = new General(this, "donovan", "ling");
    donovan->addSkill(new Huoshen);
    donovan->addSkill(new Leishen);
    donovan->addSkill(new Xueshen);
    
    addMetaObject<LeishenCard>();
    addMetaObject<XueshenCard>();
    
    donovan->addSkill(new Anita);
    donovan->addSkill(new AttackAnita);
    related_skills.insertMulti("anita", "#attack-anita");
    skills << new AnitaAnwei << new AnitaXisheng;
    
    addMetaObject<AnitaCard>();
    
    jedah = new General(this, "jedah$", "yuan", 5);
    jedah->addSkill(new Jiushu);
    jedah->addSkill(new JiushuOff);
    related_skills.insertMulti("jiushu", "#jiushu-off");
    jedah->addSkill(new Qiyue);
    jedah->addSkill(new Sizhilingyu);
    
    skills << new Zhipei << new ZhipeiGive;
    
    addMetaObject<JiushuCard>();
    addMetaObject<QiyueCard>();
    addMetaObject<ZhipeiCard>();
    addMetaObject<ZhipeiGiveCard>();
    
    anakaris = new General(this, "anakaris", "yuan");
    anakaris->addSkill(new Guanwu);
    anakaris->addSkill(new GuanwuOff);
    related_skills.insertMulti("guanwu", "#guanwu-off");
    anakaris->addSkill(new Zhicai);
    anakaris->addSkill(new ZhicaiOff);
    related_skills.insertMulti("zhicai", "#zhicai-off");
    
    addMetaObject<GuanwuCard>();
    addMetaObject<ZhicaiCard>();
    
    bulleta = new General(this, "bulleta", "kuang", 3, false);
    bulleta->addSkill(new Baijin);
    bulleta->addSkill(new Shoulie);
    bulleta->addSkill(new Youbu);
    
    addMetaObject<YoubuCard>();
    
    bishamon = new General(this, "bishamon", "yuan");
    bishamon->addSkill(new Banruo);
    bishamon->addSkill(new Guiyan);
    bishamon->addSkill(new GuiyanOff);
    related_skills.insertMulti("guiyan", "#guiyan-off");
    bishamon->addSkill(new Yuanhun);
    
    addMetaObject<GuiyanCard>();
    addMetaObject<YuanhunCard>();
    
    felicia = new General(this, "felicia", "qi", 3, false);
    felicia->addSkill(new Leyuan);
    felicia->addSkill(new Huhuan);
    
    addMetaObject<LeyuanCard>();
    
    gallon = new General(this, "gallon", "nu");
    gallon->addSkill(new Manyue);
    gallon->addSkill(new ManyueDraw);
    related_skills.insertMulti("manyue", "#manyue-draw");
    gallon->addSkill(new Huolong);
    gallon->addSkill(new HuolongEnable);
    related_skills.insertMulti("huolong", "#huolong-enable");
}

ADD_PACKAGE(MeleeDS);


