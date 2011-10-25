-- translation for melee-sf Package

return {
    ["meleesf"] = "经典街霸",
    
    -- 2011-10-23 +AI 杀意之波动
    ["gouki"] = "豪鬼",
    ["~gouki"] = "终于有比我更强的人了...（狂笑~）",
    ["shayi"] = "杀意",
    [":shayi"] = "锁定技，你每使用或打出1张【杀】，增加2点怨",
    ["shankong"] = "闪空",
    [":shankong"] = "你的回合外，当你打出一张【闪】时，你可以消耗6点怨，摸2张牌",
    ["$shankong"] = "闪！",
    ["shunyu"] = "瞬狱",
    [":shunyu"] = "必杀技，消耗18点怨，弃4张不同花色的牌，指定距离1以内任一其他角色发动。翻开3张牌，你获得红色牌，若有黑色牌，你对目标角色造成3点伤害",
    ["$shunyu"] = "一瞬千击！",
    ["@shunyu"] = "是否发动必杀技【瞬狱杀】",
    ["#ShunyuFail"] = "%from <b style='color:yellow'>【瞬狱】</b> 未能攻击到目标",
    ["#Shayi"] = "%from <b style='color:yellow'>【杀意】</b> 效果发动",
    
    -- 2011-10-25 -AI 孤高的求道者
    ["ryu"] = "隆",
    ["~ryu"] = "我的修行还远远不够啊...",
    ["longjuan"] = "龙卷",
    [":longjuan"] = "出牌阶段，你使用的【杀】被【闪】抵销时发动，此时你的攻击范围+1，且可以选择额外一个目标再使用1张【杀】，每回合限用一次",
    ["$longjuan"] = "龙卷旋风",
    ["bodong"] = "波动",
    [":bodong"] = "必杀技，攻击范围+1，消耗3点怒，对攻击范围内任一角色使用【杀】，造成3点气属性伤害，目标角色可以打出X张【闪】，来防止X点伤害",
    ["$bodong"] = "真空波动拳！",
    ["@longjuan"] = "是否发动【龙卷】",
    ["@bodong-jink"] = "你可以打出1张【闪】来防止1点伤害",
    ["@bodong-jink-more"] = "你还可以再打出1张【闪】来防止1点伤害",
    ["#LongjuanBang"] = "%from 发动 <b style='color:yellow'>【龙卷】</b>技能",
    ["@bodong"] = "是否发动必杀技【真空波动拳】",
}
