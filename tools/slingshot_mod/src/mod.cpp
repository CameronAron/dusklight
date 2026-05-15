#include "dusk/hook.hpp"
#include "dusk/mod_api.h"
#include "d/actor/d_a_alink.h"
#include <cstdio>
#include "m_Do/m_Do_audio.h"
#include "Z2AudioLib/Z2AudioMgr.h"

static DuskElemHandle g_el_slingshot = nullptr;
static int g_shot_count = 0;
static DuskElemHandle g_el_shots = nullptr;
static int g_prev_shot_state = 0;
static int g_log_audio_events_after_shot = 0;
static DuskElemHandle g_el_audio_probe = nullptr;
static u32 g_last_audio_id = 0;

static void on_checkUpperItemActionBow_post(void* args, void* retval) {
    (void)args;
    (void)retval;

    daAlink_c* link = daAlink_getAlinkActorClass();

    if (!link) {
        return;
    }

    if (link->mEquipItem != dItemNo_PACHINKO_e) {
        g_prev_shot_state = 0;
        return;
    }

    int current = link->mItemVar0.field_0x3018;

    if (g_prev_shot_state == 0 && current == 1) {
        g_shot_count++;
        g_log_audio_events_after_shot = 8;

        dusk::g_api->log_info(
            "Slingshot shot detected! Total shots: %d",
            g_shot_count);
    }

    g_prev_shot_state = current;
}

static void on_seStart_post(void* args, void* retval) {
    (void)retval;
    if (g_log_audio_events_after_shot <= 0) {
        return;
    }

    const u32 sound_id = dusk::arg<u32>(args, 1);
    g_last_audio_id = sound_id;
    g_log_audio_events_after_shot--;

    dusk::g_api->log_info("[slingshot audio probe] nearby seStart id=0x%08X (%u), remaining=%d",
        sound_id, sound_id, g_log_audio_events_after_shot);
}

static void BuildPanel(DuskPanelHandle panel, void*) {
    dusk::g_api->panel_add_section(panel, "Status");

    g_el_slingshot =
        dusk::g_api->panel_add_dyn_text(panel, "Slingshot equipped: UNKNOWN");

    g_el_shots =
        dusk::g_api->panel_add_dyn_text(panel, "Shots fired: 0");

    g_el_audio_probe =
        dusk::g_api->panel_add_dyn_text(panel, "Last probed audio id: (none)");
}

static void UpdatePanel(void*) {
    daAlink_c* link = daAlink_getAlinkActorClass();

    if (!link) {
        dusk::g_api->elem_set_text(
            g_el_slingshot,
            "Slingshot equipped: NO PLAYER");
        return;
    }

    bool equipped = link->mEquipItem == dItemNo_PACHINKO_e;

    dusk::g_api->elem_set_text(
        g_el_slingshot,
        equipped ?
        "Slingshot equipped: YES" :
        "Slingshot equipped: NO");

    char buf[96];
    snprintf(buf, sizeof(buf), "Shots fired: %d", g_shot_count);
    dusk::g_api->elem_set_text(g_el_shots, buf);

    snprintf(buf, sizeof(buf), "Last probed audio id: 0x%08X", g_last_audio_id);
    dusk::g_api->elem_set_text(g_el_audio_probe, buf);
}

extern "C" {

void mod_init(DuskModAPI* api) {
    dusk::init(api);

    api->register_tab_content(BuildPanel, nullptr);
    api->register_tab_update(UpdatePanel, nullptr);

    dusk::hookAddPost<&daAlink_c::checkUpperItemActionBow>(on_checkUpperItemActionBow_post);
    dusk::hookAddPost<&Z2AudioMgr::seStart>(on_seStart_post);
}

void mod_tick(DuskModAPI* api) {
    (void)api;
}

void mod_cleanup(DuskModAPI* api) {
    (void)api;
}

}
