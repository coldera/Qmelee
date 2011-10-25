-- gouki ---------------------------------------------------------------------------------
sgs.ai_chaofeng["gouki"] = 1

sgs.ai_card_intention["ShunyuCard"]=function(card,from,to,source)
    return sgs.ai_card_intention.general(to,80)
end

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