/*!
 * e2-sat-editor/src/gui/connectionPresets.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.3.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>
#include <map>

using std::string, std::vector, std::map;

#ifndef connectionPresets_h
#define connectionPresets_h

namespace e2se_gui
{
class connectionPresets
{
	public:
		enum PRESET {
			enigma_24,
			neutrino,
			enigma_23,
			gx_24,
			dddragon,
			wtplay,
			enigma_1
		};

		static vector<PRESET> presets();
		static map<string, string> call(PRESET preset);

		static map<string, string> preset_enigma_24();
		static map<string, string> preset_neutrino();
		static map<string, string> preset_enigma_23();
		static map<string, string> preset_gx_24();
		static map<string, string> preset_dddragon();
		static map<string, string> preset_wtplay();
		static map<string, string> preset_enigma_1();
};
}
#endif /* connectionPresets_h */
