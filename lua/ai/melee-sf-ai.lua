-- gouki ---------------------------------------------------------------------------------

sgs.ai_chaofeng["gouki"] = 1

sgs.dynamic_value.damage_card["ShunyuCard"] = true

-- shankong
sgs.ai_skill_invoke.shankong = true

-- shunyu
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

sgs.dynamic_value.damage_card["LongjuanCard"] = true
sgs.dynamic_value.damage_card["BodongCard"] = true

-- longjuan
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

sgs.dynamic_value.damage_card["JifengCard"] = true
sgs.dynamic_value.damage_card["ShenlongCard"] = true

-- jifeng
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
sgs.ai_skill_use["@@shenglong"]=function(self,prompt)
    if self:getCardsNum("Slash")<=0 or self.player:getMp()<2 then return "." end
    
    local slash = self:getCardId("Slash")
    
	self:sort(self.enemies,"defense")    
    for _,enemy in ipairs(self.enemies) do
        if self.player:inMyAttackRange(enemy) and not self:isEquip("HolyWing", enemy) then
            return "@ShenglongCard="..slash.."->"..enemy:objectName()
        end
    end
    
    return "."    
end

-- chunli ---------------------------------------------------------------------------------
sgs.ai_chaofeng["gouki"] = 4

sgs.dynamic_value.damage_card["BailieCard"] = true
sgs.dynamic_value.control_card["QigongCard"] = true

-- qigong
sgs.ai_skill_use["@@qigong"]=function(self,prompt)
    if self:getCardsNum("Slash")<=0 or self.player:getMp()<1 then return "." end

	self:sort(self.enemies,"defense")    
    for _,enemy in ipairs(self.enemies) do
        if self.player:inMyAttackRange(enemy) then
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
    
    if num>3 then return "@JiqiCard=.->." end
    
    return "."    
end

-- bailie
sgs.ai_skill_use["@@bailie"]=function(self,prompt)
    if self:getCardsNum("Slash")<2 then return "." end

	self:sort(self.enemies,"defense")    
    for _,enemy in ipairs(self.enemies) do
        if self.player:inMyAttackRange(enemy) then
            return "@BailieCard=.->"..enemy:objectName()
        end
    end
    
    return "."    
end

