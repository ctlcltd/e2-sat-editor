/*!
 * e2-sat-editor/src/gui/connectionPresets.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.7.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "connectionPresets.h"

namespace e2se_gui
{

vector<connectionPresets::PRESET> connectionPresets::presets()
{
	return {
		enigma_24,
		neutrino,
		enigma_23,
		gx_24,
		enigma_1
	};
}

map<string, string> connectionPresets::call(connectionPresets::PRESET preset)
{
	switch (preset)
	{
		case enigma_24: return preset_enigma_24();
		case neutrino: return preset_neutrino();
		case enigma_23: return preset_enigma_23();
		case gx_24: return preset_gx_24();
		case enigma_1: return preset_enigma_1();
	}

	return {};
}

map<string, string> connectionPresets::preset_enigma_24()
{
	string spath = "/var/usr/local/share/enigma2/defaults";

	return {
		{"pathTransponders", spath},
		{"pathServices", spath},
		{"pathBouquets", spath},
		{"pathPicons", "/usr/share/enigma2/picon"},
		{"customWebifReloadUrl", "/web/servicelistreload?mode=0"},
		{"customTelnetReloadCmd", "init 3"}
	};
}

map<string, string> connectionPresets::preset_neutrino()
{
	string spath = "/var/tuxbox/config/zapit";

	return {
		{"pathTransponders", "/var/tuxbox/config"},
		{"pathServices", spath},
		{"pathBouquets", spath},
		{"pathPicons", "/var/share/tuxbox/neutrino/icons"},
		{"customWebifReloadUrl", "/control/reloadchannels"},
		{"customTelnetReloadCmd", "pzapit -c"}
	};
}

map<string, string> connectionPresets::preset_enigma_23()
{
	string spath = "/etc/enigma2";

	return {
		{"pathTransponders", "/etc/tuxbox"},
		{"pathServices", spath},
		{"pathBouquets", spath},
		{"pathPicons", "/usr/share/enigma2/picon"},
		{"customWebifReloadUrl", "/web/servicelistreload?mode=0"},
		{"customTelnetReloadCmd", "init 3"}
	};
}

map<string, string> connectionPresets::preset_gx_24()
{
	string spath = "/home/gx/local/enigma_db";

	return {
		{"pathTransponders", spath},
		{"pathServices", spath},
		{"pathBouquets", spath},
		{"pathPicons", ""},
		{"customWebifReloadUrl", "/web/servicelistreload?mode=0"},
		{"customTelnetReloadCmd", "init 3"}
	};
}

map<string, string> connectionPresets::preset_enigma_1()
{
	string spath = "/var/tuxbox/config/enigma";

	return {
		{"pathTransponders", "/var/etc"},
		{"pathServices", spath},
		{"pathBouquets", spath},
		{"pathPicons", ""},
		{"customWebifReloadUrl", "/cgi-bin/reloadSettings"},
		{"customTelnetReloadCmd", "init 3"}
	};
}

}
