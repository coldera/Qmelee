#include "gamerule.h"
#include "serverplayer.h"
#include "room.h"
#include "standard.h"
#include "meleecard.h"
#include "engine.h"

#include <QTime>

GameRule::GameRule(QObject *parent)
    :TriggerSkill("game_rule")
{
    setParent(parent);

    events << GameStart << TurnStart << PhaseChange << CardUsed
            << CardEffected << HpRecover << HpLost << AskForPeachesDone
            << AskForPeaches << Death << Dying << GameOverJudge
            << SlashHit << SlashMissed << SlashEffected << SlashProceed
            << DamageDone << DamageComplete
            << StartJudge << FinishJudge << Pindian;
}

bool GameRule::triggerable(const ServerPlayer *) const{
    return true;
}

int GameRule::getPriority() const{
    return 0;
}

void GameRule::onPhaseChange(ServerPlayer *player) const{
    Room *room = player->getRoom();
    switch(player->getPhase()){
    case Player::Start: {
            player->setMark("SlashCount", 0);
            
            //modify by ce add for shtm
            ServerPlayer *dead = NULL;
            foreach(ServerPlayer *target, room -> getServerPlayers()){
                if(target->getMark("@revive_rite") && target->isDead()) {
                    dead = target;
                    break;
                }
            }
            
            if(dead) {
                QVariant tohelp = QVariant::fromValue((PlayerStar)dead);
                const Card *card = room->askForCard(player, ".touse", "revive-rite-handcard", tohelp);
                if(card) {
                    room->moveCardTo(card, dead, Player::Hand, false);
                    
                    int cardNum = dead->getHandcardNum();
                    if(cardNum >= 4) {                        
                        room->revivePlayer(dead);
                        room->setPlayerProperty(dead, "hp", 2);
                        if(dead->getRelic())
                            room->throwCard(dead->getRelic());
                    }
                }
            }
            
            break;
        }
    case Player::Judge: {
            QList<const DelayedTrick *> tricks = player->delayedTricks();
            while(!tricks.isEmpty() && player->isAlive()){
                const DelayedTrick *trick = tricks.takeLast();
                bool on_effect = room->cardEffect(trick, NULL, player);
                if(!on_effect)
                    trick->onNullified(player);
            }

            break;
        }
    case Player::Draw: {
            QVariant num = 2;
            if(room->getTag("FirstRound").toBool() && room->getMode() == "02_1v1"){
                room->setTag("FirstRound", false);
                num = 1;
            }

            room->getThread()->trigger(DrawNCards, player, num);
            int n = num.toInt();
            if(n > 0)
                player->drawCards(n, false);
            break;
        }

    case Player::Play: {
            player->clearHistory();

            while(player->isAlive()){
                CardUseStruct card_use;
                room->activate(player, card_use);
                if(card_use.isValid()){
                    room->useCard(card_use);
                }else
                    break;
            }
            break;
        }

    case Player::Discard:{
            int discard_num = player->getHandcardNum() - player->getMaxCards();
            if(player->hasFlag("jilei")){
                QSet<const Card *> jilei_cards;
                QList<const Card *> handcards = player->getHandcards();
                foreach(const Card *card, handcards){
                    if(player->isJilei(card))
                        jilei_cards << card;
                }

                if(jilei_cards.size() > player->getMaxCards()){
                    // show all his cards
                    room->showAllCards(player);

                    foreach(const Card *card, handcards.toSet() - jilei_cards){
                        room->throwCard(card);
                    }

                    return;
                }
            }

            if(discard_num > 0)
                room->askForDiscard(player, "gamerule", discard_num);
            break;
        }
    case Player::Finish: {
            break;
        }

    case Player::NotActive:{
            if(player->hasFlag("drank")){
                LogMessage log;
                log.type = "#UnsetDrankEndOfTurn";
                log.from = player;
                room->sendLog(log);

                room->setPlayerFlag(player, "-drank");
            }

            player->clearFlags();

            return;
        }
    }
}

void GameRule::setGameProcess(Room *room) const{
    int good = 0, bad = 0;
    QList<ServerPlayer *> players = room->getAlivePlayers();
    foreach(ServerPlayer *player, players){
        switch(player->getRoleEnum()){
        case Player::Lord:
        case Player::Loyalist: good ++; break;
        case Player::Rebel: bad++; break;
        case Player::Renegade: break;
        }
    }

    QString process;
    if(good == bad)
        process = "Balance";
    else if(good > bad)
        process = "LordSuperior";
    else
        process = "RebelSuperior";

    room->setTag("GameProcess", process);
}

bool GameRule::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();

    if(room->getTag("SkipGameRule").toBool()){
        room->removeTag("SkipGameRule");
        return false;
    }

    switch(event){
    case GameStart: {
            if(player->getGeneral()->getKingdom() == "god"){
                QString new_kingdom = room->askForKingdom(player);
                room->setPlayerProperty(player, "kingdom", new_kingdom);

                LogMessage log;
                log.type = "#ChooseKingdom";
                log.from = player;
                log.arg = new_kingdom;
                room->sendLog(log);
            }

            if(player->isLord())
                setGameProcess(room);

            player->drawCards(4, false);

            if(room->getMode() == "02_1v1")
                room->setTag("FirstRound", true);

            break;
        }

    case TurnStart:{
            player = room->getCurrent();
            
            //modify by ce
            if(player->getMark("@card_forbid"))
                break;
            else if(!player->faceUp())
                player->turnOver();
            else if(player->isAlive())
                player->play();

            break;
        }

    case PhaseChange: onPhaseChange(player); break;
    case CardUsed: {
            if(data.canConvert<CardUseStruct>()){
                CardUseStruct card_use = data.value<CardUseStruct>();
                const Card *card = card_use.card;

                card_use.from->playCardEffect(card);
                card->use(room, card_use.from, card_use.to);
                
                //modify by ce
                if(card->inherits("TrickCard")) {
                    if(card_use.from->getKingdom() == "ling") {
                        card_use.from->updateMp(1);
                    }
                    
                    if(card->inherits("NothingIsSomething") || card->inherits("Thunder")) {
                        if(card_use.from->getKingdom() == "qi") {
                            card_use.from->updateMp(1);
                        }
                    }
                    
                    foreach(ServerPlayer *qi, card_use.to){
                        if(qi->getKingdom() == "qi" && !card->inherits("AOE") && !card->inherits("GlobalEffect")) {
                            qi->updateMp(1);
                        }
                    }
                    
                }
                
                if(card->inherits("Slash") || card->inherits("AOE") || card->inherits("Burn") ||
                card->inherits("PoisonMarish") || card->inherits("PK")) {
                    if(card_use.from->getKingdom() == "kuang") {
                        card_use.from->updateMp(1);
                    }
                }
            }

            break;
        }

    case HpRecover:{
            RecoverStruct recover_struct = data.value<RecoverStruct>();
            int recover = recover_struct.recover;

            //modify by ce
            int mp = qMin(recover, player->getMaxHP()-player->getHp());

            int new_hp = qMin(player->getHp() + recover, player->getMaxHP());
            room->setPlayerProperty(player, "hp", new_hp);
            room->broadcastInvoke("hpChange", QString("%1:%2").arg(player->objectName()).arg(recover));
            
            //modify by ce
            if(mp != 0) {
                foreach(ServerPlayer *yuan, room->getAllPlayers()){
                    if(yuan->getKingdom() == "yuan" && yuan->getMark("@banruo")<=0){
                        yuan->updateMp(-mp);
                    }
                }
            }
            
            break;
        }

    case HpLost:{
            int lose = data.toInt();
            
            //modify by ce
            // if(room->getCurrent()->hasSkill("jueqing"))
                // return true;

            LogMessage log;
            log.type = "#LoseHp";
            log.from = player;
            log.arg = QString::number(lose);
            room->sendLog(log);

            room->setPlayerProperty(player, "hp", player->getHp() - lose);
            room->broadcastInvoke("hpChange", QString("%1:%2").arg(player->objectName()).arg(-lose));
            
            //modify by ce add
            if( player->getKingdom() == "nu") {
                player->updateMp(lose);
            }            

            if(player->getHp() <= 0)
                room->enterDying(player, NULL);

            break;
    }

    case Dying:{
            if(player->getHp() > 0){
                player->setFlags("-dying");
                break;
            }

            QList<ServerPlayer *> savers;
            // ServerPlayer *current = room->getCurrent();
            
            //modify by ce
            DyingStruct dying = data.value<DyingStruct>();
            if(dying.damage && dying.damage->card && dying.damage->card->objectName() == "ice_bang") {
                savers << player;
                
                LogMessage log;
                log.type = "#DyingByIce";
                log.to << player;
                room->sendLog(log);
                
            }else if(player->getMark("@card_forbid")) {
            
                savers = room->getOtherPlayers(player);
                
            }
            // else if(current->hasSkill("wansha") && current->isAlive()) {
                // room->playSkillEffect("wansha");

                // savers << current;

                // LogMessage log;
                // log.from = current;
                // if(current != player){
                    // savers << player;
                    // log.type = "#WanshaTwo";
                    // log.to << player;
                // }else{
                    // log.type = "#WanshaOne";
                // }

                // room->sendLog(log);

            // }
            else
                savers = room->getAllPlayers();

            LogMessage log;
            log.type = "#AskForPeaches";
            log.from = player;
            log.to = savers;
            log.arg = QString::number(1 - player->getHp());
            room->sendLog(log);

            RoomThread *thread = room->getThread();
            foreach(ServerPlayer *saver, savers){
                if(player->getHp() > 0)
                    break;

                thread->trigger(AskForPeaches, saver, data);
            }

            player->setFlags("-dying");
            thread->trigger(AskForPeachesDone, player, data);

            break;
        }

    case AskForPeaches:{
            DyingStruct dying = data.value<DyingStruct>();

            while(dying.who->getHp() <= 0){
                const Card *peach = room->askForSinglePeach(player, dying.who);
                if(peach == NULL)
                    break;

                CardUseStruct use;
                use.card = peach;
                use.from = player;
                if(player != dying.who)
                    use.to << dying.who;

                room->useCard(use, false);
            }

            break;
        }

    case AskForPeachesDone:{
            if(player->getHp() <= 0){
                DyingStruct dying = data.value<DyingStruct>();
                room->killPlayer(player, dying.damage);
            }

            break;
        }

    case DamageDone:{
            
            if(player->getMark("@invincible")) {
                LogMessage log;
                log.type = "#Invincible";
                log.from = player;
                room->sendLog(log);                
                return true;
            }
    
            DamageStruct damage = data.value<DamageStruct>();
            room->sendDamageLog(damage);
            room->applyDamage(player, damage);
            
            //modify by ce add
            if( player->getKingdom() == "nu") {
                player->updateMp(damage.damage);
            }            
            foreach(ServerPlayer *yuan, room->getAllPlayers()){
                if(yuan->getKingdom() == "yuan" && damage.damage!=0 && !yuan->hasSkill("tusha")){
                    yuan->updateMp(damage.damage*2);
                }
            }
            
            LogMessage log;
            log.type = "$DamagePunish";
            room->sendLog(log);
            
            QString kingdom = player->getKingdom();
            if(kingdom == "nu" && damage.nature == DamageStruct::Air) {
                LogMessage log;
                log.type = "#PunishNu";
                log.from = player;
                room->sendLog(log);
                
                player->updateMp(-1);
            }else if(kingdom == "qi" && damage.nature == DamageStruct::Poison) {
                
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade):(.*)");
                judge.good = false;
                judge.reason = "poison_nature";
                judge.who = player;

                room->judge(judge);
                
                if(judge.isBad()) {
                    LogMessage log;
                    log.type = "#PunishQi";
                    log.from = player;
                    room->sendLog(log);
                    room->loseHp(player,1);
                }
            }else if(kingdom == "ling" && damage.nature == DamageStruct::Thunder) {
                foreach(ServerPlayer *ling, room->getOtherPlayers(player)){
                    if(ling->getKingdom() == "ling"){
                        
                        JudgeStruct judge;
                        judge.pattern = QRegExp("(.*):(diamond):(.*)");
                        judge.good = false;
                        judge.reason = "thunder_nature";
                        judge.who = ling;

                        room->judge(judge);
                        
                        if(judge.isBad()) {
                            LogMessage log;
                            log.type = "#PunishLing";
                            log.from = ling;
                            room->sendLog(log);
                            
                            if(ling->getArmor()) {
                                room->throwCard(ling->getArmor());
                            }
                            if(ling->getHorse()) {
                                room->throwCard(ling->getHorse());
                            }
                        }
                    }
                }                
            }else if(kingdom == "kuang" && damage.nature == DamageStruct::Ice) {
                LogMessage log;
                log.type = "#PunishKuang";
                log.from = player;
                room->sendLog(log);
                
                if(player->getHandcardNum()>0) {
                    int card_id = player->getRandomHandCardId();
                    room->throwCard(card_id);
                }
            }else if(kingdom == "yuan" && damage.nature == DamageStruct::Fire) {
                
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart):(.*)");
                judge.good = false;
                judge.reason = "fire_nature";
                judge.who = player;

                room->judge(judge);
                
                if(judge.isBad()) {
                    LogMessage log;
                    log.type = "#PunishYuan";
                    log.from = player;
                    room->sendLog(log);
                    room->loseHp(player,1);
                }
            }
            
            if(player->getHp() <= 0){
                room->enterDying(player, &damage);
            }

            break;
        }

    case DamageComplete:{
            if(room->getMode() == "02_1v1" && player->isDead()){
                QString new_general = player->tag["1v1ChangeGeneral"].toString();
                if(!new_general.isEmpty())
                    changeGeneral1v1(player);
            }

            bool chained = player->isChained();
            if(!chained)
                break;

            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Normal){
                room->setPlayerProperty(player, "chained", false);

                // iron chain effect
                QList<ServerPlayer *> chained_players = room->getOtherPlayers(player);
                foreach(ServerPlayer *chained_player, chained_players){
                    if(chained_player->isChained()){
                        room->getThread()->delay();
                        room->setPlayerProperty(chained_player, "chained", false);

                        LogMessage log;
                        // log.type = "#IronChainDamage"; modify by ce
                        log.type = "#SoulChainDamage";
                        log.from = chained_player;
                        room->sendLog(log);

                        DamageStruct chain_damage = damage;
                        chain_damage.to = chained_player;
                        chain_damage.chain = true;

                        room->damage(chain_damage);
                    }
                }
            }

            break;
        }

    case CardEffected:{
            if(data.canConvert<CardEffectStruct>()){
                CardEffectStruct effect = data.value<CardEffectStruct>();
                if(room->isCanceled(effect))
                    return true;
                    
                //modify by ce
                if(effect.card->inherits("TrickCard") && effect.to->getMark("@invincible")) {
                    LogMessage log;
                    log.type = "#Invincible";
                    log.from = player;
                    room->sendLog(log);                
                    return true;
                }

                effect.card->onEffect(effect);
            }

            break;
        }

    case SlashEffected:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            QVariant data = QVariant::fromValue(effect);
            room->getThread()->trigger(SlashProceed, effect.from, data);

            break;
        }

    case SlashProceed:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            QString slasher = effect.from->objectName();
            const Card *jink = room->askForCard(effect.to, "jink", "slash-jink:" + slasher);
            room->slashResult(effect, jink);

            break;
        }

    case SlashHit:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            DamageStruct damage;
            damage.card = effect.slash;

            damage.damage = 1;
            if(effect.drank)
                damage.damage ++;

            //if(effect.to->hasSkill("jueqing") || effect.to->getGeneralName() == "zhangchunhua")
            //    damage.damage ++;

            damage.from = effect.from;
            damage.to = effect.to;
            damage.nature = effect.nature;

            room->damage(damage);

            effect.to->removeMark("qinggang");

            break;
        }

    case SlashMissed:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            effect.to->removeMark("qinggang");

            break;
        }

    case GameOverJudge:{
            if(room->getMode() == "02_1v1"){
                QStringList list = player->tag["1v1Arrange"].toStringList();

                if(!list.isEmpty()){
                    player->tag["1v1ChangeGeneral"] = list.takeFirst();
                    player->tag["1v1Arrange"] = list;

                    DamageStar damage = data.value<DamageStar>();

                    if(damage == NULL)
                        changeGeneral1v1(player);

                    return false;
                }
            }

            QString winner = getWinner(player);
            if(!winner.isNull()){
                room->gameOver(winner);
                return true;
            }

            break;
        }

    case Death:{
            player->bury();
            
            //modify by ce
            room->setPlayerProperty(player, "mp", 0);

            if(room->getTag("SkipNormalDeathProcess").toBool())
                return false;

            DamageStar damage = data.value<DamageStar>();
            ServerPlayer *killer = damage ? damage->from : NULL;
            if(killer){
                rewardAndPunish(killer, player);
                //modify by ce
                int mp = 0;
                if(killer->getKingdom() == "nu") {
                    mp = 1;
                }else if(killer->getKingdom() == "kuang") {
                    mp = 4;
                }
                
                if(mp>0) {
                    killer->updateMp(mp);
                }
            }
            
            setGameProcess(room);

            break;
        }

    case StartJudge:{
            int card_id = room->drawCard();

            JudgeStar judge = data.value<JudgeStar>();
            judge->card = Sanguosha->getCard(card_id);
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$InitialJudge";
            log.from = player;
            log.card_str = judge->card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);

            room->getThread()->delay();

            break;
        }

    case FinishJudge:{
            JudgeStar judge = data.value<JudgeStar>();
            room->throwCard(judge->card);

            LogMessage log;
            log.type = "$JudgeResult";
            log.from = player;
            log.card_str = judge->card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);

            room->getThread()->delay();

            break;
        }

    case Pindian:{
            PindianStar pindian = data.value<PindianStar>();

            LogMessage log;

            room->throwCard(pindian->from_card);
            log.type = "$PindianResult";
            log.from = pindian->from;
            log.card_str = pindian->from_card->getEffectIdString();
            room->sendLog(log);
            room->getThread()->delay();

            room->throwCard(pindian->to_card);
            log.type = "$PindianResult";
            log.from = pindian->to;
            log.card_str = pindian->to_card->getEffectIdString();
            room->sendLog(log);
            room->getThread()->delay();

            break;
        }

    default:
        ;
    }

    return false;
}

void GameRule::changeGeneral1v1(ServerPlayer *player) const{
    Room *room = player->getRoom();
    QString new_general = player->tag["1v1ChangeGeneral"].toString();
    player->tag.remove("1v1ChangeGeneral");
    room->transfigure(player, new_general, true, true);
    room->revivePlayer(player);
    //modify by ce
    player->updateMp(-player->getMp());
    
    if(player->getKingdom() != player->getGeneral()->getKingdom())
        room->setPlayerProperty(player, "kingdom", player->getGeneral()->getKingdom());

    room->broadcastInvoke("revealGeneral",
                          QString("%1:%2").arg(player->objectName()).arg(new_general),
                          player);

    if(!player->faceUp())
        player->turnOver();

    if(player->isChained())
        room->setPlayerProperty(player, "chained", false);

    player->drawCards(4);
}

void GameRule::rewardAndPunish(ServerPlayer *killer, ServerPlayer *victim) const{
    if(killer->isDead())
        return;

    if(killer->getRoom()->getMode() == "06_3v3"){
        killer->drawCards(3);
    }else{
        if(victim->getRole() == "rebel" && killer != victim){
            killer->drawCards(3);
        }else if(victim->getRole() == "loyalist" && killer->getRole() == "lord"){
            killer->throwAllEquips();
            killer->throwAllHandCards();
        }
    }
}

QString GameRule::getWinner(ServerPlayer *victim) const{
    Room *room = victim->getRoom();
    QString winner;

    if(room->getMode() == "06_3v3"){
        switch(victim->getRoleEnum()){
        case Player::Lord: winner = "renegade+rebel"; break;
        case Player::Renegade: winner = "lord+loyalist"; break;
        default:
            break;
        }
    }else{
        QStringList alive_roles = room->aliveRoles(victim);
        switch(victim->getRoleEnum()){
        case Player::Lord:{
                if(alive_roles.length() == 1 && alive_roles.first() == "renegade")
                    winner = room->getAlivePlayers().first()->objectName();
                else
                    winner = "rebel";
                break;
            }

        case Player::Rebel:
        case Player::Renegade:
            {
                if(!alive_roles.contains("rebel") && !alive_roles.contains("renegade")){
                    winner = "lord+loyalist";
                    if(victim->getRole() == "renegade" && !alive_roles.contains("loyalist"))
                        room->setTag("RenegadeInFinalPK", true);
                }
                break;
            }

        default:
            break;
        }
    }

    return winner;
}


static const int LoseHpTo1 = 1;
static const int ThrowAllCard = 2;

BossMode::BossMode(QObject *parent)
    :GameRule(parent)
{
    setObjectName("boss_mode");
}

bool BossMode::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();

    switch(event)
    {

    case GameOverJudge:{
            const static QString evil = "lord+renegade";
            const static QString justice = "rebel+loyalist";

            QStringList alive_roles = room->aliveRoles(player);
            if(!alive_roles.contains("rebel") && !alive_roles.contains("loyalist")){
                room->gameOver(evil);
                return true;
            }

            DamageStar damage = data.value<DamageStar>();
            ServerPlayer *killer = NULL;
            if(damage)
                killer = damage->from;

            if(player->isLord()){
                QString winner;
                if(!alive_roles.contains("renegade"))
                    winner = justice;
                else{
                    if(killer == NULL || evil.contains(killer->getRole()))
                        winner = justice;
                    else
                        winner = evil;
                }

                room->gameOver(winner);
                return true;
            }

            break;
        }

    case GameStart:{
            if(player->isLord()){
                // find guard
                QList<ServerPlayer *> players = room->getOtherPlayers(player);
                foreach(ServerPlayer *player, players){
                    if(player->getRoleEnum() == Player::Renegade){
                        room->broadcastProperty(player, "role");
                        break;
                    }
                }
            }

            break;
        }

    default:
        break;
    }

    return GameRule::trigger(event, player, data);
}

HulaoPassMode::HulaoPassMode(QObject *parent)
    :GameRule(parent)
{
    setObjectName("hulaopass_mode");

    events << HpChanged;
}

static int Transfiguration = 1;

bool HulaoPassMode::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    Room *room = player->getRoom();

    switch(event){
    case GameStart:{
            if(player->isLord()){
                if(setjmp(env) == Transfiguration){
                    player = room->getLord();
                    room->transfigure(player, "shenlvbu2", true, true);

                    QList<const Card *> tricks = player->getJudgingArea();
                    foreach(const Card *trick, tricks)
                        room->throwCard(trick);

                }else{
                    player->drawCards(8, false);
                }
            }else
                player->drawCards(player->getSeat() + 1, false);

            // if(player->getGeneralName() == "zhangchunhua"){
                // if(qrand() % 3 == 0)
                    // room->killPlayer(player);
            // }

            return false;
        }

    case CardUsed:{
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->inherits("Weapon") && player->askForSkillInvoke("weapon_recast", data)){
                room->throwCard(use.card);
                player->drawCards(1, false);
                return false;
            }

            break;
        }

    case HpChanged:{
            if(player->getGeneralName() == "shenlvbu1" && player->getHp() <= 4){
                longjmp(env, Transfiguration);
            }

            return false;
        }

    case Death:{
            if(player->isLord()){
                room->gameOver("rebel");
            }else{
                if(room->aliveRoles(player).length() == 1)
                    room->gameOver("lord");

                LogMessage log;
                log.type = "#Reforming";
                log.from = player;
                room->sendLog(log);

                player->bury();
                room->setPlayerProperty(player, "hp", 0);

                foreach(ServerPlayer *player, room->getOtherPlayers(room->getLord())){
                    if(player->askForSkillInvoke("draw_1v3"))
                        player->drawCards(1, false);
                }
            }

            return false;
        }

    case TurnStart:{
            if(player->isLord()){
                if(!player->faceUp())
                    player->turnOver();
                else
                    player->play();
            }else{
                if(player->isDead()){
                    if(player->getHp() + player->getHandcardNum() == 6){
                        LogMessage log;
                        log.type = "#ReformingRevive";
                        log.from = player;
                        room->sendLog(log);

                        room->revivePlayer(player);
                    }else if(player->isWounded()){
                        LogMessage log;
                        log.type = "#ReformingRecover";
                        log.from = player;
                        room->sendLog(log);

                        room->setPlayerProperty(player, "hp", player->getHp() + 1);
                    }else
                        player->drawCards(1, false);
                }else if(!player->faceUp())
                    player->turnOver();
                else
                    player->play();
            }

            return false;
        }

    default:
        break;
    }

    return GameRule::trigger(event, player, data);
}

