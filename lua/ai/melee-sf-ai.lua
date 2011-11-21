-- gouki ---------------------------------------------------------------------------------

sgs.ai_chaofeng["gouki"] = 1

-- shankong
sgs.ai_skill_invoke.shankong = true

-- shunyu
sgs.dynamic_value.damage_card["ShunyuCard"] = true

sgs.ai_skill_use["@@shunyu"]=function(self,prompt)
    local cards = self.player:getCards("he")
    if cards:length()<4 or self.player:getMp()<18 then return "." end
	
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
    
    local spade, heart, club, diamond
    
    for _,card in ipairs(cards) do
        if not spade and card:getSuit() == sgs.Card_Spade then
            spade = card
        elseif not heart and card:getSuit() == sgs.Card_Heart then
            heart = card
        elseif not club and card:getSuit() == sgs.Card_Club then
            club = card
        elseif not diamond and card:getSuit() == sgs.Card_Diamond then
            diamond = card
        end
    end
    
    if spade and heart and club and diamond then 
        for _,enemy in ipairs(self.enemies) do
            if self.player:distanceTo(enemy)<=1 then
                return ("@ShunyuCard=%d+%d+%d+%d->%s"):format(spade:getEffectiveId(), heart:getEffectiveId(), club:getEffectiveId(), diamond:getEffectiveId(), enemy:objectName())
            end
        end
    end
    
    return "."    
end

-- ryu ---------------------------------------------------------------------------------
sgs.ai_chaofeng["ryu"] = 2

-- longjuan
sgs.dynamic_value.damage_card["LongjuanCard"] = true

sgs.ai_skill_use["@@longjuan"]=function(self,prompt)
    if self:getCardsNum("Slash")<=0 then return "." end
    
    local target = nil
    local slash = self:getCardId("Slash")
    
	self:sort(self.enemies,"defense")
    for _,enemy in ipairs(self.enemies) do
        if self.player:distanceTo(enemy) <= self.player:getAttackRange()+1 then
            if target then
                target = target.."+"..enemy:objectName()
                break
            else
                target = enemy:objectName()
            end
        end
    end
    
    if target then
        return "@LongjuanCard="..slash.."->"..target
    end
    
    return "."    
end

-- bodong
sgs.dynamic_value.damage_card["BodongCard"] = true

sgs.ai_skill_use["@@bodong"]=function(self,prompt)
    if self:getCardsNum("Slash")<=0 or self.player:getMp()<3 then return "." end
    
    local slash = self:getCardId("Slash")
    
	self:sort(self.enemies,"defense")    
    for _,enemy in ipairs(self.enemies) do
        if self.player:distanceTo(enemy) <= self.player:getAttackRange()+1 and not self:isEquip("HolyWing", enemy) then
            return "@BodongCard="..slash.."->"..enemy:objectName()
        end
    end
    
    return "."    
end

-- ken ---------------------------------------------------------------------------------
sgs.ai_chaofeng["ryu"] = 4

-- jifeng
sgs.dynamic_value.damage_card["JifengCard"] = true

sgs.ai_skill_choice.jifeng = function(self, choices)
    local slash_num = self:getCardsNum("Slash")
    
    if slash_num<=0 then 
        return "NoChoice" 
    elseif slash_num==1 then
        return "JifengEffect1" 
    else
        return "JifengEffect2"
    end
    
end

sgs.ai_skill_use["@@jifeng"]=function(self,prompt)
    if self:getCardsNum("Slash")<=0 then return "." end
    
    local slash
    
	self:sort(self.enemies,"defense")    
    
    if self.player:getMark("jifeng")==1 then
    
        for _,enemy in ipairs(self.enemies) do
            if self.player:distanceTo(enemy)<=1 and enemy:hasFlag("jifenged") then
                slash = self:getCardId("Slash")
                return "@JifengCard="..slash.."->"..enemy:objectName()
            end
        end
        
    elseif  self.player:getMark("jifeng")==2 then
        
        for _,enemy in ipairs(self.enemies) do
            if self.player:distanceTo(enemy)<=1 and not enemy:hasFlag("jifenged") then
                slash = self:getCardId("Slash")
                return "@JifengCard="..slash.."->"..enemy:objectName()
            end
        end
        
    end
    
    return "."    
end

-- shenglong
sgs.dynamic_value.damage_card["ShenlongCard"] = true

sgs.ai_skill_use["@@shenglong"]=function(self,prompt)
    if self:getCardsNum("Slash")<=0 or self.player:getMp()<2 then return "." end
    
    local slash = self:getCardId("Slash")
    
	self:sort(self.enemies,"defense")    
    for _,enemy in ipairs(self.enemies) do
        if self.player:inMyAttackRange(enemy) and not self:isEquip("HolyWing", enemy) and enemy:getHandcardNum()<3 then
            return "@ShenglongCard="..slash.."->"..enemy:objectName()
        end
    end
    
    return "."    
end

-- chunli ---------------------------------------------------------------------------------
sgs.ai_chaofeng["chunli"] = 4

-- qigong
sgs.dynamic_value.control_card["QigongCard"] = true

sgs.ai_skill_use["@@qigong"]=function(self,prompt)
    if self:getCardsNum("Slash")<=0 or self.player:getMp()<1 then return "." end

	self:sort(self.enemies,"defense")    
    for _,enemy in ipairs(self.enemies) do
        if self.player:inMyAttackRange(enemy) and enemy:getArmor() then
            return "@QigongCard=.->"..enemy:objectName()
        end
    end
    
    return "."    
end

-- jiqi
sgs.ai_skill_use["@@jiqi"]=function(self,prompt)
    
    local num = self:getCardsNum("Slash");
    
    for _,friend in ipairs(self.friends_noself) do
        if self:getCardsNum("BasicCard", friend)>1 then
            num = num+1
        end
    end
    
    if num>3 or (self.player:getHandcardNum()-self.player:getHp()>2) then return "@JiqiCard=.->." end
    
    return "."    
end

-- bailie
sgs.dynamic_value.damage_card["BailieCard"] = true

sgs.ai_skill_use["@@bailie"]=function(self,prompt)
    if self:getCardsNum("Slash")<2 then return "." end
    
    local target
    local slashes = {}
    
	self:sort(self.enemies,"defense")    
    for _,enemy in ipairs(self.enemies) do
        if self.player:inMyAttackRange(enemy) then
            target = enemy:objectName()
            break
        end
    end
    
    if target then
        local cards = self.player:getCards("h")
        cards = sgs.QList2Table(cards)
        
        for _, card in ipairs(cards) do
            if card:inherits("Slash") and #slashes<3 then
                table.insert(slashes, card:getEffectiveId())
            end
        end
        
        if #slashes>0 then
            local all_slash = table.concat(slashes, "+")
            return "@BailieCard="..all_slash.."->"..target
        end
    end
    
    return "."    
end

-- blanka ---------------------------------------------------------------------------------
sgs.ai_chaofeng["blanka"] = -4

sgs.blanka_keep_value = {
	ThunderBang = 4,
}

-- dianji
sgs.ai_use_priority.DianjiCard = 3
sgs.dynamic_value.damage_card["DianjiCard"] = true

local dianji_skill={}
dianji_skill.name="dianji"
table.insert(sgs.ai_skills,dianji_skill)
dianji_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("DianjiCard") or self:getCardsNum("ThunderBang")<=0 or self.player:getMp()<2 then return end
    
    local good, bad = 0, 0
    local who = self.player   
    
    for _, friend in ipairs(self.friends_noself) do
        if friend:getHandcardNum()<2 and self:damageIsEffective(sgs.DamageStruct_Thunder, friend) then 
            if friend:getHp() == 1 then 
                bad = bad + 5
            end
            bad = bad + 3
        end
    end

    for _, enemy in ipairs(self.enemies) do
        if enemy:getHandcardNum()<2 and self:damageIsEffective(sgs.DamageStruct_Thunder, enemy) then 
            if enemy:getHp() == 1 then 
                good = good + 5
            end
            good = good + 3
        end
    end
    
    if good < bad then return nil end
    
    local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
    
    for _, card in ipairs(cards) do
        if card:inherits("ThunderBang") then
            return sgs.Card_Parse(("@DianjiCard=%d"):format(card:getEffectiveId()))
        end
    end

end

sgs.ai_skill_use_func["DianjiCard"]=function(card,use,self)
    use.card = card
end


-- dhalsim ---------------------------------------------------------------------------------
sgs.ai_chaofeng["dhalsim"] = 6

sgs.dhalsim_keep_value = {
	Weapon = 0,
    FireBang = 2,
}

-- huoyan
sgs.ai_use_priority.HuoyanCard = 3
sgs.dynamic_value.damage_card["HuoyanCard"] = true

local huoyan_skill={}
huoyan_skill.name="huoyan"
table.insert(sgs.ai_skills,huoyan_skill)
huoyan_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("HuoyanCard") or not self.player:canSlashWithoutCrossbow() or self.player:getMp()<2 then return end
    
    local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
    
    for _, card in ipairs(cards) do
        if card:inherits("Slash") and not card:inherits("FireBang") then
            return sgs.Card_Parse(("@HuoyanCard=%d."):format(card:getEffectiveId()))
        end
    end
    
end

sgs.ai_skill_use_func["HuoyanCard"]=function(card,use,self)
        
        self:sort(self.enemies,"defense")
        for _,enemy in ipairs(self.enemies) do
            if self:damageIsEffective(sgs.DamageStruct_Fire, enemy) then
                use.card = card
                if use.to then 
                    use.to:append(enemy) 
                end
                break
            end
        end
    
end

-- chuansong
sgs.ai_skill_use["@@chuansong"]=function(self,prompt)

    local target = self.room:getCurrent();
    
    if target:getNext() ~= self.player then
        return "@ChuansongCard=.->"..target:objectName()
    end
    
    return "."
    
end


-- honda ---------------------------------------------------------------------------------

sgs.ai_chaofeng["honda"] = 1

-- xushi
sgs.ai_skill_invoke.xushi = function(self, data)
    
    if self.player:hasFlag("no_bang_limit") then return false end
    
    local has_armor, has_dhorse
    local slash, dodge, water = 0, 0, 0
    
    local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
    
    for _, card in ipairs(cards) do
        if not self.player:getArmor() and card:inherits("Armor") then return false end
        if not self.player:getDefensiveHorse() and card:inherits("DefensiveHorse") then return false end
        
        if card:inherits("Slash") then 
            slash = slash+1 
        elseif card:inherits("Dodge") then
            dodge = dodge+1
        elseif card:inherits("HolyWater") then
            water = water+1
        end
    end
    
    self.room:writeToConsole("xushi----------")
    self.room:writeToConsole("hp mp num slash dodge water")
    self.room:writeToConsole(self.player:getHp())
    self.room:writeToConsole(self.player:getMp())
    self.room:writeToConsole(#cards)
    self.room:writeToConsole(slash)
    self.room:writeToConsole(dodge)
    self.room:writeToConsole(water)
    self.room:writeToConsole("----------------")
    
    if self.player:getHp()+self.player:getMp() > #cards and slash>1 then return true end
    if self.player:getHp()==1 and self.player:getMp()>1 and water == 0 then return true end
    
end

-- wushuang
sgs.ai_skill_invoke.wushuang = function(self, data)
    return self:getCardsNum("Slash") > 1
end

-- zangief ---------------------------------------------------------------------------------

sgs.ai_chaofeng["zangief"] = 6

sgs.zangief_keep_value = {
	PK = 4,
}

-- xuanfeng
sgs.dynamic_value.control_card["XuanfengCard"] = true

sgs.ai_skill_use["@@xuanfeng"]=function(self, prompt)
    if self.player:getCards("he"):length() <2 then return "." end
    
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
    self:sortByKeepValue(cards, true)
    
    if self:getUseValue(cards[2])>5 then return "." end
    
    local data = prompt:split(":")
    local card = sgs.Sanguosha:getCard(data[2])
    if not card then return "." end
    
    local target = nil
    
    for _, p in sgs.qlist(self.room:getAllPlayers()) do
        if p:objectName() == data[3] then
            target = p
        end
    end
    if self:isFriend(target) then return "." end
    
    if (self.player:inMyAttackRange(target) and self:damageIsEffective(sgs.DamageStruct_Normal, target)) 
    or card:inherits("Cure") or card:inherits("Grab") or card:inherits("WoodElf") then
        return ("@XuanfengCard=%d+%d->."):format(cards[1]:getEffectiveId(), cards[2]:getEffectiveId())
    end
    
    return "."
end

-- dazhuang
sgs.dynamic_value.control_card["DazhuangCard"] = true

sgs.ai_skill_use["@@dazhuang"]=function(self, prompt)
    if self.player:getMp()<3 then return "." end
    
    local card_str = "@DazhuangCard="
    local pk = self:getCardId("PK")
    
    if self.player:getMp()>=6 then
        card_str = card_str..".->"
    elseif pk then
        card_str = card_str..pk.."->"
    else
        return "."
    end
    
	self:sort(self.enemies,"defense")    
    for _,enemy in ipairs(self.enemies) do
        if self.player:distanceTo(enemy) <= 1 and self:damageIsEffective(sgs.DamageStruct_Normal, enemy) then
            return card_str..enemy:objectName()
        end
    end
    
    return "."
end

-- guile ---------------------------------------------------------------------------------

sgs.ai_chaofeng["guile"] = 8

sgs.zangief_keep_value = {
	Dodge = 3,
}

--shoudao
shoudao_skill={}
shoudao_skill.name="shoudao"
table.insert(sgs.ai_skills,shoudao_skill)
shoudao_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local dodge

	for _,card in ipairs(cards)  do
		if card:inherits("Dodge") and card:isRed() then
			dodge = card
			break
		end
	end

	if dodge then
		local suit = dodge:getSuitString()
		local number = dodge:getNumberString()
		local card_id = dodge:getEffectiveId()
		local card_str = ("bang:shoudao[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)

		assert(slash)

		return slash
	end
end

-- jiaodao
sgs.dynamic_value.damage_card["JiaodaoCard"] = true

sgs.ai_skill_use["@@jiaodao"]=function(self, prompt)
    
    local cards = self.player:getCards("h")
    
	cards = sgs.QList2Table(cards)
    self:sortByUseValue(cards, true)
    
    local first, second
    
    for _, card in ipairs(cards) do
        if (card:inherits("Slash") or card:inherits("Dodge")) and card:isRed() then
            if first then
                second = card
            else
                first = card
            end
        end
    end
    
    if second then
        return ("@JiaodaoCard=%d+%d->"):format(first:getEffectiveId(), second:getEffectiveId())
    end
    
    return "."
end

-- yinsu
sgs.ai_skill_invoke.yinsu = function(self, data)
    return self:needHelp(data)
end


-- sakura ---------------------------------------------------------------------------------
sgs.ai_chaofeng["sakura"] = 4

-- chongbai
sgs.ai_use_priority.ChongbaiCard = 6
sgs.dynamic_value.benefit["ChongbaiCard"] = true

local chongbai_skill={}
chongbai_skill.name="chongbai"
table.insert(sgs.ai_skills,chongbai_skill)
chongbai_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("ChongbaiCard") or self.player:getHandcardNum()<2 or #self.friends_noself==0 then return end
    
    local cards = self.player:getCards("h")    
    
    cards=sgs.QList2Table(cards)    
    self:sortByKeepValue(cards)
    
    local card_id = nil
    local water = self:getCardId("HolyWater")
    local slash = self:getCardId("Slash")
    local dodge = self:getCardId("Dodge")
    
    if water and not self.player:isWounded() then card_id = water end
    if dodge and self:getCardsNum("dodge")>1 and not self.player:isWounded() then card_id = dodge end
    if slash and not self.player:canSlashWithoutCrossbow() and self:getCardsNum("Slash")>1 then card_id = slash end
    if #cards > self.player:getMaxHP() then card_id = cards[1]:getEffectiveId() end
    
    if card_id then
        return sgs.Card_Parse("@ChongbaiCard="..card_id)
    end
    
end

sgs.ai_skill_use_func["ChongbaiCard"]=function(card,use,self)

    local target = nil

	self:sort(self.friends_noself,"defense")    
    target = self.friends_noself[1]
        
    if target then
        use.card = card
        if use.to then
            use.to:append(target)
        end
    end
    
end

-- mofang
sgs.ai_use_priority.MofangCard = 6

local mofang_skill={}
mofang_skill.name="mofang"
table.insert(sgs.ai_skills,mofang_skill)

mofang_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("MofangCard") then return end
    return sgs.Card_Parse("@MofangCard=.")
end

sgs.ai_skill_use_func["MofangCard"]=function(card,use,self)
    use.card = card
end

-- mofang ag
sgs.ai_skill_askforag.mofang = function(self, card_ids)
	local cards = {}
    
	for _, id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(id))
	end
	
    self:sortByUseValue(cards)
	
    for _, card in ipairs(cards) do
        if self:hasSuit(card:getSuitString()) then
            
            self.room:writeToConsole("mofang choice:"..card:objectName())
            
            return card:getEffectiveId()
        end
    end
    
    return nil
    
end

-- mofang viewasskill
sgs.ai_skill_use["@mofangvas"]=function(self, prompt)
    if self.player:getMark("mofang") == -1 or self.player:getHandcardNum() < 1 then return "." end
    
    local ag_card = sgs.Sanguosha:getCard(self.player:getMark("mofang"))
    local suit = ag_card:getSuitString()
	local number = ag_card:getNumberString()
    
	local cards = self.player:getCards("h")
    cards = sgs.QList2Table(cards)
    self:sortByKeepValue(cards, true)
    
    local use_card
    for _, card in ipairs(cards) do
        if card:getSuitString() == suit and self:getUseValue(card) >= self:getUseValue(ag_card) then
            use_card = card
            break
        end
    end
    
    if ag_card and use_card and ag_card:isAvailable(self.player) then
        
        if ag_card:targetFixed() then
            return (ag_card:objectName()..":mofangvas[%s:%s]=%d->."):format(suit, number, use_card:getEffectiveId())
        else
            local use={}
            local card_str = (ag_card:objectName()..":mofangvas[%s:%s]=%d->"):format(suit, number, use_card:getEffectiveId())
            
            use.from = self.player
            use.to = self.room:getAllPlayers()
            local index = use.to:length()    
    
            local type = ag_card:getTypeId()
            
            if type == sgs.Card_Basic then
                self:useBasicCard(ag_card, use, self.slash_distance_limit)
            elseif type == sgs.Card_Trick then
                self:useTrickCard(ag_card, use)
            else
                return "."
            end
            
            local target = use.to
            if target:at(index) then
                card_str = card_str..target:at(index):objectName()
                if target:at(index+1) then
                    card_str = card_str.."+"..target:at(index+1):objectName()
                end
            else
                return "."
            end
            
            return card_str
            
        end
    end
    
    return "."
end

-- yingluo
sgs.ai_skill_invoke.yingluo = function(self, data)
    return self.player:getMp()>1
end

sgs.ai_skill_use["@@yingluo"]=function(self, prompt)
    if self.player:getMp()<1 then return "." end
    return "@YingluoCard=.->."
end


-- cammy ---------------------------------------------------------------------------------
sgs.ai_chaofeng["cammy"] = 2

-- jingzhun
sgs.ai_skill_invoke.jingzhun = function(self, data)
    
	local effect = data:toSlashEffect()
    
    if self:isFriend(effect.to) or effect.to:getHandcardNum()==0 then return false end
    
    return true
    
end

-- luoxuan
sgs.ai_skill_invoke.luoxuan = function(self, data)
    local damage = data:toDamage()
    
    if self:isFriend(damage.to) then return false end
    
    if damage.to:getArmor() or damage.to:getDefensiveHorse() or damage.to:getRelic() then
        return true
    end
    
    return false
end

sgs.ai_skill_cardchosen.luoxuan = function(self, who)
    if who:getArmor() then return who:getArmor() end
    if who:getRelic() then return who:getRelic() end
    if who:getWeapon() then return who:getWeapon() end
    if who:getHorse() then return who:getHorse() end
end

-- dan ---------------------------------------------------------------------------------
sgs.ai_chaofeng["dan"] = 5

-- tiaoxin
sgs.ai_skill_invoke.tiaoxin = function(self, data)
    if self.player:getHp()>3 or self.player:getHp() - #self.enemies>1 then return true end
    return false
end

-- wodao
sgs.ai_skill_invoke.wodao = true

-- rose ---------------------------------------------------------------------------------
sgs.ai_chaofeng["rose"] = 5

sgs.caozong_target_is_friend = false
-- caozong
sgs.ai_skill_invoke.caozong = function(self, data)
    local use = data:toCardUse()
    
    if self:isFriend(use.from) then return false end
       
    if use.card:inherits("Slash") and self:isFriend(use.to:first()) then
        for _,p in ipairs(self.enemies) do
            if use.from:canSlash(p) then
                return true
            end
        end
    elseif use.card:inherits("PK") and self:isFriend(use.to:first()) then
        return true
    elseif use.card:inherits("Cure") and not self:isFriend(use.to:first()) then
        for _,p in ipairs(self.friends) do
            if use.from:distanceTo(p)<=1 and p:isWounded() then
                sgs.caozong_target_is_friend = true
                return true
            end
        end
    elseif use.card:inherits("Grab") and self:isFriend(use.to:first()) and self.player:getMp()>2 then
        for _,p in ipairs(self.enemies) do
            if use.from:distanceTo(p)<=1 and not p:isNude() then
                return true
            end
        end
    elseif use.card:inherits("SoulChain") and self.player:getMp()>2 then
        for _,p in ipairs(self.enemies) do
            if not p:isChained() then
                return true
            end
        end
    else
        return self.player:getMp()>6
    end
    
end

sgs.ai_skill_playerchosen.caozong = function (self, targets)
    for _, p in sgs.qlist(targets) do
        if sgs.caozong_target_is_friend and self:isFriend(p) then
            sgs.caozong_target_is_friend = false
            return p
        elseif not sgs.caozong_target_is_friend and not self:isFriend(p) then
            return p
        end
    end
end

-- xinling
sgs.ai_use_priority.XinlingCard = 6

local xinling_skill={}
xinling_skill.name="xinling"
table.insert(sgs.ai_skills,xinling_skill)

xinling_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("XinlingCard") or self.player:isWounded() or self.player:getMp()>5 then return end
    return sgs.Card_Parse("@XinlingCard=.")
end

sgs.ai_skill_use_func["XinlingCard"]=function(card,use,self)
    use.card = card
end

-- gen ---------------------------------------------------------------------------------
sgs.ai_chaofeng["gen"] = 6

-- jiliu
sgs.dynamic_value.damage_card["JiliuCard"] = true

local jiliu_skill={}
jiliu_skill.name="jiliu"
table.insert(sgs.ai_skills,jiliu_skill)

jiliu_skill.getTurnUseCard=function(self)
    if self.player:getMp()<15 then return end
    return sgs.Card_Parse("@JiliuCard=.")
end

sgs.ai_skill_use_func["JiliuCard"]=function(card,use,self)
    use.card = card
end

sgs.ai_skill_playerchosen.jiliu = function (self, targets)
    self:sort(self.enemies,"hp")
    self.room:writeToConsole("jiliu target is "..self.enemies[1]:getGeneralName())
    return self.enemies[1]
end

sgs.ai_skill_invoke["#jiliu-on"] = function(self, data)
    local mark = 0
    
    for _,enemy in ipairs(self.enemies) do
        if enemy:getMark("jiliu") >= enemy:getHp() then return true end
        mark = mark + enemy:getMark("jiliu")
    end
   
   return self.player:getHp()==1 or mark>2 
end

-- sangliu
sgs.dynamic_value.control_card["SangliuCard"] = true

local sangliu_skill={}
sangliu_skill.name="sangliu"
table.insert(sgs.ai_skills,sangliu_skill)

sangliu_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("SangliuCard") then return end
    
    for _,enemy in ipairs(self.enemies) do
        if enemy:getMark("@card_forbid")<=0 and enemy:getMark("sangliu_start")<=0 then
            return sgs.Card_Parse("@SangliuCard=.")
        end
    end
    
end

sgs.ai_skill_use_func["SangliuCard"]=function(card,use,self)
    self:sort(self.enemies,"hp")    
    for _,enemy in ipairs(self.enemies) do
        if enemy:getMark("@card_forbid")<=0 and enemy:getMark("sangliu_start")<=0 then
            use.card = card
            if use.to then
                use.to:append(enemy)
            end
            return
        end
    end

end

-- bison ---------------------------------------------------------------------------------
sgs.ai_chaofeng["bison"] = 0

--kuangniu
kuangniu_skill={}
kuangniu_skill.name="kuangniu"
table.insert(sgs.ai_skills,kuangniu_skill)
kuangniu_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local trick_card

	self:sortByUseValue(cards,true)

	for _,card in ipairs(cards)  do
		if card:inherits("TrickCard") and card:isRed() then
			trick_card = card
			break
		end
	end

	if trick_card then
		local suit = trick_card:getSuitString()
		local number = trick_card:getNumberString()
		local card_id = trick_card:getEffectiveId()
		local card_str = ("bang:kuangniu[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)

		assert(slash)

		return slash
	end
end

-- balrog ---------------------------------------------------------------------------------
sgs.ai_chaofeng["balrog"] = 2

sgs.balrog_keep_value = {
    Unassailable = 1,
    NothingIsSomething=6,
    Cure = 6,
    ReadyToGo=3,
    Thunder=3,
}

-- tiemian
sgs.ai_skill_invoke.tiemian = true

-- kongsha
sgs.ai_skill_invoke.kongsha = function(self, data)
    local damage = data:toDamage()
    
    if self:isFriend(damage.to) and not damage.to:faceUp() then return true end    
    if not self:isFriend(damage.to) and damage.to:faceUp() then return true end
    
    return false
end

-- sagat ---------------------------------------------------------------------------------
sgs.ai_chaofeng["sagat"] = 4

-- zengnu
local zengnu_skill={}
zengnu_skill.name="zengnu"
table.insert(sgs.ai_skills,zengnu_skill)

zengnu_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("ZengnuCard") then return end
    
    if (self.player:getHp()>2 and self:getCardsNum("HolyWater")>0) or self.player:getHp()>3 then 
        return sgs.Card_Parse("@ZengnuCard=.")
    end
end

sgs.ai_skill_use_func["ZengnuCard"]=function(card,use,self)
    use.card = card
end


-- huqie
sgs.dynamic_value.damage_card["HuqieCard"] = true

sgs.ai_skill_use["@@huqie"]=function(self,prompt)
    if self.player:getMp()<4 and self.player:getHp()>1 then return "." end
    
	self:sort(self.enemies,"defense")
    for _,enemy in ipairs(self.enemies) do
        if self.player:inMyAttackRange(enemy) and self:damageIsEffective(sgs.DamageStruct_Normal, enemy) then
            return "@HuqieCard=.->"..enemy:objectName()
        end
    end
    
    return "."    
end

-- vega ---------------------------------------------------------------------------------
sgs.ai_chaofeng["vega"] = -4

-- cuimian
sgs.ai_skill_invoke.cuimian = function(self, data)
    local who = data:toPlayer()
    
    self.room:writeToConsole("---------cuimian-----------");
    self.room:writeToConsole(self.player:getGeneralName().." attack "..who:getGeneralName());
    
    if not self.player:isWounded() or self:isFriend(who) then return false end
    if who:getHandcardNum()<2 and who:getHandcardNum()>0 then return true end
    
    for _,friends in ipairs(self.friends) do
        if who:distanceTo(friends)<=1 then
            return false
        end
    end
    
    return true
    
end

sgs.ai_skill_playerchosen.cuimian = function (self, targets)
    
    for _, p in sgs.qlist(targets) do
        if not self:isFriend(p) then return p end
    end
    
    targets = sgs.QList2Table(targets)
    self:sort(targets,"hp")
    
    return targets[1]
    
end

-- mofu
sgs.ai_skill_invoke.mofu = true
