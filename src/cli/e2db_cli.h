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
			usage,
			fread,
			fwrite,
			list,
			add,
			edit,
			remove,
			copy,
			move,
			set,
			unset,
			fimport,
			fexport,
			merge,
			print,
			parse,
			make,
			convert
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
			chid, txid, refid, tvid, tnid, trid, yname, ytype, 
			ssid, dvbns, tsid, onid, stype, snum, srcid, locked, chname, 
			sdata_p, sdata_c, sdata_C, sdata_f, 
			freq, sr, pol, fec, hpfec, lpfec, cfec, inv, tinv, cinv, sys, tsys, csys, asys, mod, tmod, cmod, amod, rol, pil, band, tmx, guard, hier, 
			isid, mts, plsmode, plscode, plsn, 
			chdata, txdata, bsdata, ubdata, tvdata, tndata, 
			pos, diseqc, uncomtd, charset, 
			tname, country, feed, 
			bname, pname, rname, qname, nname, btype, hidden, 
			mname, dname, itype, 
			flgs, oflgs
		};

		enum VALUE {
			val_int,
			val_char,
			val_bool,
			val_string,
			val_obj
		};

		enum ESCAPE {
			name_begin,
			name_end,
			divider,
			value_begin,
			value_end
		};

		enum OBJIO {
			_std = -1,
			tabular = 0,
			byline = 1,
			json = 2
		};

		struct iosets
		{
			OBJIO in = OBJIO::_std;
			OBJIO out = OBJIO::tabular;
			bool hrn = true; // human readable pair name
			bool hrv = true; // human readable pair value
			bool ppg = true; // paged preamble
		} __objio;

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
		void shell_command_help(istream* is) { shell_resolver(COMMAND::usage, is); };
		void shell_command_read(istream* is) { shell_resolver(COMMAND::fread, is); };
		void shell_command_write(istream* is) { shell_resolver(COMMAND::fwrite, is); };
		void shell_command_list(istream* is) { shell_resolver(COMMAND::list, is); };
		void shell_command_add(istream* is) { shell_resolver(COMMAND::add, is); };
		void shell_command_edit(istream* is) { shell_resolver(COMMAND::edit, is); };
		void shell_command_remove(istream* is) { shell_resolver(COMMAND::remove, is); };
		void shell_command_copy(istream* is) { shell_resolver(COMMAND::copy, is); };
		void shell_command_move(istream* is) { shell_resolver(COMMAND::move, is); };
		void shell_command_set(istream* is) { shell_resolver(COMMAND::set, is); };
		void shell_command_unset(istream* is) { shell_resolver(COMMAND::unset, is); };
		void shell_command_print(istream* is) { shell_resolver(COMMAND::print, is); };
		void shell_command_parse(istream* is) { shell_resolver(COMMAND::parse, is); };
		void shell_command_make(istream* is) { shell_resolver(COMMAND::make, is); };
		void shell_command_convert(istream* is) { shell_resolver(COMMAND::convert, is); };
		void shell_command_merge(istream* is) { shell_resolver(COMMAND::merge, is); };

		void shell_resolver(COMMAND command, istream* is);
		void shell_usage(string hint);
		void shell_file_read(string path);
		void shell_file_write(string path);
		void shell_e2db_parse();
		void shell_e2db_make();
		void shell_e2db_convert();
		void shell_e2db_merge();
		void shell_entry_list(ENTRY entry_type, string bname = "", int limit = 0);
		void shell_entry_list(ENTRY entry_type, int pos, int offset, int& end, string bname = "");
		void shell_entry_add(ENTRY entry_type);
		void shell_entry_add(ENTRY entry_type, int ref, string bname);
		void shell_entry_edit(ENTRY entry_type, string id);
		void shell_entry_edit(ENTRY entry_type, int ref, string bname, string id);
		void shell_entry_edit(ENTRY entry_type, bool edit, string id = "", int ref = 0, string bname = "");
		void shell_entry_remove(ENTRY entry_type, int ref, string bname, string id);
		void shell_entry_remove(ENTRY entry_type, string id, string bname = "");
		void shell_entry_parentallock(ENTRY entry_type, string id, bool flag);
		void shell_debug(int opt);

		void print_obj_begin(int depth = 0);
		void print_obj_end(int depth = 0);
		void print_obj_sep(int xpos = 0);
		void print_obj_dlm(int depth = 0, int xpos = 0);
		void print_obj_pair(TYPE type, std::any val);
		string obj_escape(ESCAPE esc, VALUE value_type);
		std::any field(TYPE type, bool required = false);

	private:
		e2db* dbih = nullptr;
		string last_label;
		string last_is;
};
}
#endif /* e2db_cli_h */
