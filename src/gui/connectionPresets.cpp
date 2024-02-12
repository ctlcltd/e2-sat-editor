/*!
 * e2-sat-editor/src/gui/connectionPresets.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.1
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
		enigma_24_23,
		gx_24,
		dddragon,
		wtplay,
		enigma_1
	};
}

map<string, string> connectionPresets::call(connectionPresets::PRESET)
{
	return {};
}

}
