#include "melee-ss.h"

#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"

// skill cards

//--------------------------------------------------------------------------------------------------------------haohmaru

//----------------------------------------------------------------------------- Jiuqi

class Jiuqi: public OneCardViewAsSkill{
public:
    Jiuqi():OneCardViewAsSkill("jiuqi"){}

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Schnapps::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("analeptic");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Schnapps *schnapps = new Schnapps(card->getSuit(), card->getNumber());
        schnapps->setSkillName(objectName());
        schnapps->addSubcard(card->getId());

        return schnapps;
    }
};

//----------------------------------------------------------------------------- Tianba

TianbaCard::TianbaCard(){}

bool TianbaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(!Self->inMyAttackRange(to_select))
        return false;

    return true;
}

void TianbaCard::onEffect(const CardEffectStruct &effect) const{

    effect.from->updateMp(-3);

    Room *room = effect.from->getRoom();   	
    room->broadcastInvoke("animate", "tianba");
    
	room->throwCard(this);
    
    const Card *first = NULL, *second = NULL;
    first = room->askForCard(effect.to, "jink", "@tianba-jink-1:" + effect.from->objectName());
    if(first)
        second = room->askForCard(effect.to, "jink", "@tianba-jink-2:" + effect.from->objectName());
        
    if(!(first && second)) {
        int dp = 3;    
        QStringList choices;
        
        if(effect.to->getHandcardNum() >= 2)
            choices << "TianbaEffect1";
        
        if(effect.to->hasEquip())
            choices << "TianbaEffect2";
        
        if(choices.length()) {
            choices << "NoChoice"; 
        
            QString choice = room->askForChoice(effect.to, "tianba", choices.join("+"));
            
            if(choice =="TianbaEffect1") {
                room->askForDiscard(effect.to, "tianba", 2, false, false);
                dp = 2;
            }else if(choice =="TianbaEffect2") {
                int card_id = room->askForCardChosen(effect.to, effect.to, "e", "tianba");
                room->throwCard(card_id);
                dp = 2;
            }
        }
        
        DamageStruct damageMaker;
        damageMaker.card = effect.card;
        damageMaker.from = effect.from;
        damageMaker.to = effect.to;
        damageMaker.damage = dp;
        damageMaker.nature = DamageStruct::Normal;
        room->damage(damageMaker);
        
    }
}

class TianbaViewAsSkill: public OneCardViewAsSkill{
public:
    TianbaViewAsSkill():OneCardViewAsSkill("tianba"){}

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Slash");
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@tianba";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        TianbaCard *card = new TianbaCard;
        card->addSubcard(card_item->getCard());
        card->setSkillName(objectName());
        
        return card;
    }
};

class Tianba: public PhaseChangeSkill{
public:
    Tianba():PhaseChangeSkill("tianba"){
        view_as_skill = new TianbaViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMp() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *haohmaru) const{
        Room *room = haohmaru->getRoom();
        
        if(room->askForUseCard(haohmaru, "@@tianba", "@tianba")){
            haohmaru->skip(Player::Play);
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------nakoruru

//----------------------------------------------------------------------------- Goutong

GoutongCard::GoutongCard(){
    will_throw = false;
}

bool GoutongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(to_select->getHandcardNum() < subcardsLength())
        return false;
        
    if(Self->getMp() < subcardsLength())
        return false;
        
    return true;
}

void GoutongCard::use(Room *room, ServerPlayer *nakoruru, const QList<ServerPlayer *> &targets) const {
    int n = subcardsLength();
    
    const Card *exchange_card = room->askForExchange(targets.first(), "goutong", n);
    
    AI *ai = nakoruru->getAI();
    if(ai){
        const Card *ai_exchange_card = room->askForExchange(nakoruru, "goutong", n);    
        foreach(int card_id, ai_exchange_card->getSubcards())
            room->moveCardTo(Sanguosha->getCard(card_id), targets.first(), Player::Hand, false);
        delete ai_exchange_card;
    }else {    
        foreach(int card_id, getSubcards())
            room->moveCardTo(Sanguosha->getCard(card_id), targets.first(), Player::Hand, false);
    }
    
    foreach(int card_id, exchange_card->getSubcards())
        room->moveCardTo(Sanguosha->getCard(card_id), nakoruru, Player::Hand, false);
    delete exchange_card;
    
    nakoruru->updateMp(-n);
}

class Goutong: public ViewAsSkill{
public:
    Goutong():ViewAsSkill("goutong"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMp() > 0;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        GoutongCard *card = new GoutongCard;
        card->addSubcards(cards);
        //card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Yingxuan

class XuankongSkill: public DistanceSkill{
public:
    XuankongSkill():DistanceSkill("xuankong") {}

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->hasSkill(objectName()))
            return +2;
        else
            return 0;
    }
};

YingxuanCard::YingxuanCard(){
    target_fixed = true;
    once = true;
}

void YingxuanCard::use(Room *room, ServerPlayer *nakoruru, const QList<ServerPlayer *> &) const{
    
    room->throwCard(this);    
    nakoruru->gainMark("@xuankong");
    room->setPlayerMark(nakoruru, "no_distance_limit", 1);
    room->acquireSkill(nakoruru, "xuankong");
}

class Yingxuan: public OneCardViewAsSkill{
public:
    Yingxuan():OneCardViewAsSkill("yingxuan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->getMark("@xuankong") && !player->hasUsed("YingxuanCard") && player->getHandcardNum();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YingxuanCard *card = new YingxuanCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());

        return card;
    }
};

//----------------------------------------------------------------------------- Chongci

class Chongci: public TriggerSkill{
public:
    Chongci():TriggerSkill("chongci"){
        frequency = Compulsory;
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@xuankong");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *nakoruru, QVariant &data) const{    
    
        Room *room = nakoruru->getRoom();
    
        CardUseStruct card_use = data.value<CardUseStruct>();
        CardStar card = card_use.card;
        
        if(card->inherits("BasicCard") || card->inherits("TrickCard")) {
            room->playSkillEffect(objectName());
        
            LogMessage log;
            log.type = "#Chongci";
            log.from = nakoruru;
            room->sendLog(log);
        
            nakoruru->loseMark("@xuankong");
            room->setPlayerMark(nakoruru, "no_distance_limit", 0);
            room->detachSkillFromPlayer(nakoruru, "xuankong");
        }

        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------ukyo

//----------------------------------------------------------------------------- Xuruo

class Xuruo: public TriggerSkill{
public:
    Xuruo():TriggerSkill("xuruo"){
        frequency = Compulsory;
        events << CardLost;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *ukyo, QVariant &data) const{
        if(ukyo->isKongcheng()){
            LogMessage log;
            log.type = "#Xuruo";
            log.from = ukyo;
            ukyo->getRoom()->sendLog(log);
            
            CardMoveStar move = data.value<CardMoveStar>();

            if(move->from_place == Player::Hand){
                Room *room = ukyo->getRoom();
                
                room->playSkillEffect(objectName());
                
                room->loseHp(ukyo,1);
                
                if(ukyo->isAlive())
                    ukyo->drawCards(5);
            }
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Juhe

class Juhe: public SlashBuffSkill{
public:
    Juhe():SlashBuffSkill("juhe"){}

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *ukyo = effect.from;
        Room *room = ukyo->getRoom();
        if(ukyo->getPhase() != Player::Play)
            return false;

        if(ukyo->getMp() && ukyo->getWeapon()){
            if(ukyo->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
                
                room->playSkillEffect(objectName());
            
                ukyo->updateMp(-1);
                room->slashResult(effect, NULL);

                return true;
            }
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Liulian

LiulianCard::LiulianCard(){}

bool LiulianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(!Self->inMyAttackRange(to_select)) 
        return false;

    return true;
}

void LiulianCard::onEffect(const CardEffectStruct &effect) const {
    
    ServerPlayer *ukyo = effect.from;
    Room *room = ukyo->getRoom();

    ukyo->updateMp(-4);
    room->loseHp(ukyo,1);
    
    int times = 0;
    
    for(int i=0; i<6; i++) {
        times++;
        // room->getThread()->delay();
        const Card *card = room->peek();
        
        LogMessage log;
        log.type = "$Peek";
        log.from = ukyo;
        log.card_str = card->toString();
        room->sendLog(log);
        
        room->throwCard(card);
        
        if(card->getSuit() == Card::Diamond){
            break;
        }   
    }
    
    room->broadcastInvoke("animate", "liulian");
    
    DamageStruct damageMaker;
    damageMaker.card = this;
    damageMaker.from = ukyo;
    damageMaker.to = effect.to;
    damageMaker.damage = times;
    damageMaker.nature = DamageStruct::Normal;
    room->damage(damageMaker);

}

class LiulianViewAsSkill: public ZeroCardViewAsSkill{
public:
    LiulianViewAsSkill():ZeroCardViewAsSkill("liulian"){

    }
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@liulian";
    }
    
    virtual const Card *viewAs() const{
        LiulianCard *card = new LiulianCard;
        card->setSkillName(objectName());
        return card;
    }
};

class Liulian: public PhaseChangeSkill{
public:
    Liulian():PhaseChangeSkill("liulian"){
        view_as_skill = new LiulianViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
            && target->getPhase() == Player::Start
            && target->getMp() >= 4;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        
        if(room->askForUseCard(target, "@@liulian", "@liulian")){
            target->skip(Player::Play);
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------kyoshiro

//----------------------------------------------------------------------------- Quwu

QuwuCard::QuwuCard(){
    target_fixed = true;
    once = true;
}

void QuwuCard::use(Room *room, ServerPlayer *kyoshiro, const QList<ServerPlayer *> &) const{
    
    const Card *show_card = NULL;
    
    room->throwCard(this);
    
    foreach(ServerPlayer *p, room->getOtherPlayers(kyoshiro)){
    
        QString suit_str = Sanguosha->getCard(getSubcards().first())->getSuitString();
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@quwu-card-show:%1::%2").arg(kyoshiro->getGeneralName()).arg(suit_str);
        
        show_card = room->askForCard(p, pattern, prompt);
        
        if(show_card){
            
            room->showCard(p, show_card->getEffectiveId());
           
            kyoshiro->updateMp(1);

        }else if(p->getMp()){
            p->updateMp(-1);
        }

    }
    
    
}

class Quwu: public OneCardViewAsSkill{
public:
    Quwu():OneCardViewAsSkill("quwu"){
        
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("QuwuCard") && player->getHandcardNum();
    }
    
    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QuwuCard *card = new QuwuCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());

        return card;
    }
};

//----------------------------------------------------------------------------- Yanwu

YanwuCard::YanwuCard(){
    target_fixed = true;
    once = true;
}

void YanwuCard::use(Room *room, ServerPlayer *kyoshiro, const QList<ServerPlayer *> &) const{
    
    const Card *show_card = NULL;
    
    room->broadcastInvoke("animate", "yanwu:");
    
    foreach(ServerPlayer *p, room->getOtherPlayers(kyoshiro)){
    
        QString suit_str = Sanguosha->getCard(getSubcards().first())->getSuitString();
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@yanwu-card-discard:%1::%2").arg(kyoshiro->getGeneralName()).arg(suit_str);
        
        show_card = room->askForCard(p, pattern, prompt);
        
        if(show_card){            
            room->throwCard(show_card);
            RecoverStruct recover;
            room->recover(kyoshiro, recover);
        }else{
            room->loseHp(p, 1);
        }

    }
    
    room->throwCard(this);
    kyoshiro->updateMp(-8);
}

class YanwuViewAsSkill: public OneCardViewAsSkill{
public:
    YanwuViewAsSkill():OneCardViewAsSkill("yanwu"){
        
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@yanwu";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YanwuCard *yanwu_card = new YanwuCard;
        yanwu_card->addSubcard(card_item->getCard()->getId());
        yanwu_card->setSkillName(objectName());

        return yanwu_card;
    }
};

class Yanwu: public PhaseChangeSkill{
public:
    Yanwu():PhaseChangeSkill("yanwu"){
        view_as_skill = new YanwuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
            && target->getPhase() == Player::Start
            && target->getMp() >= 8
            && target->getHandcardNum();
    }

    virtual bool onPhaseChange(ServerPlayer *kyoshiro) const{
        Room *room = kyoshiro->getRoom();
        
        if(room->askForUseCard(kyoshiro, "@@yanwu", "@yanwu")){
            kyoshiro->skip(Player::Play);
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------genjuro

//----------------------------------------------------------------------------- Yinghua

class YinghuaPindian: public TriggerSkill{
public:
    YinghuaPindian():TriggerSkill("#yinghua-pindian"){
        frequency = Compulsory;
        events << Pindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *genjuro, QVariant &data) const{
        Room *room = genjuro->getRoom();
        
        PindianStar pindian = data.value<PindianStar>();
        
        if(pindian->reason == "yinghua" 
        && pindian->from->hasSkill(objectName()) ) {
            int drawNum = pindian->from_card->getNumber() - pindian->to_card->getNumber();
            if(drawNum > 4) {
                drawNum = 4;
            }
            
            if(drawNum>0) {
                room->setPlayerMark(genjuro, "yinghua_drawcard", drawNum);
            }
        }

        return false;
    }
};

YinghuaCard::YinghuaCard(){}

bool YinghuaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(to_select->getHandcardNum()==0) 
        return false;

    return true;
}

void YinghuaCard::use(Room *room, ServerPlayer *genjuro, const QList<ServerPlayer *> &targets) const{
    genjuro->updateMp(-3);
    genjuro->pindian(targets.first(), "yinghua", this);
    
    if(genjuro->getMark("yinghua_drawcard")) {
        genjuro->drawCards(genjuro->getMark("yinghua_drawcard"));
        room->setPlayerMark(genjuro, "yinghua_drawcard", 0);
    }
}

class YinghuaViewAsSkill: public OneCardViewAsSkill{
public:
    YinghuaViewAsSkill():OneCardViewAsSkill("yinghua"){
        
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@yinghua";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YinghuaCard *yinghua_card = new YinghuaCard;
        yinghua_card->addSubcard(card_item->getCard()->getId());
        yinghua_card->setSkillName(objectName());

        return yinghua_card;
    }
};

class Yinghua: public PhaseChangeSkill{
public:
    Yinghua():PhaseChangeSkill("yinghua"){
        view_as_skill = new YinghuaViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
            && target->getPhase() == Player::Draw
            && target->getMp() >= 3
            && target->getHandcardNum();
    }

    virtual bool onPhaseChange(ServerPlayer *genjuro) const{
        Room *room = genjuro->getRoom();
        if(room->askForUseCard(genjuro, "@@yinghua", "@yinghua")){
            return true;
        }     
        
        return false;
    }
};

//----------------------------------------------------------------------------- Sudi

class SudiHaohmaru: public GameStartSkill{
public:
    SudiHaohmaru():GameStartSkill("#sudi-haohmaru"){}

    virtual bool triggerable(const ServerPlayer *genjuro) const{
        return GameStartSkill::triggerable(genjuro) && genjuro->getGeneralName() == "genjuro";
    }

    virtual void onGameStart(ServerPlayer *genjuro) const{
        Room *room = genjuro->getRoom();
        foreach(ServerPlayer *p, room->getOtherPlayers(genjuro)){
            if(p->getGeneralName()== "haohmaru") {
                room->playSkillEffect("meet_haohmaru");
                p->gainMark("@sudimark");
                break;
            }
        }
    }
};

SudiCard::SudiCard(){}

bool SudiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(to_select->getHp() != Self->getHp())
        return false;
        
    return true;
}

void SudiCard::use(Room *room, ServerPlayer *genjuro, const QList<ServerPlayer *> &targets) const{
    targets.first()->gainMark("@sudimark");
}

class SudiViewAsSkill: public ZeroCardViewAsSkill{
public:
    SudiViewAsSkill():ZeroCardViewAsSkill("sudi"){

    }
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@sudi";
    }
    
    virtual const Card *viewAs() const{
        SudiCard *card = new SudiCard;
        card->setSkillName(objectName());
        return card;
    }
};

class Sudi: public PhaseChangeSkill{
public:
    Sudi():PhaseChangeSkill("sudi"){
        view_as_skill = new SudiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        if(!PhaseChangeSkill::triggerable(player) || player->getPhase() != Player::Start) 
            return false;

        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getAllPlayers();
        
        bool canTrigger = false;
        foreach(ServerPlayer *p, players){
            if(p->getMark("@sudimark")) 
                return false;
            
            if(p!=player && p->getHp() == player->getHp())
                canTrigger = true;
        }
    
        return canTrigger;
    }

    virtual bool onPhaseChange(ServerPlayer *genjuro) const{
        
        if(genjuro->hasSkill("sudi")) {
            genjuro->getRoom()->askForUseCard(genjuro, "@@sudi", "@sudi");
        }
        return false;
    }
};

//----------------------------------------------------------------------------- Zhansha

class Zhansha: public TriggerSkill{
public:
    Zhansha():TriggerSkill("zhansha"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        if(!(player->hasSkill(objectName()) && player->isAlive() 
            && player->getPhase() == Player::Finish))
            
            return false;
            
        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getAllPlayers();
        
        foreach(ServerPlayer *p, players){
            if(p->getMark("@sudimark") && player->getMp()>=qMax(30, players.length()*5)) 
                return true;
        }
    
        return false;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *genjuro, QVariant &data) const{
        Room *room = genjuro->getRoom();
        ServerPlayer *victim = NULL;
        
        foreach(ServerPlayer *p, genjuro->getRoom()->getOtherPlayers(genjuro)){
            if(p->getMark("@sudimark")) 
                victim = p;
        }
        
        if(victim) {
            if(room->askForSkillInvoke(genjuro, objectName())) {
            
                QList<ServerPlayer *> players = room->getAllPlayers();            
                genjuro->updateMp(-qMax(30, players.length()*5));
            
                room->broadcastInvoke("animate", "zhansha:3000");
            
                DamageStruct damage;
                damage.from = genjuro;
                damage.to = victim;
                room->killPlayer(victim, &damage);
                
                if(victim->getGeneralName() == "haohmaru") {
                    room->playSkillEffect("kill_haohmaru");
                }else {
                    room->playSkillEffect(objectName());
                }
            }
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------sogetsu

//----------------------------------------------------------------------------- Fuyue

class Fuyue: public DistanceSkill{
public:
    Fuyue():DistanceSkill("fuyue"){}

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->hasSkill(objectName()))
            return +1;
        else
            return 0;
    }
};

//----------------------------------------------------------------------------- Yueyin

class Yueyin: public TriggerSkill{
public:
    Yueyin():TriggerSkill("yueyin"){
        events << CardEffected;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getPhase()== Player::NotActive;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *sogetsu, QVariant &data) const{
        
        ServerPlayer *card_user = NULL;

        CardEffectStruct effect = data.value<CardEffectStruct>();
        card_user = effect.from;
        
        if(sogetsu->getCards("he").length()<2 || card_user == sogetsu)
            return false;
        
        Room *room = sogetsu->getRoom();
            
        if(room->askForSkillInvoke(sogetsu, objectName(), data)){
            room->askForDiscard(sogetsu, objectName(), 2, false, true);
            
            room->playSkillEffect(objectName());
            
            sogetsu->updateMp(1);
            
            LogMessage log;
            log.type = "#Yueyin";
            log.from = sogetsu;
            room->sendLog(log);            
            
            return true;
        }        

        return false;
    }
};

//----------------------------------------------------------------------------- Jiefang

JiefangCard::JiefangCard(){
    target_fixed = true;
}

void JiefangCard::use(Room *room, ServerPlayer *sogetsu, const QList<ServerPlayer *> &targets) const{
    
    sogetsu->updateMp(-3);
    
    room->transfigure(sogetsu, "suija", false, false);
}

class JiefangViewAsSkill: public ZeroCardViewAsSkill{
public:
    JiefangViewAsSkill():ZeroCardViewAsSkill("jiefang"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@jiefang";
    }
    
    virtual const Card *viewAs() const{
        JiefangCard *card = new JiefangCard;
        card->setSkillName(objectName());
        return card;
    }
};

class Jiefang: public PhaseChangeSkill{
public:
    Jiefang():PhaseChangeSkill("jiefang"){
        view_as_skill = new JiefangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return PhaseChangeSkill::triggerable(player) && player->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *sogetsu) const{
        
        if(sogetsu->getMp()>=3) {
            sogetsu->getRoom()->askForUseCard(sogetsu, "@@jiefang", "@jiefang");
        }
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------suija

//----------------------------------------------------------------------------- Yuelun

class Yuelun: public SlashBuffSkill{
public:
    Yuelun():SlashBuffSkill("yuelun"){
        frequency = Compulsory;
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        if(effect.slash->inherits("IceBang") && !effect.slash->isVirtualCard())
            effect.from->getRoom()->playSkillEffect(objectName());
            
        return false;
    }
};

//----------------------------------------------------------------------------- Siyue

SiyueCard::SiyueCard(){
    once = true;
}

bool SiyueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void SiyueCard::use(Room *room, ServerPlayer *suija, const QList<ServerPlayer *> &targets) const{
    suija->updateMp(-2);

    const Card *card = Sanguosha->getCard(getSubcards().first());
    
    IceBang *iceBang = new IceBang(card->getSuit(), card->getNumber());

    LogMessage log;
    log.type = "$SiyueEffect";
    log.from = suija;
    log.to << targets;
    room->sendLog(log);    
    
    suija->getRoom()->throwCard(this);
    
    iceBang->use(room, suija, targets);

}

class Siyue: public OneCardViewAsSkill{
public:
    Siyue():OneCardViewAsSkill("siyue"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return  player->getMp()>=2;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SiyueCard *card = new SiyueCard;
        card->setSkillName(objectName());
        card->addSubcard(card_item->getCard()->getId());
        
        return card;
    }
};

//----------------------------------------------------------------------------- Fengyin

class Fengyin: public PhaseChangeSkill{
public:
    Fengyin():PhaseChangeSkill("fengyin"){}

    virtual bool triggerable(const ServerPlayer *player) const{
        return PhaseChangeSkill::triggerable(player) && player->getPhase() == Player::Finish && player->getMp()>0;
    }

    virtual bool onPhaseChange(ServerPlayer *suija) const{
        Room *room = suija->getRoom();
        
        if(room->askForSkillInvoke(suija, objectName())) {
            
            room->playSkillEffect(objectName());
        
            suija->updateMp(-1);
            room->transfigure(suija, "sogetsu", false, false);
        }

        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------kazuki

//----------------------------------------------------------------------------- Rexue

class Rexue: public PhaseChangeSkill{
public:
    Rexue():PhaseChangeSkill("rexue"){
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return PhaseChangeSkill::triggerable(player) && player->getPhase() == Player::Finish;
    }

    virtual bool onPhaseChange(ServerPlayer *kazuki) const{
    
        kazuki->getRoom()->playSkillEffect(objectName());
    
        LogMessage log;
        log.type = "#Rexue";
        log.from = kazuki;
        kazuki->getRoom()->sendLog(log);
    
        kazuki->updateMp(1);
        return false;
    }
};

//----------------------------------------------------------------------------- Yanmie

YanmieBang::YanmieBang(Card::Suit suit, int number):FireBang(suit, number) {
    mute = true;
}
    
class Yanmie: public FilterSkill{
public:
    Yanmie():FilterSkill("yanmie"){}

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Bang") && !to_select->isEquipped();
    }
    
    virtual int getEffectIndex(ServerPlayer *, const Card *) const{
        return 1;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        YanmieBang *bang = new YanmieBang(card->getSuit(), card->getNumber());
        bang->addSubcard(card_item->getCard()->getId());
        bang->setSkillName(objectName());
        
        return bang;
    }
};

class YanmieLv1: public TriggerSkill{
public:
    YanmieLv1():TriggerSkill("#yanmie"){
        frequency = Compulsory;
        events << Damage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMp()>0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *kazuki, QVariant &data) const{
    
        Room *room = kazuki->getRoom();        
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.nature == DamageStruct::Fire && !damage.to->isNude()) {
            
            kazuki->updateMp(-1);
            
            if(room->testRandomEvent(damage.from, "yanmie_bao", 20)) {
                
                room->playSkillEffect("yanmie", 2);

                int card_id = room->askForCardChosen(kazuki, damage.to, "he", "yanmie_bao");       

                LogMessage log;
                log.type = "$YanmieDiscard";
                log.to << damage.to;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);
                
                room->throwCard(card_id);
            }else {
                LogMessage log;
                log.type = "#YanmieFail";
                log.from = kazuki;
                room->sendLog(log);
            }
            
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Juexing

class Juexing: public PhaseChangeSkill{
public:
    Juexing():PhaseChangeSkill("juexing"){
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return PhaseChangeSkill::triggerable(player) 
            && player->getPhase() == Player::Start
            && player->getMp()>4;
    }

    virtual bool onPhaseChange(ServerPlayer *kazuki) const{
        Room *room = kazuki->getRoom();
        
        room->playSkillEffect(objectName());

        LogMessage log;
        log.type = "#Juexing";
        log.from = kazuki;
        room->sendLog(log);
        
        room->transfigure(kazuki, "enja", false, false);

        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------enja

//----------------------------------------------------------------------------- Fanshi

class Fanshi: public PhaseChangeSkill{
public:
    Fanshi():PhaseChangeSkill("fanshi"){
        frequency = Compulsory;
    }
    
    virtual QString getDefaultChoice(ServerPlayer *enja) const{
        return "FanshiEffect1";
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return PhaseChangeSkill::triggerable(player) 
            && player->getPhase() == Player::Finish;
    }

    virtual bool onPhaseChange(ServerPlayer *enja) const{
        
        Room *room = enja->getRoom();
        
        room->playSkillEffect(objectName());
        
        LogMessage log;
        log.type = "#Fanshi";
        log.from = enja;
        room->sendLog(log); 

        if(enja->getMp()) {
            QStringList choices;
        
            choices << "FanshiEffect1" << "FanshiEffect2"; 
            
            QString choice = room->askForChoice(enja, objectName(), choices.join("+"));
            
            if(choice == "FanshiEffect1") {
                enja->updateMp(-1);
            }else if(choice == "FanshiEffect2") {
                room->loseHp(enja,1);
            }
            
        }else {
            room->loseHp(enja,1);
        }

        return false;
    }
};

//----------------------------------------------------------------------------- YanmieHigh

class YanmieHigh: public TriggerSkill{
public:
    YanmieHigh():TriggerSkill("yanmie_high"){
        frequency = Compulsory;
        events << Predamage << Damage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *enja, QVariant &data) const{
    
        Room *room = enja->getRoom();        
        
        if(event == Predamage) {
            DamageStruct damage = data.value<DamageStruct>();
            
            room->playSkillEffect(objectName());
            
            if(damage.nature != DamageStruct::Fire) {            

                damage.nature = DamageStruct::Fire;
                data = QVariant::fromValue(damage);
                
                LogMessage log;
                log.type = "#DamageToFire";
                log.to << damage.to;
                log.arg = objectName();
                room->sendLog(log);    
            
            }
            
        }else if(event == Damage) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(!damage.to->isNude()) {
            
                int card_id = room->askForCardChosen(enja, damage.to, "he", objectName());

                LogMessage log;
                log.type = "$YanmieDiscard";
                log.to << damage.to;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);
                
                room->throwCard(card_id);
            }
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Baosha

BaoshaCard::BaoshaCard(){
    once = true;
    target_fixed = true;
}

void BaoshaCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *enja = card_use.from;
    
    enja->updateMp(-2);
    room->playSkillEffect("baosha");
   
    QList<ServerPlayer *> use_to, targets = room->getOtherPlayers(enja);
    
    foreach(ServerPlayer *p, targets) {
        if(enja->inMyAttackRange(p)) {           
            use_to << p;
        }
    }
    
    CardUseStruct use;
    use.card = Sanguosha->getCard(getSubcards().first());
    use.from = enja;
    use.to << use_to;
    
    LogMessage log;
    log.type = "#Baosha";
    log.from = enja;
    log.to << use_to;
    room->sendLog(log);    

    room->useCard(use);
}

class Baosha: public OneCardViewAsSkill{
public:
    Baosha():OneCardViewAsSkill("baosha"){}

    virtual bool isEnabledAtPlay(const Player *player) const{    
        return !player->hasUsed("BaoshaCard") && player->getMp()>=2 && Slash::IsAvailable(player);
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        BaoshaCard *card = new BaoshaCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());

        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------galford

//----------------------------------------------------------------------------- Renquan

RenquanCard::RenquanCard(){
    once = true;
    will_throw = true;
}

bool RenquanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(Self->distanceTo(to_select) > 2) 
        return false;

    return true;
}

void RenquanCard::onEffect(const CardEffectStruct &effect) const {
    Room *room = effect.from->getRoom();
    
    JudgeStruct judge;
    judge.pattern = QRegExp("(.*)");
    judge.good = true;
    judge.reason = objectName();
    judge.who = effect.from;
    
    room->judge(judge);
    
    if(judge.card->isRed()) {
    
        DamageStruct damageMaker;
        damageMaker.card = effect.card;
        damageMaker.from = effect.from;
        damageMaker.to = effect.to;
        damageMaker.damage = 1;
        damageMaker.nature = DamageStruct::Normal;
        room->damage(damageMaker);
        
    }else if(judge.card->isBlack() && !effect.to->isNude()) {
                
        int card_id = room->askForCardChosen(effect.from, effect.to, "he", "renquan");      

        LogMessage log;
        log.type = "$RenquanDiscard";
        log.to << effect.to;
        log.card_str = Sanguosha->getCard(card_id)->toString();
        room->sendLog(log);
        
        room->throwCard(card_id);
        
    }
}

class Renquan: public OneCardViewAsSkill{
public:
    Renquan():OneCardViewAsSkill("renquan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{    
        return !player->hasUsed("RenquanCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        RenquanCard *card = new RenquanCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());

        return card;
    }
};

//----------------------------------------------------------------------------- Dianguang

class DianguangOn: public TriggerSkill{
public:
    DianguangOn():TriggerSkill("#dianguang-on"){
        frequency = Compulsory;
        events << Predamage << Predamaged << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@dianguang");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *galford, QVariant &data) const{    
        Room *room = galford->getRoom();        
        
        if(event == Predamage) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(damage.nature == DamageStruct::Normal) {            

                damage.nature = DamageStruct::Thunder;
                data = QVariant::fromValue(damage);
                
                LogMessage log;
                log.type = "#DamageToThunder";
                log.to << damage.to;
                log.arg = "dianguang";
                room->sendLog(log);                
            }
            
        }else if(event == Predamaged) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(damage.nature == DamageStruct::Thunder) {            
                
                LogMessage log;
                log.type = "#DamageNullify";
                log.to << damage.to;
                log.arg = "dianguang";
                room->sendLog(log);   

                return true;
            
            }
        }else if(event == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();
            
            if(damage.from) {
                const Card *weapon = damage.from->getWeapon();
                
                if(weapon) {
                
                    LogMessage log;
                    log.type = "$DianguangEffect";
                    log.from = damage.from;
                    log.card_str = weapon->toString();
                    room->sendLog(log);      
                
                    room->throwCard(weapon);
                }
            }
            galford = damage.to;
            
            galford->loseMark("@dianguang");
        }
        
        return false;
        
    }
};

DianguangCard::DianguangCard(){
    target_fixed = true;
    once = true;
}

void DianguangCard::use(Room *room, ServerPlayer *galford, const QList<ServerPlayer *> &) const{

    galford->updateMp(-3);
    galford->gainMark("@dianguang");
    
}

class Dianguang: public ZeroCardViewAsSkill{
public:
    Dianguang():ZeroCardViewAsSkill("dianguang"){}
    
    virtual bool isEnabledAtPlay(const Player *galford) const{
        return !galford->getMark("@dianguang") && galford->getMp()>=3;
    }
    
    virtual const Card *viewAs() const{
        DianguangCard *card = new DianguangCard;
        card->setSkillName(objectName());
        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------rimururu

//----------------------------------------------------------------------------- Bingren

BingrenBang::BingrenBang(Card::Suit suit, int number)
    :IceBang(suit, number) {
}

class BingrenOn: public FilterSkill{
public:
    BingrenOn():FilterSkill("bingren_on"){}

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Bang");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        BingrenBang *bang = new BingrenBang(card->getSuit(), card->getNumber());
        bang->addSubcard(card_item->getCard()->getId());
        bang->setSkillName(objectName());

        return bang;
    }
};

class BingrenOff: public TriggerSkill{
public:
    BingrenOff():TriggerSkill("#bingren-off"){
        frequency = Compulsory;
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        
        CardMoveStar move = data.value<CardMoveStar>();

        if(move->from_place == Player::Equip && player->isAlive()){
            
            if(!player->getWeapon()) {
                Room *room = player->getRoom();
                
                LogMessage log;
                log.type = "#BingrenOff";
                log.from = player;
                room->sendLog(log);
                
                player->loseMark("@bingren");
                room->detachSkillFromPlayer(player, "bingren_on");
            }
        }
            
        return false;
        
    }
};

BingrenCard::BingrenCard(){
    once = true;
}

bool BingrenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
        
    if(!to_select->getWeapon()) 
        return false;

    return true;
}

void BingrenCard::use(Room *room, ServerPlayer *rimururu, const QList<ServerPlayer *> &targets) const{
    rimururu->updateMp(-3);
    
    ServerPlayer *target = targets.first();    
    target->gainMark("@bingren");
    rimururu->getRoom()->acquireSkill(target, "bingren_on");
    
}

class Bingren: public ZeroCardViewAsSkill{
public:
    Bingren():ZeroCardViewAsSkill("bingren"){}
    
    virtual bool isEnabledAtPlay(const Player *rimururu) const{
        return !rimururu->hasUsed("BingrenCard") && rimururu->getMp()>=3;
    }
    
    virtual const Card *viewAs() const{
        BingrenCard *card = new BingrenCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Chuixue

ChuixueBang::ChuixueBang(){
    target_fixed = true;
}

void ChuixueBang::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *jink = room->askForCard(effect.to, "jink", "chuixue-jink:" + effect.from->objectName());
    if(jink == NULL){
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.from = effect.from;
        damage.to = effect.to;
        damage.nature = DamageStruct::Ice;

        room->damage(damage);
    }else {
        room->playCardEffect("dodge", effect.to->getGeneral()->isMale());
        room->broadcastInvoke("animate", "dodge:"+effect.to->objectName());
    }
}

ChuixueCard::ChuixueCard(){
    mute = true;
    once = true;
    target_fixed = true;
}

void ChuixueCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *rimururu = card_use.from;
    
    rimururu->updateMp(-5);
    
    QList<ServerPlayer *> targets = room->getOtherPlayers(rimururu);
    
    room->playSkillEffect("chuixue");
    
    CardUseStruct use;
    use.card = new ChuixueBang;
    use.from = rimururu;
    use.to << targets;

    room->useCard(use);
}

class Chuixue: public ZeroCardViewAsSkill{
public:
    Chuixue():ZeroCardViewAsSkill("chuixue"){}
    
    virtual bool isEnabledAtPlay(const Player *rimururu) const{
        return !rimururu->hasUsed("ChuixueCard") && rimururu->getMp()>=5;
    }
    
    virtual const Card *viewAs() const{
        ChuixueCard *card = new ChuixueCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Shenjing

class Shenjing: public TriggerSkill{
public:
    Shenjing():TriggerSkill("shenjing"){
        frequency = Compulsory;
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *rimururu, QVariant &data) const{
    
        Room *room = rimururu->getRoom();
        
        LogMessage log;
        log.type = "$Shenjing";
        log.from = rimururu;
        room->sendLog(log);  
    
        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(spade|club):(.*)");
        judge.good = true;
        judge.reason = objectName();
        judge.who = rimururu;
        
        room->judge(judge);
        
        if(judge.isGood()) {
        
            room->playSkillEffect(objectName());
            
            LogMessage log;
            log.type = "#DamageNullify";
            log.to << rimururu;
            log.arg = objectName();
            room->sendLog(log);            
            
            rimururu->updateMp(1);
            return true;
        }
            
        return false;
        
    }
};

//--------------------------------------------------------------------------------------------------------------charlotte

//----------------------------------------------------------------------------- Pokong

class Pokong: public TriggerSkill{
public:
    Pokong():TriggerSkill("pokong"){
        events << SlashHit;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *charlotte, QVariant &data) const{
         SlashEffectStruct effect = data.value<SlashEffectStruct>();
        
        const Horse *horse = effect.to->getHorse();
        Room *room = charlotte->getRoom();
        
        if(horse) {
            if(charlotte->askForSkillInvoke(objectName(), data)) {
            
                room->playSkillEffect(objectName());
                
                LogMessage log;
                log.type = "$PokongEffect";
                log.to << effect.to;
                log.card_str = horse->toString();
                room->sendLog(log);
                
                room->throwCard(horse);
            }
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Xunguang

class XunguangViewAsSkill: public ZeroCardViewAsSkill{
public:
    XunguangViewAsSkill():ZeroCardViewAsSkill("xunguang"){}
    
    virtual bool isEnabledAtPlay(const Player *charlotte) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@xunguang";
    }
    
    virtual const Card *viewAs() const{
        int card_id = Self->getMark("xunguangcard")-1;
        const Card *card = Sanguosha->getCard(card_id);

        return card;
    }
};

class Xunguang: public TriggerSkill{
public:
    Xunguang():TriggerSkill("xunguang"){
        view_as_skill = new XunguangViewAsSkill;    
        events << CardFinished;
    }

    virtual int getPriority() const{
        return 2;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return (target->getPhase() == Player::Play && TriggerSkill::triggerable(target) && target->getMp()>=1) || target->getMark("xunguangcard")>0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *charlotte, QVariant &data) const{
        Room *room = charlotte->getRoom();
        
        if(charlotte->getMark("xunguangcard")>0) {
            room->setPlayerMark(charlotte, "xunguangcard", 0);
            return false;
        }
        
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Slash")) {
            
            if(charlotte->askForSkillInvoke(objectName(), data)){
            
                charlotte->updateMp(-1);

                room->getThread()->delay();            
                const Card *open_card = room->peek();
                
                LogMessage log;
                log.type = "$Peek";
                log.from = charlotte;
                log.card_str = open_card->toString();
                room->sendLog(log);
                
                if(open_card->inherits("Slash"))
                    room->setPlayerFlag(charlotte, "one_more_bang");
                
                if(open_card->isAvailable(charlotte)) {
                    room->setPlayerMark(charlotte, "xunguangcard", open_card->getId()+1);
                    
                    room->playSkillEffect(objectName());
                    
                    AI *ai = charlotte->getAI();
                    if(ai){
                        CardUseStruct ai_card_use;                        
                        ai_card_use.from = charlotte;
                        ai_card_use.card = open_card;
                        
                        if(open_card->targetFixed()) {
                            room->useCard(ai_card_use);
                        }else {
                            room->askForUseCard(charlotte, "@@xunguang", "@xunguang");
                        }
                        
                    }else {
                        while(!room->askForUseCard(charlotte, "@@xunguang", "@xunguang"))
                            ; //empty loop 
                    }
                    
                }else {
                    LogMessage log;
                    log.type = "$XunguangThrow";
                    log.from = charlotte;
                    log.card_str = open_card->toString();
                    room->sendLog(log);
                
                    room->throwCard(open_card);
                }                
            }
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------hanzo

//----------------------------------------------------------------------------- Kongchan

KongchanCard::KongchanCard(){
    mute = true;
    target_fixed = true;
}

void KongchanCard::use(Room *room, ServerPlayer *hanzo, const QList<ServerPlayer *> &) const{
    room->throwCard(Sanguosha->getCard(getSubcards().first()));
}

class KongchanViewAsSkill: public OneCardViewAsSkill{
public:
    KongchanViewAsSkill():OneCardViewAsSkill("kongchan"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@kongchan";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("EquipCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        KongchanCard *card = new KongchanCard;
        card->addSubcard(card_item->getFilteredCard());
        
        return card;
    }
};

class Kongchan: public TriggerSkill{
public:
    Kongchan():TriggerSkill("kongchan"){
        view_as_skill = new KongchanViewAsSkill;
        events << Predamaged;
    }
    
    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *hanzo, QVariant &data) const{
        Room *room = hanzo->getRoom();
        
        DamageStruct damage = data.value<DamageStruct>();
        
        if(room->askForUseCard(hanzo, "@@kongchan", "@kongchan")) {
        
            room->playSkillEffect(objectName());

            if(damage.from && damage.from != hanzo) {
                QVariant tohelp = QVariant::fromValue((PlayerStar)damage.from);
                
                const Card *bang = room->askForCard(hanzo, "slash", "@kongchan-bang:"+damage.from->objectName(), tohelp);
                
                if(bang) {
                
                    CardUseStruct use;
                    use.card = bang;
                    use.from = hanzo;
                    use.to << damage.from;

                    LogMessage log;
                    log.type = "#KongchanBang";
                    log.from = hanzo;
                    room->sendLog(log);
                    
                    room->useCard(use);
                }
            }
            
            return true;
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Yingwu

class YingwuOff: public TriggerSkill{
public:
    YingwuOff():TriggerSkill("#yingwu-off"){
        events << CardUsed;
        frequency = Compulsory;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@yingwu");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *hanzo, QVariant &data) const{
        Room *room = hanzo->getRoom();
        
        LogMessage log;
        log.type = "#YingwuOff";
        log.from = hanzo;
        room->sendLog(log);
        
        hanzo->loseMark("@yingwu");

        return false;
    }
};

class YingwuOn: public TriggerSkill{
public:
    YingwuOn():TriggerSkill("#yingwu-on"){
        events << CardEffected;
        frequency = Compulsory;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@yingwu");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *hanzo, QVariant &data) const{
        
        ServerPlayer *card_user = NULL;
        const Card *card = NULL;

        CardEffectStruct effect = data.value<CardEffectStruct>();
        card_user = effect.from;
        card = effect.card;
        
        if(card_user == hanzo)
            return false;
        
        Room *room = hanzo->getRoom();
        
        if(room->testRandomEvent(hanzo, "yingwu", 50)) {
            LogMessage log;
            log.type = "$Yingwu";
            log.from = hanzo;
            log.card_str = card->toString();
            room->sendLog(log);                
            return true; 
        }

        return false;
    }
};

class Yingwu: public TriggerSkill{
public:
    Yingwu():TriggerSkill("yingwu"){
        events << Damage;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMp()>=1 && !target->getMark("@yingwu");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *hanzo, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        
        if(!damage.card || !damage.card->inherits("Slash")) {
            return false;
        }            
        
        Room *room = hanzo->getRoom();
        
        if(hanzo->askForSkillInvoke(objectName(), data)) {
        
            room->playSkillEffect(objectName());
            
            hanzo->updateMp(-1);
            hanzo->gainMark("@yingwu");
            
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Chenyin

class ChenyinOff: public PhaseChangeSkill{
public:
    ChenyinOff():PhaseChangeSkill("#chenyin-off"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom(); 
        foreach(ServerPlayer *p, room->getAllPlayers()){
            int times = p->getMark("chenyin_on");
            if(times==1) {
                room->allSkillRelease(p);
            }
            
            if(times>0) {
                int rest = times-1;
                
                room->setPlayerMark(p, "chenyin_on", rest);
                
                if(rest>0) {
                    LogMessage log;
                    log.type = "#CountDown";
                    log.from = p;
                    log.arg = "@skill_forbid";
                    log.arg2 = QString::number(rest);
                    room->sendLog(log);
                }
            }
        }
       
        return false;
    }
};

ChenyinCard::ChenyinCard(){
    mute = true;
}

bool ChenyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty()
    || to_select == Self
    || to_select->getHandcardNum()<=0
    || Self->distanceTo(to_select) > 1
    || to_select->getMark("@skill_forbid")) {
        
        return false;
    }
        
    return true;
}

void ChenyinCard::onUse(Room *room, const CardUseStruct &card_use) const {

    ServerPlayer *target = card_use.to.first();
    ServerPlayer *hanzo = card_use.from;

    hanzo->updateMp(-3);
    bool success = hanzo->pindian(target, "chenyin", this);
    
    if(success) {
    
        room->playSkillEffect("chenyin");    
        room->broadcastInvoke("animate", "chenyin");
    
        DamageStruct damageMaker;
        damageMaker.card = this;
        damageMaker.from = hanzo;
        damageMaker.to = target;
        damageMaker.damage = 1;
        damageMaker.nature = DamageStruct::Normal;
        room->damage(damageMaker);

        room->allSkillForbid(target);
        
        //because chenyin invoke in start phase so +1
        room->setPlayerMark(target, "chenyin_on", room->getAllPlayers().length()*2+1);
    }
    
}

class ChenyinViewAsSkill: public OneCardViewAsSkill{
public:
    ChenyinViewAsSkill():OneCardViewAsSkill("chenyin"){}

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@chenyin";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ChenyinCard *card = new ChenyinCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());

        return card;
    }
};

class Chenyin: public PhaseChangeSkill{
public:
    Chenyin():PhaseChangeSkill("chenyin"){
        view_as_skill = new ChenyinViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *hanzo) const{
        Room *room = hanzo->getRoom();       
        
        if(hanzo->getMp()>=3 && hanzo->getHandcardNum()>0) {
            if(room->askForUseCard(hanzo, "@@chenyin", "@chenyin")){
                hanzo->skip(Player::Play);
            }
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------jubei

//----------------------------------------------------------------------------- Xinyan

class Xinyan: public TriggerSkill{
public:
    Xinyan():TriggerSkill("xinyan"){
        events << Predamaged;
        frequency = Compulsory;
    }
    
    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->getWeapon();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *jubei, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        
        if(damage.card && damage.card->inherits("Slash")
        && damage.from->getWeapon() 
        && !jubei->getWeapon()) {
        
            jubei->getRoom()->playSkillEffect(objectName());
        
            LogMessage log;
            log.type = "$XinyanEffect";
            log.from = jubei;
            log.to << damage.from;
            log.card_str = damage.card->toString();
            jubei->getRoom()->sendLog(log);            
        
            return true;
        }

        return false;
    }
};

//----------------------------------------------------------------------------- Shuangyue

class ShuangyueOff: public TriggerSkill{
public:
    ShuangyueOff():TriggerSkill("#shuangyue-off"){
        frequency = Compulsory;
        events << CardLost;
    }
    
    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->hasFlag("changing_weapon");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *jubei, QVariant &data) const{
        
        CardMoveStar move = data.value<CardMoveStar>();

        if(move->from_place == Player::Equip && jubei->isAlive()){
            
            if(!jubei->getWeapon()) {
                Room *room = jubei->getRoom();
                
                LogMessage log;
                log.type = "#ShuangyueOff";
                log.from = jubei;
                room->sendLog(log);
                
                room->setPlayerFlag(jubei, "-one_more_bang");
                room->detachSkillFromPlayer(jubei, "shuangyue");
                
                int card_id =  jubei->getPile("erdao_weapon").first();
                const Card *weapon = Sanguosha->getCard(card_id);
                
                room->moveCardTo(weapon, jubei, Player::Hand);
            }
        }
            
        return false;
        
    }
};

ShuangyueCard::ShuangyueCard(){
    target_fixed = true;
}

void ShuangyueCard::use(Room *room, ServerPlayer *jubei, const QList<ServerPlayer *> &) const{
    
    const Weapon *weapon = jubei->getWeapon();
    
    room->setPlayerFlag(jubei, "changing_weapon");

    weapon->onUninstall(jubei);
    jubei->addToPile("erdao_weapon", weapon->getEffectiveId(), false);

    int card_id =  jubei->getPile("erdao_weapon").first();
    const Card *new_weapon = Sanguosha->getCard(card_id);
    room->moveCardTo(new_weapon, jubei, Player::Equip, true);
    
    room->setPlayerFlag(jubei, "one_more_bang");
    room->setPlayerFlag(jubei, "-changing_weapon");
}

class Shuangyue: public ZeroCardViewAsSkill{
public:
    Shuangyue():ZeroCardViewAsSkill("shuangyue"){}
    
    virtual bool isEnabledAtPlay(const Player *jubei) const{
        return !jubei->hasUsed("ShuangyueCard");
    }
    
    virtual const Card *viewAs() const{
        ShuangyueCard *card = new ShuangyueCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Erdao

ErdaoCard::ErdaoCard(){
    target_fixed = true;
}

void ErdaoCard::use(Room *room, ServerPlayer *jubei, const QList<ServerPlayer *> &) const{
    const Card *weapon = Sanguosha->getCard(getSubcards().first());
    jubei->addToPile("erdao_weapon", weapon->getEffectiveId(), false);
    
    jubei->updateMp(-2);
    
    room->acquireSkill(jubei, "shuangyue");
}

class Erdao: public OneCardViewAsSkill{
public:
    Erdao():OneCardViewAsSkill("erdao"){}

    virtual bool isEnabledAtPlay(const Player *jubei) const{
        return jubei->getMp()>=2 && jubei->getWeapon() && !jubei->hasSkill("shuangyue");
    }
    
    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ErdaoCard *card = new ErdaoCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());

        return card;
    }
};

//----------------------------------------------------------------------------- Daoqu

DaoquCard::DaoquCard(){}

bool DaoquCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() 
    || to_select == Self 
    || Self->distanceTo(to_select) > 1
    || !to_select->getWeapon()) {
        return false;
    }
        
    return true;
}

void DaoquCard::use(Room *room, ServerPlayer *jubei, const QList<ServerPlayer *> &targets) const {
    room->throwCard(this);
    jubei->obtainCard(targets.first()->getWeapon());
}

class DaoquViewAsSkill: public OneCardViewAsSkill{
public:
    DaoquViewAsSkill():OneCardViewAsSkill("daoqu"){}

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@daoqu";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        DaoquCard *card = new DaoquCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());

        return card;
    }
};

class Daoqu: public PhaseChangeSkill{
public:
    Daoqu():PhaseChangeSkill("daoqu"){
        view_as_skill = new DaoquViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Finish;
    }

    virtual bool onPhaseChange(ServerPlayer *jubei) const{
        Room *room = jubei->getRoom(); 
        
        if(jubei->getHandcardNum()>0) {
            room->askForUseCard(jubei, "@@daoqu", "@daoqu");
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------shizumaru

//----------------------------------------------------------------------------- Wuyu

WuyuCard::WuyuCard(){
    target_fixed = true;
}

void WuyuCard::use(Room *room, ServerPlayer *shizumaru, const QList<ServerPlayer *> &) const {
    room->throwCard(this);
    
    int num = getSubcards().length();
    
    if(num) {
        RecoverStruct recover;
        recover.recover = num;
        room->recover(shizumaru, recover);
    }
}

class WuyuRecover: public ViewAsSkill{
public:
    WuyuRecover():ViewAsSkill("wuyu"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@wuyu";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int lose_hp = Self->getMaxHP() - Self->getHp();
        
        if(to_select->isEquipped())
            return false;
        
        if(to_select->getFilteredCard()->getSuit() != Card::Spade)
            return false;
        
        if(selected.length()>=lose_hp)
            return false;
        
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        WuyuCard *card = new WuyuCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        return card;
    }
};

class Wuyu:public MasochismSkill{
public:
    Wuyu():MasochismSkill("wuyu"){
        view_as_skill = new WuyuRecover;
    }

    virtual void onDamaged(ServerPlayer *shizumaru, const DamageStruct &damage) const{
        Room *room = shizumaru->getRoom();

        if(shizumaru->isAlive()) {
            room->askForUseCard(shizumaru, "@@wuyu", "@wuyu");
        }
    }
};

//----------------------------------------------------------------------------- Meiyu

MeiyuCard::MeiyuCard(){
    target_fixed = true;
}

void MeiyuCard::use(Room *room, ServerPlayer *shizumaru, const QList<ServerPlayer *> &) const {
    room->throwCard(this);    
    
    shizumaru->updateMp(-2);
    shizumaru->drawCards(2);
}

class Meiyu: public OneCardViewAsSkill{
public:
    Meiyu():OneCardViewAsSkill("meiyu"){}

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMp()>1 && player->getHandcardNum();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MeiyuCard *card = new MeiyuCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());

        return card;
    }
};

//----------------------------------------------------------------------------- Baoyu

BaoyuCard::BaoyuCard(){
    mute = true;
}

bool BaoyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(!Self->inMyAttackRange(to_select)) 
        return false;

    return true;
}

void BaoyuCard::onEffect(const CardEffectStruct &effect) const{

    ServerPlayer *shizumaru = effect.from;
    Room *room = shizumaru->getRoom();   	
    
    room->playSkillEffect("baoyu");
    
    shizumaru->updateMp(-5);    
    room->broadcastInvoke("animate", "baoyu");    
	room->throwCard(this);
    
    QList<const Card *> cards = shizumaru->getCards("he");
    
    foreach(const Card *card, cards) {
        if(card->isBlack()) {
            room->throwCard(card);
            
            const Card *jink = room->askForCard(effect.to, "jink", "@baoyu-jink:"+shizumaru->objectName());
            
            if(!jink) {
                DamageStruct damageMaker;
                damageMaker.from = shizumaru;
                damageMaker.to = effect.to;
                room->damage(damageMaker);
            }
        }
    }

}

class BaoyuViewAsSkill: public ZeroCardViewAsSkill{
public:
    BaoyuViewAsSkill():ZeroCardViewAsSkill("baoyu"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
    
    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@baoyu";
    }
    
    virtual const Card *viewAs() const{
        BaoyuCard *card = new BaoyuCard;
        card->setSkillName(objectName());
        return card;
    }
};

class Baoyu: public PhaseChangeSkill{
public:
    Baoyu():PhaseChangeSkill("baoyu"){
        view_as_skill = new BaoyuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        bool can_invoke = PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMp() >= 5
                && !target->isNude();
                
        if(can_invoke) {
            foreach(const Card *card, target->getCards("he")) {
                if(card->isBlack())
                    return true;
            }
        }
        
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *shizumaru) const{
        Room *room = shizumaru->getRoom();
        
        if(room->askForUseCard(shizumaru, "@@baoyu", "@baoyu")){
            shizumaru->skip(Player::Play);
            shizumaru->turnOver();
        }
        
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------genan

//----------------------------------------------------------------------------- ChaoxiuGet

ChaoxiuCard::ChaoxiuCard(){
    target_fixed = true;
    mute = true;
}

void ChaoxiuCard::use(Room *room, ServerPlayer *genan, const QList<ServerPlayer *> &) const {
    room->throwCard(this);    
    
    const Card *chaoxiu = room->getOffCourtCard("chaoxiu");
    
    if(chaoxiu) {
        //a mark for exclucive card same as objectName
        room->setPlayerMark(genan, "chaoxiu", 1);
    
        room->playSkillEffect("chaoxiu_get");
    
        LogMessage log;
        log.type = "#ChaoxiuGet";
        log.from = genan;
        room->sendLog(log);
        
        genan->obtainCard(chaoxiu);
    }
    
}

class ChaoxiuGet: public OneCardViewAsSkill{
public:
    ChaoxiuGet():OneCardViewAsSkill("chaoxiu_get"){}

    virtual bool isEnabledAtPlay(const Player *player) const{
    
        if(player->hasWeapon("chaoxiu"))
            return false;
            
        foreach(const Card *card, Self->getCards()) {
            if(card->inherits("Chaoxiu"))
                return false;
        }
        
        return true;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("Weapon") && !to_select->getFilteredCard()->isExclusive();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ChaoxiuCard *card = new ChaoxiuCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());

        return card;
    }
};

//----------------------------------------------------------------------------- Duchui

DuchuiBang::DuchuiBang(Card::Suit suit, int number):PoisonBang(suit, number) {
    setObjectName("duchui");
}

DuchuiCard::DuchuiCard(){
    once = true;
}

bool DuchuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;
        
    if(!Self->inMyAttackRange(to_select))
        return false;

    return true;
}


void DuchuiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *genan = card_use.from;
    
    genan->updateMp(-4);
    room->playSkillEffect("duchui");

    LogMessage log;
    log.type = "#DuchuiEffect";
    log.from = genan;
    log.to << card_use.to;
    room->sendLog(log);  
    
    CardUseStruct use;
    use.card = new DuchuiBang(Card::NoSuit, 0);
    use.from = genan;
    use.to << card_use.to;

    room->useCard(use);
}

class Duchui: public ZeroCardViewAsSkill{
public:
    Duchui():ZeroCardViewAsSkill("duchui"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMp() >= 4 && !player->hasUsed("DuchuiCard");
    }
    
    virtual const Card *viewAs() const{
        DuchuiCard *card = new DuchuiCard;
        card->setSkillName(objectName());
        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------earthquake

//----------------------------------------------------------------------------- Dashi

DashiCard::DashiCard(){
    target_fixed = true;
}

void DashiCard::use(Room *room, ServerPlayer *earthquake, const QList<ServerPlayer *> &targets) const {
    room->throwCard(this);
    
    earthquake->updateMp(-1);
    
    RecoverStruct recover;
    room->recover(earthquake, recover);
}

class Dashi: public ViewAsSkill{
public:
    Dashi():ViewAsSkill("dashi"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMp() && !player->isNude() && player->isWounded();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length()>=2)
            return false;

        if(selected.length() && to_select->getFilteredCard()->getTypeId() != selected.first()->getFilteredCard()->getTypeId())
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length()!=2)
            return NULL;

        DashiCard *card = new DashiCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Roudan

RoudanBang::RoudanBang(Card::Suit suit, int number):Bang(suit, number) {
    setObjectName("roudan");
}

class RoudanDamage: public TriggerSkill{
public:
    RoudanDamage():TriggerSkill("#roudan-damage"){
        frequency = Compulsory;
        events << CardFinished << Damage;
    }
    
    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->hasFlag("roudaning");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *earthquake, QVariant &data) const{
        Room *room = earthquake->getRoom();
        
        if(event == Damage) {
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.damage) {
                room->setPlayerMark(earthquake, "roudan", earthquake->getMark("roudan")+damage.damage);
            }            
        }else if(event == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->objectName() == "roudan") {

                if(earthquake->getMark("roudan")<2) {
                    
                    LogMessage log;
                    log.type = "#RoudanMpUp";
                    log.from = earthquake;
                    room->sendLog(log);
                    
                    room->playSkillEffect("roudan", 2);
                    
                    earthquake->updateMp(1);
                }
                
                room->setPlayerFlag(earthquake, "-roudaning");
                room->setPlayerMark(earthquake, "roudan", 0);
                
            }            
        }

        return false;

    }
};

RoudanCard::RoudanCard(){
    once = true;
    target_fixed = true;
    mute = true;
}

void RoudanCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *earthquake = card_use.from;
    
    room->loseHp(earthquake, 1);
    
    QList<ServerPlayer *> use_to, targets = room->getOtherPlayers(earthquake);
    
    foreach(ServerPlayer *p, targets) {
        if(earthquake->distanceTo(p)<=2) {           
            use_to << p;
        }
    }
    
    CardUseStruct use;
    use.card = new RoudanBang(Card::NoSuit, 0);
    use.from = earthquake;
    use.to << use_to;
    
    LogMessage log;
    log.type = "#Roudan";
    log.from = earthquake;
    log.to << use_to;
    room->sendLog(log);

    room->playSkillEffect("roudan", 1);
    
    room->setPlayerFlag(earthquake, "roudaning");
    room->useCard(use);
}

class Roudan: public ZeroCardViewAsSkill{
public:
    Roudan():ZeroCardViewAsSkill("roudan"){}
    
    virtual bool isEnabledAtPlay(const Player *earthquake) const{
        return !earthquake->hasUsed("RoudanCard");
    }
    
    virtual const Card *viewAs() const{
        RoudanCard *card = new RoudanCard;
        card->setSkillName(objectName());
        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------tamtam

//----------------------------------------------------------------------------- Mianju

MianjuCard::MianjuCard(){
    target_fixed = true;
    mute = true;
}

void MianjuCard::use(Room *room, ServerPlayer *tamtam, const QList<ServerPlayer *> &) const {
    room->throwCard(this);    
    
    const Card *mask = room->getOffCourtCard("violent_mask");
    
    if(mask) {
        //a mark for exclucive card same as objectName
        room->setPlayerMark(tamtam, "violent_mask", 1);
        room->playSkillEffect("mianju", 1);
    
        LogMessage log;
        log.type = "#Mianju";
        log.from = tamtam;
        room->sendLog(log);
        
        room->loseHp(tamtam, 1);        
        tamtam->obtainCard(mask);
    }
    
}

class Mianju: public OneCardViewAsSkill{
public:
    Mianju():OneCardViewAsSkill("mianju"){}

    virtual bool isEnabledAtPlay(const Player *player) const{
    
        if(player->hasArmorEffect("violent_mask"))
            return false;
            
        foreach(const Card *card, Self->getCards()) {
            if(card->inherits("ViolentMask"))
                return false;
        }
        
        return true;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("EquipCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MianjuCard *card = new MianjuCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());

        return card;
    }
};

//----------------------------------------------------------------------------- Tuteng

class TutengCost: public PhaseChangeSkill{
public:
    TutengCost():PhaseChangeSkill("#tuteng-cost"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Finish
                && target->hasRelic("shaman_totem");
    }

    virtual bool onPhaseChange(ServerPlayer *tamtam) const{
        Room *room = tamtam->getRoom();
        LogMessage log;
        
        if(tamtam->getMp() && room->askForSkillInvoke(tamtam, "tuteng_cost")){
            tamtam->updateMp(-1);
            
            log.type = "#TutengCost";
            log.from = tamtam;
            room->sendLog(log);
            
        }else {
            log.type = "#TutengDiscard";
            log.from = tamtam;
            room->sendLog(log);
            
            room->throwCard(tamtam->getRelic());
        }
        
        return false;
    }
};

TutengCard::TutengCard(){
    target_fixed = true;
    mute = true;
}

void TutengCard::use(Room *room, ServerPlayer *tamtam, const QList<ServerPlayer *> &) const {
    room->throwCard(this);    
    
    const Card *totem = room->getOffCourtCard("shaman_totem");
    
    if(totem) {
        //a mark for exclucive card same as objectName
        room->setPlayerMark(tamtam, "shaman_totem", 1);
        room->playSkillEffect("tuteng");
    
        LogMessage log;
        log.type = "#Tuteng";
        log.from = tamtam;
        room->sendLog(log);
        
        tamtam->obtainCard(totem);
    }
    
}

class Tuteng: public OneCardViewAsSkill{
public:
    Tuteng():OneCardViewAsSkill("tuteng"){}

    virtual bool isEnabledAtPlay(const Player *player) const{
    
        if(player->hasRelic("shaman_totem"))
            return false;
            
        foreach(const Card *card, Self->getCards()) {
            if(card->inherits("ShamanTotem"))
                return false;
        }
        
        return true;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->inherits("EquipCard") && !card->isExclusive() && !card->inherits("Horse");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        TutengCard *card = new TutengCard;
        card->addSubcard(card_item->getCard()->getId());
        card->setSkillName(objectName());

        return card;
    }
};

//--------------------------------------------------------------------------------------------------------------basara

//----------------------------------------------------------------------------- Yuannian

class Yuannian: public TriggerSkill{
public:
    Yuannian():TriggerSkill("yuannian"){
        frequency = Compulsory;
        events << AskForPeachesDone;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *basara, QVariant &data) const{
        Room *room = basara->getRoom();
        
        if(basara->getMp()>=10) {
            DyingStruct dying = data.value<DyingStruct>();
            
            room->playSkillEffect(objectName());
            room->broadcastInvoke("animate", "yuannian");
            
            LogMessage log;
            log.type = "#Yuannian";
            log.from = basara;
            room->sendLog(log);
            
            basara->updateMp(-10);
            
            RecoverStruct recover;
            recover.recover = dying.damage->damage - basara->getHp() + 1;
            room->recover(basara, recover);
            
            if(basara->getHandcardNum()>0) {
                basara->throwAllHandCards();
            }
            
        }
        
        return false;

    }
};

//----------------------------------------------------------------------------- Sinian

class SinianEffect: public TriggerSkill{
public:
    SinianEffect():TriggerSkill("#sinian-effect"){
        frequency = Compulsory;
        events << Damaged << HpRecover;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) || target->getMark("@gouhuo");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        
        if(event == Damaged && player->getMark("@gouhuo")) {
            
            foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
                if(p->hasSkill("sinian")) {
                    room->playSkillEffect("sinian", 2);
                    
                    LogMessage log;
                    log.type = "#SinianDamage";
                    log.from = player;
                    log.to << p;
                    room->sendLog(log);
                    
                    p->updateMp(5);
                    break;
                }                    
            }
            
        }else if(event == HpRecover && player->hasSkill("sinian")) {
            
            foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
                if(p->getMark("@gouhuo")) {
                    room->playSkillEffect("sinian", 3);
                    
                    LogMessage log;
                    log.type = "#SinianRecover";
                    log.from = player;
                    log.to << p;
                    room->sendLog(log);
                    
                    p->drawCards(2);
                    break;
                }
            }
            
        }
        
        return false;

    }
};

class Sinian: public PhaseChangeSkill{
public:
    Sinian():PhaseChangeSkill("sinian"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *basara) const{
        Room *room = basara->getRoom();
        
        QList<ServerPlayer *> female;
        foreach(ServerPlayer *p, room->getOtherPlayers(basara)) {
            if(p->getGeneral()->isFemale())
                female << p;
        }
        
        if(female.length() && room->askForSkillInvoke(basara, objectName())) {
            ServerPlayer *gouhuo = room->askForPlayerChosen(basara, female, objectName());
            if(gouhuo) {
                room->playSkillEffect("sinian", 1);
                
                if(gouhuo->getMark("@gouhuo")) {
                    return false;
                }else {
                    foreach(ServerPlayer *p, room->getOtherPlayers(basara)) {
                        if(p->getMark("@gouhuo")) {
                            p->loseMark("@gouhuo");
                            break;
                        }
                    }
                
                    gouhuo->gainMark("@gouhuo");
                }
            }
        }
        
        return false;
    }
};

//----------------------------------------------------------------------------- Yingxi

YingxiCard::YingxiCard(){
    target_fixed = true;
}

void YingxiCard::use(Room *room, ServerPlayer *basara, const QList<ServerPlayer *> &) const {
    basara->updateMp(-6);
    basara->turnOver();
    
    basara->gainMark("@hidden");
    room->setPlayerMark(basara, "yingxi_on", room->getAllPlayers().length());
}

class Yingxi: public ZeroCardViewAsSkill{
public:
    Yingxi():ZeroCardViewAsSkill("yingxi"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMp()>=6 && !player->hasUsed("YingxiCard");
    }
    
    virtual const Card *viewAs() const{
        YingxiCard *card = new YingxiCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Yingchu

class Yingchu: public PhaseChangeSkill{
public:
    Yingchu():PhaseChangeSkill("yingchu"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom(); 
        foreach(ServerPlayer *p, room->getAllPlayers()){
            int times = p->getMark("yingxi_on");
            if(times==1) {
                p->loseMark("@hidden");
                room->setPlayerMark(p, "yingxi_on", 0);
                
                if(p->hasSkill("yingchu")) {
                    
                    LogMessage log;
                    log.type = "#yingchu";
                    log.from = p;
                    room->sendLog(log);
                
                    RecoverStruct recover;
                    room->recover(p, recover);
                    
                    ServerPlayer *target = room->askForPlayerChosen(p, room->getAllPlayers(), "yingchu");
                    
                    if(target) {
                        Bang *bang = new Bang(Card::NoSuit, 0);
                        bang->setSkillName("yingchu");
                        CardUseStruct use;
                        use.card = bang;
                        use.from = p;
                        use.to << target;

                        room->useCard(use);
                    }
                    
                }
            }
            
            if(times>0) {
                int rest = times-1;
                
                room->setPlayerMark(p, "yingxi_on", rest);
                
                if(rest>0) {
                    LogMessage log;
                    log.type = "#CountDown";
                    log.from = p;
                    log.arg = "@hidden";
                    log.arg2 = QString::number(rest);
                    room->sendLog(log);
                }
            }
        }
       
        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------amakusa

//----------------------------------------------------------------------------- Xieyou

XieyouCard::XieyouCard(){
    once = true;
    mute = true;
}

bool XieyouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void XieyouCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *amakusa = effect.from;
    Room *room = amakusa->getRoom();
    
    room->playSkillEffect("xieyou", 1);
    
    Card::Suit suit1 = room->askForSuit(effect.to, "xieyou");
    
    if(suit1) {
        QString suit_str = Card::Suit2String(suit1);
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@xieyou-card:::%1").arg(suit_str);
        
        const Card *card = room->askForCard(amakusa, pattern, prompt);
        
        if(card) {
        
            room->playSkillEffect("xieyou", 2);
            
            room->loseHp(effect.to, 1);
            amakusa->updateMp(2);
        }else {
            room->playSkillEffect("xieyou", 3);
            
            if(!amakusa->getMp()) {
                room->loseHp(amakusa, 1);
                return;
            }
            
            QStringList choices;
            choices << "XieyouLoseHp" << "XieyouLoseMp";
        
            if(choices.length()) {
                QString choice = room->askForChoice(amakusa, "xieyou", choices.join("+"));
                
                if(choice == "XieyouLoseHp") {
                    room->loseHp(amakusa, 1);
                }else {
                    amakusa->updateMp(-2);
                }
            }
        }
    }
}

class Xieyou: public ZeroCardViewAsSkill{
public:
    Xieyou():ZeroCardViewAsSkill("xieyou"){}
    
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("XieyouCard");
    }
    
    virtual const Card *viewAs() const{
        XieyouCard *card = new XieyouCard;
        card->setSkillName(objectName());
        return card;
    }
};

//----------------------------------------------------------------------------- Mozhang

class MozhangEffect: public ProhibitSkill{
public:
    MozhangEffect():ProhibitSkill("mozhang_effect"){}

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return card->inherits("SingleTargetTrick") && from!=to;
    }
};

class MozhangOff: public PhaseChangeSkill{
public:
    MozhangOff():PhaseChangeSkill("#mozhang-off"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom(); 
        foreach(ServerPlayer *p, room->getAllPlayers()){
            int times = p->getMark("mozhang_on");
            if(times==1) {
                p->loseMark("@mozhang");
                room->detachSkillFromPlayer(p, "mozhang_effect");
            }
            
            if(times>0) {
                int rest = times-1;
                
                room->setPlayerMark(p, "mozhang_on", rest);
                
                if(rest>0) {
                    LogMessage log;
                    log.type = "#CountDown";
                    log.from = p;
                    log.arg = "@mozhang";
                    log.arg2 = QString::number(rest);                    
                    room->sendLog(log);
                }
            }
        }
       
        return false;
    }
};

MozhangCard::MozhangCard(){
    target_fixed = true;
}

void MozhangCard::use(Room *room, ServerPlayer *amakusa, const QList<ServerPlayer *> &) const {
    room->throwCard(this);
    
    amakusa->updateMp(-4);
    amakusa->gainMark("@mozhang");
    room->setPlayerMark(amakusa, "mozhang_on", room->getAllPlayers().length());
    
    room->acquireSkill(amakusa, "mozhang_effect");
}

class MozhangViewAsSkill: public OneCardViewAsSkill{
public:
    MozhangViewAsSkill():OneCardViewAsSkill("mozhang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@mozhang";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MozhangCard *card = new MozhangCard;
        card->addSubcard(card_item->getCard());
        card->setSkillName(objectName());
        
        return card;
    }
};

class Mozhang: public PhaseChangeSkill{
public:
    Mozhang():PhaseChangeSkill("mozhang"){
        view_as_skill = new MozhangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Finish
                && !target->getMark("@mozhang")
                && target->getMp() >= 4;
    }

    virtual bool onPhaseChange(ServerPlayer *amakusa) const{
        Room *room = amakusa->getRoom();        
        room->askForUseCard(amakusa, "@@mozhang", "@askformozhang");
        return false;
    }
};

//----------------------------------------------------------------------------- Xiezhilingyu

class XiezhilingyuOff: public PhaseChangeSkill{
public:
    XiezhilingyuOff():PhaseChangeSkill("#xiezhilingyu-off"){}

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom(); 
        foreach(ServerPlayer *p, room->getAllPlayers()){
            int times = p->getMark("xiezhilingyu_on");
            if(times==1) {
                p->loseMark("@card_forbid");
            }
            
            if(times>0) {
                int rest = times-1;
                
                room->setPlayerMark(p, "xiezhilingyu_on", rest);
                
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

class Xiezhilingyu: public TriggerSkill{
public:
    Xiezhilingyu():TriggerSkill("xiezhilingyu$"){
        can_forbid = false;
        events << CardDiscarded;
    }
    
    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->getLord()->hasSkill(objectName()) && target->getPhase() == Player::Discard;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    
        Room *room = player->getRoom();
        
        CardStar card = data.value<CardStar>();
        foreach(int card_id, card->getSubcards()){
            if(Sanguosha->getCard(card_id)->inherits("TrickCard")) {
                
                if(room->testRandomEvent(player, objectName(), 50)) {
                    player->gainMark("@card_forbid");
                    room->setPlayerMark(player, "xiezhilingyu_on", 2);
                }
                
                break;
            }
        }

        return false;
    }
};

//--------------------------------------------------------------------------------------------------------------End

MeleeSSPackage::MeleeSSPackage()
    :Package("meleess")
{
    General *haohmaru, *nakoruru, *ukyo, *kyoshiro, *genjuro, *sogetsu, *suija, *kazuki, *enja, *galford, *rimururu, *charlotte, *hanzo, *jubei, *shizumaru, *genan,
                *earthquake, *tamtam, *basara, *amakusa;

    haohmaru = new General(this, "haohmaru", "nu");
    haohmaru->addSkill(new Jiuqi);
    haohmaru->addSkill(new Tianba);
    
    addMetaObject<TianbaCard>();
    
    nakoruru = new General(this, "nakoruru", "ling", 3, false);
    nakoruru->addSkill(new Goutong);
    nakoruru->addSkill(new Yingxuan);
    nakoruru->addSkill(new Chongci);
    
    addMetaObject<GoutongCard>();
    addMetaObject<YingxuanCard>();
    
    skills << new XuankongSkill;
    
    ukyo = new General(this, "ukyo", "qi", 3);
    ukyo->addSkill(new Xuruo);
    ukyo->addSkill(new Juhe);
    ukyo->addSkill(new Liulian);
    
    addMetaObject<LiulianCard>();
    
    kyoshiro = new General(this, "kyoshiro", "kuang");
    kyoshiro->addSkill(new Quwu);
    kyoshiro->addSkill(new Yanwu);
    
    addMetaObject<QuwuCard>();
    addMetaObject<YanwuCard>();
    
    genjuro = new General(this, "genjuro", "yuan");
    genjuro->addSkill(new Yinghua);
    genjuro->addSkill(new YinghuaPindian);
    related_skills.insertMulti("yinghua", "#yinghua-pindian");
    genjuro->addSkill(new Sudi);
    genjuro->addSkill(new SudiHaohmaru);
    related_skills.insertMulti("yinghua", "#sudi-haohmaru");
    genjuro->addSkill(new Zhansha);    
    
    addMetaObject<YinghuaCard>();
    addMetaObject<SudiCard>();
    
    sogetsu = new General(this, "sogetsu", "ling", 3);
    sogetsu->addSkill(new Fuyue);
    sogetsu->addSkill(new Yueyin);
    sogetsu->addSkill(new Jiefang);
    
    addMetaObject<JiefangCard>();
    
    suija = new General(this, "suija", "ling", 3, true, true);
    suija->addSkill(new Yuelun);
    suija->addSkill(new Siyue);
    suija->addSkill(new Fengyin);
    
    addMetaObject<SiyueCard>();
    
    kazuki = new General(this, "kazuki", "nu");
    kazuki->addSkill(new Rexue);
    kazuki->addSkill(new Yanmie);
    kazuki->addSkill(new YanmieLv1);
    related_skills.insertMulti("yanmie", "#yanmie");
    kazuki->addSkill(new Juexing);
    
    enja = new General(this, "enja", "nu", 4, true, true);
    enja->addSkill(new Fanshi);
    enja->addSkill(new YanmieHigh);
    enja->addSkill(new Baosha);
    
    addMetaObject<BaoshaCard>();
    
    galford = new General(this, "galford", "qi");    
    galford->addSkill(new Renquan);
    galford->addSkill(new Dianguang);
    galford->addSkill(new DianguangOn);
    related_skills.insertMulti("dianguang", "#dianguang-on");
    
    addMetaObject<RenquanCard>();
    addMetaObject<DianguangCard>();
        
    rimururu = new General(this, "rimururu", "ling", 3, false);
    rimururu->addSkill(new Bingren);
    rimururu->addSkill(new Chuixue);
    rimururu->addSkill(new Shenjing);
    rimururu->addSkill(new MarkAssignSkill("@armor_forbid", 1));
    related_skills.insertMulti("shenjing", "#@armor_forbid");
    
    addMetaObject<BingrenCard>();
    addMetaObject<ChuixueCard>();
    addMetaObject<ChuixueBang>();
    
    skills << new BingrenOn << new BingrenOff;
    related_skills.insertMulti("bingren_on", "#bingren-off");
    
    charlotte = new General(this, "charlotte", "qi", 4, false);
    charlotte->addSkill(new Pokong);
    charlotte->addSkill(new Xunguang);
    
    hanzo = new General(this, "hanzo", "qi", 3);
    hanzo->addSkill(new Kongchan);
    hanzo->addSkill(new Yingwu);
    hanzo->addSkill(new YingwuOn);
    hanzo->addSkill(new YingwuOff);
    related_skills.insertMulti("yingwu", "#yingwu-on");
    related_skills.insertMulti("yingwu", "#yingwu-off");
    hanzo->addSkill(new Chenyin);
    hanzo->addSkill(new ChenyinOff);
    related_skills.insertMulti("chenyin", "#chenyin-off");
    
    addMetaObject<KongchanCard>();
    addMetaObject<ChenyinCard>();
    
    jubei = new General(this, "jubei", "qi");
    jubei->addSkill(new Xinyan);
    jubei->addSkill(new Erdao);
    jubei->addSkill(new Daoqu);
    
    addMetaObject<ErdaoCard>();
    addMetaObject<DaoquCard>();
    addMetaObject<ShuangyueCard>();
    
    skills << new Shuangyue << new ShuangyueOff;
    related_skills.insertMulti("shuangyue", "#shuangyue-off");
    
    shizumaru = new General(this, "shizumaru", "kuang", 3);
    shizumaru->addSkill(new Wuyu);
    shizumaru->addSkill(new Meiyu);
    shizumaru->addSkill(new Baoyu);
    
    addMetaObject<WuyuCard>();
    addMetaObject<MeiyuCard>();
    addMetaObject<BaoyuCard>();
    
    genan = new General(this, "genan", "kuang");
    genan->addSkill(new ChaoxiuGet);
    genan->addSkill(new Duchui);
    
    addMetaObject<ChaoxiuCard>();
    addMetaObject<DuchuiCard>();
    
    earthquake = new General(this, "earthquake", "nu", 4);
    earthquake->addSkill(new Dashi);
    earthquake->addSkill(new Roudan);
    earthquake->addSkill(new RoudanDamage);
    related_skills.insertMulti("roudan", "#roudan-damage");
    
    addMetaObject<DashiCard>();
    addMetaObject<RoudanCard>();
    
    tamtam = new General(this, "tamtam", "kuang");
    tamtam->addSkill(new Mianju);
    tamtam->addSkill(new Tuteng);
    tamtam->addSkill(new TutengCost);
    related_skills.insertMulti("tuteng", "#tuteng-cost");
    
    addMetaObject<MianjuCard>();
    addMetaObject<TutengCard>();
    
    basara = new General(this, "basara", "yuan", 3);
    basara->addSkill(new Yuannian);
    basara->addSkill(new Sinian);
    basara->addSkill(new SinianEffect);
    related_skills.insertMulti("sinian", "#sinian-effect");
    basara->addSkill(new Yingxi);
    basara->addSkill(new Yingchu);
    
    addMetaObject<YingxiCard>();
    
    amakusa = new General(this, "amakusa$", "ling");
    amakusa->addSkill(new Xieyou);
    amakusa->addSkill(new Mozhang);
    amakusa->addSkill(new MozhangOff);
    related_skills.insertMulti("mozhang", "#mozhang-off");
    amakusa->addSkill(new Xiezhilingyu);
    amakusa->addSkill(new XiezhilingyuOff);
    related_skills.insertMulti("xiezhilingyu", "#xiezhilingyu-off");
    
    skills << new MozhangEffect;
    
    addMetaObject<XieyouCard>();
    addMetaObject<MozhangCard>();
}

ADD_PACKAGE(MeleeSS);


