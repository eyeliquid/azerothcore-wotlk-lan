/*
 * mod-lan: LAN party features for AzerothCore + mod-playerbots.
 * Released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

void AddSC_lan_config();
void AddSC_lan_player();
void AddSC_lan_bots();
void AddSC_lan_vendors();
void AddSC_lan_npcs();
void AddSC_lan_route();

void Addmod_lanScripts()
{
    AddSC_lan_route();
    AddSC_lan_config();
    AddSC_lan_player();
    AddSC_lan_bots();
    AddSC_lan_vendors();
    AddSC_lan_npcs();
}
