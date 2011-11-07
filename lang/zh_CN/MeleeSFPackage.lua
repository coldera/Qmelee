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
    ["shenglong-jink"] = "%src 发动【升龙烈破】，请打出1张【闪】响应",
    
    -- 2011-10-30 +AI 世界最强女性
    ["chunli"] = "春丽",
    ["~chunli"] = "罪恶还没消亡...",
    ["bailie"] = "百烈",
    [":bailie"] = "你可以跳过出牌阶段，对攻击范围内任一角色至多使用3张【杀】",
    ["$bailie"] = "呀 呀 呀！",
    ["jiqi"] = "集气",
    [":jiqi"] = "你可以跳过摸牌和弃牌阶段，其他角色分别选择给你1张基本牌或不给",
    ["$jiqi"] = "大家请相信我吧！",
    ["qigong"] = "气功",
    [":qigong"] = "回合开始阶段，你可以消耗1点气，指定你以外任一角色发动，你对其使用【杀】时，无视该角色防具效果",
    ["$qigong"] = "隔山打牛！",
    ["@bailie"] = "是否发动技能【百烈】，对攻击范围内任一角色至多使用3张【杀】",
    ["@bailie-slash"] = "是否对 %src 使用1张【杀】",
    ["@jiqi"] = "是否发动技能【集气】，向其他角色索取基本牌",
    ["@qigong"] = "是否发动技能【气功】，无视任一角色的防具",
    ["#JiqiNoCard"] = "%from 选择不给牌",
    
    -- 2011-10-30 +AI 野性儿
    ["blanka"] = "布兰卡",
    ["~blanka"] = "母亲，我让您失望了...",
    ["shidian"] = "嗜电",
    [":shidian"] = "锁定技，你每受到一次雷属性伤害，防止该伤害，摸1张牌；你获得其他角色使用后进入弃牌堆的【雷杀】",
    ["$shidian"] = "这种感觉让我充满能量",
    ["dianji"] = "电击",
    [":dianji"] = "出牌阶段，你可以消耗2点怒，使用的【雷杀】目标视为所有其他角色",
    ["$dianji"] = "最大功率输出！",
    ["#Dianji"] = "%from 发动<b style='color:yellow'>【电击】</b>，视为对其他所有角色使用<b>【雷杀】</b>",
    ["#ShidianGet"] = "%from <b style='color:yellow'>【嗜电】</b>技能发动",
    
    -- 2011-10-30 +AI 修行憎
    ["dhalsim"] = "达尔锡",
    ["~dhalsim"] = "死亡，只是一种解脱",
    ["yujia"] = "瑜珈",
    [":yujia"] = "锁定技，你不能装备武器，你使用的【杀】无距离限制",
    ["huoyan"] = "火焰",
    [":huoyan"] = "你可以消耗2点灵，将手牌的1张【杀】当做【火杀】使用",
    ["$huoyan"] = "瑜珈火焰！",
    ["chuansong"] = "传送",
    [":chuansong"] = "你的回合外，当你打出1张【闪】时，可以选择一名角色，你的坐席变为该角色的下家",
    ["$chuansong"] = "修行让我拥有穿越空间的能力",
    ["$Huoyan"] = "%from 发动<b style='color:yellow'>【火焰】</b>技能，将 <b>%card</b> 当成<b>【火杀】</b>对 %to 使用 ",
    ["@chuansong"] = "是否发动技能【传送】",
    
    -- 2011-11-1 +AI 无敌的力士
    ["honda"] = "本田",
    ["~honda"] = "你们都不懂相扑的魅力",
    ["xushi"] = "蓄势",
    [":xushi"] = "你可以跳过出牌阶段，进入【蓄力】状态（你的手牌上限加上你的怒值），直到你使用或打出1张基本牌",
    ["$xushi"] = "等下你就知道我的厉害",
    ["wushuang"] = "无双",
    [":wushuang"] = "回合开始阶段，你可以解除【蓄力】状态，回合内你使用的【杀】无次数和攻击范围限制",
    ["$wushuang"] = "看你能不能挡住我这一波攻势",
    ["@xuli"] = "蓄力",
    
    -- 2011-11-1 +AI 钢铁摔跤手
    ["zangief"] = "桑吉尔夫",
    ["~zangief"] = "呜...愧对国家人民啊...",
    ["xuanfeng"] = "旋风",
    [":xuanfeng"] = "当其他角色使用1张非延时战术牌时，你可以弃2张牌，无效该牌的效果，如果使用锦囊的角色在你的攻击范围内，则你对该角色造成1点伤害",
    ["$xuanfeng"] = "这点伎俩在我面前一点用都没有！",
    ["dazhuang"] = "打桩",
    [":dazhuang"] = "必杀技，消耗6点怒（怒值不足时，可用1张【决斗】代替3点怒），指定你距离1以内的任一角色，进行判定，若不为红桃，则你对其造成3点伤害",
    ["$dazhuang"] = "我们来比比力量吧！哈哈~~",
    ["@xuanfeng"] = "是否弃2张牌，发动【旋风】",
    ["@dazhuang"] = "是否发动必杀技【螺旋打桩机】",
    ["$XuanfengEffect"] = "%to 使用的 %card 因 <b style='color:yellow'>【旋风】</b>的效果无效 ",
    
    -- 2011-11-2 +AI 坚韧不拔
    ["guile"] = "古烈",
    ["~guile"] = "军人的终点就是战场",
    ["shoudao"] = "手刀",
    [":shoudao"] = "锁定技，你的红色的【闪】视为【杀】",
    ["$shoudao"] = "喝！",
    ["jiaodao"] = "脚刀",
    [":jiaodao"] = "出牌阶段，当你使用红色【杀】对目标角色造成伤害时，弃2张红色【杀】，对目标角色再造成1点伤害，你增加1点狂",
    ["$jiaodao"] = "（撕裂声~~）",
    ["yinsu"] = "音速",
    [":yinsu"] = "你需要打出【闪】时，可以消耗2点狂，视为打出【闪】",
    ["$yinsu"] = "速度太慢了",
    ["@jiaodao"] = "是否弃2张红色的【杀】，发动【脚刀】",
    
    -- 2011-11-3 +AI 格斗少女
    ["sakura"] = "春日野樱",
    ["~sakura"] = "师父~你在哪里了",
    ["chongbai"] = "崇拜",
    [":chongbai"] = "出牌阶段，你可以将自己的1张手牌交给其他任一角色，你增加1点气，每回合限用一次",
    ["$chongbai"] = "跟定你了，教教我吧",
    ["mofang"] = "模仿",
    [":mofang"] = "出牌阶段，你可以从其他角色使用后进入弃牌堆中的最近5张非装备牌和非延时战术牌中挑选1张，若能使用，立即从手牌中打出相同花色的牌当做选中的牌来使用，每回合限用一次",
    ["$mofang"] = "我可是悟性很高的",
    ["yingluo"] = "樱落",
    [":yingluo"] = "摸牌阶段，你可以放弃摸牌进行判定，若不为梅花，则可以选择消耗1点气继续判定，判定结束后你得到所有判定牌",
    ["$yingluo"] = "别小看我哦，我的潜力很大的",
    ["mofang_choice"] = "请选择1张牌来发动【模仿】",
    ["$MofangUnused"] = "%from 选择了 %card，<b style='color:gray'>因为使用不了模仿失败</b>",
    ["#MofangUnused"] = "%from 放弃了模仿",
    ["$MofangUsed"] = "%from 从记忆中选择了 %card",
    ["@mofang"] = "请选择1张与【模仿】牌相同花牌的手牌",
    ["@yingluo"] = "是否消耗1点气继续发动【樱落】",
    ["mofangvas"] = "模仿",
    [":mofangvas"] = "立即从手牌中打出相同花色的牌当做选中的牌来使用",
    ["#MofangNothing"] = "%from 想不起任何牌， <b style='color:gray'>模仿失败</b>",
   
    -- 2011-11-4 +AI 失忆的特工
    ["cammy"] = "嘉米",
    ["~cammy"] = "我到底是谁！?",
    ["jingzhun"] = "精准",
    [":jingzhun"] = "当你计算与目标角色的距离等于你的攻击范围时，你对其使用【杀】时，可消耗2点狂，令此【杀】不可闪避",
    ["$jingzhun"] = "距离刚好，你逃不掉了",
    ["luoxuan"] = "螺旋",
    [":luoxuan"] = "你使用的【杀】对目标角色造成伤害后，可以消耗1点狂，弃掉该角色装备区的1张装备牌",
    ["$luoxuan"] = "螺旋！粉碎！",
    ["$LuoxuanDiscard"] = "%to 的 %card 被<b style='color:yellow'>【螺旋】</b>的效果破坏了",
    
    -- 2011-11-5 +AI 最强流的唯一
    ["dan"] = "火引弹",
    ["~dan"] = "你们都不要我的签名啦...",
    ["tiaoxin"] = "挑衅",
    [":tiaoxin"] = "回合结束阶段，你可以摸X张牌，进入【封牌】状态Y回合，X为场上存活角色数，若X大于4，Y等于X+1，否则Y等于X-1",
    ["$tiaoxin"] = "被我吓到了吧，敢来吗",
    ["wodao"] = "我道",
    [":wodao"] = "弃牌阶段，你可以消耗6点气，开始一个新的回合",
    ["$wodao"] = "让我们堂堂正正的决斗吧",
    
    -- 2011-11-5 +AI 心灵深处的魔法师    
    ["rose"] = "萝丝",
    ["~rose"] = "萝丝",
    ["anshi"] = "暗示",
    [":anshi"] = "锁定技，当其他角色要选择你的1张牌时，你代替该角色选择",
    ["$anshi"] = "（心声：对，就是它了）",
    ["caozong"] = "操纵",
    [":caozong"] = "当其他角色使用1张指定目标的牌时，你可以消耗2点灵，帮角色重新选择第一个目标",
    ["$caozong"] = "还是让我来吧，呵呵",
    ["xinling"] = "心灵",
    [":xinling"] = "出牌阶段，你可以失去1点体力，增加3点灵，每回合限用一次",
    ["$xinling"] = "我需要更多的灵力",
    ["#AnshiEffect"] = "%from <b style='color:yellow'>【暗示】</b>技能发动，%arg2 在暗示中选择了 %arg 效果需要选择的牌",
    ["$CaozongEffect"] = "%from 发动<b style='color:yellow'>【操纵】</b>技能，<b>%card</b> 的目标变为 %to",
    
    -- 2011-11-7 -AI 血斗百战元无敌
    ["gen"] = "元",
    ["~gen"] = "宁愿战死也不能病死",
    ["sidou"] = "死斗",
    [":sidou"] = "锁定技，你每受到1次伤害，你可以补充手牌至体力上限，并进入【无损】状态1回合",
    ["$sidou"] = "身手不错嘛",
    ["jiliu"] = "忌流",
    [":jiliu"] = "出牌阶段，你可以消耗15点怨，暗自指定你之外任一角色，该角色增加1枚【忌】标记（不可见）。你的回合开始阶段可以发动，拥有【忌】标记的角色失去X点体力，X为该角色【忌】标记数量",
    ["$jiliu"] = "统统在不知不觉中死去吧",
    ["sangliu"] = "丧流",
    [":sangliu"] = "出牌阶段，你可以指定你之外任一不处于【封牌】状态或【丧流】影响的角色，该角色若不能在X回合内对你造成伤害，则进入【封牌】状态Y回合，X为当前存活角色数x2，Y为状态进入时存活角色数，每回合限用一次",
    ["$sangliu"] = "心中的恐惧表现出来了吗",
    ["#JiliuSet"] = "%from 指定了一名角色，该角色增加1枚【忌】标记",
    ["#SangliuStart"] = "%to 必须在 %arg 回合内对 %from 造成伤害",
    ["#SangliuCounter"] = "%from 受<b style='color:yellow'>【丧流】</b>效果影响，还有 %arg 回合进入 %arg2 状态",
    ["#SangliuRemove"] = "%from 对 %to 造成伤害，<b style='color:yellow'>【丧流】</b>效果解除",
    ["#jiliu-on"] = "忌流--蛇穿",
    [":#jiliu-on"] = "是否释放【忌流】效果，让全部拥有【忌】标记的角色失去相应体力",
    ["#SangliuOn"] = "<b style='color:yellow'>【丧流--点咒】</b>发动，%from 进入【封牌】状态",
    ["#JiliuEffect"] = "%from 发动<b style='color:yellow'>【忌流--蛇穿】</b>，所有拥有【忌】标记的角色失去相应体力",
    ["#Sidou"] = "%from <b style='color:yellow'>【死斗】</b>效果发动",
    
}
