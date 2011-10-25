sgs.ai_card_intention={}
sgs.ai_carduse_intention={}
sgs.ai_assumed["rebel"]=0
sgs.ai_assumed["loyalist"]=0
sgs.ai_assumed["renegade"]=0
sgs.ai_renegade_suspect={}
sgs.ai_anti_lord={}
sgs.ai_lord_tolerance={}

sgs.ai_card_intention["general"]=function(to,level)
    if to:isLord() then
        return -level*2
    elseif sgs.ai_explicit[to:objectName()]=="loyalist" then
        return -level
    elseif sgs.ai_explicit[to:objectName()]=="loyalish" then
        return -level
    elseif sgs.ai_explicit[to:objectName()]=="rebel" then
        return level
    elseif sgs.ai_explicit[to:objectName()]=="rebelish" then
        return level
    else
        local nonloyals=sgs.ai_assumed["rebel"]--+sgs.ai_assumed["renegade"]
        local loyals=sgs.ai_assumed["loyalist"]
        if loyals+nonloyals<=1 then return 0 end
        
        local ratio
        if loyals<=0 then ratio=1
        elseif nonloyals<=0 then ratio =-1 
        
        else
             local ratio1=(-loyals+nonloyals-1)/(loyals+nonloyals)
             local ratio2=(-loyals+nonloyals+1)/(loyals+nonloyals)
             ratio=1-math.sqrt((1-ratio1)*(1-ratio2))
             --if ratio1*ratio1>ratio2*ratio2 then ratio=ratio1
             --else ratio=ratio2 end
             --ratio=ratio
        end
        
        --if level==80 then to:getRoom():output(ratio) end
        return level*ratio
    end
end

sgs.ai_carduse_intention["SoulAwe"]=function(card,from,to,source)
	speakTrigger(card,from,to)
    return sgs.ai_card_intention.general(to,120)
end

sgs.ai_carduse_intention["EnegyDrain"]=function(card,from,to,source)
    return sgs.ai_card_intention.general(to,120)
end

sgs.ai_carduse_intention["Icebound"]=function(card,from,to,source)
    return sgs.ai_card_intention.general(to,120)
end

sgs.ai_card_intention["Bang"]=function(card,from,to,source)
    
	if from:getRole() == to:getRole() or (from:getRole() == "lord" and to:getRole() == "loyalist") 
		or (to:getRole() == "lord" and from:getRole() == "loyalist") then return 0 end
    if sgs.ai_liuliEffect then
        sgs.ai_liuliEffect=false
        return 0
    end
    local modifier=0
    if sgs.ai_collateral then sgs.ai_collateral=false modifier=-40 end
    local value=sgs.ai_card_intention.general(to,80+modifier)
    
    speakTrigger(card,from,to)
    
    return value
end

sgs.ai_card_intention["FireBang"]=function(card,from,to,source)
    return sgs.ai_card_intention["Bang"](card,from,to,source)
end

sgs.ai_card_intention["ThunderBang"]=function(card,from,to,source)
    return sgs.ai_card_intention["Bang"](card,from,to,source)
end

sgs.ai_card_intention["AirBang"]=function(card,from,to,source)
    return sgs.ai_card_intention["Bang"](card,from,to,source)
end

sgs.ai_card_intention["PoisonBang"]=function(card,from,to,source)
    return sgs.ai_card_intention["Bang"](card,from,to,source)
end

sgs.ai_card_intention["IceBang"]=function(card,from,to,source)
    return sgs.ai_card_intention["Bang"](card,from,to,source)
end

sgs.ai_card_intention["HolyWater"]=function(card,from,to,source)
   return sgs.ai_card_intention.general(to,-80)
end

sgs.ai_card_intention["PK"]=function(card,from,to,source)
    if sgs.ai_lijian_effect then 
        sgs.ai_lijian_effect=false
        return 0 
    end
    return sgs.ai_card_intention.general(to,80)
end

sgs.ai_card_intention["BorrowWeapon"]=function(card,from,to,source)
	if from:getRole() == to:getRole() or (from:getRole() == "lord" and to:getRole() == "loyalist") 
		or (to:getRole() == "lord" and from:getRole() == "loyalist") then return 0 end
    sgs.ai_collateral=true
    return sgs.ai_card_intention.general(to,80)
end

sgs.ai_card_intention["Burn"]=function(card,from,to,source)
	speakTrigger(card,from,to)
    return sgs.ai_card_intention.general(to,80)
end

sgs.ai_card_intention["SoulChain"]=function(card,from,to,source)
    --to:getRoom():output(to:isChained())
    if not to:isChained() then
        return sgs.ai_card_intention.general(to,80)
    else return sgs.ai_card_intention.general(to,-80)
    end
end

sgs.ai_card_intention["ThousandsArrowsShot"]=function(card,from,to,source)
        --return sgs.ai_card_intention.general(to,40)
        return 0
end

sgs.ai_card_intention["Cure"]=function(card,from,to,source)
        return sgs.ai_card_intention.general(to,-80)
end

sgs.ai_card_intention["PoisonMarish"]=function(card,from,to,source)
        return 0
end

sgs.ai_card_intention["AlienInvasion"]=function(card,from,to,source)
        --return sgs.ai_card_intention.general(to,40)
		speakTrigger(card,from,to)
        return 0
end

sgs.ai_card_intention["Roar"]=function(card,from,to,source)
        --return sgs.ai_card_intention.general(to,40)
		speakTrigger(card,from,to)
        return 0
end

sgs.ai_card_intention["ReadyToGo"]=function(card,from,to,source)
        --return sgs.ai_card_intention.general(to,-20)
        return 0
end

sgs.ai_card_intention["DestroyAll"]=function(card,from,to,source)
        if to:containsTrick("soul_awe") or to:containsTrick("enegy_drain") then 
            sgs.ai_snat_disma_effect=true
            sgs.ai_snat_dism_from=from
            return 0 
        end
        return sgs.ai_card_intention.general(to,70)
end

sgs.ai_card_intention["Grab"]=function(card,from,to,source)
        if to:containsTrick("soul_awe") or to:containsTrick("enegy_drain") then 
            sgs.ai_snat_disma_effect=true
            sgs.ai_snat_dism_from=from
            return 0 
        end
        return sgs.ai_card_intention.general(to,70)
end

sgs.ai_explicit={}
sgs.ai_royalty={}
sgs.ai_offensive_card=
{
    Bang=true,
    ThunderBang=true,
    FireBang=true,
    PK=true,
    BorrowWeapon=true,
    Bang=true,
    Burn=true,
    SoulAwe=true,
    EnegyDrain=true,
    SoulChain=true,
    IceBang=true,
    AirBang=true,
    PoisonBang=true,    
    Icebound=true,
}

sgs.ai_ambiguous_card=
{
   DestroyAll=true,
   Grab=true,
   ReadyToGo=true,
   ThousandsArrowsShot=true,
   AlienInvasion=true,
   PoisonMarish=true,
   Roar=true,
}


function SmartAI:updateRoyalty(player)
end

function SmartAI:printRoyalty()
        player=self.player
        self.room:output(player:getGeneralName().." "..sgs.ai_royalty[player:objectName()].." "..(sgs.ai_explicit[player:objectName()] or " "))
end

function SmartAI:refreshRoyalty(player,intention)
    if player:isLord() then return end
    local name=player:objectName()

        if (intention>=70) or (intention<=-70) then
            if sgs.ai_royalty[name]*intention<0 then
            sgs.ai_royalty[name]=sgs.ai_royalty[name]/2
            self:refreshRoyalty(player,0)
            sgs.ai_renegade_suspect[name]=(sgs.ai_renegade_suspect[name] or 0)+1
            end
        end
        
        if ((sgs.ai_anti_lord[name] or 0)-2)>(sgs.ai_lord_tolerance[name] or 0) then 
            if intention>0 then intention=intention/5 end
        end
        sgs.ai_royalty[name]=sgs.ai_royalty[name]+intention
        
        if sgs.ai_explicit[name]=="loyalish" then
            sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]+1
        elseif sgs.ai_explicit[name]=="loyalist" then
            sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]+1
        elseif sgs.ai_explicit[name]=="rebelish" then
            sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]+1
        elseif sgs.ai_explicit[name]=="rebel" then
            sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]+1
        end
        sgs.ai_explicit[name]=nil
        
    if sgs.ai_royalty[name]<=-160 then
        if not sgs.ai_explicit[name] then sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]-1 end
        sgs.ai_explicit[name]="rebel"
        sgs.ai_royalty[name]=-160
    elseif sgs.ai_royalty[name]<=-70 then
        if not sgs.ai_explicit[name] then sgs.ai_assumed["rebel"]=sgs.ai_assumed["rebel"]-1 end
        sgs.ai_explicit[name]="rebelish"
    elseif sgs.ai_royalty[name]>=160 then
        if not sgs.ai_explicit[name] then sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]-1 end
        sgs.ai_explicit[name]="loyalist"
        sgs.ai_royalty[name]=160
    elseif sgs.ai_royalty[name]>=70 then
        if not sgs.ai_explicit[name] then sgs.ai_assumed["loyalist"]=sgs.ai_assumed["loyalist"]-1 end
        sgs.ai_explicit[name]="loyalish"
    elseif sgs.ai_explicit[name] then
    end
end

function SmartAI:printAssume()
	self.room:output(sgs.ai_assumed["rebel"])
	self.room:output(sgs.ai_assumed["loyalist"])
	self.room:output("----")
end

function SmartAI:singleRole()
    local roles=0
    local theRole
    local selfexp=sgs.ai_explicit[self.player:objectName()]
    if selfexp=="loyalish" then selfexp="loyalist"
    elseif selfexp=="rebelish" then selfexp="rebel"
    end
    local selftru=self.role
    
    if (self.role~="lord") and (self.role~="renegade") then sgs.ai_assumed[self.role]=sgs.ai_assumed[self.role]-1 end
    if selfexp then sgs.ai_assumed[selfexp]=(sgs.ai_assumed[selfexp] or 0)+1 end
    	
    
    if sgs.ai_assumed["rebel"]>0 then
        roles=roles+1
        theRole="rebel"
    end
    if sgs.ai_assumed["loyalist"]>-1 then
        roles=roles+1
        theRole="loyalist"
    end
    
    if (self.role~="lord") and (self.role~="renegade") then sgs.ai_assumed[self.role]=sgs.ai_assumed[self.role]+1 end
    if selfexp then sgs.ai_assumed[selfexp]=sgs.ai_assumed[selfexp]-1 end
    
    
    if roles==1 then
        if sgs.ai_assumed["loyalist"]==sgs.ai_assumed["rebel"] then return nil end
        return theRole
    end
    return nil
end

function SmartAI:getInflictTargets()
    
end

function SmartAI:getProtectTarget()
    
end

function SmartAI:getSupportTarget()
    
end

function SmartAI:getWeakenTarget()
    
end

function SmartAI:printCards(cards)
    local string=""
    for _,card in ipairs(cards) do
        string=string.." "..card:className()
    end
    self.room:output(string)
end