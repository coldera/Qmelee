sgs.dynamic_value = {
	damage_card = 
	{
		PK = true,
		ThousandsArrowsShot = true,
		Slash = true,
		Burn = true,
		AlienInvasion = true,
		PoisonMarish = true,
	},
	control_usecard = 
	{
		EnegyDrain = true,
		SoulAwe = true,
		Icebound = true,
	},
	control_card = 
	{
		Grab = true,
		DestroyAll = true,
		BorrowWeapon = true,
		Roar = true,
	},
	lucky_chance = 
	{
		Thunder = true,
	},
	benefit = 
	{
		Cure = true,
		Schnapps = true,
		HolyWater = true,
		SoulChain = true,
		NothingIsSomething = true,
	},
}

-- card skill --------------------------------------------------------------------------------------

sgs.ai_skill_invoke["ghost_fan"]=function(self, data)
	if self.player:hasFlag("drank") then return false end
	local effect = data:toSlashEffect() 
	local target = effect.to
	if self:isFriend(target) then return false end
	local hasPeach
	local cards = target:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("HolyWater") or card:inherits("Schnapps") then hasPeach = true break end
	end
	if hasPeach then return true end
	if (target:getHandcardNum() > 1 or target:getArmor()) and target:getHp() > 1 then
		return true
	end
	return false
end

sgs.ai_skill_invoke["deicide_bow"] = function(self, data)
    local effect = data:toSlashEffect()
    return effect and not self:isFriend(effect.to)
end

sgs.ai_skill_invoke["soul_spirit"] = function(self, data)
    local effect = data:toSlashEffect()
    return effect and not self:isFriend(effect.to)
end

sgs.ai_skill_invoke["red_dragon"] = function(self, data)
    local damage = data:toDamage()
    return damage and not self:isFriend(damage.to)
end

sgs.ai_skill_invoke["chaos_mirror"] = function(self, data)
    if data and data:toString()=="@huhuan-card" then
        for _, enemy in ipairs(self.enemies) do
            if enemy:hasSkill("huhuan") then return false end
        end
    end
    
    return true
end

sgs.ai_skill_invoke["cattle"] = function(self, data)
    return self.player:isWounded()
end

sgs.ai_skill_invoke["boar"] = function(self, data)
    local damage = data:toDamage()
    return damage and not self:isFriend(damage.from)
end

local snake_spear_skill={}
snake_spear_skill.name="snake_spear"
table.insert(sgs.ai_skills,snake_spear_skill)
snake_spear_skill.getTurnUseCard=function(self,inclusive)
    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
    
    if #cards<(self.player:getHp()+1) then return nil end
    if #cards<2 then return nil end
    if self:getCardsNum("Slash")>0 then return nil end
    
    self:sortByUseValue(cards,true)
    
    local suit1 = cards[1]:getSuitString()
	local card_id1 = cards[1]:getEffectiveId()
	
	local suit2 = cards[2]:getSuitString()
	local card_id2 = cards[2]:getEffectiveId()
	
	local suit=sgs.Card_NoSuit
	if cards[1]:isBlack() and cards[2]:isBlack() then suit=sgs.Card_Club
	elseif cards[1]:isBlack()==cards[2]:isBlack() then suit=sgs.Card_Diamond end
	
	local card_str = ("slash:snake_spear[%s:%s]=%d+%d"):format(suit, 0, card_id1, card_id2)
	
    local slash = sgs.Card_Parse(card_str)
    
    return slash
    
end

--blunderbuss

local blunderbuss_skill={}
blunderbuss_skill.name="blunderbuss"
table.insert(sgs.ai_skills,blunderbuss_skill)

blunderbuss_skill.getTurnUseCard=function(self)
    if self.player:getHandcardNum()<3 then return nil end
    
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)
    
    self:sortByUseValue(cards,true)
            
    if #cards>1 and self:getUseValue(cards[2]) > 5 then return nil end
    
    local first  = cards[1]:getEffectiveId()
    local second = cards[2]:getEffectiveId()

    local card_str = ("@BlunderbussSkillCard=%d+%d"):format(first, second)
    return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func["BlunderbussSkillCard"]=function(card,use,self)
	self:sort(self.enemies, "defense")
	
	for _, enemy in ipairs(self.enemies) do
		if enemy:getEquips():length()>0 then
			use.card=card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

sgs.ai_skill_cardchosen.blunderbussskill = function(self, who)
    if who:getArmor() then return who:getArmor() end
    if who:getRelic() then return who:getRelic() end
    if who:getWeapon() then return who:getWeapon() end
    if who:getHorse() then return who:getHorse() end
end

--spiderqueen

local spider_queen_skill={}
spider_queen_skill.name="spider_queen"
table.insert(sgs.ai_skills,spider_queen_skill)

spider_queen_skill.getTurnUseCard=function(self)
    if not self.player:getHorse():inherits("SpiderQueen") then return nil end
    
    local good, bad = 0, 0
    for _, friend in ipairs(self.friends_noself) do
        bad = bad + 20/(friend:getHp())+10
        if friend:isLord() and (friend:getHp()<3) then
            return 
        end
            
        if (friend:getHp()<2) and (self.player:isLord()) then
            return
        end
        good = good + self:getCardsNum("HolyWater", friend)
    end

    for _, enemy in ipairs(self.enemies) do
        good = good + 20/(enemy:getHp())+10
        
        if enemy:isLord() then
            good = good + 20/(enemy:getHp())
        end
        bad = bad + self:getCardsNum("HolyWater", enemy)
    end
    
    if good <= bad then return nil end
    
    local card_str = ("@SpiderQueenSkillCard=.")
    return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func["SpiderQueenSkillCard"]=function(card,use,self)
    use.card = sgs.Card_Parse("@SpiderQueenSkillCard=.")
end