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
    
    -- 2011-10-25 +AI 孤高的求道者
    ["ryu"] = "隆",
    ["~ryu"] = "修行还远远不够啊...",
    ["longjuan"] = "龙卷",
    [":longjuan"] = "出牌阶段，你使用的【杀】被【闪】抵销时发动，此时你的攻击范围+1，且可以选择额外一个目标再使用1张【杀】，每回合限用一次",
    ["$longjuan"] = "龙卷旋风！",
    ["bodong"] = "波动",
    [":bodong"] = "必杀技，攻击范围+1，消耗3点怒，对攻击范围内任一角色使用【杀】，造成3点气属性伤害，目标角色可以打出X张【闪】，来防止X点伤害",
    ["$bodong"] = "真空-波动拳！",
    ["@longjuan"] = "是否发动【龙卷】",
    ["@bodong-jink"] = "你可以打出1张【闪】来防止1点伤害",
    ["@bodong-jink-more"] = "你还可以再打出1张【闪】来防止1点伤害",
    ["#LongjuanBang"] = "%from 发动<b style='color:yellow'>【龙卷】</b>技能",
    ["@bodong"] = "是否发动必杀技【真空波动拳】",
    
    -- 2011-10-26 -AI 傲气凌人
    ["ken"] = "肯",
    ["~ken"] = "下次肯定会超越你的...", 
    ["jifeng"] = "疾风",
    [":jifeng"] = "出牌阶段，你对距离1以内的角色使用的【杀】结算之后，可以立即选择：1. 对该角色再使用1次【杀】2. 对距离1以内的其他角色各使用1次【杀】",
    ["$jifeng"] = "疾风！迅雷！",
    ["shenglong"] = "升龙",
    [":shenglong"] = "必杀技，消耗2点怒，对攻击范围内任一角色使用【杀】，造成伤害时从牌堆翻牌（至多3次），直到出现黑色牌为止，伤害变为等于翻牌次数的火属性伤害",
    ["$shenglong"] = "升龙烈破！",
    ["JifengEffect1"] = "对该角色再使用1次【杀】",
    ["JifengEffect2"] = "对距离1以内的其他角色各使用1次【杀】",
    ["@jifeng"] = "请选择1张【杀】对距离1以内的角色使用",
    ["#JifengBang"] = "%from 发动<b style='color:yellow'>【疾风】</b>技能",
    ["@shenglong"] = "是否发动必杀技【升龙烈破】",
    
    -- 2011-10-30 +AI 世界最强女性
    ["chunli"] = "春丽",
    ["~chunli"] = "罪恶还没消亡...",
    ["bailie"] = "百烈",
    [":bailie"] = "你可以跳过出牌阶段，对攻击范围内任一角色至多使用3张【杀】",
    ["$bailie"] = "呀 呀 呀！",
    ["jiqi"] = "集气",
    [":jiqi"] = "你可以跳过摸牌阶段，其他角色分别选择给你1张基本牌或不给",
    ["$jiqi"] = "大家请相信我吧！",
    ["qigong"] = "气功",
    [":qigong"] = "回合开始阶段，你可以消耗1点气，指定你以外任一角色发动，你无视该角色防具效果",
    ["$qigong"] = "隔山打牛！",
    ["@bailie"] = "是否发动技能【百烈】，对攻击范围内任一角色至多使用3张【杀】",
    ["@bailie-slash"] = "是否对 %src 使用1张【杀】",
    ["@jiqi"] = "是否发动技能【集气】，向其他角色索取基本牌",
    ["@qigong"] = "是否发动技能【气功】，无视任一角色的防具",
    ["#JiqiNoCard"] = "%from 选择不给牌",
    
    -- 2011-10-30 +AI 野性儿
    ["blank"] = "布兰卡",
    ["~blank"] = "母亲，我让您失望了...",
    ["shidian"] = "嗜电",
    [":shidian"] = "锁定技，你每受到一次雷属性伤害，防止该伤害，摸1张牌",
    ["$shidian"] = "这种感觉让我充满能量",
    ["dianji"] = "电击",
    [":dianji"] = "出牌阶段，你可以消耗2点怒，使用的【雷杀】目标视为所有其他角色",
    ["$dianji"] = "最大功率输出！",
    ["#Dianji"] = "%from 发动<b style='color:yellow'>【电击】</b>，视为对其他所有角色使用<b>【雷杀】</b>",
    
    -- 2011-10-30 +AI 修行憎
    ["dhaisim"] = "达尔锡",
    ["~dhaisim"] = "死亡，只是一种解脱",
    ["yujia"] = "瑜珈",
    [":yujia"] = "锁定技，你不能装备武器，你使用的【杀】无距离限制",
    ["huoyan"] = "火焰",
    [":huoyan"] = "你可以消耗2点灵，将任意1张【杀】当做【火杀】使用",
    ["$huoyan"] = "瑜珈火焰！",
    ["chuansong"] = "传送",
    [":chuansong"] = "你的回合外，当你打出1张【闪】时，可以选择一名角色，你的坐席变为该角色的下家",
    ["$chuansong"] = "修行让我拥有穿越空间的能力",
    ["$Huoyan"] = "%from 发动<b style='color:yellow'>【火焰】</b>技能，将 <b>%card</b> 当成<b>【火杀】</b>对 %to 使用 ",
    ["@chuansong"] = "是否发动技能【传送】",
}
