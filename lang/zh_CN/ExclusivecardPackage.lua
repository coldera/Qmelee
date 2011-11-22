-- translation for ExclusiveCardPackage

local t = {
    ["exclusivecard"] = "专属卡牌",
    ["exclusive"] = "角色专属",
    
    ["$CantUseExclusive"] = "%from 无法使用专属卡牌 <b style='color: #FF0066'>【%card】</b>，换1张牌",
    
    ["chaoxiu"] = "朝秀",
    [":chaoxiu"] = "<br /><b>不知火幻庵专属：</b>你对目标角色造成伤害时，可以防止此伤害，弃置该角色X+1张牌，X为该角色装备区的装备数<br /><b>祭品：</b>1张武器牌",
    ["$ChaoxiuSilie"] = "<b style='color:yellow'>【朝秀--撕裂】</b>效果发动，%to 的 %card 被破坏",
    
    ["violent_mask"] = "狂暴面具",
    [":violent_mask"] = "<br /><b>塔姆塔姆专属：</b>锁定技，你造成伤害时，伤害+1；摸牌阶段50%机率额外摸1张牌；手牌上限-1；弃牌阶段为随机弃置手牌<br /><b>祭品：</b>1点体力，1张装备牌",
    ["#ViolentMask"] = "<b style='color:yellow'>【狂暴面具--肆虐】</b>效果发动，%from 对 %to 造成的伤害从 %arg 变为 %arg2",
    ["#ViolentMaskDiscard"] = "<b style='color:#CCCCCC'><b style='color:yellow'>【狂暴面具--丧志】</b>效果发动，%from 的弃牌阶段不受控制</b>",
    
    ["shaman_totem"] = "萨满图腾",
    [":shaman_totem"] = "<br /><b>塔姆塔姆专属：</b>锁定技，你装备区有该装备时，【狂暴面具】的随机弃牌效果无效<br /><b>祭品：</b>1张非坐骑装备牌",
    ["#ShamanTotemEffect"] = "<b style='color:yellow'>【萨满图腾】</b>效果发动，抑制了<b style='color:yellow'>【狂暴面具--丧志】</b>的效果"
}
return t;