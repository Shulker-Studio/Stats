#include "mod/Hook/Hook.h"

#include <ll/api/memory/Hook.h>
#include <mc/world/actor/ActorDamageSource.h>
#include <mc/world/actor/ActorHurtResult.h>
#include <mc/world/actor/HurtParameters.h>
#include <mc/world/actor/player/Player.h>

#include "mod/Stats/Handlers/PlayerStatsHandlers.h"

namespace stats::hook::player {
LL_TYPE_INSTANCE_HOOK(
    PlayerHurtShieldHook,
    HookPriority::Normal,
    Player,
    &Player::$_hurt,
    ::ActorHurtResult,
    ::ActorDamageSource const& source,
    float                      damage,
    ::HurtParameters const&    hurtParameters
) {
    auto& player = *thisFor<Player>();

    // _blockUsingShield was inlined into Player::_hurt. This is its original guard.
    // 26.60 移除了 Player::getCurrentActiveShield()，其替代 getActiveShieldSlot() 的返回类型
    // (Bedrock::SmallOptional) 尚未导出，故仅以 isDamageBlocked 作为格挡判定。
    if (player.isDamageBlocked(source)) {
        handler::onPlayerBlockUsingShield(&player, damage);
    }

    return origin(source, damage, hurtParameters);
}

void hookPlayerBlockUsingShield() {
    PlayerHurtShieldHook::hook();
}
void unhookPlayerBlockUsingShield() {
    PlayerHurtShieldHook::unhook();
}
} // namespace stats::hook::player
