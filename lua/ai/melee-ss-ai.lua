-- haohmaru ---------------------------------------------------------------------------------
sgs.ai_chaofeng["haohmaru"] = 2
sgs.haohmaru_keep_value = {
	Schnapps = 2,
	Bang = 3,
	ThunderBang = 3.5,
	AirBang = 3.5,
	FireBang = 3.6,
	PoisonBang = 3.7,
	IceBang = 3.8,
}

sgs.dynamic_value.damage_card["TianbaCard"] = true

-- jiuqi
jiuqi_skill={}
jiuqi_skill.name="jiuqi"
table.insert(sgs.ai_skills,jiuqi_skill)
jiuqi_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	for _,acard in ipairs(cards)  do
		if acard:getSuit() == sgs.Card_Spade then
			card = acard
			break
		end
	end
	if not card then return nil end
	
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("schnapps:jiuqi[spade:%s]=%d"):format(number, card_id)
	local schnapps = sgs.Card_Parse(card_str)

	return schnapps
		
end

-- tianba
sgs.ai_skill_use["@@tianba"]=function(self,prompt)
    self:updatePlayers()
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
    
    for _,card in ipairs(cards) do
        if card:inherits("Slash") then
            for _,enemy in ipairs(self.enemies) do
                if self.player:canSlash(enemy, true) then
                    -- self.player:invoke("animate", "tianba:")
                    return "@TianbaCard="..card:getEffectiveId().."->"..enemy:objectName()
                end
            end
        end
    end
    
    return "."    
end

-- nakoruru ---------------------------------------------------------------------------------
sgs.ai_chaofeng["nakoruru"] = 2

sgs.dynamic_value.benefit["YingxuanCard"] = true

-- goutong
local goutong_skill={}
goutong_skill.name="goutong"
table.insert(sgs.ai_skills,goutong_skill)
goutong_skill.getTurnUseCard=function(self)
    local cards = self.player:getHandcards()    
    if cards:length()<1 or self.player:getMp()<1 then return end
    
    if self.player:hasUsed("GoutongCard") then return end
    
    return sgs.Card_Parse("@GoutongCard=.")
end

sgs.ai_skill_use_func["GoutongCard"]=function(card,use,self)
    local target, card_str
    local cards = self.player:getHandcards()    
    
    cards=sgs.QList2Table(cards)    
    self:sortByUseValue(cards)
    
    for _, friend in ipairs(self.friends_noself) do
        if friend:getHp() == 1  and self.player:getMp()>=1 and friend:getHandcardNum()>0 then
            for _, hcard in ipairs(cards) do
                if hcard:inherits("Schnapps") or hcard:inherits("HolyWater") then 
                    card_str = "@GoutongCard="..hcard:getEffectiveId().."->"..friend:objectName()
                    target = friend
                    break
                end
            end        
        end
    end
    
    if not card_str and self:getKeepValue(cards[1]) < 4 then
        for _,enemy in ipairs(self.enemies) do
            if enemy:getHandcardNum() == 1 and self.player:getMp()>=1 then
                card_str = "@GoutongCard="..cards[1]:getEffectiveId().."->"..enemy:objectName()
                target = enemy
                break
            end
        end
    end
    
    if card_str then
        use.card = sgs.Card_Parse(card_str)
        if use.to then 
            use.to:append(target)
        end
    end
  
end

-- yingxuan
local yingxuan_skill={}
yingxuan_skill.name="yingxuan"
table.insert(sgs.ai_skills,yingxuan_skill)
yingxuan_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("YingxuanCard") or self.player:getMark("@yingxuan")>0 then return end    
    
    local card_str
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)    
    self:sortByUseValue(cards, true)
    
    if self.player:getHp()<=2 and self:getKeepValue(cards[1]) < 4 then
        card_str = "@YingxuanCard="..cards[1]:getEffectiveId()
    end
    
    if card_str then
        return sgs.Card_Parse(card_str)
    end
end

sgs.ai_skill_use_func["YingxuanCard"]=function(card,use,self)
    use.card = card
end

-- ukyo ---------------------------------------------------------------------------------
sgs.ai_chaofeng["ukyo"] = 6

sgs.dynamic_value.damage_card["LiulianCard"] = true

-- juhe
sgs.ai_skill_invoke.juhe = function(self, data)
	local effect = data:toSlashEffect()
    
    if self:isFriend(effect.to) then return false end
    
    if effect.to:getHandcardNum()>2 or
    effect.to:getHp()<3 or
    self.player:hasFlag("drank") then
        return true
    end    
end

-- liulian
sgs.ai_skill_use["@@liulian"]=function(self,prompt)
    
    if self.player:getHp() >=2 then
        self:sort(self.enemies, "defense")
        
        for _,enemy in ipairs(self.enemies) do
            if self.player:canSlash(enemy, true) then
                -- self.player:invoke("animate", "liulian:")
                return "@LiulianCard=.->"..enemy:objectName()
            end
        end
    end
    
    return "."
end

-- kyoshiro ---------------------------------------------------------------------------------
sgs.ai_chaofeng["kyoshiro"] = 0

-- quwu
local quwu_skill={}
quwu_skill.name="quwu"
table.insert(sgs.ai_skills,quwu_skill)
quwu_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("QuwuCard") then return end    
    
    local good, bad = 0, 0
    for _, friend in ipairs(self.friends_noself) do
        if friend:getHandcardNum() == 0 then 
            bad = bad + 3
        end
        good = good + friend:getHandcardNum()
    end

    for _, enemy in ipairs(self.enemies) do
        if enemy:getHandcardNum() == 0 then 
            good = good + 3
        end
        bad = bad + enemy:getHandcardNum()
    end
    
    if good <= bad then return nil end

    local card_str
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)    
    self:sortByUseValue(cards, true)
    
    if self.player:getHandcardNum()>1 and self:getKeepValue(cards[1]) < 4.5 then
        card_str = ("@QuwuCard=%d"):format(cards[1]:getId())
    end
    
    if card_str then
        return sgs.Card_Parse(card_str)
    end
    
end

sgs.ai_skill_use_func["QuwuCard"]=function(card,use,self)
    use.card = card
end

-- yanwu
sgs.ai_skill_use["@@yanwu"]=function(self,prompt)
    
    local good, bad = 0, 0
    for _, friend in ipairs(self.friends_noself) do
        if friend:getHandcardNum() == 0 then 
            bad = bad + 3
        end
        good = good + friend:getHandcardNum()
    end

    for _, enemy in ipairs(self.enemies) do
        if enemy:getHandcardNum() == 0 then 
            good = good + 3
        end
        bad = bad + enemy:getHandcardNum()
    end
    
    if good <= bad and self.player:getHp()>2 then return "." end

    local card_str
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)    
    self:sortByUseValue(cards, true)
    
    if self.player:getHandcardNum() then
        -- self.player:invoke("animate", "yanwu:")
        return "@YanwuCard="..cards[1]:getEffectiveId().."->."
    end

    return "."
end

-- genjuro ---------------------------------------------------------------------------------
sgs.ai_chaofeng["genjuro"] = 6

-- yinghua
sgs.ai_skill_use["@@yinghua"]=function(self,prompt)
    local mp = self.player:getMp()
    
    if mp<3 or mp > 25 and mp < 35 then return "." end

    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)    
    self:sortByUseValue(cards, true)
    
    local players=sgs.QList2Table(self.player:getRoom():getOtherPlayers(self.player))
    
    for _, card in ipairs(cards) do
        if card:getNumber() > 10 then
            for _, p in ipairs(players) do
                if not self:isFriend(p) and p:getHandcardNum()>0 and p:getHandcardNum()<3 then 
                    return "@YinghuaCard="..card:getEffectiveId().."->"..p:objectName()
                end
            end
            
            for _, p in ipairs(players) do
                if self:isFriend(p) and p:getHandcardNum()>=3 then 
                    return "@YinghuaCard="..card:getEffectiveId().."->"..p:objectName()
                end
            end
            
            break;
        end
    end
    
    return "."
end

--sudi
sgs.ai_skill_use["@@sudi"]=function(self,prompt)
    
    local enemy = nil
    self:sort(self.enemies, "chaofeng")
    
    for _, p in ipairs(self.enemies) do
        if p:getHp() == self.player:getHp() then
            enemy = p
        end
    end
    
    if enemy then 
        return "@SudiCard=.->"..enemy:objectName()
    end
    
    return "."
end

-- zhansha
sgs.ai_skill_invoke.zhansha = function(self, data)
    -- self.room:writeToConsole("zhansha invoke")
    local players = self.room:getAllPlayers()
    if self.player:getMp()<math.max(30, players:length()*5) then return false end
    
    for _, p in ipairs(self.enemies) do        
        if p:getMark("@sudimark") then return true end
    end
    
    return false
end

-- sogetsu ---------------------------------------------------------------------------------
sgs.ai_chaofeng["sogetsu"] = -2

-- yueyin
sgs.ai_skill_invoke.yueyin = function(self, data)
    local allcards = self.player:getCards("he")
    if allcards:length()<2 then return false end
    
    card_use = data:toCardEffect()
    
    local card = card_use.card
    if card then self.room:writeToConsole("yueyin::"..card:objectName()) end 
    
    if not card 
    or card:inherits("Cure") 
    or card:inherits("GlobalEffect") 
    or card:inherits("DestroyAll")
    or card:inherits("Burn")
    or card:inherits("HolyWater")
    or card:inherits("SoulChain") then return false end
    
    local cards = self.player:getHandcards()
    
    cards=sgs.QList2Table(cards)
    self:sortByUseValue(cards, true)    
    
    if cards[1] and self:getKeepValue(cards[1]) >= 3.5 and self.player:getHp()>=2 then return false end
    
    if self.player:getHp()<2 or allcards:length()>3 then return true end
    
    return false
end

-- jiefang
sgs.ai_skill_use["@@jiefang"]=function(self,prompt)
    if self.player:getMp()<4 or self.player:getHp()<2 then return "." end
     
    for _, p in ipairs(self.enemies) do        
        if p:getHp()<2 and self.player:getMp()>4 then
            return "@JiefangCard=.->."
        end
    end
    
    return "."   
end

-- suija ---------------------------------------------------------------------------------
sgs.ai_chaofeng["sogetsu"] = 4

sgs.dynamic_value.damage_card["SiyueCard"] = true

-- siyue
local siyue_skill={}
siyue_skill.name="siyue"
table.insert(sgs.ai_skills,siyue_skill)
siyue_skill.getTurnUseCard=function(self)
    if self.player:getMp()<2 or self.player:getHandcardNum()<1 then return end    
    
    local card_str
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)    
    self:sortByUseValue(cards, true)
    
    card_str = ("@SiyueCard=%d"):format(cards[1]:getId())
    return sgs.Card_Parse(card_str)   
    
end

sgs.ai_skill_use_func["SiyueCard"]=function(card,use,self)

	self:sort(self.enemies,"defense")
    
    for _, enemy in ipairs(self.enemies) do
        if  self:damageIsEffective("ice", enemy) then 
            use.card = card
            if use.to then 
                use.to:append(self.enemies[1])
            end    
        end
    end 
    
end

-- fengyin
sgs.ai_skill_use["@@fengyin"]=function(self,prompt)
    if self.player:getMp()<1 then return "." end
     
    self:updatePlayers(true)
	self:sort(self.enemies,"defense")
     
    if getDefense(self.enemies[1]) > 8 or self.player:getHp()<2 then
        return "@FengyinCard=.->."
    end
    
    return "."   
end

-- kazuki ---------------------------------------------------------------------------------
sgs.ai_chaofeng["kazuki"] = 3

--yanmie
yanmie_skill={}
yanmie_skill.name="yanmie"
table.insert(sgs.ai_skills,yanmie_skill)
yanmie_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local normal_bang
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:objectName()=="bang" then
			normal_bang = card
			break
		end
	end

	if normal_bang then		
		local suit = normal_bang:getSuitString()
    	local number = normal_bang:getNumberString()
		local card_id = normal_bang:getEffectiveId()
		local card_str = ("fire_bang:yanmie[%s:%s]=%d"):format(suit, number, card_id)
		local fire_bang = sgs.Card_Parse(card_str)
		
		assert(fire_bang)
        
        return fire_bang
	end
end

-- enja ---------------------------------------------------------------------------------
sgs.ai_chaofeng["enja"] = 8

sgs.dynamic_value.damage_card["BaoshaCard"] = true

-- baosha
local baosha_skill={}
baosha_skill.name="baosha"
table.insert(sgs.ai_skills,baosha_skill)
baosha_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("BaoshaCard") or self.player:getMp()<2 or self.player:getHandcardNum()<=0 or not self.player:canSlashWithoutCrossbow() then return end    
    
    local card_str, use_card
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards) 
    self:sortByUseValue(cards,true)
    
    for _, hcard in ipairs(cards) do
        if hcard:inherits("Slash") then
            use_card = hcard
            break
        end
    end
    
    if not use_card then return end
    
    local good, bad = 0, 0
    local who = self.player   
    
    for _, friend in ipairs(self.friends_noself) do
        if self.player:inMyAttackRange(friend) and friend:getHandcardNum()<2 and self:damageIsEffective("fire", friend) then 
            if friend:getHp() == 1 then 
                bad = bad + 5
            end
            bad = bad + 3
        end
    end

    for _, enemy in ipairs(self.enemies) do
        if who:distanceTo(enemy)<=who:getAttackRange() and enemy:getHandcardNum()<2 and  self:damageIsEffective("fire", enemy) then 
            if enemy:getHp() == 1 then 
                good = good + 5
            end
            good = good + 3
        end
    end
    
    if good < bad then return end
    
    card_str = ("@BaoshaCard=%d"):format(use_card:getId())
    return sgs.Card_Parse(card_str)
    
end

sgs.ai_skill_use_func["BaoshaCard"]=function(card,use,self)
    use.card = card
end

-- galford ---------------------------------------------------------------------------------
sgs.ai_chaofeng["galford"] = 6

sgs.dynamic_value.damage_card["RenquanCard"] = true
sgs.dynamic_value.benefit["DianguangCard"] = true

-- renquan
local renquan_skill={}
renquan_skill.name="renquan"
table.insert(sgs.ai_skills,renquan_skill)
renquan_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("RenquanCard") or self.player:getHandcardNum()<=0 then return end
    
    local use_card, card_str
    
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards) 
    self:sortByUseValue(cards,true)
    
    for _, hcard in ipairs(cards) do
        if hcard:inherits("BasicCard") and not hcard:inherits("HolyWater") then
            use_card = hcard
            break
        end
    end
    
    if not use_card then return end
    
    card_str = ("@RenquanCard=%d"):format(use_card:getId())
    return sgs.Card_Parse(card_str)
    
end

sgs.ai_skill_use_func["RenquanCard"]=function(card,use,self)
    local target
    
    self:updatePlayers(true)
	self:sort(self.enemies,"defense")
    
   for _,enemy in ipairs(self.enemies) do
        if self.player:distanceTo(enemy)<=2 then
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

-- dianguang
local dianguang_skill={}
dianguang_skill.name="dianguang"
table.insert(sgs.ai_skills,dianguang_skill)
dianguang_skill.getTurnUseCard=function(self)
    if self.player:getMark("@dianguang")>0 or self.player:getMp()<3 then return end    
    return sgs.Card_Parse("@DianguangCard=.")
end

sgs.ai_skill_use_func["DianguangCard"]=function(card,use,self)
    use.card = card
end

-- rimururu ---------------------------------------------------------------------------------
sgs.ai_chaofeng["rimururu"] = -4

sgs.rimururu_keep_value = {
    Armor = 0,
}

sgs.dynamic_value.benefit["BingrenCard"] = true

-- bingren
local bingren_skill={}
bingren_skill.name="bingren"
table.insert(sgs.ai_skills,bingren_skill)
bingren_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("BingrenCard") or self.player:getMp()<3 then return end
    
    local card_str = "@BingrenCard=."
    return sgs.Card_Parse(card_str)
    
end

sgs.ai_skill_use_func["BingrenCard"]=function(card,use,self)
    local target
    
    self:updatePlayers(true)
    
   for _,friend in ipairs(self.friends) do
        if friend:getWeapon() and not friend:hasSkill("bingren_on") then
            target = friend
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

--bingren_on
bingren_on_skill={}
bingren_on_skill.name="bingren_on"
table.insert(sgs.ai_skills,bingren_on_skill)
bingren_on_skill.getTurnUseCard=function(self)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local normal_bang
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:objectName()=="bang" then
			normal_bang = card
			break
		end
	end

	if normal_bang then		
		local suit = normal_bang:getSuitString()
    	local number = normal_bang:getNumberString()
		local card_id = normal_bang:getEffectiveId()
		local card_str = ("ice_bang:bingren[%s:%s]=%d"):format(suit, number, card_id)
		local ice_bang = sgs.Card_Parse(card_str)
		
		assert(ice_bang)
        
        return ice_bang
	end
end

-- chuixue
local chuixue_skill={}
chuixue_skill.name="chuixue"
table.insert(sgs.ai_skills,chuixue_skill)
chuixue_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("ChuixueCard") or self.player:getMp()<5 then return end
    
    local good, bad = 0, 0
    local who = self.player   
    
    for _, friend in ipairs(self.friends_noself) do
        if friend:getHandcardNum()<2 and self:damageIsEffective("ice", friend) then 
            if friend:getHp() == 1 then 
                bad = bad + 5
            end
            bad = bad + 3
        end
    end

    for _, enemy in ipairs(self.enemies) do
        if enemy:getHandcardNum()<2 and self:damageIsEffective("ice", enemy) then 
            if enemy:getHp() == 1 then 
                good = good + 5
            end
            good = good + 3
        end
    end
    
    if good < bad then return nil end
    
    return sgs.Card_Parse("@ChuixueCard=.")
end

sgs.ai_skill_use_func["ChuixueCard"]=function(card,use,self)
    use.card = card
end

-- charlotte ---------------------------------------------------------------------------------
sgs.ai_chaofeng["charlotte"] = 0

-- pokong
sgs.ai_skill_invoke.pokong = function(self, data)
    local effect = data:toSlashEffect()
	return not self:isFriend(effect.to)
end

-- xunguang
sgs.ai_skill_invoke.xunguang = true

sgs.ai_skill_use["@@xunguang"]=function(self,prompt)
    
    local card = self.room:peek()
    local use={}
    local card_str = card:getId().."->"
    use.from = self.player
    use.to = self.room:getAllPlayers()
    local index = use.to:length()
    
    local type = card:getTypeId()

    if type == sgs.Card_Basic then
        self:useBasicCard(card, use, self.slash_distance_limit)
    elseif type == sgs.Card_Trick then
        self:useTrickCard(card, use)
    elseif type == sgs.Card_Skill then
        self:useSkillCard(card, use)
    else
        self:useEquipCard(card, use)
    end
    
    local target = use.to
    if target:at(index) then
        card_str = card_str..target:at(index):objectName()
        if target:at(index+1) then
            card_str = card_str.."+"..target:at(index+1):objectName()
        end
    end
    
    return card_str
end


-- hanzo ---------------------------------------------------------------------------------
sgs.ai_chaofeng["hanzo"] = -2
sgs.hanzo_keep_value = {
    EquipCard = 6,
}

sgs.dynamic_value.benefit["KongchanCard"] = true
sgs.dynamic_value.control_card["ChenyinCard"] = true

-- kongchan
sgs.ai_skill_use["@@kongchan"]=function(self,prompt)
    local equip
    local cards = self.player:getCards("he")
    cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
    
	for _,c in ipairs(cards)  do
		if c:inherits("EquipCard") then
			equip = c
			break
		end
	end
    
    if equip then
        return "@KongchanCard="..equip:getEffectiveId().."->."
    end
    
    return "."
end

 -- yingwu
 sgs.ai_skill_invoke.yingwu = function(self, data)
    return self.player:isWounded() and self.player:getMp()>=1    
end

-- chenyin
sgs.ai_skill_use["@@chenyin"]=function(self,prompt)
    self:sort(self.enemies, "chaofeng")
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards) 
    self:sortByUseValue(cards,true)
    for _, hcard in ipairs(cards) do
        if hcard:getNumber() > 10 then
            for _,enemy in ipairs(self.enemies) do
                if enemy:getHandcardNum()>0 and self.player:distanceTo(enemy)<=1 and enemy:getMark("@skill_forbid")==0 then
                    -- self.player:invoke("animate", "chenyin:")
                    return "@ChenyinCard="..hcard:getEffectiveId().."->"..enemy:objectName()
                end
            end
        end
    end
    
    return "."
end

-- jubei ---------------------------------------------------------------------------------
sgs.ai_chaofeng["jubei"] = 0

sgs.dynamic_value.control_card["DaoquCard"] = true
sgs.dynamic_value.benefit["ErdaoCard"] = true
sgs.dynamic_value.benefit["ShuangyueCard"] = true

-- erdao
local erdao_skill={}
erdao_skill.name="erdao"
table.insert(sgs.ai_skills,erdao_skill)
erdao_skill.getTurnUseCard=function(self)
    if not self.player:getWeapon() or self.player:hasUsed("ErdaoCard") or self.player:getMp()<2 or self.player:getMark("erdao") then return end
    
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
    
    for _,card in ipairs(cards) do
        if card:inherits("Weapon") then
            local card_str = ("@ErdaoCard=%d"):format(card:getEffectiveId())
            return sgs.Card_Parse(card_str)
        end
    end
    
    return

end

sgs.ai_skill_use_func["ErdaoCard"]=function(card,use,self)
    use.card = card
end

-- shuangyue
local shuangyue_skill={}
shuangyue_skill.name="shuangyue"
table.insert(sgs.ai_skills,shuangyue_skill)
shuangyue_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("ShuangyueCard") or self:getCardsNum("Slash")==0 then return end
    if self:getCardsNum("Slash")>0 and not self.player:canSlashWithoutCrossbow() then
        return sgs.Card_Parse("@ShuangyueCard=.")
    end
end

sgs.ai_skill_use_func["ShuangyueCard"]=function(card,use,self)
    use.card = card
end

 -- daoqu
sgs.ai_skill_use["@@daoqu"]=function(self,prompt)
    self:sort(self.enemies, "defense")
    
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards) 
    self:sortByUseValue(cards,true)
    
    for _,enemy in ipairs(self.enemies) do
        if enemy:getWeapon() and self.player:distanceTo(enemy)<=1 then
            return "@DaoquCard="..cards[1]:getEffectiveId().."->"..enemy:objectName()
        end
    end

    return "."
end