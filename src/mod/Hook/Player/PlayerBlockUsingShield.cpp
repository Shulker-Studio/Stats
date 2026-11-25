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

    // 26.60 移除了 getCurrentActiveShield()，_blockUsingShield 也被内联进 Player::_hurt。
    // 其 1.21.3 形态为：
    //   if (!invulnerable && isDamageBlocked(source) && _blockUsingShield(this, source, damage)) return 0;
    // 可见 isDamageBlocked 只做“可格挡”预判，盾槽为空时 _blockUsingShield 返回 false、伤害照常结算。
    // 因此真格挡 = 预判可挡 + _hurt 提前返回（本次不再产生任何伤害）。
    bool const mayBlock = damage > 0.0f && player.isDamageBlocked(source);

    auto result = origin(source, damage, hurtParameters);

    if (mayBlock) {
        auto const& amount  = result.mDamageAmount.get();
        bool const  blocked = !amount.has_value() || *amount <= 0.0f;
        // 排查日志：实测格挡时均为 type=0(NotHurt)/无伤害值，未出现“可挡但照常结算”，停用。
        // getLogger().warn(
        //     "[shield] blocked type={} amount={} damage={} creative={}",
        //     static_cast<int>(result.mType),
        //     amount.value_or(-1.0f),
        //     damage,
        //     player.isCreative()
        // );
        if (blocked) {
            handler::onPlayerBlockUsingShield(&player, damage);
        }
    }

    return result;
}

void hookPlayerBlockUsingShield() {
    PlayerHurtShieldHook::hook();
}
void unhookPlayerBlockUsingShield() {
    PlayerHurtShieldHook::unhook();
}
} // namespace stats::hook::player
