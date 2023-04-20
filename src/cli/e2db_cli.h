/*!
 * e2-sat-editor/src/cli/e2db_cli.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <vector>
#include <any>

using std::string, std::vector;

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

		enum COMMAND {
			read,
			write,
			list,
			add,
			edit,
			remove,
			set,
			unset,
			print
		};

		enum ENTRY {
			transponder,
			service,
			bouquet,
			userbouquet,
			channel_reference,
			tunersets,
			tunersets_table,
			tunersets_transponder
		};

		enum TYPE {
			dbtype, dbparental, idx, 
			chid, txid, refid, tnid, trid, yname, ytype, 
			ssid, dvbns, tsid, onid, stype, snum, srcid, locked, chname, 
			sdata_p, sdata_c, sdata_C, sdata_f, 
			freq, sr, pol, fec, hpfec, lpfec, cfec, inv, tinv, cinv, sys, mod, tmod, cmod, amod, rol, pil, band, tmx, guard, hier, 
			isid, mts, plsmode, plscode, plsn, 
			chdata, txdata, 
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
		void shell_error(const string& cmd);
		void shell_command_version();
		void shell_command_help();
		void shell_command_read(istream* is) { shell_resolver(COMMAND::read, is); };
		void shell_command_write(istream* is) { shell_resolver(COMMAND::write, is); };
		void shell_command_list(istream* is) { shell_resolver(COMMAND::list, is); };
		void shell_command_add(istream* is) { shell_resolver(COMMAND::add, is); };
		void shell_command_edit(istream* is) { shell_resolver(COMMAND::edit, is); };
		void shell_command_remove(istream* is) { shell_resolver(COMMAND::remove, is); };
		void shell_command_set(istream* is) { shell_resolver(COMMAND::set, is); };
		void shell_command_unset(istream* is) { shell_resolver(COMMAND::unset, is); };
		void shell_command_print(istream* is) { shell_resolver(COMMAND::print, is); };
		void shell_resolver(COMMAND command, istream* is);
		void shell_file_read(string path);
		void shell_file_write(string path);
		void shell_entry_list(ENTRY entry_type, bool paged = true, int limit = 0);
		void shell_entry_list(ENTRY entry_type, int pos, int offset, int& end);
		void shell_entry_add(ENTRY entry_type);
		void shell_entry_edit(ENTRY entry_type, string id);
		void shell_entry_edit(ENTRY entry_type, bool edit, string id = "");
		void shell_entry_remove(ENTRY entry_type, string id);
		void shell_entry_parentallock(ENTRY entry_type, string id, bool flag);
		void shell_debug(int opt);
		std::any field(TYPE type, bool required = false);

	private:
		e2db* dbih = nullptr;
		string last_label;
		string last_is;
};
}
#endif /* e2db_cli_h */
