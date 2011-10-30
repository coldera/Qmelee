-- demitri ---------------------------------------------------------------------------------
sgs.ai_chaofeng["demitri"] = -4

-- wuye
sgs.ai_skill_invoke.wuye = function(self, data) 
    return self.player:getHp()<=2 and self.player:getMp()>0
end

-- morrigan ---------------------------------------------------------------------------------
sgs.ai_chaofeng["morrigan"] = 2

-- huanying
sgs.ai_skill_invoke.huanying = function(self, data) 
    local invoke = self.player:getHandcardNum()>=2 and self.player:getMp()>=6
    if invoke then        
        -- self.player:invoke("animate", "huanying")    
        return true
    end
    
    return false
    
end

-- victor ---------------------------------------------------------------------------------
sgs.ai_chaofeng["victor"] = 0

sgs.haohmaru_keep_value = {
	VineArmor = 6,
}

-- guzhi
sgs.ai_skill_invoke.guzhi = function(self, data) 
    if self.player:getMp()<1 then return end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
    
    for _,card in ipairs(cards) do
        if card:inherits("TrickCard") and not card:inherits("DelayedTrick") then
            return true
        end
    end

    return 
end

-- sasquatch ---------------------------------------------------------------------------------
sgs.ai_chaofeng["sasquatch"] = 3

sgs.dynamic_value.damage_card["BingzhuCard"] = true

-- bingzhu
local bingzhu_skill={}
bingzhu_skill.name="bingzhu"
table.insert(sgs.ai_skills,bingzhu_skill)
bingzhu_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("BingzhuCard") or self.player:getMp()<2 or self.player:getHandcardNum()<2 then return end
    
    local card_str
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)    
    self:sortByUseValue(cards, true)
    
   for _,enemy in ipairs(self.enemies) do
        if enemy:getHp()==1 and not self:isEquip("HolyWing", enemy) then
            card_str = ("@BingzhuCard=%d+%d"):format(cards[1]:getId(),cards[2]:getId())
            return sgs.Card_Parse(card_str)   
        end
    end
    
end

sgs.ai_skill_use_func["BingzhuCard"]=function(card,use,self)
   for _,enemy in ipairs(self.enemies) do
        if enemy:getHp()==1 and not self:isEquip("HolyWing", enemy) then
            use.card = card
            if use.to then 
                use.to:append(enemy)
            end
            break
        end
    end
end

-- aulbath ---------------------------------------------------------------------------------
sgs.ai_chaofeng["aulbath"] = 0

sgs.dynamic_value.benefit["ZuanjiCard"] = true

-- zuanji
local zuanji_skill={}
zuanji_skill.name="zuanji"
table.insert(sgs.ai_skills,zuanji_skill)
zuanji_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("ZuanjiCard") or self.player:getMp()<2 or self.player:getMark("luochao")>0 then return end
    
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
    
    for _,card in ipairs(cards) do
        if card:inherits("Slash") then
            return sgs.Card_Parse("@ZuanjiCard=.")
        end
    end
    
end

sgs.ai_skill_use_func["ZuanjiCard"]=function(card,use,self)
    use.card = card
end

-- qbee ---------------------------------------------------------------------------------
sgs.ai_chaofeng["qbee"] = 3

sgs.dynamic_value.control_card["RongjieCard"] = true

-- rongjie
local rongjie_skill={}
rongjie_skill.name="rongjie"
table.insert(sgs.ai_skills,rongjie_skill)
rongjie_skill.getTurnUseCard=function(self)
    if self.player:getMp()<3 or self.player:getMp()>6 then return end
    for _,enemy in ipairs(self.enemies) do
        if not enemy:getEquips():isEmpty() and self.player:inMyAttackRange(enemy) then
            return sgs.Card_Parse("@RongjieCard=.")
        end
    end
    
end

sgs.ai_skill_use_func["RongjieCard"]=function(card,use,self)
    local target = nil
    for _,enemy in ipairs(self.enemies) do
        if not enemy:getEquips():isEmpty() and self.player:inMyAttackRange(enemy) then
            target = enemy
            break
        end
    end
    
    if not target then return end 
    use.card = card
    if use.to then
        use.to:append(target)
    end
    
end

sgs.ai_skill_cardchosen.rongjie = function(self, who)
    if who:getArmor() then return who:getArmor() end
    if who:getRelic() then return who:getRelic() end
    if who:getWeapon() then return who:getWeapon() end
    if who:getHorse() then return who:getHorse() end
end

-- fanyan
sgs.ai_skill_invoke.fanyan = true

sgs.ai_skill_invoke["#fanyan-protect"] = function(self, data)
    if self.player:getHp()<3 then return true end
    return false 
end

-- pyron ---------------------------------------------------------------------------------
sgs.ai_chaofeng["pyron"] = 1

-- huimie
local huimie_skill={}
huimie_skill.name="huimie"
table.insert(sgs.ai_skills,huimie_skill)
huimie_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("he")	
    cards=sgs.QList2Table(cards)
	
	local black_card
	
	self:sortByUseValue(cards,true)
	
	local has_weapon=false
	
	for _,card in ipairs(cards)  do
	    if card:inherits("Weapon") and card:isRed() then has_weapon=true end
	end
	
	for _,card in ipairs(cards)  do
		if card:isBlack()  and self:getUseValue(card)<sgs.ai_use_value["DestroyAll"] then
		    local shouldUse=true
		    
		    if card:inherits("Armor") then
                if not self.player:getArmor() then shouldUse=false 
                elseif self:hasEquip(card) then shouldUse=false
                end
            end
            
            if card:inherits("Weapon") then
                if not self.player:getWeapon() then shouldUse=false
                elseif self:hasEquip(card) and not has_weapon then shouldUse=false
                end
            end
		    
		    if shouldUse then
			    black_card = card
			    break
			end
			
		end
	end

	if black_card then
		local suit = black_card:getSuitString()
		local number = black_card:getNumberString()
		local card_id = black_card:getEffectiveId()
		local card_str = ("destroy_all:huimie[%s:%s]=%d"):format(suit, number, card_id)
		local destroy_all = sgs.Card_Parse(card_str)
		
		assert(destroy_all)
        
        return destroy_all
	end
end

-- yanbao
local yanbao_skill={}
yanbao_skill.name="yanbao"
table.insert(sgs.ai_skills,yanbao_skill)
yanbao_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local red_card
	
	self:sortByUseValue(cards,true)
		
	for _,card in ipairs(cards)  do
		if card:isRed()  and self:getUseValue(card)<sgs.ai_use_value["FireBang"] then
            red_card = card
            break			
		end
	end

	if red_card then
		local suit = red_card:getSuitString()
		local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("fire_bang:yanbao[%s:%s]=%d"):format(suit, number, card_id)
		local fire_bang = sgs.Card_Parse(card_str)
		
		assert(fire_bang)
        
        return fire_bang
	end
end

-- shenyu
sgs.ai_skill_invoke["@shenyu"]=function(self,prompt)
    local judge = self.player:getTag("Judge"):toJudge()
	
	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())		
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@ShenyuCard=" .. card_id
		end
	end
	
	return "."
end

-- zabal ---------------------------------------------------------------------------------
sgs.ai_chaofeng["zabal"] = 0

-- yaogun
local yaogun_skill={}
yaogun_skill.name="yaogun"
table.insert(sgs.ai_skills,yaogun_skill)
yaogun_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("YaogunCard") or self.player:getHp()>3 or self.player:getMp()<4 then return end    
    
    local good, bad = 0, 0
    for _, friend in ipairs(self.friends_noself) do
        if friend:getHp() < 3 then 
            bad = bad + 3
        end
    end
    if self:hasWizard(self.friends) then good = good + 9 end

    for _, enemy in ipairs(self.enemies) do
        if enemy:getHp() < 3 then 
            good = good + 3
        end
    end
    if self:hasWizard(self.enemies) then bad = bad + 3 end
    
    if good < bad then return nil end

    return sgs.Card_Parse("@YaogunCard=.")
    
end

sgs.ai_skill_use_func["YaogunCard"]=function(card,use,self)
    use.card = card
end

-- lilith ---------------------------------------------------------------------------------
sgs.ai_chaofeng["lilith"] = -4

sgs.dynamic_value.control_card["LieqiCard"] = true

-- lieqi
local lieqi_skill={}
lieqi_skill.name="lieqi"
table.insert(sgs.ai_skills,lieqi_skill)
lieqi_skill.getTurnUseCard=function(self)
    if self.player:getMp()<1 then return end  
    
   for _,enemy in ipairs(self.enemies) do
        if enemy:getHandcardNum()==1 then
            return sgs.Card_Parse("@LieqiCard=.")
        end
    end

end

sgs.ai_skill_use_func["LieqiCard"]=function(card,use,self)

    local target = nil
    
    self:updatePlayers(true)
	self:sort(self.enemies,"defense")
    
   for _,enemy in ipairs(self.enemies) do
        if enemy:getHandcardNum()==1 then
            target = enemy
            break
        end
    end
    
    if target then
        use.card = card
        if use.to then
            use.to:append(target)
        end
    end
    
end

-- guisu
sgs.ai_skill_invoke.guisu = function (self, data)
    return #self.friends>0
end

sgs.ai_skill_playerchosen.guisu = function (self, targets)
    self:sort(self.friends_noself, "handcard")
    return self.friends_noself[1]
end

-- leilei ---------------------------------------------------------------------------------
sgs.ai_chaofeng["leilei"] = 6

sgs.dynamic_value.damage_card["AnqiCard"] = true
sgs.dynamic_value.control_card["LingfuCard"] = true

-- anqi
local anqi_skill={}
anqi_skill.name="anqi"
table.insert(sgs.ai_skills,anqi_skill)
anqi_skill.getTurnUseCard=function(self)
    local card_str = nil
    
    if self.player:hasUsed("AnqiCard") or not self.player:canSlashWithoutCrossbow() then return end
    
    if self.player:getWeapon() then 
        local cards = self.player:getHandcards()
        cards = sgs.QList2Table(cards)
        
        for _,card in ipairs(cards) do
            if card:inherits("Weapon") then
                card_str = ("@AnqiCard=%d"):format(card:getEffectiveId())
                break
            end
        end
        
       for _,enemy in ipairs(self.enemies) do
            if enemy:getHp()==1 and enemy:getHandcardNum()<2 then
                card_str = ("@AnqiCard=%d"):format(self.player:getWeapon():getEffectiveId())
                break
            end
        end
        
    end
    
    if card_str then
        return sgs.Card_Parse(card_str)
    end
    
end

sgs.ai_skill_use_func["AnqiCard"]=function(card,use,self)

    local target = nil
    
    self:updatePlayers(true)
	self:sort(self.enemies,"defense")
    
   for _,enemy in ipairs(self.enemies) do
        if self.player:distanceTo(enemy)<=4 then
            target = enemy
            break
        end
    end
        
    if target then
        use.card = card
        if use.to then
            use.to:append(target)
        end
    end
    
end

-- fanxiang
sgs.ai_skill_use["@@fanxiang"]=function(self,prompt)
    local parsedPrompt = prompt:split(":")
    self.room:writeToConsole("fanxiang skill invoke?----"..prompt)
    local card = sgs.Sanguosha:getCard(parsedPrompt[2])

    if self.player:getMp()<1 or not card then return "." end
    
    local canInvoke = false
    
    if (card:inherits("Slash") and self:getCardsNum("Dodge")==0)
    or (card:inherits("AOE") and self.player:isWounded())
    or ((card:inherits("Grab") or card:inherits("DestroyAll")) and self.player:getHandcardNum()==1)
    then
        canInvoke = true
    end
    
    if canInvoke then
       self:sort(self.enemies,"defense")
       for _,enemy in ipairs(self.enemies) do
            if self.player:distanceTo(enemy)<=1 then
                return "@FanxiangCard=.->"..enemy:objectName()
            end
        end
    end
    
    return "."    
end

-- lingfu
local lingfu_skill={}
lingfu_skill.name="lingfu"
table.insert(sgs.ai_skills,lingfu_skill)
lingfu_skill.getTurnUseCard=function(self)
    local card_str = nil
    
    if self.player:hasUsed("LingfuCard") or self.player:getMp()<2 or self.player:getHandcardNum()<1 then return end
    
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
    self:sortByUseValue(cards, true)
    
    if cards[1] and self:getUseValue(cards[1])<4 then
        card_str = "@LingfuCard="..cards[1]:getEffectiveId()
    end
    
    if card_str then
        return sgs.Card_Parse(card_str)
    end
    
end

sgs.ai_skill_use_func["LingfuCard"]=function(card,use,self)

    local target = nil
    
    self:updatePlayers(true)
	self:sort(self.enemies,"defense")
    
    target = self.enemies[1]
        
    if target then
        use.card = card
        if use.to then
            use.to:append(target)
        end
    end
    
end

-- phobos ---------------------------------------------------------------------------------
sgs.ai_chaofeng["phobos"] = 8
sgs.phobos_keep_value = {
    VineArmor = -10,
    HolyWing = 6,
}
sgs.ai_skill_invoke.huwei = function(self, data) 
    if self.player:getMp()<2 then return false end

    local damage = data:toDamage()
	if self:isFriend(damage.to) then
        if damage.nature == sgs.DamageStruct_Normal 
        or damage.nature == sgs.DamageStruct_Poison 
        or self.player:getHp()>1 
        then
            return true
        end
    end
    
    return false
end

-- donovan ---------------------------------------------------------------------------------
sgs.ai_chaofeng["donovan"] = 2

sgs.donovan_keep_value = {
	ThunderBang = 3.5,
}

sgs.dynamic_value.benefit["LeishenCard"] = true
sgs.dynamic_value.damage_card["XueshenCard"] = true

-- huoshen
local huoshen_skill={}
huoshen_skill.name="huoshen"
table.insert(sgs.ai_skills,huoshen_skill)
huoshen_skill.getTurnUseCard=function(self)
    local card_str = nil
    
    if not self.player:canSlashWithoutCrossbow() or self.player:getHp()>=3 or self.player:getMp()>3 then return end
    
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
    self:sortByUseValue(cards, true)
    
    if cards[1] and self:getUseValue(cards[1])<3.5 then
        local suit = cards[1]:getSuitString()
        local number = cards[1]:getNumberString()
        local card_id = cards[1]:getEffectiveId()
        card_str = ("fire_bang:huoshen[%s:%s]=%d"):format(suit, number, card_id)
    end
    
    if card_str then
        local fire_bang = sgs.Card_Parse(card_str)    
        assert(fire_bang)    
        return fire_bang
    end
    
end

 -- leishen
sgs.ai_skill_use["@@leishen"]=function(self,prompt)    
    if self.player:getMp()<=3 or self.player:getHandcardNum()<=1 then return "." end
    
    local thunder_bang = 0
    
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
    self:sortByUseValue(cards, true)

   for _,hcard in ipairs(cards) do
        if hcard:inherits("ThunderBang") or (hcard:inherits("Bang") and hcard:getSuit() == sgs.Card_Diamond) then
            thunder_bang = thunder_bang+1
        end
    end
    
    if thunder_bang>2 then
        return "@LeishenCard="..cards[1]:getEffectiveId().."->."
    end
    
    return "."
end

 -- leishen_bang
local leishen_bang_skill={}
leishen_bang_skill.name="leishen_bang"
table.insert(sgs.ai_skills,leishen_bang_skill)
leishen_bang_skill.getTurnUseCard=function(self)
    local card_str = nil
    
    if not self.player:hasFlag("leishen_on") then return end
    
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
    
   for _,hcard in ipairs(cards) do
        if hcard:inherits("ThunderBang") or (hcard:inherits("Bang") and hcard:getSuit() == sgs.Card_Diamond) then
            local suit = hcard:getSuitString()
            local number = hcard:getNumberString()
            local card_id = hcard:getEffectiveId()
            if hcard:inherits("ThunderBang") then
                card_str = card_id
            else 
                card_str = ("thunder_bang:leishen_bang[%s:%s]=%d"):format(suit, number, card_id)
            end
        end
    end
    
    if card_str then
        local thunder_bang = sgs.Card_Parse(card_str)    
        assert(thunder_bang)
        return thunder_bang
    end
    
end

-- xueshen
local xueshen_skill={}
xueshen_skill.name="xueshen"
table.insert(sgs.ai_skills,xueshen_skill)
xueshen_skill.getTurnUseCard=function(self)
    local card_str = nil
    
    if not self.player:canSlashWithoutCrossbow() or self.player:getMp()<=4 then return end
    
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
    
   for _,hcard in ipairs(cards) do
        if hcard:inherits("Bang") and hcard:getSuit() == sgs.Card_Club then
            card_str = "@XueshenCard="..hcard:getEffectiveId()
            break
        end
    end
    
    if card_str then
        return sgs.Card_Parse(card_str)
    end
    
end

sgs.ai_skill_use_func["XueshenCard"]=function(card,use,self)

    local target = nil

	self:sort(self.enemies,"hp")
    
    target = self.enemies[1]
        
    if target then
        use.card = card
        if use.to then
            use.to:append(target)
        end
    end
    
end

--anita
local anita_skill={}
anita_skill.name="anita"
table.insert(sgs.ai_skills,anita_skill)
anita_skill.getTurnUseCard=function(self)
    if self.player:getMark("anita_called")~=0 then return end

    local card_str = nil
    local defense_card = 0
    
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
    
   for _,hcard in ipairs(cards) do
        if hcard:inherits("Dodge") or hcard:inherits("HolyWater") then
            defense_card = defense_card+1
        end
    end
    
    if defense_card>1 or self.player:getHp()==1 then
        card_str = "@AnitaCard=."
    end
    
    if card_str then
        return sgs.Card_Parse(card_str)
    end
    
end

sgs.ai_skill_use_func["AnitaCard"]=function(card,use,self)
    use.card = card
end

 -- anita_anwei
sgs.ai_skill_use["@@card_to_anita"]=function(self,prompt)    
    if self.player:getMark("@anita")<=0 then return "." end

    local card_str = nil
    local can_give = false
    local first_card, second_card
        
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
    self:sortByUseValue(cards, true)
    
   for _,hcard in ipairs(cards) do
        if hcard:inherits("Dodge") or hcard:inherits("HolyWater") then
            if can_give then
                first_card = hcard
                break
            else        
                can_give = true
            end
        end
    end
    
    if not first_card and self.player:getHandcardNum()>self.player:getMaxHP() then
        first_card = cards[1]
    end
    
    if first_card and self.player:getHandcardNum()-self.player:getMaxHP()>=2 then
        if cards[1] == first_card then
            second_card = cards[2]
        else 
            second_card = cards[1]
        end        
    end
    
    if first_card then
        card_str = "@AnitaAnweiCard="..first_card:getEffectiveId()
        
        if second_card then
            card_str = card_str.."+"..second_card:getEffectiveId()
        end
    end
    
    if card_str then
        return card_str.."->."
    end
    
    return "."
end

sgs.ai_skill_invoke.anita_anwei = function(self, data) 
    if self.player:getMark("@anita")>0 and self.player:getMp()>0 and (self.player:getHandcardNum() == 0 or self.player:hasFlag("leishen_on")) then
        return true
    end
    
    return false
end

sgs.ai_skill_invoke.anita_xisheng = function(self, data) 
	local dying = data:toDying()
	local need = 1 - dying.who:getHp()

	local cards = self.player:getHandcards()
	local n = 0
	for _, card in sgs.qlist(cards) do
		if card:inherits "HolyWater" or card:inherits "Schnapps" then
			n = n + 1
		end
	end

	return n < need
end

-- jedah ---------------------------------------------------------------------------------
sgs.ai_chaofeng["jedah"] = 6

sgs.dynamic_value.benefit["JiushuCard"] = true
sgs.dynamic_value.benefit["ZhipeiCard"] = true

-- jiushu
local jiushu_skill={}
jiushu_skill.name="jiushu"
table.insert(sgs.ai_skills,jiushu_skill)
jiushu_skill.getTurnUseCard=function(self)
    if self.player:getMp()<20 then return end
    
    local all = self.room:getServerPlayers()
    for _, p in sgs.qlist(all) do
        if p:isDead() then
            return sgs.Card_Parse("@JiushuCard=.")   
        end
    end
    
end

sgs.ai_skill_use_func["JiushuCard"]=function(card,use,self)
    use.card = card
end

 --qiyue
sgs.ai_skill_use["@@qiyue"]=function(self,prompt)
    local target = nil
    self:sort(self.friends_noself, "handcard")

    for _, p in ipairs(self.friends_noself) do
        if p:getMark("@jiushu")>0 then
            target = p
        end
    end
    
    if target then
        return "@QiyueCard=.->"..target:objectName()
    end

    return "."
end

 --zhipei
local zhipei_skill={}
zhipei_skill.name="zhipei"
table.insert(sgs.ai_skills,zhipei_skill)
zhipei_skill.getTurnUseCard=function(self)
    
    if self.player:hasUsed("ZhipeiCard") then return end
    
    for _, p in sgs.qlist(self.room:getAllPlayers()) do
        if p:hasFlag("jiushu_qiyue") then
            return sgs.Card_Parse("@ZhipeiCard=.")
        end
    end
    
end

sgs.ai_skill_use_func["ZhipeiCard"]=function(card,use,self)
    use.card = card
end

--zhipeigive
sgs.ai_skill_use["@zhipeigive"]=function(self,prompt)
    local card_str
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)    
    self:sortByKeepValue(cards, true)
    
    if #cards>4 then
        card_str = ("@ZhipeiGiveCard=%d+%d"):format(cards[1]:getId(),cards[2]:getId())        
    elseif #cards>2 then
        card_str = ("@ZhipeiGiveCard=%d"):format(cards[1]:getId())
    end
    
    if card_str then
        return card_str.."->."
    end
    
    return "."
end

-- anakaris ---------------------------------------------------------------------------------
sgs.ai_chaofeng["anakaris"] = 2

sgs.dynamic_value.control_card["ZhicaiCard"] = true

-- guanwu
local guanwu_skill={}
guanwu_skill.name="guanwu"
table.insert(sgs.ai_skills,guanwu_skill)
guanwu_skill.getTurnUseCard=function(self)
    if self.player:getMp()<5 then return end
    
    for _, friend in ipairs(self.friends) do
        if friend:getHp()==1 and friend:getHandcardNum()<2 and friend:getMark("@invincible")<=0 then 
            return sgs.Card_Parse("@GuanwuCard=.")
        end
    end 
   
end

sgs.ai_skill_use_func["GuanwuCard"]=function(card,use,self)
    
    local target = nil
    
    for _, friend in ipairs(self.friends) do
        if friend:getHp()==1 and friend:getHandcardNum()<2 and friend:getMark("@invincible")<=0 then 
            use.card = card
            if use.to then
                use.to:append(friend)
            end
        end
    end 
    
end

-- zhicai
local zhicai_skill={}
zhicai_skill.name="zhicai"
table.insert(sgs.ai_skills,zhicai_skill)
zhicai_skill.getTurnUseCard=function(self)
    if self.player:getMp()<18 then return end
    
    for _, enemy in ipairs(self.enemies) do
        if enemy:getHp()<=2 and enemy:getMark("@card_forbid")<=0 then 
            return sgs.Card_Parse("@ZhicaiCard=.")
        end
    end 
   
end

sgs.ai_skill_use_func["ZhicaiCard"]=function(card,use,self)
    
    local target = nil
    
    for _, enemy in ipairs(self.enemies) do
        if enemy:getHp()<=2 and enemy:getMark("@card_forbid")<=0 then
            use.card = card
            if use.to then
                use.to:append(enemy)
            end
        end
    end 
    
end

-- bulleta ---------------------------------------------------------------------------------
sgs.ai_chaofeng["bulleta"] = 4

sgs.dynamic_value.damage_card["YoubuCard"] = true

-- baijin
sgs.ai_skill_invoke.baijin = function(self, data) 
    return self.player:getMp()>0
end

-- shoulie
sgs.ai_skill_invoke.shoulie = function(self, data) 
    return self.player:getMp()>0 and self:GetDefense(self.player)<=8
end

-- youbu
local youbu_skill={}
youbu_skill.name="youbu"
table.insert(sgs.ai_skills,youbu_skill)
youbu_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("YoubuCard") then return end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
    
    if #cards>6 or self:getUseValue(cards[1])<4.5 then
        for _,card in ipairs(cards) do
            if #cards<4 and (card:inherits("Cure") or card:inherits("HolyWater")) then
                return
            end
        end
        return sgs.Card_Parse("@YoubuCard=.")
    end
end

sgs.ai_skill_use_func["YoubuCard"]=function(card,use,self)
    self:updatePlayers(true)
	self:sort(self.enemies,"defense")

    use.card = card
    if use.to then
        use.to:append(self.enemies[1])
    end
end

-- bishamon ---------------------------------------------------------------------------------
sgs.ai_chaofeng["bishamon"] = 5

sgs.dynamic_value.control_card["GuiyanCard"] = true
sgs.dynamic_value.benefit["YuanhunCard"] = true

-- guiyan
local guiyan_skill={}
guiyan_skill.name="guiyan"
table.insert(sgs.ai_skills,guiyan_skill)
guiyan_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("GuiyanCard") or self.player:getMp()<8 then return end
    
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)

    local hasSlash = false    
    for _,c in ipairs(cards) do
        if c:inherits("Slash") then
            hasSlash = true
            break
        end
    end
    
    local hasEnemy = false
    if hasSlash then
        self:sort(self.enemies, "defense")
        for _, p in ipairs(self.enemies) do
            if p:getMark("@card_forbid")<=0 
            and self.player:inMyAttackRange(p) 
            and (p:isWounded() or self.player:getMp()>32 or self.player:hasFlag("drank")) then
                hasEnemy = true
                break
            end
        end 
    end
    
    if hasEnemy then
        return sgs.Card_Parse("@GuiyanCard=.")
    end
    
end

sgs.ai_skill_use_func["GuiyanCard"]=function(card,use,self)

    for _, p in ipairs(self.enemies) do
        if p:getMark("@card_forbid")<=0 
        and self.player:inMyAttackRange(p) 
        and (p:isWounded() or self.player:getMp()>32 or self.player:hasFlag("drank")) then
            use.card = card
            if use.to then
                use.to:append(p)
            end
            break
        end
    end 
    
end

-- yuanhun
local yuanhun_skill={}
yuanhun_skill.name="yuanhun"
table.insert(sgs.ai_skills,yuanhun_skill)
yuanhun_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("YuanhunCard") or self.player:getMp()<24 or self.player:getMaxHP()<3 then return end
    
    if (self.player:getHp()==1 and self.player:getHandcardNum()<4)
    or (self.player:getHp()==2 and self.player:getHandcardNum()<1) then
        return sgs.Card_Parse("@YuanhunCard=.")
    end
    
end

sgs.ai_skill_use_func["YuanhunCard"]=function(card,use,self)
    use.card = card
end

-- felicia ---------------------------------------------------------------------------------
sgs.ai_chaofeng["felicia"] = 6

sgs.dynamic_value.benefit["LeyuanCard"] = true

-- leyuan
local leyuan_skill={}
leyuan_skill.name="leyuan"
table.insert(sgs.ai_skills,leyuan_skill)
leyuan_skill.getTurnUseCard=function(self)
    
    local use_card = nil

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)

    local cure, holywater = nil, nil
    
    for _,c in ipairs(cards) do
        if c:inherits("Cure") then
            cure = c
            break
        elseif c:inherits("HolyWater") then
            holywater = c
            break
        end
        
    end
    
    if not (cure or holywater) then return end
    
    for _, friend in ipairs(self.friends) do
        if friend:getMaxHP()-friend:getHp()>1 then 
            use_card = cure or holywater
            break
        elseif friend:isWounded() then
            use_card = holywater
            break
        end
    end    
    
    if use_card then
        return sgs.Card_Parse("@LeyuanCard="..use_card:getEffectiveId())
    end
    
end

sgs.ai_skill_use_func["LeyuanCard"]=function(card,use,self)    
    
    local user_num = 0
    
    use.card = card
    for _, friend in ipairs(self.friends) do
        if friend:getMaxHP()-friend:getHp()>1 then 
            if user_num >= 3 then break end
            if use.to then
                use.to:append(friend)
            end
            user_num = user_num+1
        end
    end 
    
    if user_num<3 then
        for _, friend in ipairs(self.friends) do
            if friend:getMaxHP()-friend:getHp()==1 then 
                if user_num >= 3 then break end
                if use.to then
                    use.to:append(friend)
                end
                user_num = user_num+1
            end
        end 
    end
end

-- huhuan
sgs.ai_skill_invoke.huhuan = function(self, data)
    return self.player:getMp()>0
end

-- gallon ---------------------------------------------------------------------------------
sgs.ai_chaofeng["gallon"] = 0

-- manyue
sgs.ai_skill_invoke.manyue = true
