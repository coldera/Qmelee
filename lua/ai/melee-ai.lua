 -- This is the Smart AI, and it should be loaded and run at the server side

-- "middleclass" is the Lua OOP library written by kikito
-- more information see: https://github.com/kikito/middleclass
require "middleclass"

-- initialize the random seed for later use
math.randomseed(os.time())

-- this table stores all specialized AI classes
sgs.ai_classes = {}
sgs.showFriendDebug = false
-- compare functions
sgs.ai_compare_funcs = {
	hp = function(a, b)
		return a:getHp() < b:getHp()
	end,

	handcard = function(a, b)
		return a:getHandcardNum() < b:getHandcardNum()
	end,

	value = function(a, b)
		return SmartAI.GetValue(a) < SmartAI.GetValue(b)
	end,

	chaofeng = function(a, b)
		local c1 = sgs.ai_chaofeng[a:getGeneralName()] or 0
		local c2 = sgs.ai_chaofeng[b:getGeneralName()] or 0

		if c1 == c2 then
			return sgs.ai_compare_funcs.value(a, b)
		else
			return c1 > c2
		end
	end,

	defense = function(a,b)
		return SmartAI.GetDefense(a) < SmartAI.GetDefense(b)
	end,

	threat = function ( a, b)
		local players=sgs.QList2Table(a:getRoom():getOtherPlayers(a))
		local d1=a:getHandcardNum()
		for _, player in ipairs(players) do
			if a:canSlash(player,true) then
				d1=d1+10/(getDefense(player))
			end
		end
		players=sgs.QList2Table(b:getRoom():getOtherPlayers(b))
		local d2=b:getHandcardNum()
		for _, player in ipairs(players) do
			if b:canSlash(player,true) then
				d2=d2+10/(getDefense(player))
			end
		end
		
		local c1 = sgs.ai_chaofeng[a:getGeneralName()] or 0
		local c2 = sgs.ai_chaofeng[b:getGeneralName()] or 0

		return d1+c1/2>d2+c2/2
	end,
}

--- this function is only function that exposed to the host program
--- and it clones an AI instance by general name
-- @param player The ServerPlayer object that want to create the AI object
-- @return The AI object
function CloneAI(player)
	local ai_class = sgs.ai_classes[player:getGeneralName()]
	if ai_class then
		return ai_class(player).lua_ai
	else
		return SmartAI(player).lua_ai
	end
end

--- FIXME: ?
function getCount(name)
	if sgs.ai_round[name] then 
                sgs.ai_round[name]=sgs.ai_round[name]+1
	else 
		sgs.ai_round[name]=1 
	end
        return sgs.ai_round[name]
end

-- SmartAI is the base class for all other specialized AI classes
SmartAI = class "SmartAI"
super = SmartAI

--- Calculate the value for a player, 1 hp = 2 handcard
-- @param player The ServerPlayer object 
-- @return Its value
function SmartAI.GetValue(player)
	return player:getHp() * 2 + player:getHandcardNum()
end

--- defense is defined as min(value, hp*3) + 2(if armor is present)+1(if dhorse is present)
function SmartAI.GetDefense(player)
	local defense = math.min(SmartAI.GetValue(player), player:getHp() * 3)
	if player:getArmor() then
		defense = defense + 2
	end
	
	if player:getDefensiveHorse() then
        defense = defense + 1
    end
	
	return defense
end

getDefense = SmartAI.GetDefense

function SmartAI.GetWeakestEnemy()
    local enemy = nil
    
    self:sort(self.enemies, "defense")
    for _, p in ipairs(self.enemies) do
        if self.player:canSlash(p, true) then
            enemy = p
            break
        end
    end
    
    return enemy
end

-- the "initialize" function is just the "constructor"
function SmartAI:initialize(player)	
	self.player = player
	self.room = player:getRoom()
	
	self.role =player:getRole()

	if sgs.ai_assumed[self.role] then sgs.ai_assumed[self.role] = sgs.ai_assumed[self.role] +1
	elseif self.role~="lord" then sgs.ai_assumed[self.role] =1
	end	
	
	self.lua_ai = sgs.LuaAI(player)
	self.lua_ai.callback = function(method_name, ...)
		local method = self[method_name]
		if method then
			local success, result1, result2 
			success, result1, result2 = pcall(method, self, ...) 
			if not success then 
				self.room:writeToConsole(result1) 
				sgs.showFriendDebug = true
                self.room:writeToConsole("========debug=======") 
                    local p = self.room:getCurrent()
                    self.room:writeToConsole(p:getGeneralName()) 
                self.room:writeToConsole("------------handcard-----------") 
                    local cards = p:getHandcards()
                    cards = sgs.QList2Table(cards)                    
                    for _,c in ipairs(cards) do
                        self.room:writeToConsole(c:objectName()) 
                    end
                self.room:writeToConsole("------------stack-----------") 
                    self.room:writeToConsole(method_name) 
                self.room:writeToConsole("===================") 
				-- self.room:writeToConsole(debug.traceback()) 
			else 
				return result1, result2 
			end  
		end
	end      
	
	self.retain=2
	--self.harsh_retain=true
	if not sgs.ai_royalty[self.player:objectName()] then
		--self.room:output("initialized"..self.player:objectName()..self.role)
		sgs.ai_royalty[self.player:objectName()]=0
	end
	if self.player:isLord() then
		sgs.ai_royalty[self.player:objectName()]=160
		sgs.ai_explicit[self.player:objectName()]="loyalist"
		if (sgs.ai_chaofeng[self.player:getGeneralName()] or 0) < 3 then
			sgs.ai_chaofeng[self.player:getGeneralName()]=3
		end
	end
        
    self.keepValue={}
    self.kept={}
end

sgs.ai_assumed={}

function SmartAI:printStand()
    self.room:output(self.player:getRole())
    self.room:output("enemies:")
    for _, player in ipairs(self.enemies) do
        self.room:output(player:getGeneralName())
    end
    self.room:output("end of enemies")
    self.room:output("friends:")
    for _, player in ipairs(self.friends) do
        self.room:output(player:getGeneralName())
    end
    self.room:output("end of friends")
end

function isRolePredictable()
    return useDefaultStrategy() or sgs.GetConfig("RolePredictable", true)	
end

function useDefaultStrategy()
	local mode=sgs.GetConfig("GameMode", "")
    if (mode=="06_3v3") or (not mode:find("0")) then return true end
    if (mode:find("02_1v1") or mode:find("03p")) then return true end
end

-- this function create 2 tables contains the friends and enemies, respectively
function SmartAI:updatePlayers(inclusive)
	self.friends = sgs.QList2Table(self.lua_ai:getFriends())
	table.insert(self.friends, self.player)

	self.friends_noself = sgs.QList2Table(self.lua_ai:getFriends())

	sgs.rebel_target=self.room:getLord()
	
	self.enemies = sgs.QList2Table(self.lua_ai:getEnemies())
	
	self.role =self.player:getRole()
	
	if isRolePredictable() then
		if (self.role=="lord") or (self.role=="loyalist") then self:refreshRoyalty(self.player,300)
		elseif (self.role=="rebel") then self:refreshRoyalty(self.player,-300)
		end
		
		self.retain=2
		self.harsh_retain=false

		if useDefaultStrategy() then return end
	end
	
	inclusive=inclusive or true
	
	local flist={}
	local elist={}
	self.enemies=elist
	self.friends=flist


	local lord=self.room:getLord()
	local role=self.role
	self.retain=2
	self.harsh_retain=true

	local players=self.room:getOtherPlayers(self.player)
	players=sgs.QList2Table(players)

	for _,player in ipairs(players) do
		if #players==1 then break end
		if self:objectiveLevel(player)<0 then table.insert(flist,player) end
	end

	self.friends_noself={}

	for _, player in ipairs (flist) do
		table.insert(self.friends_noself,player)
	end
	table.insert(self.friends,self.player)

	if self.role=="rebel" then
		sgs.rebel_target=self.room:getLord()
		self.retain=2
	end

	if self.player:getHp()<2 then self.retain=0 end
	self:sortEnemies(players)
	for _,player in ipairs(players) do
		if self:objectiveLevel(player)>=4 then self.harsh_retain=false end
		if #elist==0 then
			table.insert(elist,player)
			if self:objectiveLevel(player)<4 then self.retain=0 end
		else
			if self:objectiveLevel(player)<=0 then return end
			table.insert(elist,player)
			self:updateLoyalTarget(player)
			self:updateRebelTarget(player)
			
			if self:objectiveLevel(player)>=4 then self.harsh_retain=false end
		end
	end
end

function SmartAI:updateLoyalTarget(player)
	if self.role=="rebel" then return end
	
    if (self:objectiveLevel(player)>=4) then
        if not sgs.loyal_target then sgs.loyal_target=player 
		elseif player:getHp() == 1 and sgs.rebel_target:getHp() >= 2 then sgs.loyal_target = player
        elseif (sgs.ai_chaofeng[player:getGeneralName()] or 0)<(sgs.ai_chaofeng[sgs.loyal_target:getGeneralName()] or 0) then sgs.loyal_target=player 
        elseif (sgs.loyal_target:getArmor()) and (not player:getArmor()) then sgs.loyal_target=player 
        elseif (sgs.loyal_target:getHp()>1) and (getDefense(player)<=3) then sgs.loyal_target=player 
		elseif sgs.rebel_target:getHp()-player:getHp()>=2 then sgs.loyal_target = player
        elseif (sgs.loyal_target:getHandcardNum()>0) and (player:getHandcardNum()==0) then sgs.loyal_target=player 
		elseif self:getCardsNum(".", sgs.loyal_target, "e")>self:getCardsNum(".", player, "e") then sgs.loyal_target=player 
        end
    end
end

function SmartAI:updateRebelTarget(player)
	if self.role == "lord" or self.role == "loyalist" then return end
	if not sgs.rebel_target then sgs.rebel_target=player end
	if self.room:getLord():objectName() == player:objectName() then sgs.rebel_target=player
	elseif self:objectiveLevel(player)>=4 and self:objectiveLevel(player)<5 then
		if player:getHp() == 1 and sgs.rebel_target:getHp() >= 2 then sgs.rebel_target=player
        elseif (sgs.ai_chaofeng[player:getGeneralName()] or 0)<(sgs.ai_chaofeng[sgs.rebel_target:getGeneralName()] or 0) then sgs.rebel_target=player 
		elseif (sgs.rebel_target:getArmor()) and (not player:getArmor() and sgs.rebel_target:getHp()>player:getHp()) then sgs.rebel_target=player 
		elseif sgs.rebel_target:getHp()-player:getHp()>=2 then sgs.rebel_target=player
		end
	end
end

function SmartAI:printFEList()
    for _, player in ipairs (self.enemies) do
        self.room:output("enemy "..player:getGeneralName()..(sgs.ai_explicit[player:objectName()] or ""))
    end

    for _, player in ipairs (self.friends_noself) do
        self.room:output("friend "..player:getGeneralName()..(sgs.ai_explicit[player:objectName()] or ""))
    end
	
    self.room:output(self.player:getGeneralName().." list end")
end

function SmartAI:objectiveLevel(player)
    if useDefaultStrategy() then 
        if self.player:getRole()=="renegade" then
		elseif player:getRole()=="renegade" then
        elseif self:isFriend(player) then return -2
        elseif player:isLord() then return 5
        elseif player:getRole()=="renegade" then return 4.1
        else return 4.5 end
    end
    
    if player:objectName()==self.player:objectName() then return -2 end

    local modifier=0
    local rene=sgs.ai_renegade_suspect[player:objectName()] or 0
    if rene>1 then modifier=0.5 end
    
    local players=self.room:getOtherPlayers(self.player)
    players=sgs.QList2Table(players)
	
	
    local hasRebel, hasLoyal, hasRenegade=false, false, false
    for _,oplayer in ipairs(players) do
        if oplayer:getRole()=="rebel" then hasRebel=true end
		if oplayer:getRole()=="loyalist" then hasLoyal=true end
        if oplayer:getRole()=="renegade" then hasRenegade=true end
		if hasRebel and hasLoyal and hasRenegade then break end
	end
	
	local rebel_num, loyalish_num, loyal_num = 0, 0, 0
    for _, aplayer in ipairs (players) do
        if aplayer:getRole()=="rebel" then 
            rebel_num = rebel_num + 1
		else
			loyalish_num = loyalish_num + 1
		end
    end
	if hasRenegade then loyal_num = loyalish_num-1 else loyal_num = loyalish_num end
	
    if self.role=="lord" then
    
        if #players == 1  then return 5 end 
		
        if not hasRebel then 
            local name=player:objectName()
			if player:getRole() == "renegade" then return 5 else return -2 end
        end
		
		if not hasLoyal then 
			if #players == 2 then 
				if player:getRole() == "rebel" then return 5 else return 3.1 end
			end
			if player:getRole() == "renegade" then return -1 else return 5 end
		end
		
		if hasRenegade then 
			if rebel_num+1 == loyal_num then 
				if player:getRole() == "renegade" then return 4
				elseif player:getRole() == "rebel" then return 5
				else return -2
				end
			end
		end
		
        if sgs.ai_explicit[player:objectName()]=="rebel" then return 5-modifier
        elseif sgs.ai_explicit[player:objectName()]=="rebelish" then return 5-modifier
        elseif sgs.ai_explicit[player:objectName()]=="loyalist" then return -2
        elseif sgs.ai_explicit[player:objectName()]=="loyalish" then return -1
        elseif (self:singleRole())=="rebel" then return 4.6-modifier
        elseif (self:singleRole())=="loyalist" then return -1
        elseif (sgs.ai_royalty[player:objectName()]<=0) and 
            (sgs.ai_card_intention["general"](player,100)>0) 
            then return 3
        else return 0 end
    elseif self.role=="loyalist" then
		
		if not hasRebel then
			if player:getRole() == "renegade" or player:getRole() == "rebel" then return 5 else return -2 end
		end
		
		if loyalish_num <= rebel_num then 
			if player:getRole() == "lord" then return -2
 			elseif player:getRole() == "renegade" then return -1 
			elseif player:getRole() == "rebel" then return 5
			end
		end
		
        if sgs.ai_explicit[player:objectName()]=="rebel" then return 5-modifier
        elseif sgs.ai_explicit[player:objectName()]=="rebelish" then return 5-modifier
        elseif player:isLord() then return -2
        elseif sgs.ai_explicit[player:objectName()]=="loyalist" then return -1
        elseif sgs.ai_explicit[player:objectName()]=="loyalish" then return -1
        elseif (self:singleRole())=="rebel" then return 4-modifier
        elseif (self:singleRole())=="loyalist" then return -1
        elseif (sgs.ai_royalty[player:objectName()]<=0) and 
            (sgs.ai_card_intention["general"](player,100)>0) 
            then return 3.1
        else return 0 end
    elseif self.role=="rebel" then
        if player:isLord() then return 5 end
		if not hasRebel then 
			if player:getRole() == "renegade" and #players>2 then return -1 
			elseif player:getRole() == "renegade" and #players == 2 then
				if (self.player:getHp()+player:getHp() <= (self.room:getLord()):getHp()) then 
					return -1
				else 
					return 3.1
				end
			end
		elseif not hasLoyal then
			if player:getRole() == "lord" then return 5 
			elseif player:getRole() == "renegade" then return 3.1
            elseif player:getRole() == "loyalist" then return 2 -- the player is dead
			else return -2
			end
        elseif sgs.ai_explicit[player:objectName()]=="loyalist" then return 5-modifier
        elseif sgs.ai_explicit[player:objectName()]=="loyalish" then return 5-modifier
        elseif sgs.ai_explicit[player:objectName()]=="rebel" then return -1
        elseif sgs.ai_explicit[player:objectName()]=="rebelish" then return -1
        elseif (self:singleRole())=="rebel" then return -1
        elseif (self:singleRole())=="loyalist" then return 4-modifier
        elseif (sgs.ai_royalty[player:objectName()]>0) and 
            (sgs.ai_card_intention["general"](player,100)<0) 
            then return 3
        else return 0 end
    elseif self.role=="renegade" then
        
        if #players==1 then return 5 end
        --if (#players==2) and player:isLord() then return 0 end
		
        if not hasRebel then 
			if player:isLord() then 
				if player:getHp() > 2 then return 3.1 else return -1 end
			else return 5 end
		end

		if rebel_num > loyalish_num then 
			if player:getRole() == "rebel" then return 5 else return -1 end
		elseif rebel_num < loyalish_num then
			if player:getRole() == "rebel" then return -1 
			elseif player:getRole() == "loyalist" and not player:isLord() then return 5
			elseif player:isLord() then 
				if player:getHp() > 2 then return 3.1 else return -1 end
			end
		else
			local loyalish_count, rebel_count = 0, 0
			local loyalish_hp, rebel_hp = 0, 0
			for _, aplayer in ipairs(players) do
				if aplayer:getRole() == "rebel" then 
					rebel_hp = rebel_hp + aplayer:getHp()
					rebel_count = rebel_count + 1
				else
					loyalish_hp = loyalish_hp + aplayer:getHp()
					loyalish_count = loyalish_count + 1
				end
			end
			if (loyalish_hp-loyalish_count) <= (rebel_hp-rebel_count) then
				if player:getRole() == "rebel" then return 5
				else return -1 end
			else
				if player:getRole() == "rebel" then return 3.1
				else 
					if player:isLord() and loyalish_count == 1 then
						if loyalish_hp > rebel_hp then return 5 else return -2 end
					elseif loyalish_count > 1 then
						if player:isLord() then 
							if player:getHp() < 3 then return -1 else return 4 end
						else return 5 
						end
					end
				end
			end
		end
		
    end
    return 1
end

function SmartAI:sortEnemies(players)
    local comp_func=function(a,b)
        local alevel=self:objectiveLevel(a)
        local blevel=self:objectiveLevel(b)

        if alevel~=blevel then return alevel>blevel end
        if alevel==3 then return getDefense(a)>getDefense(b) end

        alevel=sgs.ai_chaofeng[a:getGeneralName()] or 0
        blevel=sgs.ai_chaofeng[b:getGeneralName()] or 0
        if alevel~=blevel then
            return alevel>blevel
        end

        alevel=getDefense(a)
        blevel=getDefense(b)
        if alevel~=blevel then
            return alevel<blevel
        end
    end
    table.sort(players,comp_func)
end

function SmartAI:hasWizard(players,onlyharm)
	local skill
	if onlyharm then skill = sgs.wizard_harm_skill else skill = sgs.wizard_skill end
	for _, player in ipairs(players) do
		if self:hasSkills(skill, player) then
			return true
		end
	end
end

function SmartAI:sort(players, key)
	local func= sgs.ai_compare_funcs[key or "chaofeng"]
	table.sort(players, func)
end

function SmartAI:filterEvent(event, player, data)
	if event == sgs.CardUsed then
		self:updatePlayers()
	elseif event == sgs.CardEffect then
		self:updatePlayers()
	elseif event == sgs.Death then
		self:updatePlayers()

		if self==sgs.recorder then
			speakTrigger(nil,player,nil,"death")
			local selfexp=sgs.ai_explicit[player:objectName()]
			if selfexp then
				if selfexp=="loyalish" then selfexp="loyalist"
				elseif selfexp=="rebelish" then selfexp="rebel"
				end
				sgs.ai_explicit[player:objectName()]=nil
				sgs.ai_assumed[selfexp]=sgs.ai_assumed[selfexp]+1
			end
			sgs.ai_assumed[player:getRole()]=sgs.ai_assumed[player:getRole()]-1 
		end
	end
	
	if (event == sgs.PhaseChange) or (event == sgs.GameStart) then
		self:updatePlayers()
	end

	if not sgs.recorder then
		sgs.recorder=self
	end

	if self~=sgs.recorder then return end

	if event == sgs.TurnStart then
		self:updateRoyalty(self.room:getCurrent())
	end

	if event == sgs.CardEffect then
		local struct= data:toCardEffect()
		local card  = struct.card
		local to    = struct.to
		local from  = struct.from
		local source= self.room:getCurrent()

		if sgs.ai_card_intention[card:className()] then
			local intention=sgs.ai_card_intention[card:className()](card,from,to,source)
			if to:isLord() and intention<0 then 
			sgs.ai_anti_lord[from:objectName()]=(sgs.ai_anti_lord[from:objectName()] or 0)+1
			end
			self:refreshRoyalty(from,intention)
		end
		
	elseif event == sgs.CardUsed then
		local struct= data:toCardUse()
		local card  = struct.card
		local to    = struct.to
			  to    = sgs.QList2Table(to)
		local from  = struct.from
		local source= self.room:getCurrent()                   

		for _, eachTo in ipairs(to) do
			local use_intension = sgs.ai_carduse_intention[card:className()]
			if use_intension then
				local different = true
				if self:isFriend(from, eachTo) then different = false end
				local intention =  use_intension(card,from,eachTo,source,different) or use_intension(card,from,eachTo,source)
				self:refreshRoyalty(from,intention)
				
				if eachTo:isLord() and intention < 0 then 
				sgs.ai_anti_lord[from:objectName()] = (sgs.ai_anti_lord[from:objectName()] or 0)+1
				end
				
			end
			-- self.room:output(eachTo:objectName())
		end
	elseif event == sgs.CardLost then
		local move=data:toCardMove()
		local from=move.from
		local to=  move.to
		local place=move.from_place
		if sgs.ai_snat_disma_effect then 
			sgs.ai_snat_disma_effect=false
			local intention=sgs.ai_card_intention.general(from,70)
			if place==2 then intention=-intention end
			
			if from:isLord() and intention<0 then 
			sgs.ai_anti_lord[sgs.ai_snat_dism_from:objectName()]=(sgs.ai_anti_lord[sgs.ai_snat_dism_from:objectName()] or 0)+1
			end
			
			self:refreshRoyalty(sgs.ai_snat_dism_from,intention)
		end
	end
end

function SmartAI:getFriendsNoself(player)
	if self:isFriend(self.player, player) then
		return self.friends_noself
	elseif self:isEnemy(self.player, player) then
		friends = sgs.QList2Table(self.lua_ai:getEnemies())
		for i = #friends, 1, -1 do 
			if friends[i]:objectName() == player:objectName() then
			  table.remove(friends, i)
			end
		end
		return friends
	else 
		return {}
	end
end

function SmartAI:getFriends(player)
	if self:isFriend(self.player, player) then
		return self.friends
	elseif self:isEnemy(self.player, player) then
		return self.enemies
	else 
		return {player}
	end
end

function SmartAI:getEnemies(player)
	if self:isFriend(self.player, player) then
		return self.enemies
	elseif self:isEnemy(self.player, player) then
		return self.friends
	else 
		return {}
	end
end

function SmartAI:isFriend(other, another)
	if another then 
		if self.lua_ai:isFriend(other) and self.lua_ai:isFriend(another) then return true end
	end
    
    if sgs.showFriendDebug then
        self.room:writeToConsole("========is friend ?=======")
        if useDefaultStrategy() then self.room:writeToConsole("useDefaultStrategy") end
        if (self.player:objectName())==(other:objectName()) then self.room:writeToConsole("same objectName") end 
        if self:objectiveLevel(other)<0 then self.room:writeToConsole("objectiveLevel") end
        self.room:writeToConsole("=====================")
        sgs.showFriendDebug = false
    end
    
    if useDefaultStrategy() then return self.lua_ai:isFriend(other) end
    if (self.player:objectName())==(other:objectName()) then return true end 
	if self:objectiveLevel(other)<0 then return true end
    return false
end

function SmartAI:isEnemy(other, another)
	if another then 
		if self.lua_ai:isEnemy(other) and self.lua_ai:isEnemy(another) then return true end
	end
    if useDefaultStrategy() then return self.lua_ai:isEnemy(other) end
    if (self.player:objectName())==(other:objectName()) then return false end 
	if self:objectiveLevel(other)>0 then return true end
	return false
end

function SmartAI:isNeutrality(other)
	return self.lua_ai:relationTo(other) == sgs.AI_Neutrality
end

-- get the card with the maximal card point
function SmartAI:getMaxCard(player)
	player = player or self.player

	if player:isKongcheng() then
		return nil
	end

	local cards = player:getHandcards()
	local max_card, max_point = nil, 0
	for _, card in sgs.qlist(cards) do
		local point = card:getNumber()
		if point > max_point then
			max_point = point
			max_card = card
		end
	end

	return max_card
end

-- the table that stores whether the skill should be invoked
-- used for SmartAI:askForSkillInvoke
sgs.ai_skill_invoke = {
    liyao_sword = true;
    chaos_mirror = true;
    nvwa_stone = true;
    miracle = true;
    soul_spirit = true;
}

function SmartAI:askForSkillInvoke(skill_name, data)

    self.room:writeToConsole("askForSkillInvoke::"..skill_name)

	local invoke = sgs.ai_skill_invoke[skill_name]
	if type(invoke) == "boolean" then
		return invoke
	elseif type(invoke) == "function" then
		return invoke(self, data)
	else
		local skill = sgs.Sanguosha:getSkill(skill_name)
		return skill and skill:getFrequency() == sgs.Skill_Frequent
	end
end

function SmartAI:askForYiji(cards)
    self:sort(self.friends_noself,"handcard")
		
    if self.player:getHandcardNum()<=2 then
		return nil, -1
	end
	
	for _, card_id in ipairs(cards) do
		local card = sgs.Sanguosha:getCard(card_id)
		for _, friend in ipairs(self.friends_noself) do
			if friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage") then
				if card:inherits("Nullification") then
					return friend, card_id 
				end
			else
				if card:inherits("Jink") then 
					if friend:getHp() < 2 and self:getCardsNum("Jink", friend) < 1 then 
						return friend, card_id 
					end 
				end 
				if friend:hasSkill("jizhi") then 
					if card:getTypeId() == sgs.Card_Trick then 
						return friend, card_id 
					end 
				end
				if friend:hasSkill("paoxiao") or friend:hasSkill("tianyi") or self:getCardsNum("Slash", friend) < 1 then 
					if card:inherits("Slash") then 
						return friend, card_id 
					end 
				end
				if self:hasSkills(sgs.lose_equip_skill, friend) then 
					if card:inherits("EquipCard") then return friend, card_id end 
				end
				if friend:hasSkill("guose") then 
					if card:getSuit() == sgs.Card_Diamond then return friend, card_id end 
				end
				if friend:hasSkill("tianxiang") then 
					if card:getSuit() == sgs.Card_Spade or
						card:getSuit() == sgs.Card_Heart then
						return friend, card_id 
					end 
				end
				if friend:hasSkill("leiji") then
					if self:getCardsNum("Jink", friend) < 1 and card:inherits("Jink") then return friend, card_id
					elseif card:getSuit() == sgs.Card_Spade then return friend, card_id 
					end
				end
				if friend:hasSkill("xuanhuo") then 
					if (card:getSuit() == sgs.Card_Heart and 
					(card:inherits("Equipcard") or card:inherits("Jink") or card:inherits("FireSlash"))) then 
						return friend, card_id 
					end
				end
				
				if friend:hasSkill("qingguo") then 
					if card:isBlack() then return friend, card_id end 
				end
				
				if friend:hasSkill("jijiu") then 
					if card:isRed() then return friend, card_id end 
				end
				
				if friend:getHandcardNum() < friend:getHp() and not sgs.Sanguosha:getCard(card_id):inherits("Shit") then
					return friend, card_id 
				end
			end
		end
	end
	if #self.enemies >0 then
		for _, card_id in ipairs(cards) do
			local card=sgs.Sanguosha:getCard(card_id)
			if card:inherits("Shit") then
				for _,enemy in ipairs(self.enemies) do
					local v1 = 0
					if sgs[enemy:getGeneralName().."_suit_value"] then
						v1 = sgs[enemy:getGeneralName().."_suit_value"][card:getSuitString()] or 0
					end
					if v1<=0 then
						return enemy, card_id
					end
				end
			end
		end
	end
end

-- used for SmartAI:askForUseCard
sgs.ai_skill_use = {}

function SmartAI:askForUseCard(pattern, prompt, data)
	local use_func = sgs.ai_skill_use[pattern]
	if use_func then
        self.room:writeToConsole(self.player:getGeneralName().." useCard ["..pattern.."]")
		return use_func(self, prompt, data) or "."
	else
		return "."
	end
end

-- used for SmartAI:askForSuit
sgs.ai_suit_choice = {}

function SmartAI:askForSuit(reason)
self.room:writeToConsole("ai suit choice::"..reason)
	local use_func = sgs.ai_suit_choice[reason]
	if use_func then
		return use_func(self) or "."
	else
		return "."
	end
end

-- tiger-spear-slash
sgs.ai_skill_use["slash"] = function(self, prompt)
	if prompt ~= "@tiger-spear-slash" then return "." end
    local slash=self:getCard("Slash")
	if not slash then return "." end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy, true) and not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) then
			return ("%d->%s"):format(slash:getId(), enemy:objectName())
		end
	end
	return "."
end

function SmartAI:slashIsEffective(slash, to)
    if to:getMark("@invincible")>0 then return false end
    
    --diangguang skill on
    if to:getMark("@dianguang")>0 then
        return not slash:inherits("ThunderBang")
    end
    
    --xinyan skill on
    if to:hasSkill("xinyan") and not to:getWeapon() then
        return false
    end
    
    --chaoxi skill on and status is zhangchao
    if to:getMark("@zhangchao")>0 then
        return slash:inherits("IceBang")
    end
    
    --has jijia skill
    if to:hasSkill("jijia") then
        return not slash:inherits("Bang") and not slash:inherits("PoisonBang")
    end
    
    -- shidian skill
    if to:hasSkill("shidian") then
        return not slash:inherits("ThunderBang") and not self:isFriend(to)
    end

	local armor = to:getArmor()
	if armor and not self.player:hasWeapon("greensteel_sword") then
		if armor:objectName() == "king_shield" then
			return not slash:isBlack()
		elseif armor:objectName() == "vine_armor" then
			return slash:inherits("NatureSlash")
		elseif armor:objectName() == "holy_wing" then
			return slash:inherits("Bang")
		end
	end

	return true
end

function SmartAI:slashIsAvailable(player)
    player = player or self.player
    
    if player:hasFlag("no_bang_limit") then
        return true
    end

    local num = player:usedTimes("Bang")+ player:usedTimes("FireBang")+ player:usedTimes("ThunderBang")+ player:usedTimes("AirBang")+ player:usedTimes("PoisonBang")+ player:usedTimes("IceBang")
    
    if player:hasFlag("one_more_bang") then
        return num<2
    end
    
    return num < 1
end

local function prohibitUseDirectly(card, player)
    if player:hasSkill("shoudao") then 
        return card:inherits("Dodge") and card:isRed() 
    elseif player:hasSkill("kuangniu") then
        return card:inherits("TrickCard") and card:isRed()
    end
end

local function zeroCardView(class_name, player)
	-- if class_name == "Analeptic" then
		-- if player:hasSkill("jiushi") and self.player:faceUp() then
			-- return ("analeptic:jiushi[no_suit:0]=.")
		-- end
	-- end
end

local function isCompulsoryView(card, class_name, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
    
	if class_name == "Slash" and card_place ~= sgs.Player_Equip then
        if player:hasSkill("shoudao") and card:isRed() and card:inherits("Dodge") then 
            return ("bang:shoudao[%s:%s]=%d"):format(suit, number, card_id)
        end
        if player:hasSkill("kuangniu") and card:isRed() and card:inherits("TrickCard") then 
            return ("bang:kuangniu[%s:%s]=%d"):format(suit, number, card_id)
        end
	end	
end

local function getSkillViewCard(card, class_name, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	
    if class_name == "Slash" then
        if player:hasSkill("huoshen") and player:getHp()<3 and player:getMp()<4  then
            if card:isRed() and not card:inherits("HolyWater") then
                return ("fire_bang:huoshen[%s:%s]=%d"):format(suit, number, card_id)
            end
        end
    elseif class_name == "Dodge" then
        if player:hasSkill("wuxia") then
            if card:inherits("TrickCard") and not card:inherits("Cure") and not card:inherits("NothingIsSomething") then
                return ("dodge:wuxia[%s:%s]=%d"):format(suit, number, card_id)
            end
        end
    elseif class_name == "Unassailable" then
        if player:hasSkill("wuxia") then
            if card:inherits("BasicCard") and not card:inherits("HolyWater") then
                return ("unassailable:wuxia[%s:%s]=%d"):format(suit, number, card_id)
            end
        end
    end

end

function SmartAI:searchForAnaleptic(use,enemy,slash)   
    if not self.toUse then return nil end
	
	for _,card in ipairs(self.toUse) do
		if card:getId()~=slash:getId() then return nil end
	end

    if not use.to then return nil end
    if self.player:hasUsed("Schnapps") then return nil end
    
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)
    self:fillSkillCards(cards)
	
	if (getDefense(self.player)<getDefense(enemy)) and 
		(self.player:getHandcardNum()<1+self.player:getHp()) or
		self.player:hasFlag("drank") then
			return
    end

    if enemy:getArmor() then 
        if ((enemy:getArmor():objectName())=="chaos_mirror")
            or ((enemy:getArmor():objectName())=="blood_guard") then 
			if (self.player:getHandcardNum()<=1+self.player:getHp()) then
				return
			end
		end

	end
    
        
    local card_str = self:getCardId("Schnapps")
    if card_str then return sgs.Card_Parse(card_str) end
	
    for _, anal in ipairs(cards) do
        if (anal:className() == "Schnapps") and not (anal:getEffectiveId() == slash:getEffectiveId()) and 
			not isCompulsoryView(anal, "Slash", self.player, sgs.Player_Hand) then
            return anal
        end
    end
end

function SmartAI:slashProhibit(card,enemy)
	if card == nil then
		card = sgs.Sanguosha:cloneCard("bang", sgs.Card_NoSuit, 0)
	end

    if self:isFriend(enemy) then
        if enemy:isWounded() then return true end
        if card:inherits("FireBang") then
            if self:isEquip("VineArmor", enemy) then return true end
        end
        if enemy:isChained() and card:inherits("NatureSlash") then return true end
		if self:getCardsNum("Dodge",enemy) == 0 and enemy:getHp() < 2 and self:slashIsEffective(card,enemy) then return true end
        if self:isEquip("PeacockTear") and enemy:getHandcardNum()>2 then return true end
    else    
        if self:isEquip("PeacockTear") and enemy:getHandcardNum()<3 and enemy:getHp()>1 then return true end
    end
	
	return not self:slashIsEffective(card, enemy)
end


function SmartAI:useBasicCard(card, use,no_distance)
    if self.player:hasFlag("drank") then
        self.room:writeToConsole("----------- drank and slash ? is "..card:objectName())
    end

	if (self.player:getHandcardNum() == 1 
	and self.player:getHandcards():first():inherits("Slash") 
	and self.player:getWeapon() 
	and self.player:getWeapon():inherits("WushuangHalberd")) then
		self.slash_targets=3
	end
	
	if card:inherits("Slash") and (self:slashIsAvailable() or (self.player:hasFlag("huolong_on") and card:isRed())) then
		local target_count=0
		for _, friend in ipairs(self.friends_noself) do						
			local slash_prohibit=false
			slash_prohibit=self:slashProhibit(card,friend)
		end	
        
		self:sort(self.enemies, "defense")
		for _, enemy in ipairs(self.enemies) do
			local slash_prohibit=false
			slash_prohibit=self:slashProhibit(card,enemy)
			if not slash_prohibit then
				self.predictedRange = self.player:getAttackRange()
                
				if ((self.player:canSlash(enemy, not no_distance)) or 
				(use.isDummy and self.predictedRange and (self.player:distanceTo(enemy)<=self.predictedRange))) and 
				self:objectiveLevel(enemy)>3 and
				self:slashIsEffective(card, enemy) then
					-- fill the card use struct
					local anal=self:searchForAnaleptic(use,enemy,card)
					if anal and not self:isEquip("BloodGuard", enemy)  then 
                        self.room:writeToConsole("-----------check drank and exit")
                        self.room:writeToConsole(self.player:getGeneralName().." intend to attack "..enemy:getGeneralName())
                        self.room:writeToConsole("---------------------------")
						use.card = anal
						return 
					end
					use.card=card
					if use.to then use.to:append(enemy) end
					target_count=target_count+1
					if self.slash_targets<=target_count then return end
				end
			end
		end
			
	elseif card:inherits("HolyWater") and self.player:isWounded() then
		local peaches=0
		local cards = self.player:getHandcards()
		cards=sgs.QList2Table(cards)
		for _,card in ipairs(cards) do
			if card:inherits("HolyWater") then peaches=peaches+1 end
		end
			
		use.card = card
	end
end

function SmartAI:aoeIsEffective(card, to)
	-- the AOE starter is not effected by AOE
	if self.player == to then
		return false
	end

	return true
end

function SmartAI:getDistanceLimit(card)
	if card:inherits("Grab") or card:inherits("EnegyDrain") or card:inherits("Icebound") then
		return 1
	end
end

function SmartAI:exclude(players, card)
	local excluded = {}
	local limit = self:getDistanceLimit(card)
	for _, player in sgs.list(players) do
		if not self.room:isProhibited(self.player, player, card) then
			local should_insert = true
			if limit then
				should_insert = self.player:distanceTo(player) <= limit
			end

			if should_insert then
				table.insert(excluded, player)
			end
		end
	end

	return excluded
end

function SmartAI:useCardDestroyAll(dismantlement, use)
	if (not self.has_wizard) and self:hasWizard(self.enemies) then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, dismantlement)
		for _, player in ipairs(players) do
			if player:containsTrick("thunder") then
				use.card = dismantlement
                if use.to then use.to:append(player) end
				return			
			end
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, dismantlement)
	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(dismantlement, friend) then
			if (friend:containsTrick("soul_awe") or friend:containsTrick("enegy_drain") or friend:containsTrick("icebound")) then
				use.card = dismantlement
				if use.to then use.to:append(friend) end
				return
			end		
			if self:isEquip("BloodGuard", friend) and friend:isWounded() then
				hasLion = true
				target = friend
			end
		end
	end	
	
	if hasLion then 				
		use.card = dismantlement
		if use.to then use.to:append(target) end
		return
	end
	
	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if getDefense(enemy)<8 then break
		else self:sort(self.enemies,"threat")
		break
		end
	end	
	local enemies = self:exclude(self.enemies, dismantlement)
	for _, enemy in ipairs(enemies) do
		local equips = enemy:getEquips()
		
		--if equips or not (enemy:hasSkill("kongcheng") or enemy:hasSkill("lianying")) then			--not all conditions
		
		    if  not enemy:isNude() and (self:hasTrickEffective(dismantlement, enemy) or enemy:getEquips():isEmpty()) then                   
				use.card = dismantlement
                if use.to then 
                    use.to:append(enemy) 
                    self:speak("hostile", self.player:getGeneral():isFemale())
                end
				return 
		    end
	--	end
	end
end

function SmartAI:useCardGrab(snatch, use)
    if (not self.has_wizard) and self:hasWizard(self.enemies)  then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, snatch)
		for _, player in ipairs(players) do
			if player:containsTrick("thunder") then
				use.card = snatch
                    if use.to then use.to:append(player) end
				
				return			
			end			
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, snatch)
	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(snatch, friend) then
			if (friend:containsTrick("soul_awe") or friend:containsTrick("enegy_drain") or friend:containsTrick("icebound")) then
				use.card = snatch
				if use.to then use.to:append(friend) end
				return
			end		
			if self:isEquip("BloodGuard", friend) and friend:isWounded() then
				hasLion = true
				target = friend
			end
		end
	end	
	
	if hasLion then 				
		use.card = snatch
		if use.to then use.to:append(target) end
		return
	end
	
	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if getDefense(enemy)<8 then break
		else self:sort(self.enemies,"threat")
		break
		end
	end	
	local enemies = self:exclude(self.enemies, snatch)
	for _, enemy in ipairs(enemies) do		    
		if  not enemy:isNude() and (self:hasTrickEffective(snatch, enemy) or enemy:getEquips():isEmpty()) then
			use.card = snatch
			if use.to then 
				use.to:append(enemy) 
				self:speak("hostile", self.player:getGeneral():isFemale())
			end
			return
		end
	end
end

function SmartAI:useCardBurn(fire_attack, use)
	local lack = {
		spade = true,
		club = true,
		heart = true,
		diamond = true,
	}

	local targets_succ = {}
	local targets_fail = {}
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getEffectiveId() ~= fire_attack:getEffectiveId() then
			lack[card:getSuitString()] = false
		end
	end	

	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getMark("@invincible")<=0 and (self:objectiveLevel(enemy)>3) and not enemy:isKongcheng() and self:hasTrickEffective(fire_attack, enemy) then	
			
			local cards = enemy:getHandcards()
			local success = true
			for _, card in sgs.qlist(cards) do
				if lack[card:getSuitString()] then
					success = false
					break
				end
			end

			if success then
				if self:isEquip("VineArmor", enemy) then
					table.insert(targets_succ, 1, enemy) 
					break
				else
					table.insert(targets_succ, enemy)
				end
			else
				table.insert(targets_fail, enemy)
			end
		end
	end
	
	if #targets_succ > 0 then
		use.card = fire_attack
		if use.to then use.to:append(targets_succ[1]) end
	elseif #targets_fail > 0 and self:getOverflow(self.player) > 0 then
		use.card = fire_attack
		local r = math.random(1, #targets_fail)
		if use.to then use.to:append(targets_fail[r]) end
	end		
end

function SmartAI:useCardByClassName(card, use)
	local class_name = card:className()
	local use_func = self["useCard" .. class_name]
	
	if use_func then
		use_func(self, card, use)
	end
end

-- function SmartAI:getNormalSlashNumber(player)
	-- local n = 0
    -- local cards = player:getHandcards()
    -- for _, card in sgs.qlist(cards) do
        -- if card:objectName() == "Bang" then
            -- n = n + 1
        -- end
    -- end

    -- local left = cards:length() - n
    -- if player:hasWeapon("snake_spear") then
        -- n = n + math.floor(left/2)
    -- end

	-- return n
-- end

-- function SmartAI:getSlashNumber(player)
	-- player = player or self.player
	-- local n = 0
	-- local cards = player:getCards("h")
	-- for _, card in sgs.qlist(cards) do
		-- if self:canViewAs(card, "Slash", player) then		
			-- n = n + 1
		-- end
	-- end
	-- if player:hasWeapon("snake_spear") then
		-- n = n + math.floor((cards:length() - n)/2)
	-- end	
	-- cards = player:getCards("e")
	-- for _, card in sgs.qlist(cards) do
		-- if self:getSkillViewCard(card, "Slash", true, player) then		
			-- n = n + 1
		-- end
	-- end
	
	-- if only_self then return n end
	
	-- return n
-- end

-- function getJinkNumber(player, only_self, self)
	-- player = player or self.player
	-- if not player then return 0 end
	-- local n = 0
	-- local cards = player:getCards("h")
	-- for _, card in sgs.qlist(cards) do
		-- if self:canViewAs(card, "Jink", player) then		
			-- n = n + 1
		-- end
	-- end	

	-- local armor = player:getArmor()
	-- if armor and armor:objectName() == "chaos_mirror" then
		-- local judge_card = self.room:peek()
		-- if judge_card:isRed() then
			-- n = n + 1
		-- end
	-- end

  -- if only_self then return n end
  
  -- return n
-- end

-- function SmartAI:getJinkNumber(player, only_self)
	-- return getJinkNumber(player, only_self, self)
-- end

local function factorial(n)
	if n<=0.1 then return 1 end
	return n*factorial(n-1)
end

function SmartAI:useCardPK(duel, use)
	self:sort(self.enemies,"handcard")
	local enemies = self:exclude(self.enemies, duel)
	for _, enemy in ipairs(enemies) do
    
		if enemy:getMark("@invincible")<=0 and 
        not enemy:hasSkill("jijia") and 
        self:objectiveLevel(enemy)>3 and 
        not self:isEquip("VineArmor", enemy) then
        
			local n1 = self:getCardsNum("Slash")
			local n2 = enemy:getHandcardNum()
			local useduel
			if self:hasTrickEffective(duel, enemy) then
				if n1 >= n2 then
					useduel = true
				elseif n1 > 0 then
					local percard=0.35
					local poss = percard ^ n1 * (factorial(n1)/factorial(n2)/factorial(n1-n2))
					if math.random() > poss then useduel = true end
				end
				if useduel then						
					use.card = duel
					if use.to then 
						use.to:append(enemy) 
						self:speak("duel", self.player:getGeneral():isFemale())
					end
					return
				end
			end
		end
	end
end

local function handcard_subtract_hp(a, b)
	local diff1 = a:getHandcardNum() - a:getHp()
	local diff2 = b:getHandcardNum() - b:getHp()

	return diff1 < diff2
end

function SmartAI:useCardEnegyDrain(card, use)
	table.sort(self.enemies, handcard_subtract_hp)

	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if ((#enemies==1) or not enemy:hasSkill("deer")) and not enemy:containsTrick("enegy_drain") then
			use.card = card
             if use.to then use.to:append(enemy) end

			return
		end
	end
end

function SmartAI:useCardIcebound(card, use)
	table.sort(self.enemies, handcard_subtract_hp)

	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if ((#enemies==1) or not enemy:hasSkill("deer")) and not enemy:containsTrick("icebound") and not enemy:containsTrick("soul_awe") then
			use.card = card
             if use.to then use.to:append(enemy) end

			return
		end
	end
end

local function hp_subtract_handcard(a,b)
	local diff1 = a:getHp() - a:getHandcardNum()
	local diff2 = b:getHp() - b:getHandcardNum()

	return diff1 < diff2
end

function SmartAI:useCardSoulAwe(card, use)
	table.sort(self.enemies, hp_subtract_handcard)

	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if not enemy:containsTrick("soul_awe") and not enemy:containsTrick("icebound") and enemy:getMark("@flood_dragon")<=0 then			
			use.card = card
            if use.to then use.to:append(enemy) end

			return
		end
	end
end

function SmartAI:useCardBorrowWeapon(card, use)
	self:sort(self.enemies,"threat")
	
	for _, friend in ipairs(self.friends_noself) do												
		if friend:getWeapon() and self:hasSkills(sgs.lose_equip_skill, friend) then
			
			for _, enemy in ipairs(self.enemies) do
				if friend:canSlash(enemy) then
					use.card = card
				end
				if use.to then use.to:append(friend) end
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end	

	local n = nil 
	local final_enemy = nil
	for _, enemy in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, enemy, card) 
			and self:hasTrickEffective(card, enemy)
			and not self:hasSkill(sgs.lose_equip_skill, enemy)					
								
			and enemy:getWeapon() then
			
			for _, enemy2 in ipairs(self.enemies) do
				if enemy:canSlash(enemy2) then 					
					if enemy:getHandcardNum() == 0 then
						use.card = card
						if use.to then use.to:append(enemy) end
						if use.to then use.to:append(enemy2) end
						return
					else
						n = 1;
						final_enemy = enemy2
					end
				end
			end
			if n then use.card = card end
			if use.to then use.to:append(enemy) end
			if use.to then use.to:append(final_enemy) end
			return

		end
		n = nil
	end
end

function SmartAI:useCardSoulChain(card, use)
	local targets = {}
	self:sort(self.friends,"defense")
	for _, friend in ipairs(self.friends) do
		if friend:isChained() then
			table.insert(targets, friend)
		end
	end

	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() and not self.room:isProhibited(self.player, enemy, card)
			and self:hasTrickEffective(card, enemy) and not (self:objectiveLevel(enemy)<=3) then
			table.insert(targets, enemy)
		end
	end
    
    if targets[1] then
        use.card = card
        if use.to then use.to:append(targets[1]) end
    end
	if targets[2] then        
        if use.to then use.to:append(targets[2]) end
	end    
end

function SmartAI:useCardNothingIsSomething(card, use)
        use.card = card
        if not use.isDummy then 
                    self:speak("lucky")
        end
end

-- when self has wizard (zhangjiao, simayi, use it)
function SmartAI:useCardThunder(card, use)					
	if self.player:containsTrick("thunder") then return end
	
	if not self:hasWizard(self.enemies) then--and self.room:isProhibited(self.player, self.player, card) then
		if self:hasWizard(self.friends) then
			use.card = card
			return
		end
		local players=self.room:getAllPlayers()
		players=sgs.QList2Table(players)
		
		local friends=0
		local enemies=0
		
		for _,player in ipairs(players) do
			if self:objectiveLevel(player)>=4 then
				enemies=enemies+1
			elseif self:isFriend(player) then
				friends=friends+1
			end
		end
		
		local ratio
		
		if friends==0 then ratio=999
		else ratio=enemies/friends							
		end
		
		if ratio>1.5 then
			use.card = card
			return
		end
	end
end

function SmartAI:useCardWoodElf(card, use)
	local good, bad = 0, 0
	
	for _, friend in ipairs(self.friends) do 
		if friend:isWounded() then		
			good = good + 10/(friend:getHp())
			if friend:isLord() then good = good + 10/(friend:getHp()) end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:isWounded() then
			bad = bad + 10/(enemy:getHp())
			if enemy:isLord() then 
				bad = bad + 10/(enemy:getHp()) 
			end
		end
	end

	if good > bad then
		use.card = card
	end
end

function SmartAI:useCardPoisonMarish(card, use)
	if #self.friends <= #self.enemies then
		use.card = card
	end
end

function SmartAI:useCardReadyToGo(card, use)
	if #self.friends >= #self.enemies then
		use.card = card
	end
end

function SmartAI:useCardCure(card, use)
	local friends = self:exclude(self.friends_noself, card)

	for _, friend in ipairs(friends) do
		if self.player:distanceTo(friend)<=1 and friend:isWounded() 
        and friend:getLostHp()>=self.player:getLostHp() then
            use.card = card
            if use.to then use.to:append(friend) end
			return
		end
	end
    
    if self.player:isWounded() then
        use.card = card
        if use.to then use.to:append(self.player) end
    end
    
end


-- function SmartAI:getPeachNum()
	-- local index = 0
	-- local cards = self.player:getHandcards()
	-- for _, card in sgs.qlist(cards) do
		-- if card:inherits("Peach") then index = index + 1 end
	-- end
	
	-- return index
-- end

function SmartAI:getAllPeachNum(player)
	player = player or self.player
	local n = 0
	for _, friend in ipairs(self:getFriends(player)) do
		n = n + self:getCardsNum("HolyWater")
	end
	return n
end

-- function SmartAI:getAnalepticNum(player)
	-- player = player or self.player
	-- local n = 0
	-- local cards = player:getCards("h")
	-- for _, card in sgs.qlist(cards) do
		-- if self:canViewAs(card, "Schnapps", player) then		
			-- n = n + 1
		-- end
	-- end
	-- return n
-- end

function SmartAI:useCardRoar(card, use)
    local good, bad = 0, 0
    for _, friend in ipairs(self.friends_noself) do
        if not friend:getEquips():isEmpty() then
            bad = bad+1
        end
    end
    
    for _, enemy in ipairs(self.enemies) do
        if not enemy:getEquips():isEmpty() then
            good = good+1
        end
    end
    
    if good > bad then
        use.card = card
    end
end

function SmartAI:useTrickCard(card, use)
	if card:inherits("AOE") and not card:inherits("Roar") then
		local good, bad = 0, 0
		for _, friend in ipairs(self.friends_noself) do
			if self:aoeIsEffective(card, friend) then
			    bad = bad + 20/(friend:getHp())+10
                if friend:isLord() and (friend:getHp()<3) then
					return 
				end
					
                if (friend:getHp()<2) and (self.player:isLord()) then
					return
				end
			end
			good = good + self:getCardsNum("HolyWater", friend)
		end

		for _, enemy in ipairs(self.enemies) do
			if self:aoeIsEffective(card, enemy) then
			    good = good + 20/(enemy:getHp())+10
				
				if enemy:isLord() then
					good = good + 20/(enemy:getHp())
				end
			end
			bad = bad + self:getCardsNum("HolyWater", enemy)
		end
		
		if good > bad then
			use.card = card
		end
	else
		self:useCardByClassName(card, use)
	end
end


sgs.weapon_range =
{	
	LiyaoSword = 2,
	GreenSteelSword = 2,
	SoulCane = 3,
	SnakeSpear =3,
	PanguAxe =3,
	WushuangHalberd =4,
	DeicideBow =5,
	GhostFan =2,
	GudingMachete =2,
	WhiteDragon =1,
	RedDragon =1,
	EvilDirk =1,
	TigerSpear =3,
	PeacockTear =5,
}

function SmartAI:evaluateEquip(card)
	local deltaSelfThreat = 0
	local currentRange 
			if not card then return -1
			else
			currentRange = sgs.weapon_range[card:className()] or 0
	end
	for _,enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy) <= currentRange then
				deltaSelfThreat=deltaSelfThreat+6/getDefense(enemy)
		end
	end
	
	if card:inherits("SoulCane") then 
		deltaSelfThreat = deltaSelfThreat + self:getCardsNum("Slash") - 1
    elseif  card:inherits("WhiteDragon") and self:getCardsNum("Bang")>0 then
        deltaSelfThreat=deltaSelfThreat+1
        if self.player:isWounded() then deltaSelfThreat=deltaSelfThreat+2 end
	else
		for _,enemy in ipairs(self.enemies) do
			if self.player:distanceTo(enemy) <= currentRange then
				if card:inherits("LiyaoSword") and 
					enemy:getGeneral():isMale() ~= self.player:getGeneral():isMale() then
						deltaSelfThreat=deltaSelfThreat+3
				elseif card:inherits("GreenSteelSword") and enemy:getArmor() then
					deltaSelfThreat=deltaSelfThreat+3
				elseif card:inherits("PanguAxe") and enemy:getHp()<3 then
					deltaSelfThreat=deltaSelfThreat+3-enemy:getHp()
				elseif card:inherits("DeicideBow") and (enemy:getDefensiveHorse() or enemy:getDefensiveHorse())then
					deltaSelfThreat=deltaSelfThreat+3
					break
				elseif card:inherits("GudingMachete") and enemy:getHandcardNum()<3 then
					deltaSelfThreat=deltaSelfThreat+2
					if enemy:getHandcardNum()<1 then deltaSelfThreat=deltaSelfThreat+2 end
				elseif card:inherits("RedDragon") and self:getCardsNum("Bang", enemy)>0 then
					deltaSelfThreat=deltaSelfThreat+2
				elseif card:inherits("PeacockTear")  and enemy:getHandcardNum()<3 then
					deltaSelfThreat=deltaSelfThreat - 1
				end
			end
		end
		for _, friend in ipairs(self.friends_noself) do
            if friend:getHandcardNum()<3 then deltaSelfThreat=deltaSelfThreat+2 end
		end
	end
	return deltaSelfThreat
end

function SmartAI:useEquipCard(card, use)
   if card:inherits("Weapon") then
        if self.player:getMark("@weapon_forbid")>0 then return end
        if self.player:getWeapon() and self.player:hasSkill("bingren_on") then return end
        
		if self:evaluateEquip(card) > (self:evaluateEquip(self.player:getWeapon())) then
            if (not use.to) and self.weaponUsed and (not self:hasSkills(sgs.lose_equip_skill)) then return end
            if self.player:getHandcardNum() <= self.player:getHp() then return end
            use.card = card	
		end
	elseif card:inherits("Armor") then
	    if self.player:hasSkill("deer") or self.player:getMark("@armor_forbid")>0 then return end
        if card:inherits("HolyWing") and self.player:isWounded() then return end
        
        if card:objectName()=="vine_armor" and self.player:hasSkill("jijia") then return end
        
	 	if not self.player:getArmor() then use.card=card
	 	elseif (self.player:getArmor():objectName())=="blood_guard" then use.card=card
	 	elseif self.player:isChained()  and (self.player:getArmor():inherits("vine_armor")) and not (card:objectName()=="blood_guard") then use.card=card
	 	end
	elseif self.lua_ai:useCard(card) then
		use.card = card
	end
end

function SmartAI:getTurnUse()				
    local cards = self.player:getHandcards()
    cards=sgs.QList2Table(cards)  
    
    local turnUse={}
    local slashAvail=1
    self.predictedRange=self.player:getAttackRange()
    self.predictNewHorse=false
    self.retain_thresh=5
    self.slash_targets=1
    self.slash_distance_limit=false
    
    self.weaponUsed=false
    
    if self.player:isLord() then self.retain_thresh=6 end
 
    
    self:fillSkillCards(cards)
    
    self:sortByUseValue(cards)   
            
    local i=0
    for _,card in ipairs(cards) do
        local dummy_use={}
        dummy_use.isDummy=true
        
        if not self:hasSkills(sgs.need_kongcheng) and not (card:inherits("Slash") and self.player:hasFlag("drank")) then
            if (i >= (self.player:getHandcardNum()-self.player:getHp()+self.retain)) and (self:getUseValue(card)<self.retain_thresh) then
                return turnUse
            end
        
            if (i >= (self.player:getHandcardNum()-self.player:getHp())) and (self:getUseValue(card)<8.5) and self.harsh_retain then
                return turnUse
            end
        end
        
        local type = card:getTypeId()
        if type == sgs.Card_Basic then
            self:useBasicCard(card, dummy_use, self.slash_distance_limit)
        elseif type == sgs.Card_Trick then
            self:useTrickCard(card, dummy_use)
        elseif type == sgs.Card_Equip then
            self:useEquipCard(card, dummy_use)
        elseif type == sgs.Card_Skill then
            self:useSkillCard(card, dummy_use)
        end

        if dummy_use.card then
            if (card:inherits("Slash")) then 
            
                if self.player:hasFlag("drank") then
                    self.room:writeToConsole("slashAvail after drinking: "..slashAvail)
                end
                
                if slashAvail>0 then
                    slashAvail=slashAvail-1
                    table.insert(turnUse,card)                    
                end
            else
                if card:inherits("Weapon") then 
                    self.predictedRange=sgs.weapon_range[card:className()] 
                    self.weaponUsed=true
                end
                if card:inherits("OffensiveHorse") then self.predictNewHorse=true end
                if card:inherits("Grab") then i=i-1 end
                if card:inherits("Peach") then i=i+2 end
                if card:inherits("BorrowWeapon") then i=i-1 end
                if card:inherits("ReadyToGo") then i=i-1 end
                if card:inherits("NothingIsSomething") then i=i-2 end
                table.insert(turnUse,card)
            end
            i=i+1
        end
    end

    return turnUse
end

function SmartAI:activate(use)

	self:updatePlayers()
	self:assignKeep(self.player:getHp(),true)
	self.toUse =self:getTurnUse()
    self:sortByDynamicUsePriority(self.toUse)
    
    self.room:writeToConsole("======"..self.player:getGeneralName().."=====")
    self.room:writeToConsole("======toKeep=====")
	self:printCards(self.kept, true)
    self.room:writeToConsole("=======toUse=====")
	self:printCards(self.toUse, true)
    self.room:writeToConsole("===============================")
    
	for _, card in ipairs(self.toUse) do
        
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

		if use:isValid() then
			self.toUse=nil
			return
		end
	end
	
	self.toUse=nil        
end

function SmartAI:hasEquip(card)
	return self.player:hasEquip(card)
end
	
function SmartAI:getKeepValue(card,kept)
    if not kept then return self.keepValue[card:getId()] or 0 end
    
	local class_name = card:className()		
	local suit_string = card:getSuitString()
    local value	
    if sgs[self.player:getGeneralName().."_keep_value"] then							
        value = sgs[self.player:getGeneralName().."_keep_value"][class_name]
	end
	if (not value) and sgs[self.player:getGeneralName().."_suit_value"] then
		value = sgs[self.player:getGeneralName().."_suit_value"][suit_string]
	end
	if not value then 
		value = sgs.ai_keep_value[class_name] or 0
		for _,acard in ipairs(kept) do
			if acard:className() == card:className() then value = value-1.2
			elseif acard:inherits("Slash") and card:inherits("Slash") then value = value-1 
			end
		end
	end
    return value
end

function SmartAI:getUseValue(card)
	local class_name = card:className()				
	local v=0
	
	-- if card:inherits("GuhuoCard") then
		-- local userstring = card:toString()
		-- userstring = (userstring:split(":"))[2]
		-- local guhuocard = sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
		-- local usevalue = self:getUseValue(guhuocard,player) + #self.enemies*0.3
		-- if sgs.Sanguosha:getCard(card:getSubcards():first()):objectName() == userstring and card:getSuit() == sgs.Card_Heart then usevalue = usevalue + 3 end
		-- return usevalue
	-- end
	
	if card:getTypeId() == sgs.Card_Equip then 
        if self:hasEquip(card) then
			if card:inherits("OffensiveHorse") and self.player:getAttackRange()>2 then return 5.5 end
			if card:inherits("DefensiveHorse") and self:isEquip("ChaosMirror") then return 5.5 end
			return 9
		end
		if not self:hasSameEquip(card) then v = 6.7 end
		if self.weaponUsed and card:inherits("Weapon") then v=2 end
		if self.player:hasSkill("anqi") and card:inherits("Weapon") then v = 2 end
		-- if self.player:hasSkill("kurou") and card:inherits("Crossbow") then return 9 end
		if self:hasSkill("deer") and card:inherits("Armor") then v=2 end
		if self:hasSkills(sgs.lose_equip_skill) then return 10 end
	elseif card:getTypeId() == sgs.Card_Basic then
		if card:inherits("Slash") then
			-- if (self.player:hasFlag("drank") or self.player:hasFlag("tianyi_success") or self.player:hasFlag("luoyi")) then v = 8.7 end
			-- if self:isEquip("CrossBow") then v = v + 4 end
			v=v+self:getCardsNum("Slash") 
		elseif card:inherits("Dodge") then
			if self:getCardsNum("Dodge")>1 then v=v-6 end
		end
	elseif card:getTypeId() == sgs.Card_Trick then
		if self.player:getWeapon() and not self:hasSkills(sgs.lose_equip_skill) and card:inherits("BorrowWeapon") then v=2 end
		-- if self.player:getMark("shuangxiong") and card:inherits("Duel") then v=8 end
		-- if self.player:hasSkill("jizhi") then v = 8.7 end
		if not self:hasTrickEffective(card) then v=0 end
	end
	
	if self:hasSkills(sgs.need_kongcheng) then
		if self.player:getHandcardNum()==1 then v = 10 end
	end
	if self:hasSkill({name="wushuang_halberd"}) and card:inherits("Slash") and self.player:getHandcardNum()==1 then v=10 end
	if card:getTypeId()==sgs.Card_Skill then 
		if v==0 then v=10 end
	end
	
	if v == 0 then v = sgs.ai_use_value[class_name] or 0 end
	return v
end

function SmartAI:getUsePriority(card)
	local class_name = card:className()
	local v=0
	if card:inherits("EquipCard") then
        if self:hasSkill(sgs.lose_equip_skill) then return 10 end
        if card:inherits("Armor") and not self.player:getArmor() then v = 6
        elseif card:inherits("Weapon") and not self.player:getWeapon() then v = 5.7
        elseif card:inherits("DefensiveHorse") and not self.player:getDefensiveHorse() then v = 5.8
        elseif card:inherits("OffensiveHorse") and not self.player:getOffensiveHorse() then v = 5.5
        elseif card:inherits("Relic") and not self.player:getRelic() then v = 7
        end
        return v
    end	
	
	-- if self.player:hasSkill("qingnang") then
		-- if card:inherits("DestroyAll") then v = 3.8
		-- elseif card:inherits("BorrowWeapon") then v = 3.9
		-- end
		-- if v then return v else return sgs.ai_use_priority[class_name] end
	-- end	
	
    v = sgs.ai_use_priority[class_name]
    if type(v) == "function" then
		v = v(self)
	else
        v = v or 0
	end
    
	if card:inherits("Slash") and (card:getSuit()==sgs.Card_NoSuit) then v=v-0.1 end
    
	return v
end

function SmartAI:getDynamicUsePriority(card)
	if not card then return 0 end
	
	local type = card:getTypeId()
	local dummy_use = {}
	dummy_use.isDummy = true
	if type == sgs.Card_Trick then
		self:useTrickCard(card, dummy_use)
	elseif type == sgs.Card_Basic then
		self:useBasicCard(card, dummy_use)
	elseif type == sgs.Card_Equip then
		self:useEquipCard(card, dummy_use)
	else
		self:useSkillCard(card, dummy_use)
	end
	
	local good_null, bad_null = 0, 0
	for _, friend in ipairs(self.friends) do
		good_null = good_null + self:getCardsNum("Unassailable", friend)
	end
	for _, enemy in ipairs(self.enemies) do
		bad_null = bad_null + self:getCardsNum("Unassailable", enemy)
	end
	
	local value = self:getUsePriority(card)
    
    -- self.room:writeToConsole("priority1 ---------"..card:objectName().."=="..value)
    
	if dummy_use.card then
		local use_card = dummy_use.card
		local class_name = use_card:className()
		local dynamic_value
		
		if use_card:getTypeId() == sgs.Card_Equips then
			if self:hasSkills(sgs.lose_equip_skill) then value = value + 8 end
		end
		
		if sgs.dynamic_value.benefit[class_name] then 
			dynamic_value = 10
			if use_card:inherits("ReadyToGo") then
				for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
					dynamic_value = dynamic_value - 1
					if self:isEnemy(player) then dynamic_value = dynamic_value - ((player:getHandcardNum()+player:getHp())/player:getHp())*dynamic_value
					else dynamic_value = dynamic_value + ((player:getHandcardNum()+player:getHp())/player:getHp())*dynamic_value
					end
				end
			elseif use_card:inherits("HolyWater") or use_card:inherits("Cure") then 
                
				dynamic_value = 7.85
			-- elseif use_card:inherits("QingnangCard") and self:getCardsNum("Snatch") > 0 and good_null >= bad_null then
				-- dynamic_value = 6.55
			-- elseif use_card:inherits("RendeCard") and self.player:usedTimes("RendeCard") < 2 then
				-- if not self.player:isWounded() then dynamic_value = 6.57 
				-- elseif self:isWeak() then dynamic_value = 7.9
				-- else dynamic_value = 7.5
				-- end
			-- elseif use_card:inherits("JieyinCard") and self:getCardsNum("Peach") >= self.player:getLostHp() then
			    -- dynamic_value = 7.51
			-- elseif use_card:inherits("JujianCard") then
				-- if not self.player:isWounded() then dynamic_value = 0
				-- else dynamic_value = 7.5
				-- end
			end
			value = value + dynamic_value
		elseif sgs.dynamic_value.damage_card[class_name] then
			local others
			if dummy_use.to then others = dummy_use.to else others = self.room:getOtherPlayers(self.player) end
			dummy_use.probably_hit = {}
			
			for _, enemy in sgs.qlist(others) do
				if self:isEnemy(enemy) and (enemy:getHp() <= 2 or enemy:isKongcheng()) 
					and self:getCardsNum("Schnapps", enemy) == 0 and self:getCardsNum("HolyWater", enemy) == 0 then
					table.insert(dummy_use.probably_hit, enemy)
					break
				end
			end
				
			if #dummy_use.probably_hit > 0 then
				self:sort(dummy_use.probably_hit, "defense")
				local probably_hit
				for _, hit in ipairs(dummy_use.probably_hit) do
					if not self:hasSkills(sgs.masochism_skill, hit) then
						probably_hit = hit 
						break
					end
				end
				if not probably_hit then
					probably_hit = dummy_use.probably_hit[1]
					value = value + 1.5
				else
					value = value + 3
				end
				value = value - (probably_hit:getHp() - 1)/2.0
				
				if use_card:inherits("Slash") and self:getCardsNum("Dodge", probably_hit) == 0 then
					value = value + 5
				elseif use_card:inherits("Burn") then 
					value = value + 0.5 + self.player:getHandcardNum()
				elseif use_card:inherits("PK") then
					value = value + 2 + (self.player:getHandcardNum() - self:getCardsNum("Slash", probably_hit))
				end
			end
		elseif sgs.dynamic_value.control_card[class_name] then
			if use_card:getTypeId() == sgs.Card_Trick then dynamic_value = 7 - bad_null/good_null else dynamic_value = 6.65 end
			value = value + dynamic_value
		elseif sgs.dynamic_value.control_usecard[class_name] then
			value = value + 6.6
		elseif sgs.dynamic_value.lucky_chance[class_name] then
			value = value + (#self.enemies - #self.friends)
		end
        -- self.room:writeToConsole("priority2 ---------"..use_card:objectName().."=="..value)
	end
    
	return value
end

function SmartAI:sortByKeepValue(cards,inverse,kept)
	local compare_func = function(a,b)
		local value1 = self:getKeepValue(a,kept)
		local value2 = self:getKeepValue(b,kept)

		if value1 ~= value2 then
		    if inverse then return value1>value2 end
			return value1 < value2
		else
			return a:getNumber() < b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByUseValue(cards,inverse)
	local compare_func = function(a,b)
			local value1 = self:getUseValue(a)
			local value2 = self:getUseValue(b)

			if value1 ~= value2 then
					if not inverse then return value1 > value2
					else return value1 < value2
					end
			else
					return a:getNumber() > b:getNumber()
			end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByUsePriority(cards)
	local compare_func = function(a,b)
    local value1 = self:getUsePriority(a)
    local value2 = self:getUsePriority(b)

		if value1 ~= value2 then
			return value1 > value2
		else
			return a:getNumber() > b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:sortByDynamicUsePriority(cards)
	local compare_func = function(a,b)
        local value1 = self:getDynamicUsePriority(a)
        local value2 = self:getDynamicUsePriority(b)

		if value1 ~= value2 then
			return value1 > value2
		else
            return a and a:getTypeId() ~= sgs.Card_Skill and not (b and b:getTypeId() ~= sgs.Card_Skill)
		end
	end
      
	table.sort(cards, compare_func)
end

function SmartAI:sortByCardNeed(cards)
	local compare_func = function(a,b)
    local value1 = self:cardNeed(a)
    local value2 = self:cardNeed(b)

		if value1 ~= value2 then
			return value1 < value2
		else
			return a:getNumber() > b:getNumber()
		end
	end

	table.sort(cards, compare_func)
end

function SmartAI:askForDiscard(reason, discard_num, optional, include_equip)    

	local cards = self.player:getCards("h")
	if not cards then return {} end
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	
	local to_discard = {}
    
	local weapon = self.player:getWeapon()
	local armor = self.player:getArmor()
	local offensive_horse = self.player:getOffensiveHorse()
	local defensive_horse = self.player:getDefensiveHorse()
	local relic = self.player:getRelic()
    
	for _, card in ipairs(cards) do
		if #to_discard>=discard_num then break end
		if not self.player:isJilei(card) then
			table.insert(to_discard, card:getEffectiveId())
		end
	end
	
    if include_equip and
		not (not self.player:getCards("e"):isEmpty() and self.player:isJilei(self.player:getCards("e"):first())) then
		if #to_discard < discard_num and armor then
            if armor:inherits("BloodGuard") and self.player:isWounded() then table.insert(to_discard, armor:getId()) armor = nil end
		end
		if #to_discard < discard_num and offensive_horse then table.insert(to_discard, offensive_horse:getId()) end
		if #to_discard < discard_num and weapon then table.insert(to_discard, weapon:getId()) end
		if #to_discard < discard_num and defensive_horse then table.insert(to_discard, defensive_horse:getId()) end
		if #to_discard < discard_num and armor then table.insert(to_discard, armor:getId()) end
		if #to_discard < discard_num and relic then table.insert(to_discard, relic:getId()) end
	end
	return to_discard	
end

--- Determine that the current judge is worthy retrial
-- @param judge The JudgeStruct that contains the judge information
-- @return True if it is needed to retrial
function SmartAI:needRetrial(judge)
	if self:isFriend(judge.who) then
		return not judge:isGood()
	elseif self:isEnemy(judge.who) then
		return judge:isGood()
	else
		return false
	end
end

--- Get the retrial cards with the lowest keep value
-- @param cards the table that contains all cards can use in retrial skill
-- @param judge the JudgeStruct that contains the judge information
-- @return the retrial card id or -1 if not found
function SmartAI:getRetrialCardId(cards, judge)
	local can_use = {}
	for _, card in ipairs(cards) do
		if self:isFriend(judge.who) and judge:isGood(card) then
			table.insert(can_use, card)
		elseif self:isEnemy(judge.who) and not judge:isGood(card) then
			table.insert(can_use, card)
		end
	end
	
	if next(can_use) then
		self:sortByKeepValue(can_use, true)
		return can_use[1]:getEffectiveId()
	else
		return -1
	end
end

sgs.ai_skill_playerchosen = {}

function SmartAI:askForPlayerChosen(targets, reason)
	self:log("askForPlayerChosen:"..reason)
	local playerchosen = sgs.ai_skill_playerchosen[string.gsub(reason,"%-","_")]
	local target
	if type(playerchosen) == "function" then
		target=playerchosen(self,targets)
	end
	if target then
		return target
	else
		local r = math.random(0, targets:length() - 1)
		return targets:at(r)
	end
end

-- used for SmartAI:askForChoice
sgs.ai_skill_choice = {
    roar = function(self, choices)
        if not self:isEquip("HolyWing") and self.player:isWounded() or self.player:isChained() then 
            return "RoarEffect1"
        else
            return "RoarEffect2"
        end
    end,
    
    icebound = function(self, choices)
        if self:isEquip("HolyWing") or self.player:getHandcardNum()>2 and self.player:getHp()>2 then 
            return "IceboundEffect2"
        else
            return "IceboundEffect1"
        end
    end,
    
    shtm = function(self, choices)
        local dead
        local all = self.room:getServerPlayers()
		for _, p in sgs.qlist(all) do
            sgs.showFriendDebug = true;
			if p:isDead() and self:isFriend(p) then
                return p:getGeneralName()
            end
		end
    end,
      
    air_bang = function(self, choices)
        if self.player:getHp()<3 then 
            return "AirEffectArmor"
        else
            return "AirEffectHp"
        end
    end,
}

function SmartAI:askForChoice(skill_name, choices)
    self.room:writeToConsole("-------------- askForChoice start");
    self.room:writeToConsole(skill_name);
    self.room:writeToConsole(choices);
    self.room:writeToConsole("-----------------------------------");
	local choice = sgs.ai_skill_choice[skill_name]
	if type(choice) == "string" then
		return choice
	elseif type(choice) == "function" then
		return choice(self, choices)
	else
		local skill = sgs.Sanguosha:getSkill(skill_name)		
		if skill then
			return skill:getDefaultChoice(self.player) 
		else
			local choice_table = choices:split("+");
			local r = math.random(1, #choice_table)
			return choice_table[r]
		end
	end
end


function SmartAI:getCardRandomly(who, flags)
	local cards = who:getCards(flags)
    if cards:isEmpty() then return -1 end
    
	local r = math.random(0, cards:length()-1)
	local card = cards:at(r)
	if self:isEquip("BloodGuard", who) then
		if self:isEnemy(who) and who:isWounded() and card == who:getArmor() then
			if r ~= (cards:length()-1) then
				card = cards:at(r+1)
			else
				card = cards:at(r-1)
			end
		end
	end

	return card:getEffectiveId()
end

sgs.ai_skill_cardchosen = {}
function SmartAI:askForCardChosen(who, flags, reason)
	-- self.room:output(reason)

	local cardchosen = sgs.ai_skill_cardchosen[string.gsub(reason,"%-","_")]
	local card
	if type(cardchosen) == "function" then
		card = cardchosen(self, who)
	end
	if card then
		return card:getId()
	end
	
    if self:isFriend(who) or who:hasSkill("anshi") then
		if flags:match("j") then
			local tricks = who:getCards("j")
			local lightning, indulgence, supply_shortage
			for _, trick in sgs.qlist(tricks) do
				if trick:inherits("Thunder") then
					lightning = trick:getId()
				elseif trick:inherits("SoulAwe") or trick:getSuit() == sgs.Card_Diamond then
					indulgence = trick:getId()
				elseif trick:inherits("EnegyDrain") or trick:getSuit() == sgs.Card_Club then
					supply_shortage = trick:getId()
				end
			end
			

			if self:hasWizard(self.enemies) and lightning then
				return lightning
			end

			if indulgence and supply_shortage then
				if who:getHp() < who:getHandcardNum() then
					return indulgence
				else
					return supply_shortage
				end
			end

			if indulgence or supply_shortage then
				return indulgence or supply_shortage
			end
		end
		
		if flags:match("e") then
			if who:isWounded() and who:getArmor() and self:isEquip("BloodGuard", who) then return who:getArmor():getId() end
			if self:hasSkills(sgs.lose_equip_skill, who) then
				local equips = who:getEquips()
				if not equips:isEmpty() then
					return equips:at(0):getId()
				end
			end
		end
	else 
        if (who:getHandcardNum()<2) and (not who:isKongcheng()) and
			not self:hasSkills(sgs.need_kongcheng, who) then return -1 
		end
		
		if flags:match("j") then
			local tricks = who:getCards("j")
            local lightning
			for _, trick in sgs.qlist(tricks) do
				if trick:inherits("Thunder") then
					lightning = trick:getId()
				end
			end
			if self:hasWizard(self.enemies,true) and lightning then
				return lightning
			end
		end
		
		if flags:match("e") then		    
			if who:getDefensiveHorse() then
				for _,friend in ipairs(self.friends) do
					if friend:distanceTo(who)==friend:getAttackRange()+1 then 
					 	return who:getDefensiveHorse():getId()
					end
				end
			end			
			
			if who:getArmor() then 
			    local canFire=false
			        
				-- if self.player:getWeapon() then 
					-- if self.player:getWeapon():inherits("Fan") then canFire=true end
				-- end
			    if self.toUse then
			        for _,card in ipairs(self.toUse) do 
			            if card:inherits("FireBang") then canFire=true end
			            if card:inherits("Burn") then canFire=true end
			        end
			    end
			    if canFire and (who:getArmor():objectName()=="vine_armor") then
				elseif (who:getArmor():objectName()=="silver_lion") and who:isWounded() then 
                else return who:getArmor():getId() 
                end
			end
			
			if who:getWeapon() then 
			    if not (who:hasSkill("xiaoji") and (who:getHandcardNum()>=who:getHp())) then
                    for _,friend in ipairs(self.friends) do
                        if (who:distanceTo(friend) <= who:getAttackRange()) and (who:distanceTo(friend)>1) then 
                            return who:getWeapon():getId()
                        end
                    end
				end
			end
		
			if who:getOffensiveHorse() then
			    if who:hasSkill("xiaoji") and who:getHandcardNum()>=who:getHp() then
			    else
				    for _,friend in ipairs(self.friends) do
					    if who:distanceTo(friend)==who:getAttackRange() and
					    who:getAttackRange()>1 then 
					 	    return who:getOffensiveHorse():getId() 
					    end
				    end
				end
			end
		end
		if flags:match("h") then
			if not who:isKongcheng() then
				local cards = who:getHandcards()
				cards = sgs.QList2Table(cards)
				self:sortByUseValue(cards)
				return cards[1]:getId()
			end
		end
        
        if not who:isKongcheng() then
			return -1
		end
	end
	local new_flag = ""
    if flags:match("h") then new_flag = "h" end
    if flags:match("e") then new_flag = new_flag.."e" end
    return self:getCardRandomly(who, new_flag) or who:getCards(flags):first():getEffectiveId()							
end

function SmartAI:findEffectiveSlash(to)
    
    local cards = self.player:getHandcards()
    cards = sgs.QList2Table(cards)
    
    for _,card in ipairs(cards) do
        if card:inherits("Slash") and self:slashIsEffective(card, to) then
            return card:getEffectiveId()
        end
    end
    
    return -1
end

function SmartAI:askForCard(pattern, prompt, data)
	-- self.room:output(prompt)
	
	if sgs.ai_skill_invoke[pattern] then return sgs.ai_skill_invoke[pattern](self, prompt) end
	local target, target2
	if not prompt then return end
	local parsedPrompt = prompt:split(":")
	if parsedPrompt[2] then
		local others = self.room:getOtherPlayers(self.player)
		others = sgs.QList2Table(others)
		for _, other in ipairs(others) do
			if other:getGeneralName() == parsedPrompt[2] or other:objectName() == parsedPrompt[2] then target = other break end
		end
		if parsedPrompt[3] then
			for _, other in ipairs(others) do
				if other:getGeneralName() == parsedPrompt[3] or other:objectName() == parsedPrompt[3] then target2 = other break end
			end
		end
	end
    
    --shtm
    if parsedPrompt[1] == "revive-rite-handcard" then 
        local dead = data:toPlayer()
        sgs.showFriendDebug = true;
        if self:isFriend(dead) then self.room:writeToConsole(self.player:getGeneralName().." is friend with "..dead:getGeneralName()) end
        if dead and self:isFriend(dead) and self.player:getHandcardNum()>2 then
            local hcards = self.player:getHandcards()
            hcards=sgs.QList2Table(hcards)
            self:sortByUseValue(hcards, true)
            return "$"..hcards[1]:getEffectiveId()
        else
            return "."
        end
        
    --huhuan
    elseif parsedPrompt[1] == "@huhuan-card" then 
        local who = data:toPlayer()
		if not self:isFriend(who) then return "." end
        
        if pattern == "jink" then
            return self:getCardId("Dodge") or "."
        else
            return self:getCardId("Slash") or "."
        end
    
    --jiqi
    elseif parsedPrompt[1] == "@jiqi-card" then
        local who = data:toPlayer()
        if not self:isFriend(who) then return "." end
        
		local holy_water, schnapps, slash, dodge
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if card:inherits("HolyWater") then holy_water = card
			elseif card:inherits("Schnapps") then schnapps = card
			elseif card:inherits("Slash") then slash = card
			elseif card:inherits("Dodge") then dodge = card
			end
		end
        
        if who:getHp()==1 and (holy_water or schnapps) then return "$"..(holy_water or schnapps):getEffectiveId() end
        if who:getHandcardNum()==0 and dodge then return "$"..dodge:getEffectiveId() end
        if slash then return "$"..slash:getEffectiveId() end
        
        return "."
        
    --anita_anwei
    elseif parsedPrompt[1] == "@card_to_anita" then
        if self.player:getMark("@anita")<=0 or self.player:getHandcardNum()<=0 then return "." end

        local card_str = nil
        local can_give = false
        local card_to_anita
            
        local cards = self.player:getHandcards()
        cards = sgs.QList2Table(cards)
        self:sortByUseValue(cards, true)
        
       for _,hcard in ipairs(cards) do
            if hcard:inherits("Dodge") or hcard:inherits("HolyWater") then
                if can_give then
                    card_to_anita = hcard
                    break
                else        
                    can_give = true
                end
            end
        end
        
        if not card_to_anita and self.player:getHandcardNum()>self.player:getHp() then
            card_to_anita = cards[1]
        end
        
        if card_to_anita then
            return "$"..card_to_anita:getEffectiveId()
        end
        
        return "."
    end
	
	-- aoe
	if parsedPrompt[1] == "alien-invasion-slash" or parsedPrompt[1] == "thousands-arrows-shot-jink" then
		local objectName
		if parsedPrompt[1] == "alien-invasion-slash" then 
			objectName = "alien_invasion" 
		else
			objectName = "thousands_arrows_shot" 
		end
        if (self.player:getArmor() and self.player:getArmor():inherits("VineArmor"))
        or self.player:getMark("@invincible")>0
        or self.player:getMark("@zhangchao")>0 
        or self.player:hasSkill("jijia") then 
            return "." 
        end
 	end
	
	if pattern == "slash" and self:getCardsNum("Slash") > 0 then
		if parsedPrompt[1] == "borrow-weapon-slash" then 
			if target and (not self:isFriend(target2) or target2:getHp() > 2 or self:getCardsNum("Dodge", target2) > 0) and not self:hasSkills(sgs.lose_equip_skill) then 
				return self:getCardId("Slash")
			end
			self:speak("collateral", self.player:getGeneral():isFemale())
			return "."
		elseif (parsedPrompt[1] == "duel-slash") then
			if (not self:isFriend(target) or (target:getHp() > 2 and self.player:getHp() <= 1 and self:getCardsNum("HolyWater") == 0 )) then 
				return self:getCardId("Slash")
			else return "." end
        --kongchan
        elseif (parsedPrompt[1] == "@kongchan-bang") then 
            local to = data:toPlayer()
            if self:isFriend(to) then return "." end
            
            local card_id = self:findEffectiveSlash(to)
            if card_id >= 0 then return card_id end
            return "."
        --cuimian
        elseif (parsedPrompt[1] == "@cuimian-slash") then 
            if self.player:getHandcardNum()>3 then return "." end
		end
        return self:getCardId("Slash") or "."
	elseif pattern == "jink" then
		if target then
			if not self:isFriend(target) then
				if not self:hasSkills(sgs.need_kongcheng, player) then
					if self:isEquip("PanguAxe", target) then
						if self:hasSkills(sgs.lose_equip_skill, target) and target:getEquips():length() > 1 then return "." end
						if target:getHandcardNum() - target:getHp() > 2 then return "." end
					elseif self:isEquip("SoulCane", target) then
						if self:getCardsNum("Dodge") <= self:getCardsNum("Slash", target) then return "." end
					end
				end
			end
		end
		return self:getCardId("Dodge") or "."
	elseif pattern == ".basic" then
		-- local effect = data:toCardEffect()
		-- if self:isFriend(effect.to) then return "." end
		-- local has_peach, has_anal, has_slash, slash_jink
		-- for _, card in sgs.qlist(self.player:getHandcards()) do
			-- if card:inherits("HolyWater") then has_peach = card
			-- elseif card:inherits("Schnapps") then has_anal = card
			-- elseif card:inherits("Slash") then has_slash = card
			-- elseif card:inherits("Dodge") then has_jink = card
			-- end
		-- end
		
		-- if has_slash then return "$" .. has_slash:getEffectiveId()
		-- elseif has_jink then return "$" .. has_jink:getEffectiveId()
		-- elseif has_anal or has_peach then 
			-- if self:getCardsNum("Dodge", effect.to) == 0 and self.player:hasFlag("drank") and self:getAllPeachNum(effect.to) == 0 then
				-- if has_anal then return "$" .. has_anal:getEffectiveId()
				-- else return "$" .. has_peach:getEffectiveId()
				-- end
			-- end
		-- else return "." 
		-- end
	end

	if parsedPrompt[1] == "liyao-sword-card" then 
		if target and self:isFriend(target) then return "." end					
		local cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:inherits("Slash") or card:inherits("BorrowWeapon") or card:inherits("WoodElf")
			  or card:inherits("Thunder") or card:inherits("EquipCard") or card:inherits("ReadyToGo") then
				return "$"..card:getEffectiveId()
			end
		end	
		return "."
	elseif parsedPrompt[1] == "@axe" then
		if target and self:isFriend(target) then return "." end
		
		local allcards = self.player:getCards("he")
		allcards = sgs.QList2Table(allcards)
		if self.player:hasFlag("drank") or #allcards-2>= self.player:getHp() then
			local cards = self.player:getCards("h")
			cards = sgs.QList2Table(cards)
			local index
			if self:hasSkills(sgs.need_kongcheng) then index = #cards end
			if self.player:getOffensiveHorse() then 
				if index then 
					if index < 2 then 
						index = index + 1 
						table.insert(cards, self.player:getOffensiveHorse()) 
					end 
				end
				table.insert(cards, self.player:getOffensiveHorse()) 
			end
			if self.player:getArmor() then 
				if index then 
					if index < 2 then 
						index = index + 1 
						table.insert(cards, self.player:getArmor()) 
					end 
				end
				table.insert(cards, self.player:getArmor()) 
			end
			if self.player:getDefensiveHorse() then 
				if index then 
					if index < 2 then 
						index = index + 1 
						table.insert(cards, self.player:getDefensiveHorse()) 
					end 
				end
				table.insert(cards, self.player:getDefensiveHorse()) 
			end
			if #cards>= 2 then
				self:sortByUseValue(cards, true)
				return "$"..cards[1]:getEffectiveId().."+"..cards[2]:getEffectiveId()
			end
		end
	end

end

sgs.ai_skill_askforag = {}

function SmartAI:askForAG(card_ids, refusable, reason)	
	local cardchosen = sgs.ai_skill_askforag[string.gsub(reason, "%-", "_")]
	if type(cardchosen) == "function" then
		local card_id = cardchosen(self, card_ids)
		if card_id then return card_id end
	end
	
	local ids = card_ids
	local cards = {}
	for _, id in ipairs(ids) do
		table.insert(cards, sgs.Sanguosha:getCard(id))
	end
	self:sortByCardNeed(cards, true)
	return cards[#cards]:getEffectiveId()
end

function SmartAI:askForNullification(trick_name, from, to, positive)   							
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local null_card
	null_card = self:getCardId("Unassailable") 
	if null_card then null_card = sgs.Card_Parse(null_card) else return end
      
	if positive then	
		if from and self:isEnemy(from) then
			if trick_name:inherits("NothingIsSomething") and self:getOverflow(from) == 0 then return null_card end
			if trick_name:inherits("SoulChain") and not self:isEquip("VineArmor", to) then return nil end
			if self:isFriend(to) then
				if trick_name:inherits("DestroyAll") then
					if to:getArmor() then return null_card end
				else
					if trick_name:inherits("Grab") then return null_card end
					if self:isWeak(to) then
						if trick_name:inherits("PK") then
							return null_card
						elseif trick_name:inherits("Burn") then
							if from:getHandcardNum() > 2 then return null_card end
						end
					end
				end
			elseif self:isEnemy(to) then
				if (trick_name:inherits("Snatch") or trick_name:inherits("Dismantlement")) and to:getCards("j"):length() > 0 then
					return null_card
				end
			end
		end
		
		if self:isFriend(to) then
			if trick_name:inherits("SoulAwe") or trick_name:inherits("EnegyDrain")  or trick_name:inherits("Icebound") then
				return null_card
			end	
			if self:isWeak(to) then 
				if trick_name:inherits("ThousandsArrowsShot") then
					if self:getCardsNum("Dodge", to) == 0 then return null_card end
				elseif trick_name:inherits("AlienInvasion") then
					if self:getCardsNum("Slash", to) == 0 then return null_card end
				end
			end
		end
		if from then
			if self:isEnemy(to) then
				if trick_name:inherits("WoodElf") and self:isWeak(to) then
					return null_card
				end
			end
		end		
	else
		if from then
			if from:objectName() == to:objectName() then
				if self:isFriend(from) then return null_card
				else return nil end
			end
			if not (trick_name:inherits("ReadyToGo") or trick_name:inherits("WoodElf") or trick_name:inherits("PoisonMarish") or trick_name:inherits("AOE")) then
				if self:isFriend(from) then return null_card end
			end
		else
			if self:isEnemy(to) then return null_card else return end
		end
	end
	
end


function SmartAI:askForSinglePeach(dying)										
	local card_str
	
	if self:isFriend(dying) then
		if (self.player:objectName() == dying:objectName()) then
			card_str = self:getCardId("Schnapps") or self:getCardId("HolyWater")
		else
			card_str = self:getCardId("HolyWater")
		end
		if card_str then return sgs.Card_Parse(card_str) end
	end
	
	return "."
end

function SmartAI:getOneFriend()
	for _, friend in ipairs(self.friends) do
		if friend ~= self.player then
			return friend
		end
	end
end

function SmartAI:getChainedFriends()
	local chainedFriends={}
	for _, friend in ipairs(self.friends) do
		if friend:isChained() then
			table.insert(chainedFriends,friend)
		end
	end
	return chainedFriends
end

function SmartAI:getChainedEnemies()
	local chainedEnemies={}
	for _, enemy in ipairs(self.enemies) do
		if enemy:isChained() then
			table.insert(chainedEnemies,enemy)
		end
	end
	return chainedEnemies
end

-- function SmartAI.newSubclass(theClass, name)
	-- local class_name = name:sub(1, 1):upper() .. name:sub(2) .. "AI"
	-- local new_class = class(class_name, theClass)

	-- function new_class:initialize(player)
		-- super.initialize(self, player)
	-- end

	-- sgs.ai_classes[name] = new_class

	-- return new_class
-- end

function SmartAI:setOnceSkill(name)
	function self:filterEvent(event, player, data)
		super.filterEvent(self, event, player, data)
		if not player then return end
		if event == sgs.PhaseChange and player:objectName() == self.player:objectName()
			and player:getPhase() == sgs.Player_Play then
			self[name .. "_used"] = false
            self.toUse=nil
		end
	end
end

function SmartAI:hasSkill(skill)
	local skill_name = skill 
	if type(skill) == "table" then
		skill_name = skill.name
	end
	
	local real_skill = sgs.Sanguosha:getSkill(skill_name)
	if real_skill and real_skill:isLordSkill() then
		return self.player:hasLordSkill(skill_name)
	else
		return self.player:hasSkill(skill_name)
	end
end

function SmartAI:fillSkillCards(cards)
    for _,skill in ipairs(sgs.ai_skills) do
        if self:hasSkill(skill) then       
        
            for index, card in ipairs(cards) do
                if prohibitUseDirectly(card, self.player) then table.remove(cards, index) end
            end

            local skill_card = skill.getTurnUseCard(self)
            if #cards == 0 then skill_card = skill.getTurnUseCard(self,true) end
            if skill_card then table.insert(cards, skill_card) end            
        end
    end
end

function SmartAI:useSkillCard(card,use)
    sgs.ai_skill_use_func[card:className()](card,use,self)
end

sgs.ai_skill_use_func={}
sgs.ai_skills={}


function SmartAI:cardNeed(card)
	local class_name = card:className()
	local suit_string = card:getSuitString()
    local value	
    if sgs[self.player:getGeneralName().."_keep_value"] then							
        value=sgs[self.player:getGeneralName().."_keep_value"][class_name]
		if value then return value+4 end
	end
	if sgs[self.player:getGeneralName().."_suit_value"] then
		value=(sgs[self.player:getGeneralName().."_suit_value"][suit_string])
		if value then return value+4 end
	end
	
    if card:inherits("Dodge") and (self:getCardsNum("Dodge")==0) then return 5.9 end
    if card:inherits("HolyWater") then
        self:sort(self.friends,"hp")
        if self.friends[1]:getHp()<2 then return 10 end
        return self:getUseValue(card)
    end
    if card:inherits("Schnapps") then
        if self.player:getHp()<2 then return 10 end
    end
    if card:inherits("Slash") and (self:getCardsNum("Slash") > 0) then return 4 end
    if card:inherits("Weapon") and (not self.player:getWeapon()) and (self:getCardsNum("Slash") > 1) then return 6 end
	if card:inherits("Unassailable") and self:getCardsNum("Unassailable") == 0 then
		if self.player:containsTrick("soul_awe") or self.player:containsTrick("enegy_drain") or self.player:containsTrick("icebound") then return 10 end
        if #self.friends > 0 then
            for _,friend in ipairs(self.friends) do
                if friend:containsTrick("soul_awe") or friend:containsTrick("enegy_drain") or self.player:containsTrick("icebound") then return 7 end
            end
        end
	end
    return self:getUseValue(card)
end

sgs.ai_cardshow = {}

function SmartAI:askForCardShow(requestor, reason)	
	local func = sgs.ai_cardshow[reason]
	if func then
		return func(self, requestor)
	else
		return self.player:getRandomHandCard()
	end
end


sgs.ai_cardshow.burn = function(self, requestor)
	local priority =
	{
	heart = 4,
	spade = 3,
	club = 2,
	diamond = 1
	}
	local index = 0
	local result
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if priority[card:getSuitString()]>index then
			result = card
			index = priority[card:getSuitString()]
		end
	end
	-- if self.player:hasSkill("hongyan") and result:getSuit() == sgs.Card_Spade then 
		-- result = sgs.Sanguosha:cloneCard(card:objectName(), sgs.Card_heart, card:getNumber())
		-- result:setSkillName("hongyan")
	-- end

	return result
end

function SmartAI:hasTrickEffective(card, player)
    if card:inherits("Burn") then
        if self:isEquip("HolyWing", player) then return false end
    end
    
	return true
end
	
function SmartAI:hasSameEquip(card, player)
	player = player or self.player
	if player:getEquips():isEmpty() then return false end
	if card:inherits("Weapon") then 
		if player:getWeapon() then return true end
	elseif card:inherits("Armor") then
		if player:getArmor() then return true end
	elseif card:inherits("DefensiveHorse") then
		if player:getDefensiveHorse() then return true end
	elseif card:inherits("OffensiveHorse") then 
		if player:getOffensiveHorse() then return true end
	elseif card:inherits("Relic") then 
		if player:getRelic() then return true end
	end
	return false
end

sgs.lose_equip_skill = "noskill"
sgs.need_kongcheng = "xuruo"
sgs.masochism_skill = "noskill"
sgs.wizard_skill = "shenyu"
sgs.wizard_harm_skill = "shenyu"

function SmartAI:damageIsEffective(nature, player)
	player = player or self.player
    
    if player:getMark("@invincible") > 0 then return false end
    
    if self:isEquip("HolyWing", player) and nature ~= sgs.DamageStruct_Normal then return false end
    
    if player:getMark("@zhangchao") > 0 and nature ~= sgs.DamageStruct_Ice then return false end
    
    if nature == sgs.DamageStruct_Normal then
        return not (self:isEquip("VineArmor", player) or player:hasSkill("jijia"))
    elseif nature == sgs.DamageStruct_Poison then
        return not (player:hasSkill("jijia"))
    elseif nature == sgs.DamageStruct_Thunder then
        return not (player:getMark("@diangguang")>0 or player:hasSkill("shidian"))
    end
end

function SmartAI:needHelp(data)

    if data then
        local parse = data:toString()
        
        self.room:writeToConsole("needHelp : "..parse)
        
        for _,p in ipairs(self.enemies) do
            if parse=="@huhuan-card" and p:hasSkill("huhuan") then
                return false
            end
        end
        
    end
    
    return true
end

function SmartAI:hasSkills(skill_names, player)
	player = player or self.player
	for _, skill_name in ipairs(skill_names:split("|")) do
		if player:hasSkill(skill_name) then
			return true
		end
	end
end

function SmartAI:isEquip(equip_name, player)
	player = player or self.player
	local cards = player:getCards("e")
	for _, card in sgs.qlist(cards) do
		if card:inherits(equip_name) then return true end
	end
	if equip_name == "ChaosMirror" and player:hasSkill("deer") and not player:getArmor() then return true end
	return false
end

function SmartAI:getCardId(class_name, player)
	player = player or self.player
	local cards = player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByUsePriority(cards)
	-- local card_str = self:getGuhuoCard(class_name, player) or zeroCardView(class_name, player)
	local card_str = zeroCardView(class_name, player)
	if card_str then return card_str end
	
	for _, card in ipairs(cards) do
		local card_place = self.room:getCardPlace(card:getEffectiveId())
		if card:inherits(class_name) and not prohibitUseDirectly(card, player) then
			return card:getEffectiveId()
		elseif isCompulsoryView(card, class_name, player, card_place) then 
			return isCompulsoryView(card, class_name, player, card_place)
		end
	end
	for _, card in ipairs(cards) do
		local card_place = self.room:getCardPlace(card:getEffectiveId())
		card_str = getSkillViewCard(card, class_name, player, card_place)
		if card_str then return card_str end
	end
end

function SmartAI:isWeak(player)
	player = player or self.player
	return (player:getHp() <= 2 and player:getHandcardNum() <= 2) or player:getHp() <= 1
end

function SmartAI:canViewAs(card,class_name,player)
	player = player or self.player
	local card_place = self.room:getCardPlace(card:getEffectiveId())
	local r1 = card:inherits(class_name) or getSkillViewCard(card, class_name, player, card_place)
	local r2 = isCompulsoryView(card, class_name, player, card_place)
	if class_name == "Slash" then
		return r1 or r2
	else
		return r1 and not r2
	end 
end

function SmartAI:isCompulsoryView(card,class_name,player)
	player = player or self.player
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
end

function SmartAI:getJiemingChaofeng(player)
	local max_x , chaofeng = 0 , 0
    for _, friend in ipairs(self:getFriends(player)) do
		local x = math.min(friend:getMaxHP(), 5) - friend:getHandcardNum()		
		if x > max_x then
			max_x = x
		end
    end
	if max_x < 2 then
		chaofeng = 5 - max_x * 2
	else
		chaofeng = (-max_x) * 2
	end    
    return chaofeng
end

function SmartAI:getOverflow(player)
	player = player or self.player
	return math.max(player:getHandcardNum() - player:getHp(), 0)
end

function SmartAI:hasSuit(suit_strings, include_equip, player)
	return self:getSuitNum(suit_strings, include_equip, player) > 0
end

function SmartAI:getSuitNum(suit_strings, include_equip, player)
	player = player or self.player
	local n = 0
	local flag = "h"
	if include_equip then flag = "he" end
	local allcards = player:getCards(flag)	
	for _, card in sgs.qlist(allcards) do
		for _, suit_string in ipairs(suit_strings:split("|")) do
			if card:getSuitString() == suit_string then
				n = n + 1
			end
		end	
	end
	return n
end


function SmartAI:isWeak(player)
	player = player or self.player
	return (player:getHp() <= 2 and player:getHandcardNum() <= 2) or player:getHp() <= 1
end

function SmartAI:getAoeValue(card, player)
	player = player or self.player 
	friends_noself = self:getFriendsNoself(player)
	enemies = self:getEnemies(player)
	local good, bad = 0, 0
	for _, friend in ipairs(friends_noself) do
		good = good + self:getAoeValueTo(card, friend, player)
	end

	for _, enemy in ipairs(enemies) do
		bad = bad + self:getAoeValueTo(card, enemy, player)
	end
	
	-- if player:hasSkill("jizhi") then
		-- good = good + 40	
	-- end
	return good - bad
end

function SmartAI:getAoeValueTo(card, to , from)
	if not from then from = self.player end 
	local value = 0
	local sj_num

	-- if to:hasSkill("buqu") then
		-- value = value + 10
	-- end
	
	-- if to:hasSkill("longdan") then
		-- value = value + 5
	-- end	
	
	-- if to:hasSkill("danlao") then
		-- value = value + 15
	-- end
	
	if card:inherits("AlienInvasion") then
		sj_num = self:getCardsNum("Slash", to)
		-- if to:hasSkill("juxiang") then
			-- value = value + 20
		-- end
	end
	if card:inherits("ThousandsArrowsShot") then
		sj_num = self:getCardsNum("Dodge", to)
	end
	
	if self:aoeIsEffective(card, to) then
		-- if to:getHp() > 1 or (self:getCardsNum("HolyWater", to) + self:getCardsNum("Schnapps", to) > 0) then
			-- if to:hasSkill("yiji") or to:hasSkill("jianxiong") then
				-- value = value + 20
			-- end
			-- if to:hasSkill("jieming") then
				-- value = value - self:getJiemingChaofeng(to) * 3
			-- end
			-- if to:hasSkill("ganglie") or to:hasSkill("fankui") or to:hasSkill("enyuan") then
				-- if not self:isFriend(from, to) then
					-- value = value + 10
				-- else 
					-- value = value - 10
				-- end
			-- end
		-- end
		
		-- if card:inherits("ThousandsArrowsShot") then
			-- sj_num = self:getCardsNum("Dodge", to)
			-- if (to:hasSkill("leiji") and self:getCardsNum("Jink", to) > 0) or self:isEquip("EightDiagram", to) then
				-- value = value + 30
				-- if self:hasSuit("spade", true, to) then
					-- value = value + 20
				-- end
			-- end
			-- if to:hasSkill("qingguo") or self:isEquip("EightDiagram", to) then
				-- value = value + 10
			-- end	
		-- end	
		
		if to:getHp() ~= 0 then
			value = value - 24 / to:getHp() - 10
		end
		
		if self:isFriend(from, to) then
	    if to:isLord() or from:isLord() then
				if to:getHp() <= 1 and self:getCardsNum("HolyWater", from) == 0 and sj_num == 0 then
					if to:getRole() == "renegade" then
						value = value - 50
					else
						value = value - 150
					end
				end
			end
			value = value + self:getCardsNum("HolyWater", from) * 2
		elseif to:getRole() == "rebel" or (to:isLord() and from:getRole() == "rebel") then
			if to:getHp() <= 1 and self:getCardsNum("HolyWater", to) == 0 and sj_num == 0 then
				value = value - 50
			end
		end
	else
		value = value + 10
	end
	
	return value 
end

local function getGuhuoViewCard(self, class_name, player)
	local card_use = {}
	card_use = self:getCards(class_name, player)
	
	if #card_use > 1 or (#card_use > 0 and card_use[1]:getSuit() == sgs.Card_Heart) then
		local index = 1
		if class_name == "Peach" or class_name == "Analeptic" or class_name == "Jink" then 
			index = #card_use 
		end
		return "@GuhuoCard=" .. card_use[index]:getEffectiveId() ..":".. card_use[index]:objectName()
	end
end

-- function SmartAI:getGuhuoCard(class_name, player, at_play)
	-- player = player or self.player
	-- if not player or not player:hasSkill("guhuo") then return end
	-- if at_play then
		-- if class_name == "Peach" and not player:isWounded() then return
		-- elseif class_name == "Analeptic" and player:hasUsed("Analeptic") then return
		-- elseif class_name == "Slash" and not self:slashIsAvailable(player) then return 
		-- elseif class_name == "Jink" or class_name == "Nullification" then return
		-- end
	-- end
	-- return getGuhuoViewCard(self, class_name, player)
-- end

function SmartAI:getCard(class_name, player)
	player = player or self.player
	local card_id = self:getCardId(class_name, player)
	if card_id then return sgs.Card_Parse(card_id) end
end

function getCards(class_name, player, room, flag)
	flag = flag or "he"
	local cards = {}
	local card_place, card_str
	if not room then card_place = sgs.Player_Hand end

	for _, card in sgs.qlist(player:getCards(flag)) do			
		card_place = card_place or room:getCardPlace(card:getEffectiveId())
		
		if class_name == "." then table.insert(cards, card)
		elseif isCompulsoryView(card, class_name, player, card_place) then
			cards_str = isCompulsoryView(card, class_name, player, card_place)
			card_str = sgs.Card_Parse(card_str)
			table.insert(cards, card_str)
        elseif card:inherits(class_name) and not prohibitUseDirectly(card, player) then table.insert(cards, card) 
		elseif getSkillViewCard(card, class_name, player, card_place) then
			cards_str = getSkillViewCard(card, class_name, player, card_place)
			card_str = sgs.Card_Parse(card_str)
			table.insert(cards, card_str)
		end
	end
	return cards
end

function SmartAI:getCards(class_name, player, flag)
	player = player or self.player
	return getCards(class_name, player, self.room, flag)
end

function getCardsNum(class_name, player)
	return #getCards(class_name, player)
end

function SmartAI:getCardsNum(class_name, player, flag)
	player = player or self.player
	local n = 0
	if type(class_name) == "table" then
		for _, each_class in ipairs(class_name) do
			n = n + #getCards(each_class, player, self.room, flag)
		end
		return n
	end
	n = #getCards(class_name, player, self.room, flag)
	
	-- if class_name == "Jink" then
		-- if player:hasLordSkill("hujia") then
			-- local lieges = self.room:getLieges("wei", player)
			-- for _, liege in sgs.qlist(lieges) do
				-- if self:isFriend(liege, player) then
				-- n = n + self:getCardsNum("Jink", liege)
				-- end
			-- end
		-- end
	-- elseif class_name == "Slash" then
		-- if player:hasSkill("wushuang") then
			-- n = n * 2
		-- end
		-- if player:hasLordSkill("jijiang") then
			-- local lieges = self.room:getLieges("shu", player)
			-- for _, liege in sgs.qlist(lieges) do
				-- if self:isFriend(liege, player) then
				-- n = n + self:getCardsNum("Slash", liege)
				-- end
			-- end
		-- end
	-- end
	return n
end

function SmartAI:cardProhibit(card, to)
	if card:inherits("Slash") then return self:slashProhibit(card, to) end
	if card:getTypeId() == sgs.Card_Trick then 
		if card:isBlack() and to:hasSkill("weimu") then return true end
		if card:inherits("Indulgence") or card:inherits("Snatch") and to:hasSkill("qianxun") then return true end
		if card:inherits("Duel") and to:hasSkill("kongcheng") and to:isKongcheng() then return true end
	end
	return false
end

-- load other ai scripts

dofile "lua/ai/general_config.lua"
dofile "lua/ai/intention-ai.lua"
dofile "lua/ai/state-ai.lua"
dofile "lua/ai/playrule-ai.lua"
dofile "lua/ai/chat-ai.lua"
dofile "lua/ai/meleecard-ai.lua"

dofile "lua/ai/melee-ss-ai.lua"
dofile "lua/ai/melee-ds-ai.lua"
dofile "lua/ai/melee-sf-ai.lua"


