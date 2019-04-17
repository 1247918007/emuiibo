/*
 * Copyright (c) 2018 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <mutex>
#include <switch.h>
#include "nfpuser_mitm_service.hpp"
#include "nfp_shim.h"

extern HosMutex g_toggleLock;
extern u32 g_toggleEmulation;

void NfpUserMitmService::PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx) {
    /* Do nothing. */
}

bool NfpUserMitmService::ShouldMitm(u64 pid, u64 tid) {
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    return (g_toggleEmulation > 0);
}

Result NfpUserMitmService::CreateUserInterface(Out<std::shared_ptr<NfpUserInterface>> out) {
    std::scoped_lock<HosMutex> lck(g_toggleLock);
    if(g_toggleEmulation == 0) return ResultAtmosphereMitmShouldForwardToSession;
    std::shared_ptr<NfpUserInterface> intf = nullptr;
    u32 out_domain_id = 0;
    Result rc = 0;
    ON_SCOPE_EXIT {
        if (R_SUCCEEDED(rc)) {
            out.SetValue(std::move(intf));
            if (out.IsDomain()) {
                out.ChangeObjectId(out_domain_id);
            }
        }
    };

    NfpUser user;
    rc = nfpCreateUserInterface(this->forward_service.get(), &user);
    if (R_SUCCEEDED(rc)) {
        intf = std::make_shared<NfpUserInterface>(new NfpUser(user));
        if (out.IsDomain()) {
            out_domain_id = user.s.object_id;
        }
    }
    if(g_toggleEmulation == 2) g_toggleEmulation = 0;
    return rc;
}
