/*!
 * e2-sat-editor/src/cli/e2db-cli.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <any>

using std::string;

#ifndef e2db_cli_h
#define e2db_cli_h
#include "../e2db/e2db.h"

using namespace e2se_e2db;

namespace e2se_cli
{
//TODO
class e2db_cli
{
	public:
		e2db_cli(int argc, char* argv[]);
		virtual ~e2db_cli() = default;
		int exited();

	protected:

		enum ENTRY {
			transponder, service, bouquet, userbouquet, channel_reference, tunersets, tunersets_table, tunersets_transponder
		};

		enum TYPE {
			dbtype, dbparental, idx, 
			chid, txid, refid, tnid, trid, yname, ytype, 
			ssid, dvbns, tsid, onid, stype, snum, srcid, locked, chname, chdata,
			sdata_p, sdata_c, sdata_C, sdata_f,
			freq, sr, pol, fec, hpfec, lpfec, cfec, inv, tinv, cinv, sys, mod, tmod, cmod, amod, rol, pil, band, tmx, guard, hier, 
			isid, mts, plsmode, plscode, plsn, 
			pos, diseqc, uncomtd, charset, 
			tname, country, feed, 
			bname, pname, rname, qname, nname, btype, hidden, 
			mname, dname, itype, 
			flgs, oflgs
		};

		void options(int argc, char* argv[]);
		void version(bool verbose);
		void cmd_shell();
		void cmd_version();
		void cmd_error(string option);
		void cmd_usage(bool descriptive = false);
		void shell_exit();
		void shell_header();
		void shell_error(string is);
		void shell_command_version();
		void shell_command_help();
		void shell_command_read();
		void shell_command_write() {};
		void shell_command_list() {};
		void shell_command_add();
		void shell_command_edit() {};
		void shell_command_remove() {};
		void shell_command_set() {};
		void shell_command_unset() {};
		void shell_command_print() {};
		void shell_debugger();
		void shell_entry_edit(ENTRY entry_type, string id = "");
		std::any field(TYPE type, bool required = false);

	private:
		e2db* dbih = nullptr;
		string last_label;
		string last_is;
};
}
#endif /* e2db_cli_h */
