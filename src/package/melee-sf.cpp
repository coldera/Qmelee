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
            ryu->updateMp(-3);
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
    
    room->broadcastInvoke("animate", "shenglong");
    
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
            ken->updateMp(-2);
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

    return true;
}

void BailieCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer *chunli = effect.from;
    Room *room = chunli->getRoom();
    
    QVariant tohelp = QVariant::fromValue((PlayerStar)effect.to);
    
    for(int i=0; i<3; i++) {
        
        const Card *bang = room->askForCard(chunli, "slash", "@bailie-slash:" + effect.to->objectName(), tohelp);
        
        if(bang) {
            
            CardUseStruct use;
            use.card = bang;
            use.from = chunli;
            use.to << effect.to;

            room->useCard(use);
        }else {
            break;
        }
    }
}

class BailieViewAsSkill: public ZeroCardViewAsSkill{
public:
    BailieViewAsSkill():ZeroCardViewAsSkill("bailie"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@bailie";
    }
    
    virtual const Card *viewAs() const{
        BailieCard *card = new BailieCard;
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
            return true;
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Qigong

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
    effect.to->addMark("qinggang");
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

//--------------------------------------------------------------------------------------------------------------blank

//----------------------------------------------------------------------------- Shidian

class Shidian: public TriggerSkill{
public:
    Shidian():TriggerSkill("shidian"){
        events << Predamaged;
        frequency = Compulsory;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *blank, QVariant &data) const{
        Room *room = blank->getRoom();
        
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Thunder) {
            
            LogMessage log;
            log.type = "#DamageNullify";
            log.to << blank;
            log.arg = objectName();
            room->sendLog(log);
            
            blank->drawCards(1);
        
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
    ServerPlayer *blank = card_use.from;
    
    room->playSkillEffect("dianji");
    
    LogMessage log;
    log.type = "#Dianji";
    log.from = blank;
    room->sendLog(log);    
    
    blank->updateMp(-2);
    
    QList<ServerPlayer *> targets = room->getOtherPlayers(blank);
    
    CardUseStruct use;
    use.card = Sanguosha->getCard(getSubcards().first());
    use.from = blank;
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

//--------------------------------------------------------------------------------------------------------------dhaisim

//----------------------------------------------------------------------------- Yujia

class Yujia: public GameStartSkill{
public:
    Yujia():GameStartSkill("yujia"){
        frequency = Compulsory;
    }

    virtual void onGameStart(ServerPlayer *dhaisim) const{
        dhaisim->getRoom()->setPlayerMark(dhaisim, "no_range_limit", 1);
    }
};

//----------------------------------------------------------------------------- Huoyan

HuoyanCard::HuoyanCard(){}

void HuoyanCard::onUse(Room *room, const CardUseStruct &card_use) const{

    ServerPlayer *dhaisim = card_use.from;

    const Card *card = Sanguosha->getCard(getSubcards().first());    
    FireBang *fire_bang = new FireBang(card->getSuit(), card->getNumber());

    LogMessage log;
    log.type = "$Huoyan";
    log.from = dhaisim;
    log.to << card_use.to;
    log.card_str = fire_bang->toString();
    room->sendLog(log); 
    
    room->playSkillEffect("huoyan");
    
    dhaisim->updateMp(-2);
    
    fire_bang->use(room, dhaisim, card_use.to);
    
    room->throwCard(this);
    
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
        return card;
    }
};

//----------------------------------------------------------------------------- Chuansong

ChuansongCard::ChuansongCard(){}

void ChuansongCard::onEffect(const CardEffectStruct &effect) const {
    ServerPlayer *dhaisim = effect.from;
    Room *room = dhaisim->getRoom();
    
    ServerPlayer *next;
    
    do{
        next = dhaisim->getNext();
        room->swapSeat(dhaisim, next);
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
    
    virtual bool trigger(TriggerEvent event, ServerPlayer *dhaisim, QVariant &data) const{
        Room *room = dhaisim->getRoom();
    
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Dodge"))
            return false;
        
        room->askForUseCard(dhaisim, "@@chuansong", "@chuansong");

        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------End

MeleeSFPackage::MeleeSFPackage()
    :Package("meleesf")
{
    General *gouki, *ryu, *ken, *chunli, *blank, *dhaisim;
    
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
    
    addMetaObject<BailieCard>();
    addMetaObject<JiqiCard>();
    addMetaObject<QigongCard>();
    
    blank = new General(this, "blank", "nu");
    blank->addSkill(new Shidian);
    blank->addSkill(new Dianji);

    addMetaObject<DianjiCard>();
    
    dhaisim = new General(this, "dhaisim", "ling", 3);
    dhaisim->addSkill(new Yujia);
    dhaisim->addSkill(new MarkAssignSkill("@weapon_forbid", 1));
    related_skills.insertMulti("yujia", "#@weapon_forbid");
    dhaisim->addSkill(new Huoyan);
    dhaisim->addSkill(new Chuansong);
    
    addMetaObject<HuoyanCard>();
    addMetaObject<ChuansongCard>();
    
    
}

ADD_PACKAGE(MeleeSF);


