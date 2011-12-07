#include "melee-sf.h"
#include "meleecard.h"
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
    mute = true;
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
    
    gouki->updateMp(-18);
    
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
    
        room->playSkillEffect("shunyu");
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
    
    room->playSkillEffect("longjuan");
    
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
        room->askForUseCard(ryu, "@@longjuan", "@longjuan");

        return false;
    }
};

//----------------------------------------------------------------------------- Bodong

BodongCard::BodongCard(){
    will_throw = true;
    mute = true;
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
	
    effect.from->updateMp(-3);
    
    room->playSkillEffect("bodong");
    room->broadcastInvoke("animate", "bodong");
    
    int dp = 3;

    if(room->askForCard(effect.to, "jink", "@bodong-jink:" + effect.from->objectName()))
        dp--;
    if(room->askForCard(effect.to, "jink", "@bodong-jink-more:" + effect.from->objectName()))
        dp--;
    if(room->askForCard(effect.to, "jink", "@bodong-jink-more:" + effect.from->objectName()))
        dp--;
    
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
            ryu->skip(Player::Play);
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------ken

//----------------------------------------------------------------------------- Jifeng

class JifengEnable: public SlashBuffSkill{
public:
    JifengEnable():SlashBuffSkill("#jifeng-enable"){}

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *ken = effect.from;
        Room *room = ken->getRoom();

        if(ken->distanceTo(effect.to) <= 1) {
            if(!effect.to->hasFlag("jifenged")) {
                room->setPlayerFlag(effect.to, "jifenged");                
            }
            
            if(ken->getMark("jifeng")) {
                room->setPlayerFlag(ken, "-jifeng_on");
            }else {
                room->setPlayerFlag(ken, "jifeng_on");
            }
            
        }

        return false;
    }
};

JifengCard::JifengCard(){
    mute = true;
    will_throw = true;
}

bool JifengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(to_select->hasFlag("jifenged") && Self->getMark("jifeng")==2)
        return false;
        
    if(!to_select->hasFlag("jifenged") && Self->getMark("jifeng")==1)
        return false;
        
    if(Self->distanceTo(to_select)>1) 
        return false;

    return true;
}

void JifengCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *ken = card_use.from;
    
    CardUseStruct use;
    use.card = Sanguosha->getCard(getSubcards().first());
    use.from = ken;
    use.to << card_use.to;
    
    LogMessage log;
    log.type = "#JifengBang";
    log.from = ken;
    room->sendLog(log);

    room->useCard(use);
}

class JifengViewAsSkill: public OneCardViewAsSkill{
public:
    JifengViewAsSkill():OneCardViewAsSkill("jifeng"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@jifeng";
    }
    
    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Slash") && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JifengCard *card = new JifengCard;
        card->addSubcard(card_item->getFilteredCard());
        card->setSkillName(objectName());
        return card;
    }
};

class Jifeng: public TriggerSkill{
public:
    Jifeng():TriggerSkill("jifeng"){
        view_as_skill = new JifengViewAsSkill;    
        events << CardFinished;
    }

    virtual int getPriority() const{
        return 2;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Play && 
            TriggerSkill::triggerable(target) && 
            target->hasFlag("jifeng_on") &&
            target->getHandcardNum();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *ken, QVariant &data) const{
        Room *room = ken->getRoom();
        
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Slash")) {
        
            QStringList choices;
            choices << "JifengEffect1" << "JifengEffect2" << "NoChoice"; 
            
            QString choice = room->askForChoice(ken, objectName(), choices.join("+"));
            
            if(choice == "JifengEffect1") {
                room->setPlayerMark(ken, "jifeng", 1);
            }else if(choice == "JifengEffect2") {
                room->setPlayerMark(ken, "jifeng", 2);
            }
            
            if(choice!="NoChoice") {
                room->playSkillEffect(objectName());
            
                while(room->askForUseCard(ken, "@@jifeng", "@jifeng") && choice!="JifengEffect1")
                    ;//Empty loop
                    
                room->setPlayerMark(ken, "jifeng", 0);
                
                foreach(ServerPlayer *p, room->getOtherPlayers(ken)){
                    if(p->hasFlag("jifenged")){
                        room->setPlayerFlag(p, "-jifenged");
                    }
                } 
            }
        
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Shenglong

ShenglongCard::ShenglongCard(){
    will_throw = true;
}

bool ShenglongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(Self->distanceTo(to_select) > Self->getAttackRange()) 
        return false;

    return true;
}

void ShenglongCard::onEffect(const CardEffectStruct &effect) const {
    
    ServerPlayer *ken = effect.from;
    Room *room = ken->getRoom();

    ken->updateMp(-2);
    
    room->broadcastInvoke("animate", "shenglong");
    
    if(room->askForCard(effect.to, "jink", "shenglong-jink:" + ken->objectName()))
        return;
    
    int times = 0;
    
    for(int i=0; i<3; i++) {
        times++;
        // room->getThread()->delay();
        const Card *card = room->peek();
        
        LogMessage log;
        log.type = "$Peek";
        log.from = ken;
        log.card_str = card->toString();
        room->sendLog(log);
        
        room->throwCard(card);
        
        if(card->isBlack()){
            break;
        }   
    }
    
    DamageStruct damageMaker;
    damageMaker.card = this;
    damageMaker.from = ken;
    damageMaker.to = effect.to;
    damageMaker.damage = times;
    damageMaker.nature = DamageStruct::Fire;
    room->damage(damageMaker);

}

class ShenglongViewAsSkill: public OneCardViewAsSkill{
public:
    ShenglongViewAsSkill():OneCardViewAsSkill("shenglong"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@shenglong";
    }
    
    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Slash") && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShenglongCard *card = new ShenglongCard;
        card->addSubcard(card_item->getFilteredCard());
        card->setSkillName(objectName());
        return card;
    }
};

class Shenglong: public PhaseChangeSkill{
public:
    Shenglong():PhaseChangeSkill("shenglong"){
        view_as_skill = new ShenglongViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMp()>=2;
    }

    virtual bool onPhaseChange(ServerPlayer *ken) const{
        Room *room = ken->getRoom();
        
        if(room->askForUseCard(ken, "@@shenglong", "@shenglong")){
            ken->skip(Player::Play);
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------chunli

//----------------------------------------------------------------------------- Bailie

BailieCard::BailieCard(){}

bool BailieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(!Self->inMyAttackRange(to_select)) 
        return false;
        
    if(!Self->canSlash(to_select))
        return false;

    return true;
}

void BailieCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer *chunli = effect.from;
    Room *room = chunli->getRoom();
    
    foreach(int card_id, getSubcards()) {
        CardUseStruct use;
        use.card = Sanguosha->getCard(card_id);
        use.from = chunli;
        use.to << effect.to;

        room->useCard(use);
    }
}

class BailieViewAsSkill: public ViewAsSkill{
public:
    BailieViewAsSkill():ViewAsSkill("bailie"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@bailie";
    }
    
    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 3)
            return false;

        if(!to_select->getFilteredCard()->inherits("Slash"))
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() >3 || cards.length() < 1)
            return NULL;

        BailieCard *card = new BailieCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        
        return card;
    }
};

class Bailie: public PhaseChangeSkill{
public:
    Bailie():PhaseChangeSkill("bailie"){
        view_as_skill = new BailieViewAsSkill;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Play && target->getHandcardNum();
    }

    virtual bool onPhaseChange(ServerPlayer *chunli) const{
        Room *room = chunli->getRoom();

        if(room->askForUseCard(chunli, "@@bailie", "@bailie")){
            return true;
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Jiqi

JiqiCard::JiqiCard(){
    target_fixed = true;
}

void JiqiCard::use(Room *room, ServerPlayer *chunli, const QList<ServerPlayer *> &) const{
    
    QVariant tohelp = QVariant::fromValue((PlayerStar)chunli);

    foreach(ServerPlayer *p, room->getOtherPlayers(chunli)){
        
        const Card *card = room->askForCard(p, ".basic", "@jiqi-card:" + chunli->objectName(), tohelp);
        
        if(card) {
            room->moveCardTo(card, chunli, Player::Hand, true);
        }else {
            LogMessage log;
            log.type = "#JiqiNoCard";
            log.from = p;
            room->sendLog(log);            
        }
        
    }
    
}

class JiqiViewAsSkill: public ZeroCardViewAsSkill{
public:
    JiqiViewAsSkill():ZeroCardViewAsSkill("jiqi"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@jiqi";
    }
    
    virtual const Card *viewAs() const{
        JiqiCard *card = new JiqiCard;
        card->setSkillName(objectName());
        return card;
    }
};

class Jiqi: public PhaseChangeSkill{
public:
    Jiqi():PhaseChangeSkill("jiqi"){
        view_as_skill = new JiqiViewAsSkill;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Draw;
    }

    virtual bool onPhaseChange(ServerPlayer *chunli) const{
        Room *room = chunli->getRoom();

        if(room->askForUseCard(chunli, "@@jiqi", "@jiqi")){
            chunli->skip(Player::Discard);            
            return true;
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Qigong

class QigongBuff: public SlashBuffSkill{
public:
    QigongBuff():SlashBuffSkill("#qigong-buff"){}

    virtual bool buff(const SlashEffectStruct &effect) const{
        
        if(effect.to->hasFlag("qigong"))
            effect.to->addMark("qinggang");

        return false;
    }
};

class QigongOff: public PhaseChangeSkill{
public:
    QigongOff():PhaseChangeSkill("#qigong-off"){}
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Finish;
    }

    virtual bool onPhaseChange(ServerPlayer *chunli) const{
        Room *room = chunli->getRoom();
        
        foreach(ServerPlayer *p, room->getOtherPlayers(chunli)) {
            if(p->hasFlag("qigong"))
                room->setPlayerFlag(p, "-qigong");
        }

        return false;
    }
};

QigongCard::QigongCard(){}

bool QigongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void QigongCard::onEffect(const CardEffectStruct &effect) const {
    effect.from->updateMp(-1);
    effect.from->getRoom()->setPlayerFlag(effect.to, "qigong");
}

class QigongViewAsSkill: public ZeroCardViewAsSkill{
public:
    QigongViewAsSkill():ZeroCardViewAsSkill("qigong"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@qigong";
    }
    
    virtual const Card *viewAs() const{
        QigongCard *card = new QigongCard;
        card->setSkillName(objectName());
        return card;
    }
};

class Qigong: public PhaseChangeSkill{
public:
    Qigong():PhaseChangeSkill("qigong"){
        view_as_skill = new QigongViewAsSkill;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Start && target->getMp()>0;
    }

    virtual bool onPhaseChange(ServerPlayer *chunli) const{
        Room *room = chunli->getRoom();

        room->askForUseCard(chunli, "@@qigong", "@qigong");

        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------blanka

//----------------------------------------------------------------------------- Shidian

class ShidianGet: public TriggerSkill{
public:
    ShidianGet():TriggerSkill("#shidian-get"){
        events << CardFinished;
        frequency = Compulsory;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("shidian");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("ThunderBang")){
            Room *room = player->getRoom();
            if(room->getCardPlace(use.card->getEffectiveId()) == Player::DiscardedPile){

                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *p, players){
                    if(p->hasSkill("shidian")){
                    
                        room->playSkillEffect("shidian");
                        
                        LogMessage log;
                        log.type = "#ShidianGet";
                        log.from = p;
                        room->sendLog(log);
                    
                        p->obtainCard(use.card);
                        break;
                    }
                }
            }
        }

        return false;
    }
};

class Shidian: public TriggerSkill{
public:
    Shidian():TriggerSkill("shidian"){
        events << Predamaged;
        frequency = Compulsory;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *blanka, QVariant &data) const{
        Room *room = blanka->getRoom();
        
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Thunder) {
        
            room->playSkillEffect("shidian");
            
            LogMessage log;
            log.type = "#DamageNullify";
            log.to << blanka;
            log.arg = objectName();
            room->sendLog(log);
            
            blanka->drawCards(1);
        
            return true;
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Dianji

DianjiCard::DianjiCard(){
    once = true;
    target_fixed = true;
}

void DianjiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *blanka = card_use.from;
    
    room->playSkillEffect("dianji");
    
    LogMessage log;
    log.type = "#Dianji";
    log.from = blanka;
    room->sendLog(log);    
    
    blanka->updateMp(-2);
    
    QList<ServerPlayer *> targets = room->getOtherPlayers(blanka);
    
    CardUseStruct use;
    use.card = Sanguosha->getCard(getSubcards().first());
    use.from = blanka;
    use.to << targets;

    room->useCard(use);
    
}

class Dianji: public OneCardViewAsSkill{
public:
    Dianji():OneCardViewAsSkill("dianji"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DianjiCard") && player->getMp()>1 && player->getHandcardNum();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("ThunderBang") && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        DianjiCard *card = new DianjiCard;
        card->addSubcard(card_item->getFilteredCard());
        card->setSkillName(objectName());
        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------dhalsim

//----------------------------------------------------------------------------- Yujia

class Yujia: public GameStartSkill{
public:
    Yujia():GameStartSkill("yujia"){
        frequency = Compulsory;
    }

    virtual void onGameStart(ServerPlayer *dhalsim) const{}
};

//----------------------------------------------------------------------------- Huoyan

HuoyanCard::HuoyanCard(){
    mute = true;
}

void HuoyanCard::onUse(Room *room, const CardUseStruct &card_use) const{

    ServerPlayer *dhalsim = card_use.from;

    const Card *card = Sanguosha->getCard(getSubcards().first());
    FireBang *fire_bang = new FireBang(card->getSuit(), card->getNumber());
    
    dhalsim->updateMp(-2);
    room->throwCard(this);
    room->playSkillEffect("huoyan");    
    
    LogMessage log;
    log.type = "$Huoyan";
    log.from = dhalsim;
    log.to << card_use.to;
    log.card_str = card->toString();
    room->sendLog(log); 
    
    fire_bang->use(room, dhalsim, card_use.to);
}

class Huoyan: public OneCardViewAsSkill{
public:
    Huoyan():OneCardViewAsSkill("huoyan"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMp()>=2 && Slash::IsAvailable(player);
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Slash") && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        HuoyanCard *card = new HuoyanCard;
        card->addSubcard(card_item->getFilteredCard());
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Chuansong

ChuansongCard::ChuansongCard(){}

void ChuansongCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer *dhalsim = effect.from;
    Room *room = dhalsim->getRoom();
    
    ServerPlayer *next;
    
    do{
        next = dhalsim->getNext();
        room->swapSeat(dhalsim, next);
    }while(next != effect.to);
    
}

class ChuansongViewAsSkill: public ZeroCardViewAsSkill{
public:
    ChuansongViewAsSkill():ZeroCardViewAsSkill("chuansong"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@chuansong";
    }
    
    virtual const Card *viewAs() const{
        ChuansongCard *card = new ChuansongCard;
        card->setSkillName(objectName());
        return card;
    }
};

class Chuansong: public TriggerSkill{
public:
    Chuansong():TriggerSkill("chuansong"){
        view_as_skill = new ChuansongViewAsSkill;
        events << CardResponsed;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }
    
    virtual bool trigger(TriggerEvent event, ServerPlayer *dhalsim, QVariant &data) const{
        Room *room = dhalsim->getRoom();
    
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Dodge"))
            return false;
        
        room->askForUseCard(dhalsim, "@@chuansong", "@chuansong");

        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------honda

//----------------------------------------------------------------------------- Xushi

class XushiOff: public TriggerSkill{
public:
    XushiOff():TriggerSkill("#xushi-off"){
        events << CardResponsed;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@xuli")>0;
    }
    
    virtual bool trigger(TriggerEvent event, ServerPlayer *honda, QVariant &data) const{
    
        CardStar card_star = data.value<CardStar>();
        if(card_star->inherits("BasicCard")) {
            honda->loseMark("@xuli");
        }
        
        return false;
    }
};

class Xushi: public PhaseChangeSkill{
public:
    Xushi():PhaseChangeSkill("xushi"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Play && !target->getMark("@xuli");
    }

    virtual bool onPhaseChange(ServerPlayer *honda) const{
        Room *room = honda->getRoom();
        
        if(honda->askForSkillInvoke(objectName())) {
        
            room->playSkillEffect(objectName());
        
            honda->gainMark("@xuli");
            return true;
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Wushuang

class Wushuang: public PhaseChangeSkill{
public:
    Wushuang():PhaseChangeSkill("wushuang"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Start && target->getMark("@xuli");
    }

    virtual bool onPhaseChange(ServerPlayer *honda) const{
        Room *room = honda->getRoom();
        
        if(honda->askForSkillInvoke(objectName())) {
        
            room->playSkillEffect(objectName());
        
            honda->loseMark("@xuli");
            room->setPlayerFlag(honda, "no_range_limit");
            room->setPlayerFlag(honda, "no_bang_limit");
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------zangief

//----------------------------------------------------------------------------- Xuanfeng

XuanfengCard::XuanfengCard(){
    mute = true;
    target_fixed = true;
}

void XuanfengCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const {    
    room->throwCard(this);    
}

class XuanfengViewAsSkill: public ViewAsSkill{
public:
    XuanfengViewAsSkill():ViewAsSkill("xuanfeng"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@xuanfeng";
    }
    
    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2)
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        XuanfengCard *card = new XuanfengCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        
        return card;
    }
};

class Xuanfeng: public TriggerSkill{
public:
    Xuanfeng():TriggerSkill("xuanfeng"){
        events << CardUsed;
        view_as_skill = new XuanfengViewAsSkill;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("xuanfeng");
    }
    
    virtual bool trigger(TriggerEvent event, ServerPlayer *target, QVariant &data) const{
        
        Room *room = target->getRoom();

        ServerPlayer *zangief = NULL;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->hasSkill("xuanfeng")) {
                zangief = p;
                break;
            }
        }
        
        if(!zangief || zangief->getCards("he").length()<2)
            return false;
        
        CardUseStruct use = data.value<CardUseStruct>();   
        QString prompt = QString("@xuanfeng:%1:%2").arg(use.card->getEffectiveId()).arg(use.from->objectName());
        
        if(use.card->isNDTrick() && room->askForUseCard(zangief, "@@xuanfeng", prompt)) {
        
            room->playSkillEffect("xuanfeng");
            
            if(use.card->willThrow())
                room->throwCard(use.card);
                
            LogMessage log;
            log.type = "$XuanfengEffect";
            log.from = zangief;
            log.to << use.from;
            log.card_str = use.card->toString();
            room->sendLog(log);            
            
            if(zangief->distanceTo(use.from)<=1) {
                DamageStruct damageMaker;
                damageMaker.from = zangief;
                damageMaker.to = use.from;
                damageMaker.damage = 1;
                damageMaker.nature = DamageStruct::Normal;
                room->damage(damageMaker);
            }
            
            return true;
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Dazhuang

DazhuangCard::DazhuangCard(){
    mute = true;
}

bool DazhuangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(Self->distanceTo(to_select) > 1) 
        return false;

    return true;
}

void DazhuangCard::onEffect(const CardEffectStruct &effect) const {
    
    ServerPlayer *zangief = effect.from;
    Room *room = zangief->getRoom();

    if(zangief->getMp()>=6) {
        zangief->updateMp(-6);
    }else {
        zangief->updateMp(-3);
        room->throwCard(this);
    }
    
    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(heart):(.*)");
    judge.good = false;
    judge.reason = objectName();
    judge.who = zangief;
    
    room->judge(judge);
    
    if(judge.isGood()) {
    
        room->playSkillEffect("dazhuang");
        room->broadcastInvoke("animate", "dazhuang");
        
        DamageStruct damageMaker;
        damageMaker.from = zangief;
        damageMaker.to = effect.to;
        damageMaker.damage = 3;
        damageMaker.nature = DamageStruct::Normal;
        room->damage(damageMaker);
        
    }
}

class DazhuangViewAsSkill: public ViewAsSkill{
public:
    DazhuangViewAsSkill():ViewAsSkill("dazhuang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@dazhuang";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(Self->getMp()>=6)
            return false;
    
        if(!to_select->getFilteredCard()->inherits("PK"))
            return false;
            
        if(selected.length()>=1)
            return false;
        
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->getMp()<6 && (cards.length() != 1 || !cards.first()->getFilteredCard()->inherits("PK")))
            return NULL;
    
        DazhuangCard *card = new DazhuangCard;
        
        if(cards.length())
            card->addSubcards(cards);
        
        card->setSkillName(objectName());
        return card;
    }
};

class Dazhuang: public PhaseChangeSkill{
public:
    Dazhuang():PhaseChangeSkill("dazhuang"){
        view_as_skill = new DazhuangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) &&
            target->getPhase() == Player::Start &&
            target->getMp()>=3;
    }

    virtual bool onPhaseChange(ServerPlayer *zangief) const{
        Room *room = zangief->getRoom();
        
        if(room->askForUseCard(zangief, "@@dazhuang", "@dazhuang")){
            zangief->skip(Player::Play);
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------guile

//----------------------------------------------------------------------------- Shoudao

class Shoudao: public FilterSkill{
public:
    Shoudao():FilterSkill("shoudao"){}

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Dodge") && to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Bang *bang = new Bang(card->getSuit(), card->getNumber());
        bang->addSubcard(card->getId());
        bang->setSkillName(objectName());

        return bang;
    }
};

//----------------------------------------------------------------------------- Jiaodao

JiaodaoCard::JiaodaoCard(){
    will_throw = true;
    target_fixed = true;
}

void JiaodaoCard::use(Room *room, ServerPlayer *guile, const QList<ServerPlayer *> &) const {
    
    room->throwCard(this);
    
    ServerPlayer *target = NULL;
    
    foreach(ServerPlayer *p, room->getOtherPlayers(guile)) {
        if(p->hasFlag("jiaodao_on")) {
            target = p;
            break;
        }
    }
    
    DamageStruct damageMaker;
    damageMaker.card = this;
    damageMaker.from = guile;
    damageMaker.to = target;
    damageMaker.damage = 1;
    damageMaker.nature = DamageStruct::Normal;
    room->damage(damageMaker);
    
    guile->updateMp(1);
}

class JiaodaoViewAsSkill: public ViewAsSkill{
public:
    JiaodaoViewAsSkill():ViewAsSkill("jiaodao"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@jiaodao";
    }
    
    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2)
            return false;
        
        const Card *card = to_select->getFilteredCard();
        
        // if(!(card->isRed() && (card->inherits("Slash") || card->inherits("Dodge"))))
        if(!(card->isRed() && (card->inherits("Slash"))))
            return false;
            
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        JiaodaoCard *card = new JiaodaoCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        
        return card;
    }
};

class Jiaodao: public TriggerSkill{
public:
    Jiaodao():TriggerSkill("jiaodao"){
        events << Damage;
        view_as_skill = new JiaodaoViewAsSkill;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *guile, QVariant &data) const{
    
        Room *room = guile->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        
        if(!damage.card)
            return false;
        
        if(damage.to->isAlive() && (damage.card->inherits("Dodge") || damage.card->inherits("Slash")) && damage.card->isRed()) {
            
            room->setPlayerFlag(damage.to, "jiaodao_on");
            
            room->askForUseCard(guile, "@@jiaodao", "@jiaodao");
            
            room->setPlayerFlag(damage.to, "-jiaodao_on");
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Yinsu

class Yinsu:public TriggerSkill{
public:
    Yinsu():TriggerSkill("yinsu"){
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *guile) const{
        return TriggerSkill::triggerable(guile) && guile->getMp()>=2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *guile, QVariant &data) const{
        QStringList prompt = data.toString().split(":");
        QString pattern = prompt.at(0);
        QString reason = prompt.at(1);
        
        if(pattern != "jink")
            return false;

        if(guile->askForSkillInvoke(objectName(), reason)) {
        
            guile->getRoom()->playSkillEffect(objectName());
            
            guile->updateMp(-2);
            
            Dodge *dodge = new Dodge(Card::NoSuit, 0);
            dodge->setSkillName(objectName());
            guile->getRoom()->provide(dodge);
            return true;
        }

        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------sakura

//----------------------------------------------------------------------------- Chongbai

ChongbaiCard::ChongbaiCard(){
    will_throw = false;
    once = true;
}

void ChongbaiCard::onEffect(const CardEffectStruct &effect) const {
    
    ServerPlayer *sakura = effect.from;
    Room *room = sakura->getRoom();
    
    const Card *card = Sanguosha->getCard(getSubcards().first());

    room->moveCardTo(card, effect.to, Player::Hand, false);
    
    sakura->updateMp(1);
}

class Chongbai: public OneCardViewAsSkill{
public:
    Chongbai():OneCardViewAsSkill("chongbai"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getHandcardNum() && !player->hasUsed("ChongbaiCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ChongbaiCard *card = new ChongbaiCard;
        card->addSubcard(card_item->getFilteredCard());
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Mofang

class MofangCollect: public TriggerSkill{
public:
    MofangCollect():TriggerSkill("#mofang-collect"){
        events << CardFinished;
        frequency = Compulsory;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("mofang");
    }
    
    virtual bool trigger(TriggerEvent event, ServerPlayer *target, QVariant &data) const{
    
        Room *room = target->getRoom();
    
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->isVirtualCard() 
        || use.card->inherits("EquipCard") || use.card->inherits("DelayedTrick") 
        || room->getCardPlace(use.card->getEffectiveId()) != Player::DiscardedPile)
            return false;
        
        QVariantList mofang_list = room->getTag("MofangCardList").toList();
        
        mofang_list << use.card->getId();
        while(mofang_list.length()>5)
            mofang_list.pop_front();
            
        room->setTag("MofangCardList", mofang_list);
        
        return false;
    }
};

class MofangViewAsSkill: public OneCardViewAsSkill{
public:
    MofangViewAsSkill():OneCardViewAsSkill("mofangvas"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@mofangvas";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        int card_id = Self->getMark("mofang");
        const Card *card = Sanguosha->getCard(card_id);
        return to_select->getFilteredCard()->getSuit() == card->getSuit();
    }

    virtual const Card *viewAs(CardItem *card_item) const{

        int card_id = Self->getMark("mofang");
        const Card *card = Sanguosha->getCard(card_id);
        const Card *to_select = card_item->getFilteredCard();

        Card *new_card = Sanguosha->cloneCard(card->objectName(), to_select->getSuit(), to_select->getNumber());
        new_card->addSubcard(to_select);
        new_card->setSkillName(objectName());
        return new_card;
    }
};

MofangCard::MofangCard(){
    once = true;
    target_fixed = true;
    mute = true;
}

void MofangCard::use(Room *room, ServerPlayer *sakura, const QList<ServerPlayer *> &) const {
    
    QVariantList mofang_list = room->getTag("MofangCardList").toList();
    
    if(!mofang_list.length()) {
    
        LogMessage log;
        log.type = "#MofangNothing";
        log.from = sakura;
        room->sendLog(log);
        
        return;
    }
    
    QList<int> card_ids;
    foreach(QVariant card_id, mofang_list)
        card_ids << card_id.toInt();
        
    room->fillAG(card_ids);
    int card_id = room->askForAG(sakura, card_ids, true, "mofang_choice");
    room->broadcastInvoke("clearAG");
    
    if(card_id != -1){
        const Card *card = Sanguosha->getCard(card_id);
        
        if(!card->isAvailable(sakura)) {
            LogMessage log;
            log.type = "$MofangUnused";
            log.from = sakura;
            log.card_str = card->toString();
            room->sendLog(log);
            
            return;
        }else {
            
            LogMessage log;
            log.type = "$MofangUsed";
            log.from = sakura;
            log.card_str = card->toString();
            room->sendLog(log);
        
            room->setPlayerMark(sakura, "mofang", card_id);                            
            if(room->askForUseCard(sakura, "@mofangvas", "@mofang")){
                
                room->playSkillEffect("mofang");
            
                LogMessage log;
                log.type = "#MofangUnused";
                log.from = sakura;
                room->sendLog(log);
            }
            room->setPlayerMark(sakura, "mofang", -1);
        }

    }
}

class Mofang: public ZeroCardViewAsSkill{
public:
    Mofang():ZeroCardViewAsSkill("mofang"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("MofangCard");
    }
    
    virtual const Card *viewAs() const{
        return new MofangCard;
    }
};

//----------------------------------------------------------------------------- Yingluo

YingluoCard::YingluoCard(){
    target_fixed = true;
}

void YingluoCard::use(Room *room, ServerPlayer *sakura, const QList<ServerPlayer *> &) const{
    sakura->updateMp(-1);
}

class YingluoViewAsSkill: public ZeroCardViewAsSkill{
public:
    YingluoViewAsSkill():ZeroCardViewAsSkill("yingluo"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@yingluo";
    }
    
    virtual const Card *viewAs() const{
        YingluoCard *card = new YingluoCard;
        card->setSkillName(objectName());
        return card;
    }
};

class Yingluo: public PhaseChangeSkill{
public:
    Yingluo():PhaseChangeSkill("yingluo"){
        view_as_skill = new YingluoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Draw;
    }

    virtual bool onPhaseChange(ServerPlayer *sakura) const{
        Room *room = sakura->getRoom();
        
        QList<const Card*> cards;
        
        if(sakura->askForSkillInvoke(objectName())) {
            
            do{
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(club):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = sakura;

                room->judge(judge);
                
                cards << judge.card;
                
                if(judge.isBad()) {
                    if(!sakura->getMp() || !room->askForUseCard(sakura, "@@yingluo", "@yingluo"))
                        room->setPlayerFlag(sakura, "yingluo_off");
                }
                
            }while(!sakura->hasFlag("yingluo_off"));
            
            foreach(const Card *card, cards)
                sakura->obtainCard(card);
            
            return true;
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------cammy

//----------------------------------------------------------------------------- Jingzhun

class Jingzhun: public SlashBuffSkill{
public:
    Jingzhun():SlashBuffSkill("jingzhun"){}

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *cammy = effect.from;
        Room *room = cammy->getRoom();
        
        bool canInvoke = cammy->distanceTo(effect.to) == cammy->getAttackRange();
        
        if(canInvoke && cammy->getMp()>=2 && cammy->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
            room->playSkillEffect(objectName());
        
            cammy->updateMp(-2);
            room->slashResult(effect, NULL);

            return true;
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Luoxuan

class Luoxuan: public TriggerSkill{
public:
    Luoxuan():TriggerSkill("luoxuan"){
        events << Damage;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMp()>=1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *cammy, QVariant &data) const{
    
        Room *room = cammy->getRoom();
        
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.card && damage.card->inherits("Slash") && room->askForSkillInvoke(cammy, objectName(), data)) {
            room->playSkillEffect(objectName());
        
            cammy->updateMp(-1);
            
            int card_id = room->askForCardChosen(cammy, damage.to, "e", "luoxuan");
            
            if(card_id != -1) {
                LogMessage log;
                log.type = "$LuoxuanDiscard";
                log.to << damage.to;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);
                
                room->throwCard(card_id);
            }
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------dan

//----------------------------------------------------------------------------- Tiaoxin

class TiaoxinOff: public TriggerSkill{
public:
    TiaoxinOff():TriggerSkill("#tiaoxin-off"){
        frequency = Compulsory;
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start || target->getMark("tiaoxin_on");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *source, QVariant &data) const{
        Room *room = source->getRoom();
        
        if(source->getPhase() == Player::Start) {
            foreach(ServerPlayer *p, room->getAllPlayers()){
                int times = p->getMark("tiaoxin_on");
                if(times==1) {
                    p->loseMark("@card_forbid");
                }
                
                if(times>0) {
                    int rest = times-1;                
                    room->setPlayerMark(p, "tiaoxin_on", rest);
                    
                    if(rest>0) {
                        LogMessage log;
                        log.type = "#CountDown";
                        log.from = p;
                        log.arg = "@card_forbid";
                        log.arg2 = QString::number(rest);
                        room->sendLog(log);
                    }
                }
            } 
        }
        
        return false;
    }
};

class Tiaoxin: public PhaseChangeSkill{
public:
    Tiaoxin():PhaseChangeSkill("tiaoxin"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Finish && !target->getMark("@card_forbid");
    }

    virtual bool onPhaseChange(ServerPlayer *dan) const{
        Room *room = dan->getRoom();
        
        if(room->askForSkillInvoke(dan, objectName())) {
            
            room->playSkillEffect(objectName());
            
            int x = room->getAllPlayers().length();
            
            dan->drawCards(x);
            
            x = x>4 ? x+1 : x-1;

            dan->gainMark("@card_forbid");
            //because it invokes on phase finish so +1
            room->setPlayerMark(dan, "tiaoxin_on", x+1);
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Wodao

class Wodao: public PhaseChangeSkill{
public:
    Wodao():PhaseChangeSkill("wodao"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Discard && target->getMp()>=6;
    }

    virtual bool onPhaseChange(ServerPlayer *dan) const{
        Room *room = dan->getRoom();
        
        if(room->askForSkillInvoke(dan, objectName())) {
            
            room->playSkillEffect(objectName());
            
            dan->updateMp(-6);
            
            dan->play();
            
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------rose

//----------------------------------------------------------------------------- Anshi

class Anshi: public TriggerSkill{
public:
    Anshi():TriggerSkill("anshi"){
        frequency = Compulsory;
        events << CardChosen;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *rose, QVariant &data) const{
        if(rose->hasSkill(objectName())) {
            rose->getRoom()->playSkillEffect(objectName());
            
            QStringList prompt = data.toString().split(":");
            
            LogMessage log;
            log.type = "#AnshiEffect";
            log.from = rose;
            log.arg = prompt.at(0);
            log.arg2 = prompt.at(1);
            rose->getRoom()->sendLog(log);
            
            return true;
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Caozong

class Caozong: public TriggerSkill{
public:
    Caozong():TriggerSkill("caozong"){
        events << CardUsed;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("caozong");
    }
    
    virtual bool trigger(TriggerEvent event, ServerPlayer *target, QVariant &data) const{
        
        Room *room = target->getRoom();

        ServerPlayer *rose = NULL;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->hasSkill("caozong")) {
                rose = p;
                break;
            }
        }
        
        CardUseStruct use = data.value<CardUseStruct>();   
        if(rose 
        && rose->getMp()>=2 
        && rose->getMark("caozonging")<=0 
        && !use.card->isVirtualCard() 
        && !use.card->targetFixed() 
        && !use.card->inherits("Unassailable")) {
            
            QVariant tohelp = QVariant::fromValue(use);
            if(room->askForSkillInvoke(rose, objectName(), tohelp)) {

                QList<ServerPlayer *> players;
                
                if(use.card->inherits("Slash")) {
                    foreach(ServerPlayer *p, room->getOtherPlayers(use.from)) {
                        if(use.from->canSlash(p, true))
                            players << p;
                    }
                }else if(use.card->inherits("BorrowWeapon")) {
                    foreach(ServerPlayer *p, room->getOtherPlayers(use.from)) {
                        if(p->getWeapon())
                            players << p;
                    }
                }else if(use.card->inherits("Grab") || use.card->inherits("Icebound")) {
                    foreach(ServerPlayer *p, room->getOtherPlayers(use.from)) {
                        if(use.from->distanceTo(p)<=1)
                            players << p;
                    }
                }else if(use.card->inherits("Cure")) {
                    foreach(ServerPlayer *p, room->getAllPlayers()) {
                        if(use.from->distanceTo(p)<=1 || use.from == p)
                            players << p;
                    }
                }else if(use.card->inherits("Burn")) {
                    foreach(ServerPlayer *p, room->getAllPlayers()) {
                        if(p->getHandcardNum())
                            players << p;
                    }
                }else {
                    players = room->getOtherPlayers(use.from);
                }
                
                ServerPlayer *new_target = room->askForPlayerChosen(rose, players, objectName());
                
                use.to.removeFirst();
                use.to.insert(0,new_target);
                
                rose->updateMp(-2);
                
                room->playSkillEffect(objectName());

                LogMessage log;
                log.type = "$CaozongEffect";
                log.from = rose;
                log.to << use.to;
                log.card_str = use.card->toString();
                room->sendLog(log);
                
                room->setPlayerMark(rose, "caozonging", 1);
                room->useCard(use, true);
                room->setPlayerMark(rose, "caozonging", 0);
                
                return true;
                
            }
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Xinling

XinlingCard::XinlingCard(){
    target_fixed = true;
}

void XinlingCard::use(Room *room, ServerPlayer *rose, const QList<ServerPlayer *> &) const{
    room->loseHp(rose,1);
    rose->updateMp(3);
}

class Xinling: public ZeroCardViewAsSkill{
public:
    Xinling():ZeroCardViewAsSkill("xinling"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("XinlingCard");
    }
    
    virtual const Card *viewAs() const{
        XinlingCard *card = new XinlingCard;
        card->setSkillName(objectName());
        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------gen

//----------------------------------------------------------------------------- Sidou

class SidouOff: public TriggerSkill{
public:
    SidouOff():TriggerSkill("#sidou-off"){
        frequency = Compulsory;
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Finish;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *source, QVariant &data) const{
        Room *room = source->getRoom();
        
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->getMark("sidou_on")) {
                room->setPlayerMark(p, "sidou_on", 0);
                p->loseMark("@invincible");
                break;
            }
        } 
        
        return false;
    }
};

class Sidou: public TriggerSkill{
public:
    Sidou():TriggerSkill("sidou"){
        events << Damaged;
        frequency = Compulsory;
    }
    
    virtual int getPriority() const{
        return 3;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *gen, QVariant &data) const{
    
        Room *room = gen->getRoom();
        
        LogMessage log;
        log.type = "#Sidou";
        log.from = gen;
        room->sendLog(log);
        
        room->playSkillEffect(objectName());
        
        if(gen->getHandcardNum() < gen->getMaxHP()) {
            gen->drawCards(gen->getMaxHP()-gen->getHandcardNum());
        }
        if(!gen->getMark("@invincible"))
            gen->gainMark("@invincible");
        room->setPlayerMark(gen, "sidou_on", 1);
        
        return false;
    }
};

//----------------------------------------------------------------------------- Jiliu

class JiliuOn: public PhaseChangeSkill{
public:
    JiliuOn():PhaseChangeSkill("#jiliu-on"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        if(PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Start) {    
            foreach(ServerPlayer *p, target->getRoom()->getAllPlayers()) {
                if(p->getMark("jiliu")) {
                    return true;
                }
            }
        }
        
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *gen) const{
        Room *room = gen->getRoom();
        
        if(room->askForSkillInvoke(gen, objectName())) {
        
            LogMessage log;
            log.type = "#JiliuEffect";
            log.from = gen;
            room->sendLog(log);
            
            room->playSkillEffect("jiliu", 2);
            
            foreach(ServerPlayer *p, room->getOtherPlayers(gen)) {
                int num = p->getMark("jiliu");
                
                if(num) {
                    room->setPlayerMark(p, "jiliu", 0);
                    room->loseHp(p, num);
                }
            }
            
        }
        
        return false;
    }
};

JiliuCard::JiliuCard(){
    mute = true;
    target_fixed = true;
}

void JiliuCard::use(Room *room, ServerPlayer *gen, const QList<ServerPlayer *> &) const{

    ServerPlayer *target = room->askForPlayerChosen(gen, room->getOtherPlayers(gen), "jiliu");
    
    if(target) {
        
        room->playSkillEffect("jiliu", 1);
        
        gen->updateMp(-15);
        
        LogMessage log;
        log.type = "#JiliuSet";
        log.from = gen;
        room->sendLog(log);
        
        target->addMark("jiliu");
    }
}

class Jiliu: public ZeroCardViewAsSkill{
public:
    Jiliu():ZeroCardViewAsSkill("jiliu"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMp()>=15;
    }
    
    virtual const Card *viewAs() const{
        JiliuCard *card = new JiliuCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Sangliu

class SangliuOff: public PhaseChangeSkill{
public:
    SangliuOff():PhaseChangeSkill("#sangliu-off"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom(); 
        
        foreach(ServerPlayer *p, room->getAllPlayers()){
            int times = p->getMark("sangliu_on");
            if(times==1) {
                p->loseMark("@card_forbid");
            }
            
            if(times>0) {
                int rest = times-1;                
                room->setPlayerMark(p, "sangliu_on", rest);
                
                if(rest>0) {
                    LogMessage log;
                    log.type = "#CountDown";
                    log.from = p;
                    log.arg = "@card_forbid";
                    log.arg2 = QString::number(rest);
                    room->sendLog(log);
                }
            }
        }        
       
        return false;
    }
};

class SangliuResult: public TriggerSkill{
public:
    SangliuResult():TriggerSkill("#sangliu-result"){
        frequency = Compulsory;
        events << Damage << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start || target->getMark("sangliu_start");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *source, QVariant &data) const{
        Room *room = source->getRoom();
        
        if(event == PhaseChange && source->getPhase() == Player::Start) {
            foreach(ServerPlayer *p, room->getAllPlayers()){
                int times = p->getMark("sangliu_start");
                if(times==1) {
                
                    LogMessage log;
                    log.type = "#SangliuOn";
                    log.from = p;
                    room->sendLog(log);
                
                    p->gainMark("@card_forbid");
                    room->setPlayerMark(p, "sangliu_on", room->getAllPlayers().length());
                }
                
                if(times>0) {
                    int rest = times-1;                
                    room->setPlayerMark(p, "sangliu_start", rest);
                    
                    if(rest>0) {
                        LogMessage log;
                        log.type = "#SangliuCounter";
                        log.from = p;
                        log.arg = QString::number(rest);
                        log.arg2 = "card_forbid";
                        room->sendLog(log);
                    }
                }
            } 
        }else if(event == Damage && source->getMark("sangliu_start")) {
        
            DamageStruct damage = data.value<DamageStruct>();
            QString gen_name = room->getTag("SangliuSource").toString();
            
            if(damage.to->objectName() == gen_name) {
                room->setPlayerMark(source, "sangliu_start", 0);
                
                LogMessage log;
                log.type = "#SangliuRemove";
                log.from = source;
                log.to << damage.to;
                room->sendLog(log);
            }
        }
        
        return false;
    }
};

SangliuCard::SangliuCard(){
    once = true;
    mute = true;
}

bool SangliuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(to_select->getMark("sangliu_start") || to_select->getMark("@card_forbid"))
        return false;

    return true;
}

void SangliuCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer *gen = effect.from;
    Room *room = gen->getRoom();
    
    room->playSkillEffect("sangliu");
    
    int num = room->getAllPlayers().length()*2;
    
    room->setPlayerMark(effect.to, "sangliu_start", num);
    
    LogMessage log;
    log.type = "#SangliuStart";
    log.from = gen;
    log.to << effect.to;
    log.arg = QString::number(num);
    room->sendLog(log);
    
    room->setTag("SangliuSource", gen->objectName());
}

class Sangliu: public ZeroCardViewAsSkill{
public:
    Sangliu():ZeroCardViewAsSkill("sangliu"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{       
        return !player->hasUsed("SangliuCard");
    }
    
    virtual const Card *viewAs() const{
        SangliuCard *card = new SangliuCard;
        card->setSkillName(objectName());
        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------bison

//----------------------------------------------------------------------------- Kuangniu

class Kuangniu: public FilterSkill{
public:
    Kuangniu():FilterSkill("kuangniu"){}

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("TrickCard") && to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Bang *bang = new Bang(card->getSuit(), card->getNumber());
        bang->addSubcard(card->getId());
        // bang->setSkillName(objectName());

        return bang;
    }
};

//----------------------------------------------------------------------------- Chongji

class Chongji: public SlashBuffSkill{
public:
    Chongji():SlashBuffSkill("chongji"){
        frequency = Compulsory;
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        if(effect.slash->getSuit() == Card::Heart)
            effect.from->getRoom()->playSkillEffect(objectName());
        return false;
    }
};

//----------------------------------------------------------------------------- Shikong

class Shikong: public PhaseChangeSkill{
public:
    Shikong():PhaseChangeSkill("shikong"){
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Finish;
    }

    virtual bool onPhaseChange(ServerPlayer *bison) const{
        Room *room = bison->getRoom();
        
        if(bison->getSlashCount()==0 && !bison->hasUsed("PK")) {
            
            LogMessage log;
            log.type = "#ShikongEffect",
            log.from = bison;
            room->sendLog(log);
        
            if(bison->getMp()>=4) {
                room->playSkillEffect("shikong", 2);
                
                bison->updateMp(-4);
            }else {
                room->playSkillEffect("shikong", 1);
                
                room->loseHp(bison, 1);
            }
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------balrog

//----------------------------------------------------------------------------- Wuxia

class Wuxia:public OneCardViewAsSkill{
public:
    Wuxia():OneCardViewAsSkill("wuxia"){}

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        if(ClientInstance->getStatus() == Client::Responsing) {
        
            QString pattern = ClientInstance->getPattern();
        
            if(pattern == "nullification")
                return card->inherits("BasicCard");
            else if(pattern == "jink")
                return card->inherits("TrickCard");
        }

        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink" || pattern == "nullification";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        if(card->inherits("TrickCard")){
        
            Dodge *jink = new Dodge(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            
            return jink;
            
        }else if(card->inherits("BasicCard")){
        
            Unassailable *unassailable = new Unassailable(card->getSuit(), card->getNumber());
            unassailable->addSubcard(card);
            unassailable->setSkillName(objectName());
            return unassailable;
            
        }else
            return NULL;
    }
};

//----------------------------------------------------------------------------- Tiemian

class Tiemian: public TriggerSkill{
public:
    Tiemian():TriggerSkill("tiemian"){
        events << CardLost;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMp()>=2 && target->getPhase() == Player::NotActive;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *balrog, QVariant &data) const{
    
        CardMoveStar move = data.value<CardMoveStar>();
        const Card *card = Sanguosha->getCard(move->card_id);
        
        if(move->from_place == Player::Equip && card->inherits("Armor")){
            Room *room = balrog->getRoom();
            
            if(room->askForSkillInvoke(balrog, objectName())) {
                room->playSkillEffect(objectName());
                
                balrog->updateMp(-2);
                
                balrog->obtainCard(card);
                
                room->moveCardTo(card, balrog, Player::Equip, true);
            }
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Kongsha

class Kongsha: public TriggerSkill{
public:
    Kongsha():TriggerSkill("kongsha"){
        events << Damage;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMp()>=2;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *balrog, QVariant &data) const{
    
        Room *room = balrog->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        
        if(!damage.card)
            return false;
        
        if(damage.to->isAlive() && damage.card->inherits("Slash")
        && balrog->distanceTo(damage.to) < balrog->getAttackRange()
        && room->askForSkillInvoke(balrog, objectName(), data)) {
            
            room->playSkillEffect(objectName());
            
            balrog->updateMp(-2);
            
            damage.to->turnOver();
            
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------sagat

//----------------------------------------------------------------------------- Zengnu

class ZengnuRyu: public GameStartSkill{
public:
    ZengnuRyu():GameStartSkill("#zengnu-ryu"){}

    virtual int getPriority() const{
        return -1;
    }    

    virtual bool triggerable(const ServerPlayer *sagat) const{
        return GameStartSkill::triggerable(sagat) && sagat->getGeneralName() == "sagat";
    }

    virtual void onGameStart(ServerPlayer *sagat) const{
        Room *room = sagat->getRoom();
        foreach(ServerPlayer *p, room->getOtherPlayers(sagat)){
            if(p->getGeneralName()== "ryu") {

                room->playSkillEffect("meet-ryu");
                
                room->broadcastInvoke("skillInvoked", QString("%1:%2").arg(sagat->objectName()).arg("zengnu"));
                
                room->loseHp(sagat,1);
                sagat->updateMp(1);
                
                break;
            }
        }
    }
};

ZengnuCard::ZengnuCard(){
    target_fixed = true;
}

void ZengnuCard::use(Room *room, ServerPlayer *sagat, const QList<ServerPlayer *> &) const{
    room->loseHp(sagat,1);
    sagat->updateMp(1);
}

class Zengnu: public ZeroCardViewAsSkill{
public:
    Zengnu():ZeroCardViewAsSkill("zengnu"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ZengnuCard");
    }
    
    virtual const Card *viewAs() const{
        ZengnuCard *card = new ZengnuCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Huqie

HuqieCard::HuqieCard(){}

bool HuqieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(!Self->inMyAttackRange(to_select))
        return false;

    return true;
}

void HuqieCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer *sagat = effect.from;
    Room *room = sagat->getRoom();
    
    int x = sagat->getMp();
    int times = 0;
    int y = 0;
    
    for(int i=0; i<x; i++) {
        times++;
        const Card *card = room->peek();
        
        LogMessage log;
        log.type = "$Peek";
        log.from = sagat;
        log.card_str = card->toString();
        room->sendLog(log);
        
        room->throwCard(card);
        
        if(card->isRed()){
            y++;
        }   
    }
    
    sagat->updateMp(-x);
    
    room->broadcastInvoke("animate", "huqie");
    
    DamageStruct damageMaker;
    damageMaker.card = this;
    damageMaker.from = sagat;
    damageMaker.to = effect.to;
    damageMaker.damage = y;
    damageMaker.nature = DamageStruct::Normal;
    room->damage(damageMaker);
    
    if(y==0){
        LogMessage log;
        log.type = "#HuqieFail";
        log.from = sagat;
        log.to << effect.to;
        room->sendLog(log);
    }
}

class HuqieViewAsSkill: public ZeroCardViewAsSkill{
public:
    HuqieViewAsSkill():ZeroCardViewAsSkill("huqie"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@huqie";
    }
    
    virtual const Card *viewAs() const{
        HuqieCard *card = new HuqieCard;
        card->setSkillName(objectName());
        return card;
    }
};

class Huqie: public PhaseChangeSkill{
public:
    Huqie():PhaseChangeSkill("huqie"){
        view_as_skill = new HuqieViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMp();
    }

    virtual bool onPhaseChange(ServerPlayer *sagat) const{
        Room *room = sagat->getRoom();
        
        if(room->askForUseCard(sagat, "@@huqie", "@huqie")){
            sagat->skip(Player::Play);
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------vega

//----------------------------------------------------------------------------- Cuimian

class Cuimian: public PhaseChangeSkill{
public:
    Cuimian():PhaseChangeSkill("cuimian"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("cuimian") && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        
        ServerPlayer *vega = NULL;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->hasSkill("cuimian") && p->getMp()) {
                vega = p;
                break;
            }
        }
        
        if(!vega)
            return false;
            
        QVariant tohelp = QVariant::fromValue((PlayerStar)target);
        if(room->askForSkillInvoke(vega, objectName(), tohelp)) {
            room->playSkillEffect(objectName());
            
            vega->updateMp(-1);
            
            int x = vega->getMaxHP() - vega->getHp();
            
            QString prompt = QString("@cuimian-slash:%1").arg(vega->objectName());
            const Card *slash = room->askForCard(target, "slash", prompt);
            
            QList<ServerPlayer *> players;
            foreach(ServerPlayer *player, room->getOtherPlayers(target)) {
                if(target->distanceTo(player) <= 1)
                    players << player;
            }
            
            if(slash && players.length()) {
                
                ServerPlayer *slash_target = room->askForPlayerChosen(target, players, objectName());
                
                LogMessage log;
                log.type = "#CuimainSlash";
                log.to << target;
                room->sendLog(log);
                
                CardUseStruct use;
                use.card = slash;
                use.from = target;
                use.to << slash_target;
                room->useCard(use);
                
            }else {
            
                LogMessage log;
                log.type = "#CuimainDiscard";
                log.to << target;
                room->sendLog(log);
                
                if(target->getHandcardNum() <= x) {
                    target->throwAllHandCards();
                }else if(x > 0){
                    room->askForDiscard(target, "cuimain-discard:"+vega->objectName(), x, false, false);
                }
            }
            
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Mofu

class Mofu: public TriggerSkill{
public:
    Mofu():TriggerSkill("mofu"){
        events << AskForPeaches;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMp()>1 && target->getHandcardNum();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *vega, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if(dying_data.who != vega)
            return false;

        Room *room = vega->getRoom();
        if(vega->askForSkillInvoke(objectName(), data)){
            
            room->askForDiscard(vega, "mofu-discard", 1, false, false);
        
            room->playSkillEffect(objectName());            
            room->broadcastInvoke("animate", "mofu:3000");
            
            int x = vega->getMp();            
            vega->updateMp(-x);            
            room->setPlayerProperty(vega, "hp", qMin(x/2, vega->getMaxHP()));

        }

        return false;
    }
};

//----------------------------------------------------------------------------- Dianliu

class Dianliu: public TriggerSkill{
public:
    Dianliu():TriggerSkill("dianliu"){
        frequency = Compulsory;
        events << Damage << Predamage;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *vega, QVariant &data) const{
    
        Room *room = vega->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        
        if(event == Damage && vega->getMp()<3) {
            
            LogMessage log;
            log.type = "#DianliuEffect";
            log.from = vega;
            room->sendLog(log);
            
            room->playSkillEffect(objectName());
        
            vega->updateMp(1);
        
        }else if(event == Predamage && vega->getMp()>5) {
        
            if(damage.nature != DamageStruct::Thunder) {            

                damage.nature = DamageStruct::Thunder;
                data = QVariant::fromValue(damage);
                
                LogMessage log;
                log.type = "#DamageToThunder";
                log.to << damage.to;
                log.arg = objectName();
                room->sendLog(log);    
            
            }
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Mozhilingyu

class Mozhilingyu: public PhaseChangeSkill{
public:
    Mozhilingyu():PhaseChangeSkill("mozhilingyu$"){}
    
    virtual int getPriority() const{
        return 3;
    }    
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start && target->getRoom()->getLord()->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();

        if(room->testRandomEvent(target, objectName(), 10)) {
            
            int mp=0;
            
            LogMessage log;
            log.type = "#Mozhilingyu";
            log.from = target;
            room->sendLog(log);
            
            if(target->getKingdom() == "nu"){
                mp = 1;
            }else if(target->getKingdom() == "qi" || target->getKingdom() == "ling") {
                mp = 2;
            }else if(target->getKingdom() == "kuang") {
                mp = 3;
            }else if(target->getKingdom() == "yuan") {
                mp = 8;
            }
            
            target->updateMp(mp);
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------End

MeleeSFPackage::MeleeSFPackage()
    :Package("meleesf")
{
    General *gouki, *ryu, *ken, *chunli, *blanka, *dhalsim, *honda, *zangief, *guile, *sakura, *cammy, *dan, *rose, *gen, *bison, *balrog, *sagat, *vega;
    
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
    
    ken = new General(this, "ken", "nu");
    ken->addSkill(new Jifeng);
    ken->addSkill(new JifengEnable);
    related_skills.insertMulti("jifeng", "#jifeng-enable");
    ken->addSkill(new Shenglong);
    
    addMetaObject<JifengCard>();
    addMetaObject<ShenglongCard>();
    
    chunli = new General(this, "chunli", "qi", 3, false);
    chunli->addSkill(new Bailie);
    chunli->addSkill(new Jiqi);
    chunli->addSkill(new Qigong);
    chunli->addSkill(new QigongOff);
    chunli->addSkill(new QigongBuff);
    related_skills.insertMulti("qigong", "#qigong-off");
    related_skills.insertMulti("qigong", "#qigong-buff");
       
    addMetaObject<BailieCard>();
    addMetaObject<JiqiCard>();
    addMetaObject<QigongCard>();
    
    blanka = new General(this, "blanka", "nu");
    blanka->addSkill(new Shidian);
    blanka->addSkill(new ShidianGet);
    related_skills.insertMulti("shidian", "#shidian-get");
    blanka->addSkill(new Dianji);

    addMetaObject<DianjiCard>();
    
    dhalsim = new General(this, "dhalsim", "ling", 3);
    dhalsim->addSkill(new Yujia);
    dhalsim->addSkill(new MarkAssignSkill("@weapon_forbid", 1));
    related_skills.insertMulti("yujia", "#@weapon_forbid");
    dhalsim->addSkill(new Huoyan);
    dhalsim->addSkill(new Chuansong);
    
    addMetaObject<HuoyanCard>();
    addMetaObject<ChuansongCard>();
    
    honda = new General(this, "honda", "nu", 5);
    honda->addSkill(new Xushi);
    honda->addSkill(new XushiOff);
    related_skills.insertMulti("xushi", "#xushi-off");
    honda->addSkill(new Wushuang);
    
    zangief = new General(this, "zangief", "nu");
    zangief->addSkill(new Xuanfeng);
    zangief->addSkill(new Dazhuang);
    
    addMetaObject<XuanfengCard>();
    addMetaObject<DazhuangCard>();
    
    guile = new General(this, "guile", "kuang");
    guile->addSkill(new Shoudao);
    guile->addSkill(new Jiaodao);
    guile->addSkill(new Yinsu);
    
    addMetaObject<JiaodaoCard>();
    
    sakura = new General(this, "sakura", "qi", 3, false);
    sakura->addSkill(new Chongbai);
    sakura->addSkill(new Mofang);
    sakura->addSkill(new MofangCollect);
    related_skills.insertMulti("mofang", "#mofang-collect");
    sakura->addSkill(new Yingluo);
    
    skills << new MofangViewAsSkill;
    
    addMetaObject<ChongbaiCard>();
    addMetaObject<MofangCard>();
    addMetaObject<YingluoCard>();
    
    cammy = new General(this, "cammy", "kuang", 4, false);
    cammy->addSkill(new Jingzhun);
    cammy->addSkill(new Luoxuan);
    
    dan = new General(this, "dan", "qi");
    dan->addSkill(new Tiaoxin);
    dan->addSkill(new TiaoxinOff);
    related_skills.insertMulti("tiaoxin", "#tiaoxin-off");
    dan->addSkill(new Wodao);

    rose = new General(this, "rose", "ling", 3, false);
    rose->addSkill(new Anshi);
    rose->addSkill(new Caozong);
    rose->addSkill(new Xinling);
    
    addMetaObject<XinlingCard>();
    
    gen = new General(this, "gen", "yuan", 3);
    gen->addSkill(new Sidou);
    gen->addSkill(new SidouOff);
    related_skills.insertMulti("sidou", "#sidou-off");
    gen->addSkill(new Jiliu);
    gen->addSkill(new JiliuOn);
    related_skills.insertMulti("jiliu", "#jiliu-on");
    gen->addSkill(new Sangliu);
    gen->addSkill(new SangliuResult);
    gen->addSkill(new SangliuOff);
    related_skills.insertMulti("sangliu", "#sangliu-result");
    related_skills.insertMulti("sangliu", "#sangliu-off");
    
    addMetaObject<JiliuCard>();
    addMetaObject<SangliuCard>();
    
    bison = new General(this, "bison", "kuang", 5);
    bison->addSkill(new Kuangniu);
    bison->addSkill(new Chongji);
    bison->addSkill(new Shikong);
    
    balrog = new General(this, "balrog", "qi", 3);
    balrog->addSkill(new Wuxia);
    balrog->addSkill(new Tiemian);
    balrog->addSkill(new Kongsha); 
    
    sagat = new General(this, "sagat", "nu");
    sagat->addSkill(new Zengnu);
    sagat->addSkill(new ZengnuRyu);
    related_skills.insertMulti("zengnu", "#zengnu-ryu");
    sagat->addSkill(new Huqie); 
    
    addMetaObject<ZengnuCard>();
    addMetaObject<HuqieCard>();
    
    vega = new General(this, "vega$", "ling", 3);
    vega->addSkill(new Cuimian);
    vega->addSkill(new Mofu);
    vega->addSkill(new Dianliu); 
    vega->addSkill(new Mozhilingyu); 
}

ADD_PACKAGE(MeleeSF);
